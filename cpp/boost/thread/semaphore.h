#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/chrono.hpp>

class semaphore
{
public:
    semaphore():m_terminate(false){}
    semaphore(bool terminate):m_terminate(terminate){}
    ~semaphore(){};

    template<typename R,typename P>
    bool wait_for(boost::chrono::duration<R,P> const& time)
    {
        boost::unique_lock lock(m_lock);
        return m_cv.wait_for(lock, time, [&]{ return m_terminate;});
    }

    void set()
    {
        boost::unique_lock lock(m_lock);
        m_terminate = true;
        m_cv.notify_all();
    }

    void reset()
    {
        boost::unique_lock lock(m_lock);
        m_terminate = false;
        m_cv.notify_all();
    }

private:
    boost::condition_variable m_cv;
    boost::mutex m_lock;
    bool m_terminate;
};

#endif // SEMAPHORE_H
