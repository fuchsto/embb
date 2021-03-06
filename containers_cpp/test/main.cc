/*
 * Copyright (c) 2014-2015, Siemens AG. All rights reserved.
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

#include <embb/containers/lock_free_tree_value_pool.h>
#include <embb/containers/wait_free_array_value_pool.h>
#include <embb/containers/wait_free_spsc_queue.h>
#include <embb/containers/object_pool.h>
#include <embb/containers/lock_free_stack.h>
#include <embb/containers/lock_free_mpmc_queue.h>
#include <embb/containers/lock_free_chromatic_tree.h>
#include <embb/base/c/memory_allocation.h>

#include <partest/partest.h>
#include <embb/base/thread.h>

#include "./pool_test.h"
#include "./queue_test.h"
#include "./stack_test.h"
#include "./hazard_pointer_test.h"
#include "./object_pool_test.h"
#include "./tree_test.h"

#define COMMA ,

using embb::containers::WaitFreeArrayValuePool;
using embb::containers::LockFreeTreeValuePool;
using embb::containers::WaitFreeSPSCQueue;
using embb::containers::LockFreeMPMCQueue;
using embb::containers::LockFreeStack;
using embb::containers::LockFreeTreeValuePool;
using embb::containers::WaitFreeArrayValuePool;
using embb::containers::ChromaticTree;
using embb::containers::test::PoolTest;
using embb::containers::test::HazardPointerTest;
using embb::containers::test::QueueTest;
using embb::containers::test::StackTest;
using embb::containers::test::ObjectPoolTest;
using embb::containers::test::TreeTest;

PT_MAIN("Data Structures C++") {
  unsigned int max_threads = static_cast<unsigned int>(
    2 * partest::TestSuite::GetDefaultNumThreads());
  embb_thread_set_max_count(max_threads);

  PT_RUN(PoolTest< WaitFreeArrayValuePool<int COMMA -1> >);
  PT_RUN(PoolTest< LockFreeTreeValuePool<int COMMA -1> >);
  PT_RUN(HazardPointerTest);
  PT_RUN(QueueTest< WaitFreeSPSCQueue< ::std::pair<size_t COMMA int> > >);
  PT_RUN(QueueTest< LockFreeMPMCQueue< ::std::pair<size_t COMMA int> >
    COMMA true COMMA true >);
  PT_RUN(StackTest< LockFreeStack<int> >);
  PT_RUN(ObjectPoolTest< LockFreeTreeValuePool<bool COMMA false > >);
  PT_RUN(ObjectPoolTest< WaitFreeArrayValuePool<bool COMMA false> >);
  PT_RUN(TreeTest< ChromaticTree<size_t COMMA int> >);

  PT_EXPECT(embb_get_bytes_allocated() == 0);
}
