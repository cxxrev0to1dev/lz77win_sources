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
#include "ckcore/locker.hh"
#include "ckcore/types.hh"
#include "ckcore/thread.hh"

class TestThread1 : public ckcore::Thread
{
private:
    void run()
    {
        result_++;
    }

public:
    int result_;

    TestThread1() : result_(0) {}
};

class TestThread2 : public ckcore::Thread
{
private:
    void run()
    {
        ckcore::thread::sleep(20);
        result_++;
    }

public:
    int result_;

    TestThread2() : result_(0) {}
};

/**
 * @brief Test thread for mutexes.
 */
class TestThread3 : public ckcore::Thread
{
private:
    int &value_;
    ckcore::thread::Mutex &mutex_;

    void run()
    {
        ckcore::thread::sleep(20);

        ckcore::Locker<ckcore::thread::Mutex> lock(mutex_);

        for (int i = 0; i < 1024; i++)
            value_++;
    }

public:
    TestThread3(int &value,ckcore::thread::Mutex &mutex) : value_(value),mutex_(mutex) {}
};

/**
 * @brief Test thread for wait condition using shared mutex.
 */
class TestThread4 : public ckcore::Thread
{
private:
    int &value_;
    ckcore::thread::Mutex &mutex_;
    ckcore::thread::WaitCondition &wc_;

    void run()
    {
        ckcore::Locker<ckcore::thread::Mutex> lock(mutex_);
        wc_.wait(mutex_);
        value_ = 1;
    }

public:
    TestThread4(int &value,ckcore::thread::Mutex &mutex,
                ckcore::thread::WaitCondition &wc) : value_(value),mutex_(mutex),wc_(wc) {}
};

/**
 * @brief Test thread for wait condition using non-shared mutex.
 */
class TestThread5 : public ckcore::Thread
{
private:
    int &value_;
    ckcore::thread::Mutex mutex_;
    ckcore::thread::WaitCondition &wc_;

    void run()
    {
        ckcore::Locker<ckcore::thread::Mutex> lock(mutex_);
        wc_.wait(mutex_);
        value_ = 1;
    }

public:
    TestThread5(int &value,ckcore::thread::WaitCondition &wc) : value_(value),wc_(wc) {}
};

class ThreadTestSuite : public CxxTest::TestSuite
{
public:
    void testThreadReuse()
    {
        TestThread1 thread;
        TS_ASSERT_EQUALS(thread.result_,0);
        TS_ASSERT(thread.start());
        //TS_ASSERT(thread.running());
        ckcore::thread::sleep(20);
        TS_ASSERT_EQUALS(thread.result_,1);

        for (size_t i = 0; i < 9; i++)
        {
            thread.start();
            thread.wait();
        }

        TS_ASSERT_EQUALS(thread.result_,10);
    }

    void testThreadWait()
    {
        TestThread2 thread;
        TS_ASSERT_EQUALS(thread.result_,0);
        TS_ASSERT(thread.start());
        TS_ASSERT(thread.running());
        TS_ASSERT(thread.wait());
        TS_ASSERT(!thread.running());
        TS_ASSERT_EQUALS(thread.result_,1);
    }

    void testThreadKill()
    {
        TestThread2 thread;
        TS_ASSERT_EQUALS(thread.result_,0);
        TS_ASSERT(thread.start());
        TS_ASSERT(thread.running());
        TS_ASSERT(thread.kill());
        TS_ASSERT(!thread.running());
        TS_ASSERT_EQUALS(thread.result_,0);

        // Test that a killed thread can be restarted.
        TS_ASSERT(thread.start());
        TS_ASSERT(thread.running());
        TS_ASSERT(thread.wait());
        TS_ASSERT_EQUALS(thread.result_,1);
    }

    void testThreadMutex()
    {
        // This test is based on the idea that it's unlikeley that a large
        // number of threads reading/writing to the same memory will generate
        // expected results without a proper locking mechanism.
        int value = 0;
        ckcore::thread::Mutex mutex;

        TestThread3 thread[32] =
        {
            TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),
            TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),
            TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),
            TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),
            TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),
            TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),
            TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),
            TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex),TestThread3(value,mutex)
        };

        for (size_t i = 0; i < 32; i++)
        {
            thread[i].start();
        }

        // Wait for threads to finish.
        for (size_t i = 0; i < 32; i++)
        {
            while (thread[i].running())
                ckcore::thread::sleep(20);
        }

        // Verify result.
        TS_ASSERT_EQUALS(value,32 * 1024);
    }

    void testThreadWaitConditionSignalOne()
    {
        int value[32] = { 0 };
        ckcore::thread::Mutex mutex;
        ckcore::thread::WaitCondition wc;

        TestThread4 thread[32] =
        {
            TestThread4(value[ 0],mutex,wc),TestThread4(value[ 1],mutex,wc),TestThread4(value[ 2],mutex,wc),TestThread4(value[ 3],mutex,wc),
            TestThread4(value[ 4],mutex,wc),TestThread4(value[ 5],mutex,wc),TestThread4(value[ 6],mutex,wc),TestThread4(value[ 7],mutex,wc),
            TestThread4(value[ 8],mutex,wc),TestThread4(value[ 9],mutex,wc),TestThread4(value[10],mutex,wc),TestThread4(value[11],mutex,wc),
            TestThread4(value[12],mutex,wc),TestThread4(value[13],mutex,wc),TestThread4(value[14],mutex,wc),TestThread4(value[15],mutex,wc),
            TestThread4(value[16],mutex,wc),TestThread4(value[17],mutex,wc),TestThread4(value[18],mutex,wc),TestThread4(value[19],mutex,wc),
            TestThread4(value[20],mutex,wc),TestThread4(value[21],mutex,wc),TestThread4(value[22],mutex,wc),TestThread4(value[23],mutex,wc),
            TestThread4(value[24],mutex,wc),TestThread4(value[25],mutex,wc),TestThread4(value[26],mutex,wc),TestThread4(value[27],mutex,wc),
            TestThread4(value[28],mutex,wc),TestThread4(value[29],mutex,wc),TestThread4(value[30],mutex,wc),TestThread4(value[31],mutex,wc)
        };

        for (size_t i = 0; i < 32; i++)
        {
            thread[i].start();
        }

        ckcore::thread::sleep(20);

        // Verify that all threads are waiting.
        for (size_t i = 0; i < 32; i++)
        {
            TS_ASSERT(thread[i].running());
            TS_ASSERT_EQUALS(value[i],0);
        }

        // Signal the threads one by one.
        for (size_t i = 0; i < 32; i++)
        {
            wc.signal_one();
            ckcore::thread::sleep(20);

            int count = 0;
            for (size_t j = 0; j < 32; j++)
                count += value[j];

            TS_ASSERT_EQUALS(count,i + 1);
        }
    }

    void testThreadWaitConditionSignalOneNonShared()
    {
        int value[32] = { 0 };
        ckcore::thread::Mutex mutex;
        ckcore::thread::WaitCondition wc;

        TestThread5 thread[32] =
        {
            TestThread5(value[ 0],wc),TestThread5(value[ 1],wc),TestThread5(value[ 2],wc),TestThread5(value[ 3],wc),
            TestThread5(value[ 4],wc),TestThread5(value[ 5],wc),TestThread5(value[ 6],wc),TestThread5(value[ 7],wc),
            TestThread5(value[ 8],wc),TestThread5(value[ 9],wc),TestThread5(value[10],wc),TestThread5(value[11],wc),
            TestThread5(value[12],wc),TestThread5(value[13],wc),TestThread5(value[14],wc),TestThread5(value[15],wc),
            TestThread5(value[16],wc),TestThread5(value[17],wc),TestThread5(value[18],wc),TestThread5(value[19],wc),
            TestThread5(value[20],wc),TestThread5(value[21],wc),TestThread5(value[22],wc),TestThread5(value[23],wc),
            TestThread5(value[24],wc),TestThread5(value[25],wc),TestThread5(value[26],wc),TestThread5(value[27],wc),
            TestThread5(value[28],wc),TestThread5(value[29],wc),TestThread5(value[30],wc),TestThread5(value[31],wc)
        };

        for (size_t i = 0; i < 32; i++)
        {
            thread[i].start();
        }

        ckcore::thread::sleep(20);

        // Verify that all threads are waiting.
        for (size_t i = 0; i < 32; i++)
        {
            TS_ASSERT(thread[i].running());
            TS_ASSERT_EQUALS(value[i],0);
        }

        // Signal the threads one by one.
        for (size_t i = 0; i < 32; i++)
        {
            wc.signal_one();
            ckcore::thread::sleep(20);

            int count = 0;
            for (size_t j = 0; j < 32; j++)
                count += value[j];

            TS_ASSERT_EQUALS(count,i + 1);
        }
    }

    void testThreadWaitConditionSignalAll()
    {
        int value[32] = { 0 };
        ckcore::thread::Mutex mutex;
        ckcore::thread::WaitCondition wc;

        TestThread4 thread[32] =
        {
            TestThread4(value[ 0],mutex,wc),TestThread4(value[ 1],mutex,wc),TestThread4(value[ 2],mutex,wc),TestThread4(value[ 3],mutex,wc),
            TestThread4(value[ 4],mutex,wc),TestThread4(value[ 5],mutex,wc),TestThread4(value[ 6],mutex,wc),TestThread4(value[ 7],mutex,wc),
            TestThread4(value[ 8],mutex,wc),TestThread4(value[ 9],mutex,wc),TestThread4(value[10],mutex,wc),TestThread4(value[11],mutex,wc),
            TestThread4(value[12],mutex,wc),TestThread4(value[13],mutex,wc),TestThread4(value[14],mutex,wc),TestThread4(value[15],mutex,wc),
            TestThread4(value[16],mutex,wc),TestThread4(value[17],mutex,wc),TestThread4(value[18],mutex,wc),TestThread4(value[19],mutex,wc),
            TestThread4(value[20],mutex,wc),TestThread4(value[21],mutex,wc),TestThread4(value[22],mutex,wc),TestThread4(value[23],mutex,wc),
            TestThread4(value[24],mutex,wc),TestThread4(value[25],mutex,wc),TestThread4(value[26],mutex,wc),TestThread4(value[27],mutex,wc),
            TestThread4(value[28],mutex,wc),TestThread4(value[29],mutex,wc),TestThread4(value[30],mutex,wc),TestThread4(value[31],mutex,wc)
        };

        for (size_t i = 0; i < 32; i++)
        {
            thread[i].start();
        }

        ckcore::thread::sleep(20);

        // Verify that all threads are waiting.
        for (size_t i = 0; i < 32; i++)
        {
            TS_ASSERT(thread[i].running());
            TS_ASSERT_EQUALS(value[i],0);
        }

        // Signal all threads.
        wc.signal_all();
        ckcore::thread::sleep(20);

        int count = 0;
        for (size_t i = 0; i < 32; i++)
        {
            count += value[i];
        }

        TS_ASSERT_EQUALS(count,32);
    }

    void testThreadWaitConditionSignalAllNonShared()
    {
        int value[32] = { 0 };
        ckcore::thread::Mutex mutex;
        ckcore::thread::WaitCondition wc;

        TestThread5 thread[32] =
        {
            TestThread5(value[ 0],wc),TestThread5(value[ 1],wc),TestThread5(value[ 2],wc),TestThread5(value[ 3],wc),
            TestThread5(value[ 4],wc),TestThread5(value[ 5],wc),TestThread5(value[ 6],wc),TestThread5(value[ 7],wc),
            TestThread5(value[ 8],wc),TestThread5(value[ 9],wc),TestThread5(value[10],wc),TestThread5(value[11],wc),
            TestThread5(value[12],wc),TestThread5(value[13],wc),TestThread5(value[14],wc),TestThread5(value[15],wc),
            TestThread5(value[16],wc),TestThread5(value[17],wc),TestThread5(value[18],wc),TestThread5(value[19],wc),
            TestThread5(value[20],wc),TestThread5(value[21],wc),TestThread5(value[22],wc),TestThread5(value[23],wc),
            TestThread5(value[24],wc),TestThread5(value[25],wc),TestThread5(value[26],wc),TestThread5(value[27],wc),
            TestThread5(value[28],wc),TestThread5(value[29],wc),TestThread5(value[30],wc),TestThread5(value[31],wc)
        };

        for (size_t i = 0; i < 32; i++)
        {
            thread[i].start();
        }

        ckcore::thread::sleep(20);

        // Verify that all threads are waiting.
        for (size_t i = 0; i < 32; i++)
        {
            TS_ASSERT(thread[i].running());
            TS_ASSERT_EQUALS(value[i],0);
        }

        // Signal all threads.
        wc.signal_all();
        ckcore::thread::sleep(20);

        int count = 0;
        for (size_t i = 0; i < 32; i++)
        {
            count += value[i];
        }

        TS_ASSERT_EQUALS(count,32);
    }
};
