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

#include <cxxtest/TestSuite.h>
#include "ckcore/types.hh"
#include "ckcore/task.hh"
#include "ckcore/threadpool.hh"

class TestTask1: public ckcore::Task
{
private:
    void start()
    {
        ckcore::thread::sleep(100);
        (*result_)++;
    }

public:
    int *result_;
    int *deleted_;

    TestTask1(int *result,int *deleted) : result_(result),deleted_(deleted)
    {
    }

    ~TestTask1()
    {
        *deleted_ = 1;
    }
};

class ThreadPoolTestSuite : public CxxTest::TestSuite
{
public:
    void testThreadPoolIdleReuse()
    {
#if 1
        ckcore::ThreadPool &tp = ckcore::ThreadPool::instance();
        tp.set_retire_timeout(ckcore::ThreadPool::THREAD_RETIRE_TIMEOUT);

        TS_ASSERT_EQUALS(tp.active_threads(),0);
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);

        int results[32];
        int deleted[32];
        memset(results,0,sizeof(int) * 32);
        memset(deleted,0,sizeof(int) * 32);

        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TestTask1 *task = new TestTask1(&results[i],&deleted[i]);
            TS_ASSERT_EQUALS(results[i],0);
            TS_ASSERT(tp.start(task));
        }

        // Verify that results has not yet been written.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],0);
            TS_ASSERT_EQUALS(deleted[i],0);
        }

        TS_ASSERT_EQUALS(tp.active_threads(),ckcore::thread::ideal_count());
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);

        ckcore::thread::sleep(200);

        // Verify results.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],1);
            TS_ASSERT_EQUALS(deleted[i],1);
        }

        TS_ASSERT_EQUALS(tp.active_threads(),0);
        TS_ASSERT_EQUALS(tp.idle_threads(),ckcore::thread::ideal_count());
        TS_ASSERT_EQUALS(tp.retired_threads(),0);

        /*
         * Run second pass in order to test thread re-use.
         */
        memset(deleted,0,sizeof(int) * 32);

        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TestTask1 *task = new TestTask1(&results[i],&deleted[i]);
            TS_ASSERT_EQUALS(results[i],1);
            TS_ASSERT(tp.start(task));
        }

        // Verify that results has not yet been written.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],1);
            TS_ASSERT_EQUALS(deleted[i],0);
        }

        ckcore::thread::sleep(200);

        // Verify results.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],2);
            TS_ASSERT_EQUALS(deleted[i],1);
        }

        tp.wait();
        TS_ASSERT_EQUALS(tp.active_threads(),0);
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);
#endif
    }

    void testThreadPoolRetiredReuse()
    {
#if 1
        ckcore::ThreadPool &tp = ckcore::ThreadPool::instance();
        tp.set_retire_timeout(0);

        TS_ASSERT_EQUALS(tp.active_threads(),0);
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);

        int results[32];
        int deleted[32];
        memset(results,0,sizeof(int) * 32);
        memset(deleted,0,sizeof(int) * 32);

        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TestTask1 *task = new TestTask1(&results[i],&deleted[i]);
            TS_ASSERT_EQUALS(results[i],0);
            TS_ASSERT(tp.start(task));
        }

        // Verify that results has not yet been written.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],0);
            TS_ASSERT_EQUALS(deleted[i],0);
        }

        TS_ASSERT_EQUALS(tp.active_threads(),ckcore::thread::ideal_count());
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);

        // Wait for all tasks to finish.
        while (tp.active_threads() > 0)
            ckcore::thread::sleep(200);

        // Verify results.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],1);
            TS_ASSERT_EQUALS(deleted[i],1);
        }

        TS_ASSERT_EQUALS(tp.active_threads(),0);
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),ckcore::thread::ideal_count());

        /*
         * Run second pass in order to test thread re-use.
         */
        memset(deleted,0,sizeof(int) * 32);

        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TestTask1 *task = new TestTask1(&results[i],&deleted[i]);
            TS_ASSERT_EQUALS(results[i],1);
            TS_ASSERT(tp.start(task));
        }

        // Verify that results has not yet been written.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],1);
            TS_ASSERT_EQUALS(deleted[i],0);
        }

        ckcore::thread::sleep(200);

        // Verify results.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],2);
            TS_ASSERT_EQUALS(deleted[i],1);
        }

        tp.wait();
        TS_ASSERT_EQUALS(tp.active_threads(),0);
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);
#endif
    }

    void testThreadPoolWait()
    {
#if 1
        ckcore::ThreadPool &tp = ckcore::ThreadPool::instance();
        tp.set_retire_timeout(ckcore::ThreadPool::THREAD_RETIRE_TIMEOUT);

        TS_ASSERT_EQUALS(tp.active_threads(),0);
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);

        int results[32];
        int deleted[32];
        memset(results,0,sizeof(int) * 32);
        memset(deleted,0,sizeof(int) * 32);

        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TestTask1 *task = new TestTask1(&results[i],&deleted[i]);
            TS_ASSERT_EQUALS(results[i],0);
            TS_ASSERT(tp.start(task));
        }

        // Verify that results has not yet been written.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],0);
            TS_ASSERT_EQUALS(deleted[i],0);
        }

        TS_ASSERT_EQUALS(tp.active_threads(),ckcore::thread::ideal_count());
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);

        tp.wait();

        // Verify results.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],1);
            TS_ASSERT_EQUALS(deleted[i],1);
        }

        TS_ASSERT_EQUALS(tp.active_threads(),0);
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);

        /*
         * Run second pass in order to test thread re-use.
         */
        memset(deleted,0,sizeof(int) * 32);

        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TestTask1 *task = new TestTask1(&results[i],&deleted[i]);
            TS_ASSERT_EQUALS(results[i],1);
            TS_ASSERT(tp.start(task));
        }

        // Verify that results has not yet been written.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],1);
            TS_ASSERT_EQUALS(deleted[i],0);
        }

        tp.wait();

        // Verify results.
        for (ckcore::tuint32 i = 0; i < ckcore::thread::ideal_count(); i++)
        {
            TS_ASSERT_EQUALS(results[i],2);
            TS_ASSERT_EQUALS(deleted[i],1);
        }

        tp.wait();
        TS_ASSERT_EQUALS(tp.active_threads(),0);
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);
#endif
    }

    void testThreadPoolReserve()
    {
#if 1
        ckcore::ThreadPool &tp = ckcore::ThreadPool::instance();
        tp.set_retire_timeout(ckcore::ThreadPool::THREAD_RETIRE_TIMEOUT);

        TS_ASSERT_EQUALS(tp.active_threads(),0);
        TS_ASSERT_EQUALS(tp.idle_threads(),0);
        TS_ASSERT_EQUALS(tp.retired_threads(),0);

        int result = 0;
        int deleted[32];
        memset(deleted,0,sizeof(int) * 32);

        // Reserve all threads.
        tp.reserve(ckcore::thread::ideal_count());

        TestTask1 *task = new TestTask1(&result,&deleted[0]);
        TS_ASSERT(tp.start(task));
        
        // Make sure the task is queued and not executed.
        TS_ASSERT_EQUALS(tp.queued(),1);
        ckcore::thread::sleep(200);
        TS_ASSERT_EQUALS(tp.queued(),1);

        // Decrease the reserve count allowing the task to run.
        tp.reserve(ckcore::thread::ideal_count() - 1);
        ckcore::thread::sleep(200);
        TS_ASSERT_EQUALS(tp.queued(),0);

        tp.wait();

        TS_ASSERT_EQUALS(result,1);
        TS_ASSERT_EQUALS(deleted[0],1);

        // Run a bounch if tasks in sequence. How to we test it? We let all
        // tasks update the same value which very unlikely would work in a
        // parallel environment where all tasks run simultaneously.
        result = 0;
        memset(deleted,0,sizeof(int) * 32);

        for (ckcore::tuint32 i = 0; i < 32; i++)
        {
            task = new TestTask1(&result,&deleted[i]);
            TS_ASSERT(tp.start(task));
        }

        tp.wait();

        // Verify results.
        TS_ASSERT_EQUALS(result,32);
        for (ckcore::tuint32 i = 0; i < 32; i++)
        {
            TS_ASSERT_EQUALS(deleted[i],1);
        }
#endif
    }
};
