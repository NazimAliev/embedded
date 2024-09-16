#ifndef OTLP_TRACE_HPP
#define OTLP_TRACE_HPP

#include <mutex>
#include <thread>

#include <opentelemetry/context/propagation/global_propagator.h>
#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_grpc_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>
#include <opentelemetry/sdk/trace/batch_span_processor_factory.h>
#include <opentelemetry/sdk/trace/batch_span_processor_options.h>
#include <opentelemetry/sdk/trace/processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/propagation/http_trace_context.h>
#include <opentelemetry/trace/provider.h>

namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace nostd = opentelemetry::nostd;
namespace otlp = opentelemetry::exporter::otlp;
namespace context = opentelemetry::context;
namespace common = opentelemetry::common;

class OtlpTrace {
    // no need static here: object is alone
    std::string m_url;    // colletor address, gets from config
    int m_level;          // trace level [0..2], gets from config
    std::string m_filter; // CP filter, gets from config
    bool m_bfilter;       // false if filter doesn't much CP
    nostd::shared_ptr<trace_api::Span> m_span;
    std::mutex m_mutex;
    std::string m_scope_name;
    int m_statement_id;
    std::string m_sql;

public:
    OtlpTrace(const std::string& url, int level);
    ~OtlpTrace();

    nostd::shared_ptr<trace_api::Tracer> get_tracer(std::string name);

    void new_trace_impl(
        const char* func,
        const char* path,
        int line,
        int statemend_id,
        const std::string& sql);

    void set_scope_name_impl(const std::string& name);

    void add_trace_point_impl(int pos);

    void add_trace_point_impl(
        const char* func, const char* path, int line, const std::string& msg);

    void add_event_impl(
        const char* func, const char* path, int line, const std::string& event);

}; // class

#endif // OTLP_TRACE_HPP
