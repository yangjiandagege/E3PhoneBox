/**
 * @file CLock.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Mutex, utility mutex tool
 * @detail Cubic Mutex, utility mutex tool
 */

#ifndef _CLOCK_CC_
#define _CLOCK_CC_ 1

#include <pthread.h>

class CLock
{
public:
    CLock()
    {
        pthread_mutexattr_init(&m_mutex_attr);
        pthread_mutexattr_settype(&m_mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
        pthread_mutex_init(&m_mutex, &m_mutex_attr);
    };

    ~CLock()
    {
        pthread_mutex_destroy(&m_mutex);
        pthread_mutexattr_destroy(&m_mutex_attr);
    };

    inline bool lock()
    {
        return (0 == pthread_mutex_lock(&m_mutex));
    };

    inline bool unlock()
    {
        return (0 == pthread_mutex_unlock(&m_mutex));
    };

    inline bool trylock()
    {
        return (0 == pthread_mutex_trylock(&m_mutex));
    };
private:
    pthread_mutex_t m_mutex;
    pthread_mutexattr_t m_mutex_attr;
};

#endif //_CLOCK_CC_
