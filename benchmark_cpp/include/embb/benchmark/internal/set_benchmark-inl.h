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

#ifndef EMBB_BENCHMARK_CPP_INTERNAL_SETS_SET_BENCHMARK_INL_H_
#define EMBB_BENCHMARK_CPP_INTERNAL_SETS_SET_BENCHMARK_INL_H_

#include <embb/benchmark/benchmark.h>
#include <embb/benchmark/scenario.h>
#include <embb/benchmark/sets/set_benchmark.h>
#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/internal/util.h>

#include <embb/base/perf/timer.h>
#include <embb/base/thread.h>

#include <vector>
#include <iostream>
#include <stdexcept>

namespace embb {
namespace benchmark {

using embb::base::perf::Timer;
using embb::benchmark::internal::Console;

template<typename TSet>
SetBenchmark<TSet>::
SetBenchmark(TSet * unitInstance, const CallArgs & callArgs) 
: unit(unitInstance), args(callArgs), measurements(callArgs) {
}

template<typename TSet>
SetBenchmark<TSet>::
~SetBenchmark()
{
}

template< typename TSet >
void SetBenchmark<TSet>::SetProducerConsumerThread::
Task()
{
  for (unsigned int i = 0; i < this->NumIterations(); ++i) {
    // Current iteration will always be completed, but 
    // checks 'stopped' flag before starting a new iteration. 
    if (this->IsStopped()) {
      break;
    }
    // Producer loop: 
    for (unsigned int n_a = 0; n_a < this->NumProduceElements(); ++n_a) {
      unsigned int token = (i * 20) + (this->Id() + 1) * 10 + n_a;
      SetLatencyMeasurements::node_index_t key = static_cast<int>(token);

      Timer addTime; 
      bool res = this->Unit().TryAdd(key);        
      Timer::timestamp_t addEnd = Timer::Now(); 
        
      if (res) {
        this->Measurements().MeasureAdd(this->Id(), addTime.Start(), addEnd);
          
        // Measure successfull call of Contains: 
        Timer containsTime;
        res = this->Unit().Contains(key);
        Timer::timestamp_t containsEnd = Timer::Now();

        if (res) {
          this->Measurements().MeasureContains(this->Id(), containsTime.Start(), containsEnd);
        }
      }
    }
    // Consumer loop: 
    for (unsigned int n_d = 0; n_d < this->NumConsumeElements(); ++n_d) {
      // Remove all keys added in the last producer loop: 
      unsigned int token = (i * 20) + (this->Id() + 1) * 10 + n_d;
      SetLatencyMeasurements::node_index_t key = static_cast<int>(token);

      Timer removeTime;        
      bool res = this->Unit().TryRemove(key);
      Timer::timestamp_t removeEnd = Timer::Now();

      if (res) {
        this->Measurements().MeasureRemove(this->Id(), removeTime.Start(), removeEnd);          
        // Measure failing call of Contains: 
        Timer containsTime;
        res = this->Unit().Contains(key);
        Timer::timestamp_t containsEnd = Timer::Now();
          
        if (!res) {
          this->Measurements().MeasureContains(this->Id(), containsTime.Start(), containsEnd);
        }
      }
    }
  } // for iterations
}

template<typename TSet>
void SetBenchmark<TSet>::
Run() {
  switch (args.ScenarioId()) {
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
    case Scenario::NUM_SCENARIOS: break; 
    default: break; 
  }
}

template<typename TSet>
void SetBenchmark<TSet>::
RunScenario_0_EnqueueDequeuePairs() {  
  // Prepare thread instances: 
  ::std::vector<SetProducerConsumerThread *> producerConsumers;
  for (unsigned int a_id = 0; a_id < args.NumThreads(); ++a_id) {
    producerConsumers.push_back(
      new SetProducerConsumerThread(unit, &measurements, a_id, a_id, args));
  }

  Console::WriteStep("Starting threads");

  // Start allocator threads first to reduce startup latency: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<SetProducerConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<SetProducerConsumerThread *>::const_iterator const c_end =
    producerConsumers.end();
  for (c_it = producerConsumers.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }
  // Tell allocators to stop and collect results: 
  for (c_it = producerConsumers.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
}

template<typename TSet>
void SetBenchmark<TSet>::
RunScenario_1_EnqueueDequeueBulk() {
  // Allocate r * numElements elements;  
  unsigned int nPreallocElements = static_cast<unsigned int>(
                                     static_cast<double>(args.NumElements()) * 
                                     (static_cast<double>(args.RPrealloc()) / 100.0f));

  Console::WriteValue("Preallocating", nPreallocElements, " elements");

  unsigned int bulkAllocatorId = args.NumThreads();

  SetProducerConsumerThread * bulkAllocator =
    new SetProducerConsumerThread(unit, &measurements, bulkAllocatorId, args);
  bulkAllocator->NumConsumeElements(nPreallocElements);
  bulkAllocator->NumProduceElements(nPreallocElements);
  bulkAllocator->Run();

  Console::WriteStep("Starting threads");
  
  // Threads now each allocate and deallocate one element 
  // in iterations: 
  ::std::vector<SetProducerConsumerThread *> reallocators;
  // Number of iterations per reallocator:
  for (unsigned int a_id = 0; a_id < args.NumThreads(); ++a_id) {
    reallocators.push_back(
      new SetProducerConsumerThread(unit, &measurements, a_id, a_id, args));
  }

  // Start reallocator threads: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<SetProducerConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<SetProducerConsumerThread *>::const_iterator const c_end =
    reallocators.end();
  for (c_it = reallocators.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }
  // Tell reallocators to stop and collect results: 
  for (c_it = reallocators.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
  // Fast-spinnign reallocator threads have completed, 
  // request bulk allocator thread to join after current 
  // iteration: 
  bulkAllocator->RequestStop(); 
  bulkAllocator->Join(); 
}

template<typename TSet>
void SetBenchmark<TSet>::
RunScenario_2_FillUp() {
  // Allocator threads: 
  ::std::vector<SetProducerConsumerThread *> allocators;
  unsigned int numAllocs = args.NumElements() / args.NumThreads(); 
  for (unsigned int a_id = 1; a_id < args.NumThreads(); ++a_id) {
    SetProducerConsumerThread * pct =
      new SetProducerConsumerThread(unit, &measurements, a_id, a_id, args);
    pct->NumIterations(1);
    pct->NumProduceElements(0);
    pct->NumConsumeElements(numAllocs);

    allocators.push_back(pct); 
  }

  Console::WriteStep("Starting threads");

  // Start allocator threads: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<SetProducerConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<SetProducerConsumerThread *>::const_iterator const c_end =
    allocators.end();
  for (c_it = allocators.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }
  // Tell allocators to stop and collect results: 
  for (c_it = allocators.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
}

} // namespace benchmark
} // namespace embb

#endif // EMBB_BENCHMARK_CPP_INTERNAL_SETS_SET_BENCHMARK_INL_H_
