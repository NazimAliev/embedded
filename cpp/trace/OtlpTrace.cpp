#include <basic/utils/string_utils.h>
#include <basic/utils/trace/OtlpTrace.hpp>
#include <basic/utils/utils.h>

OtlpTrace::~OtlpTrace()
{
    m_span->End();
    std::shared_ptr<trace_api::TracerProvider> none;
    trace_api::Provider::SetTracerProvider(none);
}

OtlpTrace::OtlpTrace(const std::string& url, int level)
: m_url(url), m_level(level)
{
    setenv("OTEL_SERVICE_NAME", "Sqream worker", 1);
    std::string prefix = m_url.substr(0, 4);
    std::unique_ptr<trace_sdk::SpanExporter> exporter;
    if (prefix == "http") {
        otlp::OtlpHttpExporterOptions opts;
        // opts.url = "http://localhost:4318/v1/traces";
        // opts.url = "http://192.168.4.99:4318/v1/traces";
        opts.url = m_url;
        exporter = otlp::OtlpHttpExporterFactory::Create(opts);
    }
    else if (prefix == "grpc") {
        otlp::OtlpGrpcExporterOptions opts;
        // opts.endpoint = "http://192.168.4.99:3200";
        opts.endpoint = m_url;
        exporter = otlp::OtlpGrpcExporterFactory::Create(opts);
    }
    else {
        // FIXME
        otlp::OtlpHttpExporterOptions opts;
        opts.url = "http://localhost:4318/v1/traces";
        exporter = otlp::OtlpHttpExporterFactory::Create(opts);
    }
    trace_sdk::BatchSpanProcessorOptions bspOpts{};
    auto processor = trace_sdk::BatchSpanProcessorFactory::Create(
        std::move(exporter),
        bspOpts);
    std::shared_ptr<trace_api::TracerProvider> provider =
        trace_sdk::TracerProviderFactory::Create(std::move(processor));
    trace_api::Provider::SetTracerProvider(provider);
    m_scope_name = "main";
    m_span = get_tracer(m_scope_name)->StartSpan("Otlp init");
    std::cout << m_span << std::endl;
    // needed for parsing collector log
    m_span->SetAttribute("Source", __FILE__);
    m_span->End();
}

nostd::shared_ptr<trace_api::Tracer> OtlpTrace::get_tracer(std::string name)
{
    auto provider = trace_api::Provider::GetTracerProvider();
    return provider->GetTracer(name);
}

void OtlpTrace::new_trace_impl(
    const char* func,
    const char* path,
    int line,
    int statement_id,
    const std::string& sql)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_level = get_session_flags()->otlp_trace_level;
    if (m_level == 0)
        return;
    m_filter = tolower(get_session_flags()->otlp_trace_filter);
    m_statement_id = statement_id;
    m_sql = sql;
    m_scope_name = "main";
    m_span = get_tracer("New trace")->StartSpan("New Statement ID");
    trace_api::Scope scope(m_span);
    m_span->SetAttribute("StatementID", m_statement_id);
    m_span->SetAttribute("SQL", m_sql);
    m_span->SetAttribute("Function", func);
    m_span->SetAttribute("Source", path);
    m_span->SetAttribute("Line", line);
    m_span->AddEvent("New trace", {{"Thread ID", "Not implemented yet"}});
    m_bfilter = true;
}

void OtlpTrace::set_scope_name_impl(const std::string& name)
{
    m_scope_name = name;
    if (m_filter == "all") {
        m_bfilter = true;
        return;
    }
    // apply filter for all trace points belong to this scope
    m_bfilter = (m_filter == tolower(m_scope_name));
}

// -----------------------------------
// TRACE(0)
void OtlpTrace::add_trace_point_impl(int pos)
{
    if (!m_bfilter || m_level < 1) {
        return;
    }
    const std::lock_guard<std::mutex> lock(m_mutex);
    trace_api::Scope scope(m_span);
    m_span = get_tracer(m_scope_name)->StartSpan(m_scope_name);
    m_span->SetAttribute("Statement ID", m_statement_id);
    m_span->SetAttribute("Position", pos);
    // m_span->End();
}

// TRACE_DEBUG("")
void OtlpTrace::add_trace_point_impl(
    const char* func, const char* path, int line, const std::string& msg)
{
    if (!m_bfilter || m_level < 2) {
        return;
    }
    const std::lock_guard<std::mutex> lock(m_mutex);
    trace_api::Scope scope(m_span);
    m_span = get_tracer(m_scope_name)->StartSpan(m_scope_name);
    m_span->SetAttribute("Statement ID", m_statement_id);
    m_span->SetAttribute("Function", func);
    m_span->SetAttribute("Source", path);
    m_span->SetAttribute("Line", line);
    m_span->SetAttribute("Message", msg);
    // m_span->End();
}

void OtlpTrace::add_event_impl(
    const char* func, const char* path, int line, const std::string& event)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    trace_api::Scope scope(m_span);
    m_span->AddEvent(
        event,
        {{"Name: ", m_scope_name},
         {"Event: ", event},
         {"Function: ", func},
         {"Statement ID: ", m_statement_id},
         // separate from Source int addTrace to unify collector output
         // processing
         {"Src: ", path},
         {"Line: ", line}});
    // m_span->End();
}
