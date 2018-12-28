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

#include "stdafx.hh"
#include <memory>
#include "ckcore/assert.hh"
#include "ckcore/locker.hh"
#include "ckcore/thread.hh"

namespace ckcore
{
    Thread::Thread()
        : thread_(NULL),start_event_(NULL),running_(false)
    {
        start_event_ = CreateEvent(NULL,false,false,NULL);
    }

    Thread::~Thread()
    {
        kill();

        // Destroy start event.
        if (start_event_ != NULL)
        {
            ckVERIFY(0 != CloseHandle(start_event_));
            start_event_ = NULL;
        }
    }

    unsigned long __stdcall Thread::native_thread(void *param)
    {
        Thread *thread = static_cast<Thread *>(param);

        // Now when we have the mutex we can notify the thread creator that we have
        // started.
        SetEvent(thread->start_event_);

        try
        {
            thread->run();
        }
        catch (...)
        {
        }

        Locker<thread::Mutex> lock(thread->mutex_);
        thread->thread_done_.signal_all();
        thread->running_ = false;

        return NULL;
    }

    bool Thread::start()
    {
        Locker<thread::Mutex> lock(mutex_);

        if (running_)
            return false;

        // Create the thread.
        unsigned long thread_id = 0;
        thread_ = CreateThread(NULL,0,native_thread,this,0,&thread_id);
        if (thread_ == NULL)
            return false;

        // Wait for the thread to start before returning.
        if (WaitForSingleObject(start_event_,INFINITE) == WAIT_FAILED)
            Sleep(200);

        running_ = true;
        return true;
    }

    bool Thread::wait(tuint32 timeout)
    {
        Locker<thread::Mutex> lock(mutex_);

        // Make sure a thread is not waiting on itself.
        if (thread_ == GetCurrentThread())
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

        if (TerminateThread(thread_,1) == FALSE)
            return false;

        // We might have to trigger thread shutdown manually.
        if (running_)
            thread_done_.signal_all();

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
         * @return Always returns 0.
         */
        unsigned long __stdcall native_thread(void *param)
        {
            std::auto_ptr<NativeThreadParam> native_param(
                static_cast<NativeThreadParam *>(param));
            native_param->func_(native_param->param_);

            return 0;
        }

        bool create(tfunction func,void *param)
        {
            // Setup the parameters to pass to the native thread.
            NativeThreadParam *native_param = new NativeThreadParam(func,param);
            if (native_param == NULL)
                return false;

            // Create the thread.
            unsigned long thread_id = 0;
            if (CreateThread(NULL,0,native_thread,native_param,0,&thread_id) == NULL)
            {
                delete native_param;
                return false;
            }
            
            return true;
        }

        bool sleep(ckcore::tuint32 milliseconds)
        {
            Sleep(milliseconds);
            return true;
        }

        tuint32 ideal_count()
        {
            SYSTEM_INFO sys_info;
            GetSystemInfo(&sys_info);
            return sys_info.dwNumberOfProcessors;
        }

        thandle identifier()
        {
            return (thandle)static_cast<tuint64>(GetCurrentThreadId());
        }

        Mutex::Mutex() : handle_(CreateMutex(NULL,FALSE,NULL))
        {
        }

        Mutex::~Mutex()
        {
            if (handle_ != NULL)
            {
                ckVERIFY(0 != CloseHandle(handle_));
                handle_ = NULL;
            }
        }

        bool Mutex::lock()
        {
            if (handle_ == NULL)
                return false;

            bool res = WaitForSingleObject(handle_,INFINITE) == WAIT_OBJECT_0;
            ckTRACE_IF(!res,"waiting for thread mutex failed, last error %d\n",GetLastError());
            return res;
        }

        bool Mutex::unlock()
        {
            if (handle_ == NULL)
                return false;

            bool res = ReleaseMutex(handle_) != 0;
            ckTRACE_IF(!res,"releasing thread mutex failed, last error %d\n",GetLastError());
            return res;
        }

        bool Mutex::try_lock()
        {
            return WaitForSingleObject(handle_,0) == WAIT_OBJECT_0;
        }

        WaitCondition::WaitCondition()
            : broadcast_(false),waiters_(0),
              sema_(NULL),waiters_done_(NULL)
        {
            InitializeCriticalSection(&critical_);

            sema_ = CreateSemaphore(NULL,0,0x7fffffff,NULL);
            waiters_done_ = CreateEvent(NULL,FALSE,FALSE,NULL);
        }

        WaitCondition::~WaitCondition()
        {
            if (waiters_done_ != NULL)
            {
                CloseHandle(waiters_done_);
                waiters_done_ = NULL;
            }

            if (sema_ != NULL)
            {
                CloseHandle(sema_);
                sema_ = NULL;
            }

            DeleteCriticalSection(&critical_);
        }

        bool WaitCondition::wait(Mutex &mutex,tuint32 timeout)
        {
            EnterCriticalSection(&critical_);
            waiters_++;
            LeaveCriticalSection(&critical_);

            // Release the mutex and wait on the semaphore which will be set
            // through signal_one or signal_all.
            if (SignalObjectAndWait(mutex.handle_,sema_,
                                    timeout == std::numeric_limits<tuint32>::max() ? INFINITE : timeout,
                                    FALSE) != WAIT_OBJECT_0)
            {
                EnterCriticalSection(&critical_);
                waiters_--;
                LeaveCriticalSection(&critical_);

                mutex.lock();
                return false;
            }

            EnterCriticalSection(&critical_);
            waiters_--;
            // The last waiter to receive a broadcast must notify the signaler.
            bool last_waiter = broadcast_ && waiters_ == 0;
            LeaveCriticalSection(&critical_);

            // If we're the last waiter thread during this particular broadcast
            // then let all the other threads proceed.
            if (last_waiter)
                SignalObjectAndWait(waiters_done_,mutex.handle_,INFINITE,FALSE);
            else
                mutex.lock();   // The mutex must always be re-locked. 

            return true;
        }

        void WaitCondition::signal_one()
        {
            EnterCriticalSection(&critical_);
            int have_waiters = waiters_ > 0;
            LeaveCriticalSection(&critical_);

            if (have_waiters)
                ReleaseSemaphore(sema_,1,0);
        }

        void WaitCondition::signal_all()
        {
            EnterCriticalSection(&critical_);

            if (waiters_ > 0)
            {
                // We are broadcasting, even if there is just one waiter...
                // Record that we are broadcasting, which helps optimize
                // <pthread_cond_wait> for the non-broadcast case.
                broadcast_ = true;

                // Wake up all the waiters atomically.
                ReleaseSemaphore(sema_,waiters_,0);

                LeaveCriticalSection(&critical_);

                // Wait for all the awakened threads to acquire the counting
                // semaphore. 
                WaitForSingleObject(waiters_done_,INFINITE);

                // This assignment is okay, even without the <waiters_count_lock_> held 
                // because no other waiter threads can wake up to access it.
                broadcast_ = false;
            }
            else
            {
                LeaveCriticalSection(&critical_);
            }
        }
    };
};
