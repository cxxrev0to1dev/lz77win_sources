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

/**
 * @file include/ckcore/windows/thread.hh
 * @brief Defines the Windows thread implementation.
 */
#pragma once
#include <limits>
#include <windows.h>
#include "ckcore/types.hh"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace ckcore
{
    namespace thread
    {
        /**
         * Creates and starts the execution of a new thread.
         * @param [in] func The thread function entry point.
         * @param [in] param Optional thread parameter.
         * @return If the thread was successfully created true is returned, if
         *         not false is returned.
         */
        bool create(tfunction func,void *param);

        /**
         * Sleeps the current thread for a specified amount of milliseconds.
         * @param [in] milliseconds The number of milliseconds to sleep the
         *                          thread.
         * @return If successful true is returned, if not false is returned.
         */
        bool sleep(tuint32 milliseconds);

        /**
         * Returns the ideal number of threads that the current system can
         * execute in parallel.
         * @return The ideal number of threads that the current system can
         *         execute in parallel.
         */
        tuint32 ideal_count();

        /**
         * Returns the current thread identifier.
         * @return The current thread identifier.
         */
        thandle identifier();

        /**
         * @brief Thead mutex class.
         */
        class Mutex
        {
        public:
            friend class WaitCondition;

        private:
            HANDLE handle_;

        public:
            /**
             * Constructs a Mutex object.
             */
            Mutex();

            /**
             * Destructs the Mutex object.
             */
            ~Mutex();

            /**
             * Locks the mutex.
             * @return If successful true is returned, if unsuccessful false is
             *         returned.
             */
            bool lock();

            /**
             * Unlocks the mutex.
             * @return If successful true is returned, if unsuccessful false is
             *         returned.
             */
            bool unlock();

            /**
             * Tries to lock the mutex and returns immediately if the mutex is
             * locked by another thread.
             * @return If the mutex was successfully locked true is returned, if
             *         the mutex could not be locked the function returns false.
             */
            bool try_lock();
        };

        /**
         * @brief Wait condition class.
         */
        class WaitCondition
        {
        private:
            bool broadcast_;
            tuint32 waiters_;
            CRITICAL_SECTION critical_;
            HANDLE sema_;
            HANDLE waiters_done_;

        public:
            /**
             * Constructs a wait condition object.
             */
            WaitCondition();

            /**
             * Destructs the wait condition object.
             */
            ~WaitCondition();

            /**
             * Waits on the mutex.
             * @param [in] mutex Mutex to wait on.
             * @param [in] timeout Time out in milliseconds.
             * @return If successfully waited in the event with no time out true
             *         is returned, otherwise false is returned.
             */
            bool wait(Mutex &mutex,tuint32 timeout = std::numeric_limits<tuint32>::max());

            /**
             * Signals one waiting thread to continue.
             */
            void signal_one();

            /**
             * Signals all waiting threads to continue.
             */
            void signal_all();
        };
    };

    /**
     * @brief Thread class.
     */
    class Thread
    {
    private:
        HANDLE thread_;
        HANDLE start_event_;
        bool running_;
        mutable thread::Mutex mutex_;
        thread::WaitCondition thread_done_;

        /**
         * The main thread entry point for new threads.
         * @param [in] param Pointer to thread object.
         * @return Always returns NULL.
         */
        static unsigned long __stdcall native_thread(void *param);

    protected:
        virtual void run() = 0;

    public:
        /**
         * Constructs a new thread object.
         */
        Thread();

        /**
         * Destructs the thread object and kills the thread if it's running.
         */
        virtual ~Thread();

        /**
         * Starts the thread.
         * @return If the thread was successfully started true is returned,
         *         otherwise false is returned.
         */
        bool start();

        /**
         * Waits until the thread has finished.
         * @param [in] timout Maximum time to wait in milliseconds.
         * @return If no timeout ocurred true is returned, otherwise false is
         *         returned.
         */
        bool wait(tuint32 timeout = std::numeric_limits<tuint32>::max());

        /**
         * Immediately kills the thread, the function does not return until the
         * the thread has exited.
         * @return If the thread was successfully killed true is returned, if not
         *         false is returned.
         */
        bool kill();

        /**
         * Checks if the thread is currently running.
         * @return If the thread is running true is returned, if not false is
         *         returned.
         */
        bool running() const;
    };
};
