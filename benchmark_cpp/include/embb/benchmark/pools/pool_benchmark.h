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

#ifndef EMBB_BENCHMARK_CPP_POOLS_POOL_BENCHMARK_H_
#define EMBB_BENCHMARK_CPP_POOLS_POOL_BENCHMARK_H_

#include <embb/benchmark/report.h>
#include <embb/benchmark/pools/pool_latency_measurements.h>
#include <embb/benchmark/internal/producer_consumer_thread.h>
#include <embb/base/perf/timer.h>
#include <embb/base/thread.h>

#include <vector>

namespace embb {
namespace benchmark {
  
template< typename TPool >
class PoolBenchmark {
 public: 
  typedef PoolLatencyMeasurements measurements_t;
  typedef PoolLatencyMeasurements::node_index_t element_t;

 private:
  typedef PoolBenchmark<TPool> self_t;
  
 private:
  TPool *        p;
  CallArgs       args;
  size_t         n_nodes;
  measurements_t measurements;

 private:
  PoolLatencyMeasurements::node_indices_t * elements;
  PoolLatencyMeasurements::node_pool_t      nodes;  
  /// Disable default construction. 
  PoolBenchmark() { }
  /// Disable copy construction. 
  PoolBenchmark(const self_t & other) { }
  /// Disable assignment. 
  self_t & operator=(const self_t & other) { }
  
 public:  
  PoolBenchmark(TPool * pool, const CallArgs & args, size_t numNodes = 0);
  ~PoolBenchmark();
  
  /// Starts the benchmark. Selects specific scenario 
  /// implementation to run from settings in call arguments. 
  void Run();
  inline const measurements_t & Measurements() const {
    return measurements;
  }

 private:
  void RunScenario_0_EnqueueDequeuePairs();
  void RunScenario_1_EnqueueDequeueBulk();
  void RunScenario_2_FillUp();
  void RunScenario_3_Race();
  
 protected:
  class BagProducerConsumerThread : 
  public internal::ProducerConsumerThread< 
    TPool, PoolLatencyMeasurements > {
   private:
     typedef typename internal::ProducerConsumerThread<
       TPool, PoolLatencyMeasurements > base_t;
     typedef BagProducerConsumerThread self_t;
   protected:
     /// Node pool to operate on. 
     PoolLatencyMeasurements::node_pool_t * const nodePool;
     /// Milliseconds to sleep between allocation 
     /// and deallocation loop. 
     size_t n_sleep_ms_iteration;
     /// Pool indices of acquired nodes in node pool. 
     ::std::vector< PoolLatencyMeasurements::node_index_t > 
       allocatedNodeIndices;    
   public: 
     inline BagProducerConsumerThread(
       TPool * benchmarkUnit,
       PoolLatencyMeasurements * measurements,
       unsigned int id,
       unsigned int core,
       const CallArgs & params, 
       PoolLatencyMeasurements::node_pool_t * pool)
     : base_t(
       benchmarkUnit, measurements, id, core, params),
       nodePool(pool),
       n_sleep_ms_iteration(0)
       { }
   public:
     virtual void PreallocateElements(size_t n, bool forDeallocation = false);
     inline virtual void NumConsumeElements(size_t n) {
       base_t::NumConsumeElements(n);
       allocatedNodeIndices.reserve(n);  
     }
     inline virtual size_t NumConsumeElements() const {
       return base_t::NumConsumeElements();
     }
     inline virtual void SleepAfterConsumeIteration(size_t ms) {
       n_sleep_ms_iteration = ms;
     }
     inline virtual size_t SleepAfterConsumeIteration() {
       return n_sleep_ms_iteration;
     }
     /// Thread task body. 
     virtual void Task();
  };  
};

} // namespace benchmark
} // namespace embb

#include <embb/benchmark/internal/pool_benchmark-inl.h>

#endif /* EMBB_BENCHMARK_CPP_POOLS_POOL_BENCHMARK_H_ */
