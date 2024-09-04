/*
 *
 * Database views traversal is needed after recreating tables to keep relationships 
 * between child and parent views 
 *
 * 

CREATE SCHEMA sc;

--- tables: level 0
CREATE TABLE sc.t1 (x int);
CREATE TABLE sc.t2 (x int);
CREATE TABLE sc.t3 (z int);

-- views: level 1
CREATE VIEW sc.va AS SELECT * FROM sc.t1;
CREATE VIEW sc.vb AS SELECT * FROM sc.t1, sc.t2;
CREATE VIEW sc.vc AS SELECT * FROM sc.t2;
CREATE VIEW sc.vd AS SELECT * FROM sc.t2, sc.t3;
CREATE VIEW sc.ve AS SELECT * FROM sc.t1, sc.t2, sc.t3;
CREATE VIEW sc.vf AS SELECT * FROM sc.t3;

--- views: level 2
CREATE VIEW sc.vbd AS SELECT * FROM sc.vb, sc.vd;
CREATE VIEW sc.vcf AS SELECT * FROM sc.vc, sc.vf;
CREATE VIEW sc.vad1 AS SELECT * FROM sc.t1, sc.va, sc.vd;

-- views: level3
CREATE VIEW sc.vcfb23 AS SELECT * FROM sc.vcf, sc.vb, sc.t2, sc.t3;

--- modify tables

CREATE OR REPLACE TABLE sc.t1 (x smallint);
CREATE OR REPLACE TABLE sc.t2 (x smallint);
CREATE OR REPLACE TABLE sc.t3 (z smallint);

--- views become inconsistency here!

SELECT recompile_views('/var/db/storage');

--- ok, views are recompiled

SELECT * FROM sc.vcfb23;

CREATE OR REPLACE TABLE sc.t1 (x int);
CREATE OR REPLACE TABLE sc.t2 (x int);
CREATE OR REPLACE TABLE sc.t3 (z int);

--- views become inconsistency here!

SELECT recompile_views('/var/db/storage');

--- ok, views are recompiled

SELECT * FROM sc.vcfb23;

*/

struct full_table_path_t {
    std::string database;
    std::string schema;
    std::string table;
}

struct compiler_view_desc {
    size_t view_id;
    full_table_path_t view_path;
    std::vector<view_result_type_t> view_result_type;
    std::string view_data;
    std::string view_query_text;
    std::vector<full_table_path_t> view_dependencies;
}

size_t recompile_views()
{
    struct node_t {
        compiler_view_desc view;
        std::vector<full_table_path_t> parents;
        std::vector<full_table_path_t> childs;
    };
    cluster_handle tmp();
    using record_t = std::pair<full_table_path_t, node_t>;
    std::map<full_table_path_t, node_t> map_graph;

    cluster_handle ch(nullptr, args[0]);
    auto h_md = smd::open_snapshot(
        ch.metadb_handle->db.get(),
        snapshot_reason::storage_version_updater);
    auto mdh = h_md.get();

    compiler_params cp(get_compiler_flags(session_flags(nullptr, false)));

    for (auto& db_metadata : smd::query_databases(mdh)) {
        auto dh = connect_database(
            &ch,
            db_metadata.database_name,
            {special_stmts::validate_leveldb_stmt,
             snapshot_reason::storage_version_updater});
        query_full_metadata(cp, dh.get());
        cp.current_database_name = db_metadata.database_name;
        if (cp.views.empty())
            continue;
        printf("Got %ld views\n", cp.views.size());

        // Pass 1: fill graph with view nodes
        std::cout << "\n === Fill graph with view nodes === \n" << std::endl;
        for (auto& v : cp.views) {
            node_t node;
            node.view = v;
            node.childs = node.view.view_dependencies;
            map_graph[node.view.view_path] = node;
            std::cout << "\tAdded to graph: " << v.view_path.to_string()
                      << std::endl;
        }
        std::cout << "\n === Create links in graph === \n" << std::endl;
        std::queue<record_t> qnode;
        for (auto& m : map_graph) {
            std::cout << "\nMap record :" << m.second.view.view_path.to_string()
                      << std::endl;
            bool is_leaf = true;
            for (auto& dep : m.second.childs) {
                if (map_graph.count(dep) == 0) {
                    std::cout << "\tSkip table: " << dep.to_string()
                              << std::endl;
                }
                else {
                    is_leaf = false;
                    // TODO: remove the check
                    try {
                        map_graph.at(dep).parents.push_back(
                            m.second.view.view_path);
                    }
                    catch (...) {
                        std::cout << "\t=== EXCEPTION Dependency missing: "
                                  << dep.to_string() << std::endl;
                    }
                    std::cout << "\tParent "
                              << m.second.view.view_path.to_string()
                              << " assigned to child " << dep.to_string()
                              << std::endl;
                } // if
            }     // for
            if (is_leaf) {
                qnode.push(m);
                std::cout << "\tLEAF. added to queue: "
                          << m.second.view.view_path.to_string() << std::endl;
            }
        }
        std::cout << "\n === Loop in queue === \n" << std::endl;
        int faults = 0;
        std::vector<compiler_view_desc> v_ordered_views;
        while (true) {
            if (qnode.empty()) {
                std::cout << "\tQueue is empty " << std::endl;
                break;
            }
            node_t node = qnode.front().second;
            qnode.pop();
            std::cout << "\nGot node from queue: " << node.view.view_path.table
                      << " parents: " << node.parents.size()
                      << ", childs: " << node.childs.size() << std::endl;
            bool is_leaf = true;
            for (auto& dep : node.childs) {
                if (map_graph.count(dep) == 0) {
                    std::cout << "\tSkip table: " << dep.to_string()
                              << std::endl;
                }
                else {
                    is_leaf = false;
                }
            }
            if (is_leaf) {
                v_ordered_views.push_back(node.view);
                std::cout << "\tAdd leaf to ordered list: "
                          << node.view.view_path.to_string() << std::endl;
                for (auto& p : node.parents) {
                    auto en =
                        std::remove(begin(node.childs), end(node.childs), p);
                    node.childs.erase(en, end(node.childs));
                    std::cout << "\t\tRemove me from parents: " << p.to_string()
                              << std::endl;
                    std::cout << "\t\tPush parent to queue " << p.to_string()
                              << std::endl;
                    qnode.push({p, map_graph.at(p)});
                }
                map_graph.erase(node.view.view_path);
                std::cout << "\tRemove me from graph: "
                          << node.view.view_path.to_string() << std::endl;
            }
            else {
                if (qnode.empty()) {
                    if (!map_graph.empty()) {
                        auto it = begin(map_graph);
                        qnode.push(*it);
                        std::cout << "\tPush next from graph to queue: "
                                  << (*it).second.view.view_path.to_string()
                                  << std::endl;
                    }
                    else {
                        break;
                    }
                }
            } // if(is_leaf)
            if (faults++ > 100) {
                std::cout << "\nQueue TIMEOUT\n" << std::endl;
                break;
            }
        } // while
        std::cout << "\nQueue is finished. Ordered views list:\n" << std::endl;
        for (auto& v : v_ordered_views) {
            std::cout << v.view_path.to_string() << std::endl;
        }
        int idx = 0;
        std::cout << "\n === Iterate ordered views and compile === \n"
                  << std::endl;
        std::cout << "Vies size: " << v_ordered_views.size() << std::endl;
        cluster_handle ch(nullptr, args[0]);
        for (auto& v : v_ordered_views) {
            auto h_md = smd::open_snapshot(
                ch.metadb_handle->db.get(),
                snapshot_reason::storage_version_updater);

            compiler_params cp(
                get_compiler_flags(session_flags(nullptr, false)));
            auto dh = connect_database(
                &ch,
                db_metadata.database_name,
                {special_stmts::validate_leveldb_stmt,
                 snapshot_reason::storage_version_updater});
            query_full_metadata(cp, dh.get());
            cp.current_database_name = db_metadata.database_name;
            cp.current_role = DEFAULT_USER_NAME;
            cp.session_role = DEFAULT_USER_NAME;

            string recompile_str =
                "select recompile_view('" + v.view_path.to_string() + "')";
            printf("%d) running: %s\n", ++idx, recompile_str.c_str());

            // **********************************
            auto compiled_results = compile(&cp, recompile_str.c_str());
            // **********************************
            if (tolower(compiled_results.data.stmts[0].type).compare("error") ==
                0) {
                std::cout << "\nQuery isn't compiled!\n" << std::endl;
                return 0;
            }

            check_compile_error(compiled_results.data);
            auto& statements = compiled_results.data.stmts;

            auto& h_metadata = dh->mdh;
            {
                auto uh = smd::begin_update(h_metadata.get());
                smd::drop_view(uh.get(), dh->database_name, v.view_id);
                smd::commit(uh.get());
            }
            h_metadata->options = smd::open_snapshot(
                                      ch.metadb_handle->db.get(),
                                      snapshot_reason::storage_version_updater)
                                      ->options;
            auto uh = smd::begin_update(h_metadata.get());
            view_desc vd = create_view_desc(
                statements[1].create_view,
                smd::next_insert_view(uh.get(), dh->database_name));
            // dh->database_name);
            smd::create_view(uh.get(), dh->database_name, vd);
            smd::commit(uh.get());
        } // for v_ordered_views
    }     // for databases
    return 0;
}
