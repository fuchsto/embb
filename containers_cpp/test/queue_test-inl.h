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

#ifndef CONTAINERS_CPP_TEST_QUEUE_TEST_INL_H_
#define CONTAINERS_CPP_TEST_QUEUE_TEST_INL_H_

#include <algorithm>
#include <vector>

namespace embb {
namespace containers {
namespace test {
template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::QueueTest() :
  n_threads(static_cast<int>(partest::TestSuite::GetDefaultNumThreads())),
  n_iterations(200),
  next_consumer_id(0), 
  next_producer_id(0) {
  CreateUnit("QueueTestSingleThreadEnqueueDequeue").
  Pre(&QueueTest::QueueTestSingleThreadEnqueueDequeue_Pre, this).
  Add(&QueueTest::QueueTestSingleThreadEnqueueDequeue_ThreadMethod, this).
  Post(&QueueTest::QueueTestSingleThreadEnqueueDequeue_Post, this);
  CreateUnit("QueueTestTwoThreadsSingleProducerSingleConsumer").
  Pre(&QueueTest::QueueTestSingleProducerSingleConsumer_Pre, this).
  Add(&QueueTest::QueueTestSingleProducerSingleConsumer_ThreadMethod,
    this,
    2,
    TOTAL_PRODUCE_CONSUME_COUNT).
  Post(&QueueTest::QueueTestSingleProducerSingleConsumer_Post, this);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127)
#endif
  if (MultipleProducers == true && MultipleConsumers == true) {
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    CreateUnit("QueueTestOrderMultipleProducerMultipleConsumer").
    Pre(&QueueTest::QueueTestOrderMPMC_Pre, this).
    Add(&QueueTest::QueueTestOrderMPMC_ConsumerThreadMethod,
      this,
      static_cast<size_t>(PRODUCER_CONSUMER_THREADS / 2),
      static_cast<size_t>(1)).
    Add(&QueueTest::QueueTestOrderMPMC_ProducerThreadMethod,
      this,
      static_cast<size_t>(PRODUCER_CONSUMER_THREADS / 2),
      static_cast<size_t>(1)).
    Post(&QueueTest::QueueTestOrderMPMC_Post, this);
  }
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::
QueueTestOrderMPMC_Pre() {
  queue = new Queue_t(static_cast<size_t>(QUEUE_SIZE));
  embb_internal_thread_index_reset();
  next_producer_id = 0;
  next_consumer_id = 0;
  consumers.clear();
  producers.clear();
  for (size_t i = 0; i < PRODUCER_CONSUMER_THREADS / 2; ++i) {
    consumers.push_back(Consumer(queue));
    producers.push_back(Producer(queue, i));
  }
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::
QueueTestOrderMPMC_Post() {
  delete queue;
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::
QueueTestOrderMPMC_ProducerThreadMethod() {
  size_t p_id = next_producer_id.Load(); 
  next_producer_id.FetchAndAdd(1);
  producers[p_id].Run();
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::
QueueTestOrderMPMC_ConsumerThreadMethod() {
  size_t c_id = next_consumer_id.Load();
  next_consumer_id.FetchAndAdd(1);
  consumers[c_id].Run();
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::Producer::
Run() {
  for (int i = 0; i < QUEUE_SIZE;) {    
    if (!q->TryEnqueue(element_t(producer_id, i))) {
      continue;
    }
    else {
      ++i;
    }
  }
  // Enqueue -1 as terminator element of this procuder: 
  q->TryEnqueue(element_t(producer_id, -1));
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::Consumer::
Consumer(Queue_t * const queue) :
  q(queue) {
  for (int i = 0; i < PRODUCER_CONSUMER_THREADS; ++i) {
    sequenceNumber[i] = -1;
  }
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::Consumer::
Run() {
  element_t element;
  size_t producerId;
  bool forever = true;
  while (forever) {
    if (!q->TryDequeue(element)) {
      continue;
    }
    if (element.second < 0) {
      break;
    }
    producerId = element.first;
    // Assert on dequeued element:
    PT_ASSERT_LT_MSG(producerId, PRODUCER_CONSUMER_THREADS,
      "Invalid producer id in dequeue");    
    PT_ASSERT_LT_MSG(sequenceNumber[producerId], element.second,
      "Invalid element sequence");    
    // Store last value received from the element's producer:
    sequenceNumber[producerId] = element.second;
    const size_t pos((producerId * QUEUE_SIZE) +
      static_cast<size_t>(element.second));
    PT_ASSERT_MSG(!consumer_tally.test(pos),
      "Element dequeued twice");
    // Set bit at dequeued element's position:
    consumer_tally.set(static_cast<unsigned int>(pos));
  }
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::
QueueTestSingleProducerSingleConsumer_Pre() {
  embb_internal_thread_index_reset();
  queue = new Queue_t(QUEUE_SIZE);
  thread_selector_producer = -1;
  produce_count = 0;
  consume_count = 0;
  consumed_elements.clear();
  produced_elements.clear();
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::
QueueTestSingleProducerSingleConsumer_Post() {
  embb_atomic_memory_barrier();
  ::std::sort(consumed_elements.begin(), consumed_elements.end());
  ::std::sort(produced_elements.begin(), produced_elements.end());
  PT_ASSERT(consumed_elements.size() == produced_elements.size());
  for (unsigned int i = 0;
    i != static_cast<unsigned int>(consumed_elements.size()); i++) {
    PT_ASSERT(consumed_elements[i] == produced_elements[i]);
  }
  delete queue;
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::
QueueTestSingleProducerSingleConsumer_ThreadMethod() {
  unsigned int thread_index;
  int return_val = embb_internal_thread_index(&thread_index);
  PT_ASSERT(return_val == EMBB_SUCCESS);
  if (thread_selector_producer == -1) {
    int expected = -1;
    thread_selector_producer.CompareAndSwap(expected,
      static_cast<int>(thread_index));
    while (thread_selector_producer == -1) {}
  }
  if (static_cast<unsigned int>(thread_selector_producer.Load()) ==
    thread_index) {
    // we are the producer
    while (produce_count >= QUEUE_SIZE) { }

    element_t random_var(0, rand() % 10000);
    bool success = queue->TryEnqueue(random_var);
    PT_ASSERT(success == true);
    produce_count++;
    produced_elements.push_back(random_var);
  } else {
    // we are the consumer
    while (consume_count < TOTAL_PRODUCE_CONSUME_COUNT) {
      consume_count++;
      while (produce_count == 0) {}

      element_t consumed;
      bool success = queue->TryDequeue(consumed);
      PT_ASSERT(success == true);
      produce_count--;
      consumed_elements.push_back(consumed);
    }
  }
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::
QueueTestSingleThreadEnqueueDequeue_ThreadMethod() {
  for (int i = 0; i != QUEUE_SIZE; ++i) {
    bool success = queue->TryEnqueue(element_t(0, i * 133));
    PT_ASSERT(success == true);
  }
  for (int i = 0; i != QUEUE_SIZE; ++i) {
    element_t dequ(0, -1);
    bool success = queue->TryDequeue(dequ);
    PT_ASSERT(success == true);
    PT_ASSERT(dequ.second == i * 133);
  }
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::
QueueTestSingleThreadEnqueueDequeue_Pre() {
  queue = new Queue_t(QUEUE_SIZE);
}

template<typename Queue_t, bool MultipleProducers, bool MultipleConsumers>
void QueueTest<Queue_t, MultipleProducers, MultipleConsumers>::
QueueTestSingleThreadEnqueueDequeue_Post() {
  delete queue;
}
} // namespace test
} // namespace containers
} // namespace embb

#endif  // CONTAINERS_CPP_TEST_QUEUE_TEST_INL_H_
