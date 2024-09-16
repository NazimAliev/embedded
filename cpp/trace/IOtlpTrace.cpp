#include <iostream>

#include <basic/utils/trace/IOtlpTrace.hpp>
#include <basic/utils/trace/OtlpTrace.hpp>

IOtlpTrace* IOtlpTrace::s_instance = nullptr;
IOtlpTrace* IOtlpTrace::get_instance(const std::string& url, int level)
{
    if (s_instance == nullptr) {
        s_instance = new IOtlpTrace(url, level);
    }
    return s_instance;
}

IOtlpTrace* IOtlpTrace::get_instance()
{
    return s_instance;
}

void IOtlpTrace::terminate_instance()
{
    delete s_instance;
    s_instance = nullptr;
}

IOtlpTrace::IOtlpTrace(const std::string& url, int level)
: pimpl(std::make_unique<OtlpTrace>(url, level))
{}

IOtlpTrace::~IOtlpTrace() = default;

void IOtlpTrace::new_trace(
    const char* func,
    const char* path,
    int line,
    int statement_id,
    const std::string& sql)
{
    pimpl->new_trace_impl(func, path, line, statement_id, sql);
}

void IOtlpTrace::set_scope_name(const std::string& name)
{
    pimpl->set_scope_name_impl(name);
}

// TRACE(0)
void IOtlpTrace::add_trace_point(int pos)
{
    pimpl->add_trace_point_impl(pos);
}

// TRACE_DEBUG("")
void IOtlpTrace::add_trace_point(
    const char* func, const char* path, int line, const std::string& msg)
{
    pimpl->add_trace_point_impl(func, path, line, msg);
}

void IOtlpTrace::add_event(
    const char* func, const char* path, int line, std::string event)
{
    pimpl->add_event_impl(func, path, line, event);
}
