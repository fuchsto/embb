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

#ifndef EMBB_BENCHMARK_CPP_INTERNAL_POOLS_POOL_BENCHMARK_INL_H_
#define EMBB_BENCHMARK_CPP_INTERNAL_POOLS_POOL_BENCHMARK_INL_H_

#include <embb/benchmark/benchmark.h>
#include <embb/benchmark/scenario.h>
#include <embb/benchmark/pools/pool_benchmark.h>
#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/internal/util.h>
#include <embb/base/thread.h>

#include <embb/base/c/internal/thread_index.h>

#include <vector>
#include <utility>
#include <iostream>
#include <stdexcept>

namespace embb {
namespace benchmark {

using embb::base::perf::Timer;
using embb::benchmark::internal::Console;

template<typename TPool>
PoolBenchmark<TPool>::
PoolBenchmark(TPool * pool, const CallArgs & callArgs, size_t numNodes)
: p(pool), args(callArgs), n_nodes(numNodes), measurements(callArgs) {
  if (n_nodes == 0) { 
    n_nodes = args.NumElements(); 
  }
}

template<typename TPool>
PoolBenchmark<TPool>::
~PoolBenchmark() {
}

template< typename TPool >
void PoolBenchmark<TPool>::BagProducerConsumerThread::
PreallocateElements(size_t n_prealloc, bool forDeallocation)
{
  for (size_t r_a = 0; r_a < n_prealloc; ++r_a) {
    element_t node_index;
    int index = this->Unit().Allocate(node_index);
    // If elements are preallocated for later deallocation, 
    // save preallocated node indices: 
    if (forDeallocation) {
      allocatedNodeIndices.push_back(node_index); 
      PoolLatencyMeasurements::node_t nodeAllocated;
      nodeAllocated.allocatorId = static_cast<int>(this->Id());
      nodeAllocated.timestamp   = Timer::Now();
      nodeAllocated.poolIndex   = index;
      nodePool->operator[](static_cast<size_t>(node_index)) = nodeAllocated;
    }
  }
}

template< typename TPool >
void PoolBenchmark<TPool>::BagProducerConsumerThread::
Task()
{
  // Current iteration will always be completed, but 
  // checks 'stopped' flag before starting a new iteration. 
  for (unsigned int i = 0; i < this->NumIterations(); ++i) {
    if (this->IsStopped()) {
      break;
    }
    for (unsigned int n_a = 0; n_a < this->NumConsumeElements(); ++n_a) {
      element_t node_index;

      // Allocate (reserve) a node index and measure duration of allocation: 
      Timer acquisitionTime; 
      int index = this->Unit().Allocate(node_index);
      Timer::timestamp_t acquisitionEnd = Timer::Now(); 

      if (index < 0) {
        if (this->QuitOnFailedConsume()) {
          return; 
        } 
        throw ::std::runtime_error("Allocation failed");
      }
      this->Measurements().MeasureRemoveAny(
        this->Id(), acquisitionTime.Start(), acquisitionEnd);
        
      // Store acquired index element for deallocation: 
      allocatedNodeIndices.push_back(node_index); 
      // Modify the node at the acquired node index in the pool: 
      PoolLatencyMeasurements::node_t nodeAllocated;
      nodeAllocated.allocatorId = static_cast<int>(this->Id());
      nodeAllocated.timestamp   = Timer::Now();
      nodeAllocated.poolIndex   = index;
      nodePool->operator[](static_cast<unsigned int>(node_index)) = nodeAllocated;
    }
    if (SleepAfterConsumeIteration() > 0) {
      EMBB_BASE_CPP_BENCHMARK__SLEEP_MS(SleepAfterConsumeIteration());
    }
    for (unsigned int n_d = 0; n_d < this->NumProduceElements(); ++n_d) {
      // Pop previously acquired node index from list of acquired elements: 
      element_t node_index = allocatedNodeIndices.back();
      // Load node at this node index: 
      PoolLatencyMeasurements::node_t nodeDeallocated = 
        nodePool->operator[](static_cast<unsigned int>(node_index));
      // Check for correctness:
      if (nodeDeallocated.allocatorId != static_cast<int>(this->Id())) {
        throw ::std::runtime_error("Validation failed");
      }
      // Deallocate the node's pool index and measure duration of deallocation: 
      Timer freeTime;
      this->Unit().Free(node_index, nodeDeallocated.poolIndex);
      Timer::timestamp_t freeEnd = Timer::Now(); 

      this->Measurements().MeasureAdd(
        this->Id(), freeTime.Start(), freeEnd);
        
      allocatedNodeIndices.pop_back();
    }
  }
}

template<typename TPool>
void PoolBenchmark<TPool>::
Run() {
  // Initialize node pool: 
  nodes.clear(); 
  for (unsigned int node_index = 0; node_index < n_nodes; ++node_index) {
    PoolLatencyMeasurements::node_t node;
    node.allocatorId = -1;
    node.poolIndex   = -1;
    node.nodeIndex   = static_cast<int>(node_index);
    nodes.push_back(node);
  }
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
    case Scenario::SCENARIO__RACE: 
      Console::WriteStep("Scenario: Writer-/Reader-intensive Race");
      RunScenario_3_Race(); 
      break;
    case Scenario::UNDEFINED: break;
    case Scenario::NUM_SCENARIOS: break; 
    default: break; 
  }
}

template<typename TPool>
void PoolBenchmark<TPool>::
RunScenario_0_EnqueueDequeuePairs() {
  size_t n_prealloc = 0; 
  size_t n_threads  = args.NumThreads(); 
  if (args.RPrealloc() > 0) { 
    size_t n_capacity = args.NumElements();
    n_prealloc = ((n_capacity * args.RPrealloc()) / 100) / n_threads;
  }
  else if (args.NPrealloc() > 0) {
    n_prealloc = args.NPrealloc() / n_threads; 
  }
  
  Console::WriteStep("Initializing threads");
  // Prepare thread instances: 
  ::std::vector<BagProducerConsumerThread *> allocators;
  for (unsigned int a_id = 0; a_id < args.NumThreads(); ++a_id) {
    BagProducerConsumerThread * allocator = 
      new BagProducerConsumerThread(p, &measurements, a_id, a_id, args, &nodes);
    // Preallocate elements if necessary: 
    allocator->PreallocateElements(n_prealloc, false);
    allocators.push_back(allocator);    
  }

  Console::WriteStep("Starting threads");
  // Start allocator threads:
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<BagProducerConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<BagProducerConsumerThread *>::const_iterator const c_end = 
    allocators.end();
  for (c_it = allocators.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }

  // Tell allocators to stop and collect results: 
  for (c_it = allocators.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
}

template<typename TPool>
void PoolBenchmark<TPool>::
RunScenario_1_EnqueueDequeueBulk() {
  Console::WriteStep("Initializing threads");
  // Threads now each allocate and deallocate one element 
  // in iterations: 
  ::std::vector<BagProducerConsumerThread *> reallocators;
  // Number of iterations per reallocator:
  for (unsigned int a_id = 0; a_id < args.NumThreads(); ++a_id) {
    Console::WriteValue("Initializing thread ", a_id);
    BagProducerConsumerThread * reallocator = 
      new BagProducerConsumerThread(p, &measurements, a_id, a_id, args, &nodes);
    reallocators.push_back(reallocator);    
  }

  Console::WriteStep("Starting threads");

  // Start reallocator threads: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<BagProducerConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<BagProducerConsumerThread *>::const_iterator const c_end = 
    reallocators.end();
  for (c_it = reallocators.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }
  // Tell reallocators to stop and collect results: 
  for (c_it = reallocators.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
}

template<typename TPool>
void PoolBenchmark<TPool>::
RunScenario_2_FillUp() {
  Console::WriteStep("Initializing threads");
  // Allocator threads: 
  ::std::vector<BagProducerConsumerThread *> allocators;
  for (unsigned int a_id = 0; a_id < args.NumThreads(); ++a_id) {
    BagProducerConsumerThread * pct =
      new BagProducerConsumerThread(p, &measurements, a_id, a_id, args, &nodes);
    pct->NumIterations(1);
    pct->NumProduceElements(0);
    pct->NumConsumeElements(args.NumElements());
    pct->QuitOnFailedConsume(true); 
    allocators.push_back(pct); 
  }

  Console::WriteStep("Starting threads");
  // Start allocator threads: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<BagProducerConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<BagProducerConsumerThread *>::const_iterator const c_end = 
    allocators.end();
  for (c_it = allocators.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }
  // Tell allocators to stop and collect results: 
  for (c_it = allocators.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
}

template<typename TPool>
void PoolBenchmark<TPool>::
RunScenario_3_Race() {
  // Allocator threads: 
  ::std::vector<BagProducerConsumerThread *> threads;
  for (unsigned int a_id = 0; a_id < args.NumConsumers(); ++a_id) {
    BagProducerConsumerThread * pct =
      new BagProducerConsumerThread(
        p, 
        &measurements, 
        a_id, 
        a_id, 
        args, 
        &nodes);
    pct->NumProduceElements(0);
    pct->NumConsumeElements(args.NumAllocsPerIt());
    threads.push_back(pct); 
  }

  // Threads from 2  producers vs 32 consumers 
  //           to 32 producers vs  2 consumers
  // -> Max. preallocated elements for 32 = (p + c - 2) producers
  size_t n_prealloc = (args.NumIterations() * args.NumAllocsPerIt()) *
                        (args.NumThreads() - 1) / 
                           args.NumProducers();
  
  Console::WriteValue("Max producers", args.NumThreads() - 1); 
  Console::WriteValue("Preallocs / p", n_prealloc); 

  // Deallocator threads. Need to pre-allocate elements 
  // to deallocate: 
  for (unsigned int d_id = 0; d_id < args.NumProducers(); ++d_id) {
    BagProducerConsumerThread * pct =
      new BagProducerConsumerThread(
        p, &measurements, 
        d_id, 
        args.NumConsumers() + d_id, 
        args, 
        &nodes);
    pct->NumProduceElements(args.NumAllocsPerIt());
    pct->PreallocateElements(n_prealloc, true); 
    pct->NumConsumeElements(0);
    threads.push_back(pct); 
  }

  Console::WriteStep("Starting threads");
  // Start threads: 
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<BagProducerConsumerThread *>::iterator c_it;
  EMBB_BASE_CPP_BENCHMARK_DEPENDANT_TYPENAME
    std::vector<BagProducerConsumerThread *>::const_iterator const c_end = 
    threads.end();
  for (c_it = threads.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Run();
  }
  // Tell threads to stop and collect results: 
  for (c_it = threads.begin(); c_it != c_end; ++c_it) {
    (*c_it)->Join();
  }
}

} // namespace benchmark
} // namespace embb

#endif // EMBB_BENCHMARK_CPP_INTERNAL_POOLS_POOL_BENCHMARK_INL_H_
