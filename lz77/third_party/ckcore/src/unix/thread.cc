/*
 * The ckCore library provides core software functionality.
 * Copyright (C) 2006-2012 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <limits>
#include <memory>
#include <sys/time.h>
#include "ckcore/thread.hh"

namespace ckcore
{
    Thread::Thread()
        : thread_(0),running_(false)
    {
    }

    Thread::~Thread()
    {
        kill();
    }

    void *Thread::native_thread(void *param)
    {
        pthread_cleanup_push(cleanup,param);

        Thread *thread = static_cast<Thread *>(param);

        try
        {
            thread->run();
        }
        catch (...)
        {
        }

        pthread_cleanup_pop(1);
        return NULL;
    }

    void Thread::cleanup(void *param)
    {
        Thread *thread = static_cast<Thread *>(param);

        Locker<thread::Mutex> lock(thread->mutex_);
        thread->running_ = false;
        thread->thread_done_.signal_all();
    }

    bool Thread::start()
    {
        Locker<thread::Mutex> lock(mutex_);

        if (running_)
            return false;

        // Create the thread.
        if (pthread_create(&thread_,NULL,native_thread,this) != 0)
            return false;

        running_ = true;
        return true;
    }

    bool Thread::wait(tuint32 timeout)
    {
        Locker<thread::Mutex> lock(mutex_);

        // Make sure a thread is not waiting on itself.
        if (thread_ == pthread_self())
            return false;

        if (!running_)
            return false;

        return thread_done_.wait(mutex_,timeout);
    }

    bool Thread::kill()
    {
        Locker<thread::Mutex> lock(mutex_);

        if (!running_)
            return false;

        if (pthread_cancel(thread_) != 0)
            return false;

        thread_done_.wait(mutex_);

        running_ = false;
        return true;
    }

    bool Thread::running() const
    {
        Locker<thread::Mutex> lock(mutex_);
        return running_;
    }

    namespace thread
    {
        /**
         * @brief Class for passing parameters to a native thread instance.
         */
        class NativeThreadParam
        {
        public:
            tfunction func_;
            void *param_;
            
            NativeThreadParam(tfunction func,void *param) :
                func_(func),param_(param)
            {
            }
        };

        /**
         * The main thread entry point for new threads.
         * @param [in] param The thread parameter.
         * @return Always returns NULL.
         */
        void *native_thread(void *param)
        {
            std::auto_ptr<NativeThreadParam> native_param(
                static_cast<NativeThreadParam *>(param));
            native_param->func_(native_param->param_);

            return NULL;
        }

        bool create(tfunction func,void *param)
        {
            // Setup the parameters to pass to the native thread.
            NativeThreadParam *native_param = new NativeThreadParam(func,param);
            if (native_param == NULL)
                return false;

            // Create the thread.
            pthread_t thread;
            if (pthread_create(&thread,NULL,native_thread,native_param) != 0)
            {
                delete native_param;
                return false;
            }
            
            return true;
        }

        bool sleep(ckcore::tuint32 milliseconds)
        {
            return usleep(milliseconds * 1000) == 0;
        }

        tuint32 ideal_count()
        {
/*#ifdef _MAC
            return MPProcessorsScheduled();
#endif*/
            long count = sysconf(_SC_NPROCESSORS_ONLN);
            return count == -1 ? 1 : count;
        }

        thandle identifier()
        {
            return reinterpret_cast<thandle>(pthread_self());
        }

        Mutex::Mutex()
        {
            pthread_mutex_init(&mutex_,NULL);
        }

        Mutex::~Mutex()
        {
            pthread_mutex_destroy(&mutex_);
        }

        bool Mutex::lock()
        {
            return pthread_mutex_lock(&mutex_) == 0;
        }

        bool Mutex::unlock()
        {
            return pthread_mutex_unlock(&mutex_) == 0;
        }

        bool Mutex::try_lock()
        {
            return pthread_mutex_trylock(&mutex_) == 0;
        }

        WaitCondition::WaitCondition()
            : waiters_(0),wakeups_(0)
        {
            pthread_mutex_init(&mutex_,NULL);
            pthread_cond_init(&cond_,NULL);
        }

        WaitCondition::~WaitCondition()
        {
            pthread_cond_destroy(&cond_);
            pthread_mutex_destroy(&mutex_);
        }

        bool WaitCondition::wait(tuint32 timeout)
        {
            int res = 0;

            waiters_++;

            while (true)
            {
                if (timeout == std::numeric_limits<tuint32>::max())
                {
                    res = pthread_cond_wait(&cond_,&mutex_);
                }
                else
                {
                    struct timeval tv;
                    gettimeofday(&tv,0);

                    timespec ti;
                    ti.tv_nsec = (tv.tv_usec + (timeout % 1000) * 1000) * 1000;
                    ti.tv_sec = tv.tv_sec + (timeout / 1000) + (ti.tv_nsec / 1000000000);
                    ti.tv_nsec %= 1000000000;

                    res = pthread_cond_timedwait(&cond_,&mutex_,&ti);
                }

                // Check that we actually woke up any thread.
                if (res == 0 && wakeups_ == 0)
                    continue;

                break;
            }

            waiters_--;

            // Check if we successfully awoken one thread.
            if (res == 0)
                wakeups_--;

            return res == 0;
        }

        bool WaitCondition::wait(Mutex &mutex,tuint32 timeout)
        {
            pthread_mutex_lock(&mutex_);
            mutex.unlock();
            bool res = wait(timeout);

            pthread_mutex_unlock(&mutex_);
            mutex.lock();

            return res;
        }

        void WaitCondition::signal_one()
        {
            pthread_mutex_lock(&mutex_);
            wakeups_ = std::min(wakeups_ + 1,waiters_); // Wake at most one.
            pthread_cond_signal(&cond_);
            pthread_mutex_unlock(&mutex_);
        }

        void WaitCondition::signal_all()
        {
            pthread_mutex_lock(&mutex_);
            wakeups_ = waiters_;
            pthread_cond_broadcast(&cond_);
            pthread_mutex_unlock(&mutex_);
        }
    }
}

