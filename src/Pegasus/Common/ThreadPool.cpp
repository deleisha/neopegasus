//%2006////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; Symantec Corporation; The Open Group.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//==============================================================================
//
// Author: Mike Day (mdday@us.ibm.com)
//
// Modified By: Rudy Schuet (rudy.schuet@compaq.com) 11/12/01
//              added nsk platform support
//              Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Amit K Arora, IBM (amita@in.ibm.com) for PEP#101
//              Sean Keenan, Hewlett-Packard Company (sean.keenan@hp.com)
//              David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include "ThreadPool.h"
#include "Thread.h"
#include <exception>
#include <Pegasus/Common/Tracer.h>
#include "Time.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//
// ThreadPool
//
///////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(Sint16 initialSize,
                       const char *key,
                       Sint16 minThreads,
                       Sint16 maxThreads,
                       struct timeval
                       &deallocateWait):_maxThreads(maxThreads),
_minThreads(minThreads), _currentThreads(0), _idleThreads(),
_runningThreads(), _dying(0)
{
    _deallocateWait.tv_sec = deallocateWait.tv_sec;
    _deallocateWait.tv_usec = deallocateWait.tv_usec;

    memset(_key, 0x00, 17);
    if (key != 0)
    {
        strncpy(_key, key, 16);
    }

    if ((_maxThreads > 0) && (_maxThreads < initialSize))
    {
        _maxThreads = initialSize;
    }

    if (_minThreads > initialSize)
    {
        _minThreads = initialSize;
    }

    for (int i = 0; i < initialSize; i++)
    {
        _addToIdleThreadsQueue(_initializeThread());
    }
}

ThreadPool::~ThreadPool()
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::~ThreadPool");

    try
    {
        // Set the dying flag so all thread know the destructor has been
        // entered
        _dying++;
        Tracer::trace(TRC_THREAD, Tracer::LEVEL2,
                      "Cleaning up %d idle threads. ", _currentThreads.get());

        while (_currentThreads.get() > 0)
        {
            Thread *thread = _idleThreads.remove_front();
            if (thread != 0)
            {
                _cleanupThread(thread);
                _currentThreads--;
            }
            else
            {
                Threads::yield();
            }
        }
    }
    catch(...)
    {
    }
}

ThreadReturnType PEGASUS_THREAD_CDECL ThreadPool::_loop(void *parm)
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::_loop");

    try
    {
        Thread *myself = (Thread *) parm;
        PEGASUS_ASSERT(myself != 0);

        // Set myself into thread specific storage
        // This will allow code to get its own Thread
        Thread::setCurrent(myself);

        ThreadPool *pool = (ThreadPool *) myself->get_parm();
        PEGASUS_ASSERT(pool != 0);

        Semaphore *sleep_sem = 0;
        struct timeval *lastActivityTime = 0;

        try
        {
            sleep_sem = (Semaphore *) myself->reference_tsd("sleep sem");
            myself->dereference_tsd();
            PEGASUS_ASSERT(sleep_sem != 0);

            lastActivityTime =
                (struct timeval *) myself->
                reference_tsd("last activity time");
            myself->dereference_tsd();
            PEGASUS_ASSERT(lastActivityTime != 0);
        }
        catch(...)
        {
            Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                          "ThreadPool::_loop: Failure getting sleep_sem or "
                          "lastActivityTime.");
            PEGASUS_ASSERT(false);
            pool->_idleThreads.remove(myself);
            pool->_currentThreads--;
            PEG_METHOD_EXIT();
            return ((ThreadReturnType) 1);
        }

        while (1)
        {
            try
            {
                sleep_sem->wait();
            }
            catch(...)
            {
                Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                              "ThreadPool::_loop: failure on sleep_sem->wait().");
                PEGASUS_ASSERT(false);
                pool->_idleThreads.remove(myself);
                pool->_currentThreads--;
                PEG_METHOD_EXIT();
                return ((ThreadReturnType) 1);
            }

            // When we awaken we reside on the _runningThreads queue, not the
            // _idleThreads queue.

            ThreadReturnType(PEGASUS_THREAD_CDECL * work) (void *) = 0;
            void *parm = 0;
            Semaphore *blocking_sem = 0;

            try
            {
                work = (ThreadReturnType(PEGASUS_THREAD_CDECL *) (void *))
                    myself->reference_tsd("work func");
                myself->dereference_tsd();
                parm = myself->reference_tsd("work parm");
                myself->dereference_tsd();
                blocking_sem =
                    (Semaphore *) myself->reference_tsd("blocking sem");
                myself->dereference_tsd();
            }
            catch(...)
            {
                Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                              "ThreadPool::_loop: Failure accessing work func, work parm, "
                              "or blocking sem.");
                PEGASUS_ASSERT(false);
                pool->_idleThreads.remove(myself);
                pool->_currentThreads--;
                PEG_METHOD_EXIT();
                return ((ThreadReturnType) 1);
            }

            if (work == 0)
            {
                Tracer::trace(TRC_THREAD, Tracer::LEVEL4,
                              "ThreadPool::_loop: work func is 0, meaning we should exit.");
                break;
            }

            Time::gettimeofday(lastActivityTime);

            try
            {
                PEG_TRACE_STRING(TRC_THREAD, Tracer::LEVEL4,
                                 "Work starting.");
                work(parm);
                PEG_TRACE_STRING(TRC_THREAD, Tracer::LEVEL4,
                                 "Work finished.");
            }
            catch(Exception & e)
            {
                PEG_TRACE_STRING(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                                 String
                                 ("Exception from work in ThreadPool::_loop: ")
                                 + e.getMessage());
            }
#if !defined(PEGASUS_OS_LSB)
            catch(const exception & e)
            {
                PEG_TRACE_STRING(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                                 String
                                 ("Exception from work in ThreadPool::_loop: ")
                                 + e.what());
            }
#endif
            catch(...)
            {
                PEG_TRACE_STRING(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                                 "Unknown exception from work in ThreadPool::_loop.");
            }

            // put myself back onto the available list
            try
            {
                Time::gettimeofday(lastActivityTime);
                if (blocking_sem != 0)
                {
                    blocking_sem->signal();
                }

                pool->_runningThreads.remove(myself);
                pool->_idleThreads.insert_front(myself);
            }
            catch(...)
            {
                Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                              "ThreadPool::_loop: Adding thread to idle pool failed.");
                PEGASUS_ASSERT(false);
                pool->_currentThreads--;
                PEG_METHOD_EXIT();
                return ((ThreadReturnType) 1);
            }
        }
    }
    catch(const Exception & e)
    {
        PEG_TRACE_STRING(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                         "Caught exception: \"" + e.getMessage() +
                         "\".  Exiting _loop.");
    }
    catch(...)
    {
        PEG_TRACE_STRING(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                         "Caught unrecognized exception.  Exiting _loop.");
    }

    PEG_METHOD_EXIT();
    return ((ThreadReturnType) 0);
}

ThreadStatus ThreadPool::allocate_and_awaken(void *parm,
                                             ThreadReturnType
                                             (PEGASUS_THREAD_CDECL *
                                              work) (void *),
                                             Semaphore * blocking)
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::allocate_and_awaken");

    // Allocate_and_awaken will not run if the _dying flag is set.
    // Once the lock is acquired, ~ThreadPool will not change
    // the value of _dying until the lock is released.

    try
    {
        if (_dying.get())
        {
            Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                          "ThreadPool::allocate_and_awaken: ThreadPool is dying(1).");
            return PEGASUS_THREAD_UNAVAILABLE;
        }
        struct timeval start;
        Time::gettimeofday(&start);
        Thread *th = 0;

        th = _idleThreads.remove_front();

        if (th == 0)
        {
            if ((_maxThreads == 0) ||
                (_currentThreads.get() < Uint32(_maxThreads)))
            {
                th = _initializeThread();
            }
        }

        if (th == 0)
        {
            // ATTN-DME-P3-20031103: This trace message should not be
            // be labeled TRC_DISCARDED_DATA, because it does not
            // necessarily imply that a failure has occurred.  However,
            // this label is being used temporarily to help isolate
            // the cause of client timeout problems.
            Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                          "ThreadPool::allocate_and_awaken: Insufficient resources: "
                          " pool = %s, running threads = %d, idle threads = %d",
                          _key, _runningThreads.size(), _idleThreads.size());
            return PEGASUS_THREAD_INSUFFICIENT_RESOURCES;
        }

        // initialize the thread data with the work function and parameters
        Tracer::trace(TRC_THREAD, Tracer::LEVEL4,
                      "Initializing thread with work function and parameters: parm = %p",
                      parm);

        th->delete_tsd("work func");
        th->put_tsd("work func", NULL,
                    sizeof (ThreadReturnType(PEGASUS_THREAD_CDECL *)
                            (void *)), (void *) work);
        th->delete_tsd("work parm");
        th->put_tsd("work parm", NULL, sizeof (void *), parm);
        th->delete_tsd("blocking sem");
        if (blocking != 0)
            th->put_tsd("blocking sem", NULL, sizeof (Semaphore *), blocking);

        // put the thread on the running list
        _runningThreads.insert_front(th);

        // signal the thread's sleep semaphore to awaken it
        Semaphore *sleep_sem = (Semaphore *) th->reference_tsd("sleep sem");
        PEGASUS_ASSERT(sleep_sem != 0);

        Tracer::trace(TRC_THREAD, Tracer::LEVEL4, "Signal thread to awaken");
        sleep_sem->signal();
        th->dereference_tsd();
    }
    catch(...)
    {
        Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                      "ThreadPool::allocate_and_awaken: Operation Failed.");
        PEG_METHOD_EXIT();
        // ATTN: Error result has not yet been defined
        return PEGASUS_THREAD_SETUP_FAILURE;
    }
    PEG_METHOD_EXIT();
    return PEGASUS_THREAD_OK;
}

// caller is responsible for only calling this routine during slack periods
// but should call it at least once per _deallocateWait interval.

Uint32 ThreadPool::cleanupIdleThreads()
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::cleanupIdleThreads");

    Uint32 numThreadsCleanedUp = 0;

    Uint32 numIdleThreads = _idleThreads.size();
    for (Uint32 i = 0; i < numIdleThreads; i++)
    {
        // Do not dip below the minimum thread count
        if (_currentThreads.get() <= (Uint32) _minThreads)
        {
            break;
        }

        Thread *thread = _idleThreads.remove_back();

        // If there are no more threads in the _idleThreads queue, we're
        // done.
        if (thread == 0)
        {
            break;
        }

        struct timeval *lastActivityTime;
        try
        {
            lastActivityTime =
                (struct timeval *) thread->
                try_reference_tsd("last activity time");
            PEGASUS_ASSERT(lastActivityTime != 0);
        }
        catch(...)
        {
            PEGASUS_ASSERT(false);
            _idleThreads.insert_back(thread);
            break;
        }

        Boolean cleanupThisThread =
            _timeIntervalExpired(lastActivityTime, &_deallocateWait);
        thread->dereference_tsd();

        if (cleanupThisThread)
        {
            _cleanupThread(thread);
            _currentThreads--;
            numThreadsCleanedUp++;
        }
        else
        {
            _idleThreads.insert_front(thread);
        }
    }

    PEG_METHOD_EXIT();
    return numThreadsCleanedUp;
}

void ThreadPool::_cleanupThread(Thread * thread)
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::cleanupThread");

    // Set the "work func" and "work parm" to 0 so _loop() knows to exit.
    thread->delete_tsd("work func");
    thread->put_tsd("work func", 0,
                    sizeof (ThreadReturnType(PEGASUS_THREAD_CDECL *)
                            (void *)), (void *) 0);
    thread->delete_tsd("work parm");
    thread->put_tsd("work parm", 0, sizeof (void *), 0);

    // signal the thread's sleep semaphore to awaken it
    Semaphore *sleep_sem = (Semaphore *) thread->reference_tsd("sleep sem");
    PEGASUS_ASSERT(sleep_sem != 0);
    sleep_sem->signal();
    thread->dereference_tsd();

    thread->join();
    delete thread;

    PEG_METHOD_EXIT();
}

Boolean ThreadPool::_timeIntervalExpired(struct timeval *start,
                                         struct timeval *interval)
{
    // never time out if the interval is zero
    if (interval && (interval->tv_sec == 0) && (interval->tv_usec == 0))
    {
        return false;
    }

    struct timeval now, finish, remaining;
    Uint32 usec;
    Time::gettimeofday(&now);
    Time::gettimeofday(&remaining);     // Avoid valgrind error

    finish.tv_sec = start->tv_sec + interval->tv_sec;
    usec = start->tv_usec + interval->tv_usec;
    finish.tv_sec += (usec / 1000000);
    usec %= 1000000;
    finish.tv_usec = usec;

    return (Time::subtract(&remaining, &finish, &now) != 0);
}

void ThreadPool::_deleteSemaphore(void *p)
{
    delete(Semaphore *) p;
}

Thread *ThreadPool::_initializeThread()
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::_initializeThread");

    Thread *th = (Thread *) new Thread(_loop, this, false);

    // allocate a sleep semaphore and pass it in the thread context
    // initial count is zero, loop function will sleep until
    // we signal the semaphore
    Semaphore *sleep_sem = (Semaphore *) new Semaphore(0);
    th->put_tsd("sleep sem", &_deleteSemaphore, sizeof (Semaphore),
                (void *) sleep_sem);

    struct timeval *lastActivityTime =
        (struct timeval *)::operator  new(sizeof (struct timeval));
    Time::gettimeofday(lastActivityTime);

    th->put_tsd("last activity time", thread_data::default_delete,
                sizeof (struct timeval), (void *) lastActivityTime);
    // thread will enter _loop() and sleep on sleep_sem until we signal it

    if (th->run() != PEGASUS_THREAD_OK)
    {
        Tracer::trace(TRC_THREAD, Tracer::LEVEL2,
                      "Could not create thread. Error code is %d.", errno);
        delete th;
        return 0;
    }
    _currentThreads++;
    Threads::yield();

    PEG_METHOD_EXIT();
    return th;
}

void ThreadPool::_addToIdleThreadsQueue(Thread * th)
{
    if (th == 0)
    {
        Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                      "ThreadPool::_addToIdleThreadsQueue: Thread pointer is null.");
        throw NullPointer();
    }

    try
    {
        _idleThreads.insert_front(th);
    }
    catch(...)
    {
        Tracer::trace(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                      "ThreadPool::_addToIdleThreadsQueue: _idleThreads.insert_front "
                      "failed.");
    }
}

PEGASUS_NAMESPACE_END