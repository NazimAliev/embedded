// g++ pmdk.cpp -o pmdk -std=c++11 -lpmemobj
//
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/make_persistent_array.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/transaction.hpp>

using pmem::obj::p;
using pmem::obj::persistent_ptr;
using pmem::obj::make_persistent;
using pmem::obj::transaction;
using pmem::obj::pool;
using namespace std;

#define PMEMPOOLSIZE 1024*1024*1024
#define DATASIZE 32
#define LOG(msg) std::cout << msg << "\n"

struct MEMBLOCK{
    persistent_ptr<MEMBLOCK> next;
    persistent_ptr<char[]> value_ptr;
    p<size_t> size;
};

int main() {
    const size_t size = PMEMPOOLSIZE;
    string path = "/mnt/mem/pmempool";
    pool<MEMBLOCK> pmpool;
	
    /* Open and create in persistent memory device */
    if ((access(path.c_str(), F_OK) != 0) && (size > 0)) {
        LOG("Creating filesystem pool, path=" << path << ", size=" << to_string(size));
        pmpool = pool<MEMBLOCK>::create(path.c_str(), "", size, S_IRWXU);
    } else {
        LOG("Opening pool, path=" << path);
        pmpool = pool<MEMBLOCK>::open(path.c_str(), "");
    }

    /* Get root persistent_ptr, 
    the first struct pointer 
    and link to the next one */
    auto block_ptr = pmpool.root();

    /* Cool part, submit your lambda func to pmpool
    , and when the func is ready to run (consistency design),
    it executes below lines.*/
	transaction::run(pmpool, [&] {
        /* Data structure looks like below
        MEMBLOCK0(root) -> MEMBLOCK1 -> MEMBLOCK2 -> ...
            ||                 ||           ||
            \/                 \/           \/
          char[]              char[]       char[]
        */
        auto next_block_ptr = make_persistent<MEMBLOCK>();
        auto data_ptr = make_persistent<char[]>(DATASIZE);
        block_ptr->value_ptr = data_ptr;
        block_ptr->size = DATASIZE;
        block_ptr->next = next_block_ptr;
        block_ptr = next_block_ptr;

        /* To write something, just like doing to a memory pointer */
		char* data_in_mem_ptr = data_ptr.get();
        memset(data_in_mem_ptr, 'a', DATASIZE);

        string data_str(data_in_mem_ptr, DATASIZE);
        LOG("Input Data: " << data_str);
    });

    pmpool.close();
}
