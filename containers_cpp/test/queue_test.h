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

#ifndef CONTAINERS_CPP_TEST_QUEUE_TEST_H_
#define CONTAINERS_CPP_TEST_QUEUE_TEST_H_

#include <vector>
#include <utility>
#include <partest/partest.h>
#include <embb/base/duration.h>

namespace embb {
namespace containers {
namespace test {
template<typename Queue_t,
  bool MultipleProducers = false,
  bool MultipleConsumers = false>
class QueueTest : public partest::TestCase {
 public:
  typedef ::std::pair<size_t, int> element_t;
 private:
  static const int QUEUE_SIZE = 1024;
  static const int TOTAL_PRODUCE_CONSUME_COUNT = 10000;

 private:
  class Consumer {
   private:
    Queue_t * q;
    int n_producers;
    ::std::vector<unsigned char> consumer_tally;
    ::std::vector<int> sequence_number;
   public:
    Consumer(Queue_t * const queue, int numProducers);
    void Run();
    const ::std::vector<unsigned char> & Tally() const {
      return consumer_tally;
    }
  };
  class Producer {
   private:
    Queue_t * q;
    size_t producer_id;
   public:
    Producer(Queue_t * const queue, size_t id) :
      q(queue), producer_id(id) { }
    void Run();
  };

 private:
  int n_threads;
  embb::base::Atomic<int> thread_selector_producer;
  embb::base::Atomic<int> produce_count;
  ::std::vector<element_t> consumed_elements;
  ::std::vector<element_t> produced_elements;
  ::std::vector<Consumer> consumers;
  ::std::vector<Producer> producers;

  // for multiple p/c
  int n_producers;
  int n_consumers;
  embb::base::Atomic<size_t> next_producer_id;
  embb::base::Atomic<size_t> next_consumer_id;
  int n_queue_elements_per_thread;
  int n_queue_elements;

  int consume_count;
  Queue_t* queue;

  void QueueTestOrderMPMC_Pre();
  void QueueTestOrderMPMC_Post();
  void QueueTestOrderMPMC_ProducerThreadMethod();
  void QueueTestOrderMPMC_ConsumerThreadMethod();
  void QueueTestSingleProducerSingleConsumer_Pre();
  void QueueTestSingleProducerSingleConsumer_Post();
  void QueueTestSingleProducerSingleConsumer_ThreadMethod();
  void QueueTestSingleThreadEnqueueDequeue_Pre();
  void QueueTestSingleThreadEnqueueDequeue_Post();
  void QueueTestSingleThreadEnqueueDequeue_ThreadMethod();
  
 public:
  QueueTest();
};
} // namespace test
} // namespace containers
} // namespace embb

#include "./queue_test-inl.h"

#endif  // CONTAINERS_CPP_TEST_QUEUE_TEST_H_
