/*
 * Copyright (c) 2014, Siemens AG. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <mutex_test.h>
#include <embb/base/mutex.h>
#include <embb/base/thread.h>
#include <iostream>

namespace embb {
namespace base {
namespace test {

MutexTest::MutexTest() : mutex_(), counter_(0),
    number_threads_(partest::TestSuite::GetDefaultNumThreads()),
    number_iterations_(partest::TestSuite::GetDefaultNumIterations()) {
  CreateUnit("Mutex protected counter")
      .Pre(&MutexTest::PreMutexCount, this)
      .Add(&MutexTest::TestMutexCount, this, number_threads_,
          number_iterations_)
      .Post(&MutexTest::PostMutexCount, this);
  CreateUnit("Recursive mutex")
      .Add(&MutexTest::TestRecursiveMutex, this);
  CreateUnit("Lock guard protected counter")
      .Pre(&MutexTest::PreLockGuardCount, this)
      .Add(&MutexTest::TestLockGuardCount, this, number_threads_,
          number_iterations_)
      .Post(&MutexTest::PostLockGuardCount, this);
  CreateUnit("Unique Lock").Add(&MutexTest::TestUniqueLock, this);
}

void MutexTest::PreMutexCount() {
  counter_ = 0;
}

void MutexTest::TestMutexCount() {
  mutex_.Lock();
  ++counter_;
  mutex_.Unlock();
}

void MutexTest::PostMutexCount() {
  PT_EXPECT_EQ((size_t)counter_, number_iterations_ * number_threads_);
}

void MutexTest::TestRecursiveMutex() {
  embb::base::RecursiveMutex mutex;
  int number = 5;
  for (int i = 0; i < number; i++) {
    mutex.Lock();
    bool obtained = mutex.TryLock();
    PT_EXPECT_EQ(obtained, true);
  }
  for (int i = 0; i < number; i++) {
    mutex.Unlock();
    mutex.Unlock();
  }
}

void MutexTest::PreLockGuardCount() {
  counter_ = 0;
}

void MutexTest::TestLockGuardCount() {
  LockGuard<Mutex> guard(mutex_);
  ++counter_;
}

void MutexTest::PostLockGuardCount() {
  PT_EXPECT_EQ((size_t)counter_, number_iterations_ * number_threads_);
}

void MutexTest::TestUniqueLock() {
#ifdef EMBB_USE_EXCEPTIONS
  bool exception_thrown = false;
#endif
  { // Test standard usage and releasing
    UniqueLock<Mutex> lock(mutex_);
    PT_EXPECT_EQ(lock.OwnsLock(), true);

#ifdef EMBB_USE_EXCEPTIONS
    // Locked lock should not by re-lockable
    EMBB_TRY {
      lock.Lock();
    }
    EMBB_CATCH(ErrorException&) {
      exception_thrown = true;
    }
    PT_EXPECT_EQ(exception_thrown, true);

    // Locked lock should not by re-try-lockable
    EMBB_TRY {
      lock.TryLock();
    }
    EMBB_CATCH(ErrorException&) {
      exception_thrown = true;
    }
    PT_EXPECT_EQ(exception_thrown, true);
#endif // EMBB_USE_EXCEPTIONS

    lock.Unlock();
    PT_EXPECT_EQ(lock.OwnsLock(), false);

#ifdef EMBB_USE_EXCEPTIONS
    // Unlocked lock should not by re-unlockable
    exception_thrown = false;
    EMBB_TRY {
      lock.Unlock();
    }
    EMBB_CATCH(ErrorException&) {
      exception_thrown = true;
    }
    PT_EXPECT_EQ(exception_thrown, true);
#endif // EMBB_USE_EXCEPTIONS

    lock.Lock();
    PT_EXPECT_EQ(lock.OwnsLock(), true);

    lock.Release()->Unlock();
    PT_EXPECT_EQ(lock.OwnsLock(), false);

#ifdef EMBB_USE_EXCEPTIONS
    // Released lock should not by lockable
    exception_thrown = false;
    EMBB_TRY {
      lock.Lock();
    }
    EMBB_CATCH(ErrorException&) {
      exception_thrown = true;
    }
    PT_EXPECT_EQ(exception_thrown, true);

    // Released lock should not by try-lockable
    exception_thrown = false;
    EMBB_TRY {
      lock.TryLock();
    }
    EMBB_CATCH(ErrorException&) {
      exception_thrown = true;
    }
    PT_EXPECT_EQ(exception_thrown, true);

    // Released lock should not by unlockable
    exception_thrown = false;
    EMBB_TRY {
      lock.Unlock();
    }
    EMBB_CATCH(ErrorException&) {
      exception_thrown = true;
    }
    PT_EXPECT_EQ(exception_thrown, true);
#endif // EMBB_USE_EXCEPTIONS
  }

  { // Test deferred lock construction
    UniqueLock<> lock(mutex_, embb::base::defer_lock);
    PT_EXPECT_EQ(lock.OwnsLock(), false);
  }

  { // Test try-lock construction
    UniqueLock<> lock(mutex_, embb::base::try_lock);
    PT_EXPECT_EQ(lock.OwnsLock(), true);
  }

  { // Test adopt lock construction
    mutex_.Lock();
    UniqueLock<> lock(mutex_, embb::base::adopt_lock);
    PT_EXPECT_EQ(lock.OwnsLock(), true);
  }

  { // Test lock swapping
    UniqueLock<> lock1;
    UniqueLock<> lock2(mutex_);
    PT_EXPECT_EQ(lock1.OwnsLock(), false);
    PT_EXPECT_EQ(lock2.OwnsLock(), true);
    lock1.Swap(lock2);
    PT_EXPECT_EQ(lock1.OwnsLock(), true);
    PT_EXPECT_EQ(lock2.OwnsLock(), false);
  }
}

} // namespace test
} // namespace base
} // namespace embb