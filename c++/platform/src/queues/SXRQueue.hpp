#ifndef SXR_QUEUE
#define SXR_QUEUE

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "SXRTypes.hpp"

DLT_IMPORT_CONTEXT(mainCtx)
namespace sendxreceive
{

template<typename queueType> 
class SXRQueue
{
private:
    /* data */
public:
    SXRQueue(const std::string &queueName)
        : queueName(queueName)
  
    { }

    void setSendMessageCallback(std::function<void(queueType)> sendMessage)
    {
        this->sendMessage = sendMessage;
    }

    ~SXRQueue()
    {
        DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING(queueName.c_str()), DLT_CSTRING(" destroyed"));
        stop();
        clear();
    }

    SXRError start()
    {
        SXRError err = SXRError::GENERAL_ERROR;
        m_queueThread = std::thread(&SXRQueue::processQueue, this);

        if (m_queueThread.joinable())
        {
            err = SXRError::SUCCESS;
        }
        return err;
    }

    SXRError stop()
    {
        SXRError err = SXRError::GENERAL_ERROR;

        m_stop = true;
        m_queueCondVar.notify_one();

        if (m_queueThread.joinable())
        {
            m_queueThread.join();

            err = SXRError::SUCCESS;
        }
        return err;
    }

    SXRError clear()
    {
        SXRError err = SXRError::GENERAL_ERROR;
        std::queue<queueType> emptyQueue;

        std::lock_guard<std::mutex> lck(m_queueMutex);
        std::swap(m_queue, emptyQueue);

        if (m_queue.empty() == true)
        {
            err = SXRError::SUCCESS;
        }
        return err;
    }

    virtual SXRError insert(queueType msg)
    {
        SXRError err = SXRError::GENERAL_ERROR;
        {
            std::lock_guard<std::mutex> lck(m_queueMutex);
            m_queue.push(msg);
            err = SXRError::SUCCESS;
        }
        m_queueCondVar.notify_one();
        return err;
    }

protected:

    void processQueue()
    {
        std::unique_lock<std::mutex> lck(m_queueMutex);
        queueType msg;
        while (m_stop == false)
        {
            m_queueCondVar.wait(lck,
                                [this]() { return !m_queue.empty() | m_stop; });

            if (m_stop)
            {
                return;
            }

            while (m_queue.empty() == false)
            {
                msg = m_queue.front();
                if (sendMessage)
                { 
                    sendMessage(msg);
                    m_queue.pop();
                } 
                else 
                {
                    DLT_LOG(mainCtx, DLT_LOG_ERROR, DLT_CSTRING("pointer to sendMessage function equals nullptr"));
                }
            }
        }
    }

    std::function<void(queueType)> sendMessage = nullptr;

    std::queue<queueType> m_queue;
    std::string queueName; 
    std::thread m_queueThread;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCondVar;
    bool m_stop = false;
};

}  // namespace sendxreceive
#endif