/**
 * @file CEvent.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Event, utility tool, to help wait or set event
 * @detail Cubic Event, utility tool, to help wait or set event
 */

#ifndef _CEVENT_CC_
#define _CEVENT_CC_ 1

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

class CEvent
{

private:
    bool m_manual;
    bool m_state;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    pthread_mutexattr_t m_mutex_attr;

protected:
    void init()
    {
        pthread_mutexattr_init(&m_mutex_attr);
        pthread_mutexattr_settype(&m_mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
        pthread_mutex_init(&m_mutex, &m_mutex_attr);
        pthread_cond_init(&m_cond, NULL);
    }

    void deinit()
    {
        pthread_cond_destroy(&m_cond);
        pthread_mutex_destroy(&m_mutex);
        pthread_mutexattr_destroy(&m_mutex_attr);
    }

public:
    typedef enum ErrCode
    {
        ERR_NO_ERROR = 0,
        ERR_LOCK_FAIL = -1,
        ERR_SET_EVENT_FAIL = -2,
        ERR_WAIT_EVENT_FAIL = -3,
        ERR_WAIT_TIMEOUT = -4,
    } EErrCode;

    CEvent() : m_manual(false), m_state(false)
    {
        init();
    };

    CEvent(bool b_manual, bool b_state) : m_manual(b_manual), m_state(b_state)
    {
        init();
    };

    ~CEvent()
    {
        deinit();
    };

    EErrCode set()
    {
        if( pthread_mutex_lock(&m_mutex) )
        {
            perror("Lock Mutex");
            return ERR_LOCK_FAIL;
        }

        m_state = true;
        if( pthread_cond_broadcast(&m_cond) )
        {
            perror("Set Event");
            pthread_mutex_unlock(&m_mutex);
            return ERR_SET_EVENT_FAIL;
        }
        pthread_mutex_unlock(&m_mutex);
        return ERR_NO_ERROR;
    };

    EErrCode reset()
    {
        if( pthread_mutex_lock(&m_mutex) )
        {
            perror("Lock Mutex");
            return ERR_LOCK_FAIL;
        }

        m_state = false;
        pthread_mutex_unlock(&m_mutex);
        return ERR_NO_ERROR;
    };

    EErrCode wait()
    {
        if( pthread_mutex_lock(&m_mutex) )
        {
            perror("Lock Mutex");
            return ERR_LOCK_FAIL;
        }

        while( !m_state )
        {
            if (pthread_cond_wait(&m_cond, &m_mutex))
            {
                pthread_mutex_unlock(&m_mutex);
                perror("Wait Event");
                return ERR_WAIT_EVENT_FAIL;
            }
        }

        if( !m_manual )
        {
            m_state = false;
        }
        pthread_mutex_unlock(&m_mutex);
        return ERR_NO_ERROR;
    };

    EErrCode wait(unsigned int milliseconds)
    {
        int rc = 0;
        struct timespec abstime;
        struct timeval tv;

        gettimeofday(&tv, NULL);
        abstime.tv_sec  = tv.tv_sec + milliseconds / 1000;
        abstime.tv_nsec = tv.tv_usec * 1000 + (milliseconds % 1000) * 1000000;

        if (abstime.tv_nsec >= 1000000000)
        {
            abstime.tv_nsec -= 1000000000;
            abstime.tv_sec++;
        }

        if (pthread_mutex_lock(&m_mutex) != 0)
        {
            perror("Lock Mutex");
            return ERR_LOCK_FAIL;
        }

        while (!m_state )
        {
            if ((rc = pthread_cond_timedwait(&m_cond, &m_mutex, &abstime)))
            {
                pthread_mutex_unlock(&m_mutex);

                if (rc == ETIMEDOUT)
                {
                    return ERR_WAIT_TIMEOUT;
                }

                perror("Wait Event");
                return ERR_WAIT_EVENT_FAIL;
            }
        }

        if( !m_manual )
        {
            m_state = false;
        }
        pthread_mutex_unlock(&m_mutex);
        return ERR_NO_ERROR;
    };
};

#endif //_CEVENT_CC_