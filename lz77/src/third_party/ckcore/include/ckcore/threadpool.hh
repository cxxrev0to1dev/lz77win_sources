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
 * @file include/ckcore/threadpool.hh
 * @brief Thread pool interface.
 */

#pragma once
#include <queue>
#include "ckcore/types.hh"
#include "ckcore/task.hh"
#include "ckcore/thread.hh"

namespace ckcore
{
    /**
     * @brief Thread pool singleton class.
     */
    class ThreadPool
    {
    public:
        /**
         * @brief Defines constants specifying the class behaviour.
         */
        enum
        {
            THREAD_RETIRE_TIMEOUT = 20000   ///< How long an idle thread will wait for a new task before retiring.
        };

    private:
        /**
         * @brief Internal thread class.
         */
        class InternalThread : public Thread
        {
        private:
            ThreadPool &host_;

            /**
             * Executes the thread.
             */
            void run();

        public:
            Task *task_;

            /**
             * Constructs an internal thread object.
             * @param [in] host The hosting thread pool object.
             * @param [in] task The task to execute in the thread.
             */
            InternalThread(ThreadPool &host,Task *task);
        };

    private:
        bool exiting_;          ///< Set to true when thread pool is exiting.
        const tuint32 max_threads_;   ///< Maximum number of threads.
        tuint32 pol_threads_;   ///< Number of active threads in the pool.
        tuint32 res_threads_;   ///< Number of reserved threads.
        tuint32 idl_threads_;   ///< Number of idle threads.
        thread::Mutex mutex_;

        thread::WaitCondition task_ready_;          ///< Signaled to a thread when a task is ready for execution.

        std::vector<InternalThread *> all_threads_; ///< All threads.
        std::vector<InternalThread *> ret_threads_; ///< Retired threads.

        tuint32 ret_timeout_;   ///< How long a thread can indle before being retired.

        std::priority_queue<std::pair<Task *,tuint32> > queue_;

        /**
         * Puts a task into the work queue.
         * @param [in] task Task to enqueue.
         * @param [in] priority Task priority.
         */
        void enqueue(Task *task,tuint32 priority = 0);

        /**
         * Spawn a new thread and start executing the task.
         * @param [in] task The task to execute.
         * @return If the task could not be spawned in a new thread false is
         *         returned, otherwise true is returned.
         */
        bool spawn(Task *task);

        /**
         * Check if we're currently serving more threads than we should. This may
         * happen if threads are reserved while executing tasks.
         * @return If we'we overworking true is returned, if not false is returned.
         */
        bool overworking() const;

        /**
         * Tries to start the specified task immediately. If that's not possible
         * the function will fail.
         * @param [in] task The task to execute.
         * @return If the task was started true is returned, otherwise false is
         *         returned.
         */
        bool try_start(Task *);

        /**
         * Constructs a thread pool object. The pool will configure itself to
         * handle the ideal number of threads for the current system. That is the
         * ideal number of threads the system can execute in parallel.
         */
        ThreadPool();
        ThreadPool(const ThreadPool &rhs);

        /**
         * Destructs the thread pool object.
         */
        ~ThreadPool();
        ThreadPool &operator=(const ThreadPool &rhs);

    public:
        /**
         * Returns the single instance to the thread pool.
         * @return The single instance to the thread pool.
         */
        static ThreadPool &instance();

        /**
         * Returns the total number of active threads in the application. This is
         * the number of threads used by the pool plus the number of reserved
         * threads.
         * @return The total number of active threads.
         */
        tuint32 active_threads() const;

        /**
         * Returns the number of idle threads.
         * @return The number of idle threads.
         */
        tuint32 idle_threads() const;

        /**
         * Returns the number of retired threads.
         * @return The number of retired threads.
         */
        tuint32 retired_threads() const;

        /**
         * Returns the number of queued tasks that are not yet assigned to a
         * thread.
         * @return The number of queued tasks.
         */
        tuint32 queued() const;

        /**
         * Tries to start the specified task immediately, if that's not possible
         * it will be queued with the specified task priority.
         * @param [in] task The task to execute.
         * @param [in] priority The task priority.
         * @return If successful true is returned, otherwise false is returned.
         */
        bool start(Task *task,tuint32 priority = 0);

        /**
         * Executes the specified task immediatly if there is a free thread
         * available. If there is no free thread available so that the task can
         * execute right away the function will fail and the task will not be
         * queued.
         * @param [in] task The task to execute.
         * @return If the task was executed right away true is returned, if not
         *         false is returned.
         */
        bool start_now(Task *task);

        /**
         * Waits for all tasks to finish and shutdown all threads essentially
         * restoing the thread pool. This does not reset the number of reserved
         * threads.
         */
        void wait();

        /**
         * Reserve a number of threads for use outside the thread pool.
         * @param [in] num_threads Number of threads to reserve.
         */
        void reserve(tuint32 num_threads);

        /**
         * Sets the timeout before retiring idle threads.
         * @param [in] timeout New timeout in milliseconds.
         */
        void set_retire_timeout(tuint32 timeout);
    };
}

