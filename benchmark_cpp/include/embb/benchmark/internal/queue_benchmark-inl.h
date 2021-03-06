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

#ifndef EMBB_BENCHMARK_CPP_INTERNAL_QUEUES_QUEUE_BENCHMARK_INL_H_
#define EMBB_BENCHMARK_CPP_INTERNAL_QUEUES_QUEUE_BENCHMARK_INL_H_

#include <embb/benchmark/internal/util.h>
#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/benchmark.h>
#include <embb/benchmark/scenario.h>
#include <embb/benchmark/queues/queue_benchmark.h>

#include <embb/base/thread.h>

#include <vector>

namespace embb {
namespace benchmark {

using embb::base::perf::Timer;
using embb::benchmark::internal::Console;

template< typename TQueue > 
QueueBenchmark<TQueue>::
QueueBenchmark(TQueue * queue, const CallArgs & params) 
: q(queue),  
  callArgs(params), 
  measurements(params) {
}

template< typename TQueue >
QueueBenchmark<TQueue>::
~QueueBenchmark() {
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    ::std::vector<ProducerThread *>::iterator p_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    ::std::vector<ProducerThread *>::const_iterator const p_end = producers.end();
  for (p_it = producers.begin(); p_it != p_end; ++p_it) {
    delete(*p_it);
  }
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    ::std::vector<ConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    ::std::vector<ConsumerThread *>::const_iterator const c_end = consumers.end();
  for (c_it = consumers.begin(); c_it != c_end; ++c_it) {
    delete(*c_it);
  }
}

template< typename TQueue >
void QueueBenchmark<TQueue>::
Run() {  
  switch (callArgs.ScenarioId()) {
    case Scenario::SCENARIO__ENQUEUE_DEQUEUE_PAIRS:
      Console::WriteStep("Scenario: Enqueue/Dequeue Pairs");
      RunScenario_0_EnqueueDequeuePairs(); 
      break;
    case Scenario::SCENARIO__ENQUEUE_DEQUEUE_BULK:
      Console::WriteStep("Scenario: Enqueue/Dequeue Bulk");
      RunScenario_1_EnqueueDequeueBulk(); 
      break;
    case Scenario::SCENARIO__FILL_UP: 
      Console::WriteStep("Scenario: Fill Up");
      RunScenario_2_FillUp(); 
      break;
    case Scenario::SCENARIO__RACE: 
      Console::WriteStep("Scenario: Writer-/Reader-intensive Race");
      RunScenario_3_Race(); 
      break;
    case Scenario::SCENARIO__CAPACITY_BUFFER: 
      Console::WriteStep("Scenario: Capacity Buffer");
      RunScenario_4_CapacityBuffer(); 
      break;
    case Scenario::NUM_SCENARIOS: break;
    case Scenario::UNDEFINED: break;
    default: break; 
  }
}

template< typename TQueue >
void QueueBenchmark<TQueue>::
RunScenario_0_EnqueueDequeuePairs() 
{
  size_t n_prealloc = 0;
  if (callArgs.RPrealloc() > 0) { 
    size_t n_capacity = callArgs.NumElements();
    n_prealloc = (n_capacity * callArgs.RPrealloc()) / 100;
  }
  else if (callArgs.NPrealloc() > 0) {
    n_prealloc = callArgs.NPrealloc(); 
  }
  
  Console::WriteValue("Pre-enqueue", n_prealloc);

  for (size_t i = 0; i < n_prealloc; ++i) {
    element_t element;
    element.producer_id = 100; 
    element.timestamp   = embb::base::perf::Timer::Now();
    element.value       = 0; 
    q->TryEnqueue(element); 
  }

  ::std::vector< QueueProducerConsumerThread *> producerConsumers;
  for (unsigned int pc_id = 0; pc_id < callArgs.NumThreads(); ++pc_id) {
    QueueProducerConsumerThread * pct =
      new QueueProducerConsumerThread(q, &measurements, pc_id, pc_id, callArgs);
    pct->NumProduceElements(callArgs.NumAllocsPerIt());
    pct->NumConsumeElements(callArgs.NumAllocsPerIt());

    producerConsumers.push_back(pct);
  }

  Console::WriteStep("Starting threads");

  // Start producer / consumer threads: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    ::std::vector<QueueProducerConsumerThread *>::iterator p_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    ::std::vector<QueueProducerConsumerThread *>::const_iterator const p_end =
    producerConsumers.end();
  for (p_it = producerConsumers.begin(); p_it != p_end; ++p_it) {
    (*p_it)->Run();
  }
  // Wait for producer / constumer threads: 
  for (p_it = producerConsumers.begin(); p_it != p_end; ++p_it) {
    (*p_it)->Join();
  }
}

template< typename TQueue >
void QueueBenchmark<TQueue>::
RunScenario_1_EnqueueDequeueBulk() 
{
  // Threads now each allocate and deallocate one element 
  // in iterations: 
  ::std::vector<QueueProducerConsumerThread *> threads;
  // Number of iterations per thread:
  for (unsigned int a_id = 0; a_id < callArgs.NumThreads(); ++a_id) {
    QueueProducerConsumerThread * t = 
      new QueueProducerConsumerThread(q, &measurements, a_id, a_id, callArgs);
    threads.push_back(t);    
  }
  Console::WriteStep("Starting threads");
  // Start reallocator threads: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<QueueProducerConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<QueueProducerConsumerThread *>::const_iterator const c_end = 
    threads.end();
  for (c_it = threads.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }
  // Tell reallocators to stop and collect results: 
  for (c_it = threads.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
}

template< typename TQueue >
void QueueBenchmark<TQueue>::
RunScenario_2_FillUp() 
{
  // Enqueuer threads: 
  ::std::vector<QueueProducerConsumerThread *> threads;
  for (unsigned int a_id = 0; a_id < callArgs.NumThreads(); ++a_id) {
    QueueProducerConsumerThread * pct =
      new QueueProducerConsumerThread(q, &measurements, a_id, a_id, callArgs);
    pct->NumIterations(1);
    pct->NumProduceElements(0);
    pct->NumConsumeElements(callArgs.NumElements());
    pct->QuitOnFailedConsume(true); 
    threads.push_back(pct); 
  }
  Console::WriteStep("Starting threads");
  // Start allocator threads: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<QueueProducerConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<QueueProducerConsumerThread *>::const_iterator const c_end = 
    threads.end();
  for (c_it = threads.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }
  // Tell allocators to stop and collect results: 
  for (c_it = threads.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
}

template< typename TQueue >
void QueueBenchmark<TQueue>::
RunScenario_3_Race() 
{
  // Need to pre-allocate (enqueue) elements for 
  // dequeuer threads: 
  // Threads from 2  producers vs 32 consumers 
  //           to 32 producers vs  2 consumers
  // -> Max. preallocated elements for 32 = (p + c - 2) producers
  size_t n_prealloc = (callArgs.NumIterations() * callArgs.NumAllocsPerIt()) *
                            (callArgs.NumThreads() - 1);
  Console::WriteValue("Pre-enqueue", n_prealloc);

  for (size_t i = 0; i < n_prealloc; ++i) {
    element_t element;
    element.producer_id = 100; 
    element.timestamp   = embb::base::perf::Timer::Now();
    element.value       = 0;
    q->TryEnqueue(element); 
  }
  // Enqueuer threads: 
  ::std::vector<QueueProducerConsumerThread *> threads;
  for (unsigned int a_id = 0; a_id < callArgs.NumConsumers(); ++a_id) {
    QueueProducerConsumerThread * pct =
      new QueueProducerConsumerThread(
      q, 
      &measurements, 
      a_id, 
      a_id, 
      callArgs);
    pct->NumProduceElements(0);
    pct->NumConsumeElements(callArgs.NumAllocsPerIt());
    threads.push_back(pct); 
  }  
  // Dequeuer threads:
  for (unsigned int a_id = 0; a_id < callArgs.NumProducers(); ++a_id) {
    QueueProducerConsumerThread * pct =
      new QueueProducerConsumerThread(
      q, 
      &measurements, 
      a_id, 
      a_id + callArgs.NumConsumers(), 
      callArgs);
    pct->NumProduceElements(callArgs.NumAllocsPerIt());
    pct->NumConsumeElements(0);
    threads.push_back(pct); 
  }
  Console::WriteStep("Starting threads");
  // Start threads: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<QueueProducerConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<QueueProducerConsumerThread *>::const_iterator const c_end = 
    threads.end();
  for (c_it = threads.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }
  // Tell threads to stop and collect results: 
  for (c_it = threads.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
}

template< typename TQueue >
void QueueBenchmark<TQueue>::
RunScenario_4_CapacityBuffer() 
{
  // Allocate r * numElements elements;  
  size_t nPreallocElements = static_cast<size_t>(
                                     static_cast<double>(callArgs.NumElements()) * 
                                     (static_cast<double>(callArgs.RPrealloc()) / 100.0f));

  Console::WriteValue("Enqueueing", nPreallocElements, " elements");
  
  for (size_t i = 0; i < nPreallocElements; ++i) {
    element_t element;
    element.producer_id = 100; 
    element.timestamp   = embb::base::perf::Timer::Now();
    element.value       = 0;
    if (!q->TryEnqueue(element)) {
      throw ::std::runtime_error("Failed to enqueue element");
    }
  }
  for (unsigned int producer_id = 0; 
       producer_id < callArgs.NumProducers(); 
       ++producer_id) {
    producers.push_back(
      new ProducerThread(
        q, 
        &measurements, 
        producer_id, 
        producer_id, 
        callArgs,
        callArgs.NumAllocsPerIt(), 
        // sleep after each iteration:
        callArgs.QParam()));
  }
  for (unsigned int consumer_id = 0; 
       consumer_id < callArgs.NumConsumers();
       ++consumer_id) {
    consumers.push_back(
      new ConsumerThread(
        q, 
        &measurements,
        consumer_id,
        consumer_id + callArgs.NumProducers(),
        callArgs, 
        0));
  }
  Console::WriteStep("Starting threads");
  // Start consumer threads first to reduce startup latency: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    ::std::vector<ConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    ::std::vector<ConsumerThread *>::const_iterator const c_end = consumers.end();
  for (c_it = consumers.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }

  EMBB_BASE_CPP_BENCHMARK__SLEEP_MS(1000);

  // Start producer threads: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    ::std::vector<ProducerThread *>::iterator p_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    ::std::vector<ProducerThread *>::const_iterator const p_end = producers.end();
  for (p_it = producers.begin(); p_it != p_end; ++p_it) {
    (*p_it)->Run();
  }
  // Wait for producer threads: 
  for (p_it = producers.begin(); p_it != p_end; ++p_it) {
    (*p_it)->Join();
  }
  // Tell consumers to stop: 
  for (c_it = consumers.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
}

template< typename TQueue >
void QueueBenchmark<TQueue>::QueueProducerConsumerThread::
Task() 
{
  for (size_t i = 0; i < this->NumIterations(); ++i) {
    if (this->IsStopped()) {
      break; 
    }
    // Enqueue loop
    Timer iterationTime; 
    for (size_t n_e = 0; n_e < this->NumProduceElements(); ++n_e) {
      element_t element;
      element.producer_id = this->Id();
      element.timestamp   = embb::base::perf::Timer::Now();
      element.value       = static_cast<unsigned int>((n_e + 1)) * this->Id();
      // Measure time for operation 'Add': 
      Timer addTime; 
      bool res = this->Unit().TryEnqueue(element);
      Timer::timestamp_t enqueueEnd = Timer::Now(); 
      
      if (!res) { 
        if (this->QuitOnFailedProduce()) {
          return; 
        }
        throw ::std::runtime_error("Dequeue failed");
      }
      else {
        // Do not register measurement if the operation 
        // failed, as failed calls are incomparable
        this->Measurements().MeasureAdd(this->Id(), addTime.Start(), enqueueEnd);
      }
    }
    element_t deqElement;
    // Deqeue loop
    for (size_t n_d = 0; n_d < this->NumConsumeElements(); ++n_d) {
      // Measure time for operation 'RemoveAny': 
      Timer removeAnyTime;
      bool res = this->Unit().TryDequeue(deqElement);
      Timer::timestamp_t deqEnd = Timer::Now(); 
      
      if (!res) { 
        if (this->QuitOnFailedConsume()) {
          return; 
        }
        throw ::std::runtime_error("Dequeue failed");
      }
      else {
        // Only measure successfull dequeues
        this->Measurements().MeasureRemoveAny(this->Id(), removeAnyTime.Start(), deqEnd);
      }
    } 
    // Measure iteration time: 
    this->Measurements().MeasureBufferLatency(this->Id(), iterationTime.Start(), Timer::Now());
  } // iterations
}

template< typename TQueue >
void QueueBenchmark<TQueue>::ProducerThread::
Task() 
{
  for (size_t i = 0; i < this->callArgs.NumIterations(); ++i) {
    if (this->IsStopped()) {
      break;
    }
    for (size_t e = 0; e < this->NumProduceElements(); ++e) {
      element_t element;
      element.producer_id = this->Id();
      element.timestamp   = embb::base::perf::Timer::Now();
      element.value       = static_cast<unsigned int>(e);
      // Measure time for operation 'Add': 
      Timer addTime; 
      bool res = this->Unit().TryEnqueue(element);
      Timer::timestamp_t enqueueEnd = Timer::Now(); 

      if (!res) { 
        // Requested to stop and queue at capacity, quit
        if (this->IsStopped() || this->QuitOnFailedProduce()) {
          return; 
        }
        throw ::std::runtime_error("Enqueue failed");
      }      
      this->Measurements().MeasureAdd(
        this->Id(), addTime.Start(), enqueueEnd);
    }
    if (this->n_sleep_ms > 0) {
      EMBB_BASE_CPP_BENCHMARK__SLEEP_MS(this->n_sleep_ms);
    }
  }
}

template< typename TQueue >
void QueueBenchmark<TQueue>::ConsumerThread::
Task() 
{
  element_t element;
  for (size_t i = 0; i < this->callArgs.NumIterations(); ++i) {
    // To avoid compiler warning
    bool forever = this->NumConsumeElements() == 0;
    // Actually depending on n_dequeues == 0
    while (forever) {
      // Measure time for operation 'RemoveAny': 
      Timer removeAnyTime;
      bool res = this->Unit().TryDequeue(element);
      Timer::timestamp_t deqEnd = Timer::Now(); 

      // Ignore elements from pre-allocation (producer_id = 100)
      if (res && element.producer_id < 100) { 
        // Only measure successfull dequeues
        this->Measurements().MeasureRemoveAny(
          this->Id(), removeAnyTime.Start(), deqEnd);
        // Measure element throughput time: 
        this->Measurements().MeasureBufferLatency(
          this->Id(), element.timestamp, deqEnd);
      }
      else {
        // Requested to stop and empty queue, quit
        if (this->IsStopped()) { break; }
        // Empty queue but not requested to stop, retry
        continue;
      }
    }
  }
}

} // namespace benchmark
} // namespace embb

#endif // EMBB_BENCHMARK_CPP_INTERNAL_QUEUES_QUEUE_BENCHMARK_INL_H_
