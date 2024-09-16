#ifndef IOTLP_TRACE_HPP
#define IOTLP_TRACE_HPP
#include <memory>

class OtlpTrace;

/**
 * @brief Interface class for implementation OtlpTrace.
 * @class IOtlpTrace
 * @link TODO
 */
class IOtlpTrace {
    /**
     * @brief Points to OtlpTrace to get access to implemented functions.
     * @param pimpl pointer to implementation.
     */
    std::unique_ptr<OtlpTrace> pimpl;

protected:
    IOtlpTrace(const std::string& url, int level);
    static IOtlpTrace* s_instance;
    IOtlpTrace();
    ~IOtlpTrace();

public:
    IOtlpTrace(IOtlpTrace& other) = delete;
    void operator=(const IOtlpTrace&) = delete;
    /**
     * @name FunctionGroup
     * @brief Get instance functions
     * @retval instance always be returned
     */
    ///@{
    //* get_instance First time singleton initialization */
    //* @param  url the collector endpoint address, got from configuration
    //* @param  level the debug level, got from configuration
    static IOtlpTrace* get_instance(const std::string& url, int level);
    //* get_instance get instance to set trace points */
    static IOtlpTrace* get_instance();
    ///@}
    static void terminate_instance();

    /**
     * @name FunctionGroup
     * @brief trace manage functions called by trace macros
     * @retval void will always be returned
     */
    ///@{
    //* NewTrace trace creation. At the moment, should run for every new
    // statement */
    //* @param  func tracepoint function name, got from tracepoint macro
    //* @param  path tracepoint function source path + line number, got from
    // tracepoint macro
    //* @param  line line number of code
    //* @param  statement_id statement id
    void new_trace(
        const char* func,
        const char* path,
        int line,
        int statement_id,
        const std::string& sql);

    //* set_scope_name set name that will be inclded in trace until other name
    // meets */
    //* @param name name of scope
    void set_scope_name(const std::string& name);

    //* add_trace_point sent tracepoint to collector */
    //* @param pos place in the code
    void add_trace_point(int pos);

    //* add_trace_point sent debug tracepoint to collector */
    //* @param  func tracepoint function name
    //* @param  path tracepoint function
    //* @param  line line number of code
    //* @param  msg custom message
    void add_trace_point(
        const char* func, const char* path, int line, const std::string& msg);

    //* add_event send event to collector */
    //* @param  func tracepoint function name, got from tracepoint macro
    //* @param  path tracepoint function
    //* @param  line line number of code
    //* @param  event event
    void
    add_event(const char* func, const char* path, int line, std::string event);
    ///@}
};

#define SQ_TRACE(POS) IOtlpTrace::get_instance()->add_trace_point(POS)

#define SQ_TRACE_DEBUG(MSG)                                                    \
    IOtlpTrace::get_instance()                                                 \
        ->add_trace_point(__FUNCTION__, __FILE__, __LINE__, MSG)

#define SQ_TRACE_EVENT(EVENT)                                                  \
    IOtlpTrace::get_instance()                                                 \
        ->add_event(__FUNCTION__, __FILE__, __LINE__, EVENT)

#define SQ_TRACE_SET_NAME(MSG) IOtlpTrace::get_instance()->set_scope_name(MSG)

#define SQ_TRACE_NEW(ID, SQL)                                                  \
    IOtlpTrace::get_instance()                                                 \
        ->new_trace(__FUNCTION__, __FILE__, __LINE__, ID, SQL)

#endif // IOTLP_TRACE_HPP
