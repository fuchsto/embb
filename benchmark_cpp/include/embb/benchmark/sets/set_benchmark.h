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

#ifndef EMBB_BENCHMARK_CPP_SETS_SET_BENCHMARK_H_
#define EMBB_BENCHMARK_CPP_SETS_SET_BENCHMARK_H_

#include <embb/benchmark/call_args.h>
#include <embb/benchmark/internal/producer_consumer_thread.h>
#include <embb/benchmark/report.h>
#include <embb/benchmark/sets/set_latency_measurements.h>
#include <embb/base/perf/timer.h>
#include <embb/base/thread.h>

#include <vector>

namespace embb {
namespace benchmark {
  
template< typename TSet >
class SetBenchmark {
 public: 
  typedef SetLatencyMeasurements measurements_t;
  typedef SetLatencyMeasurements::node_t element_t;

 private:
  typedef SetBenchmark<TSet> self_t;
  
 private:
  TSet *     unit;
  CallArgs   args;
  measurements_t measurements;

 private:
  /// Disable default construction. 
  SetBenchmark() { }
  /// Disable copy construction. 
  SetBenchmark(const self_t & other) { }
  /// Disable assignment. 
  self_t & operator=(const self_t & other) { }
  
 public:  
  SetBenchmark(TSet * unitInstance, const CallArgs & args);
  ~SetBenchmark();
  
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
  
 protected:
  class SetProducerConsumerThread : 
  public internal::ProducerConsumerThread< 
    TSet, SetLatencyMeasurements > {
   private:
     typedef typename internal::ProducerConsumerThread<
       TSet, SetLatencyMeasurements > base_t;   
   public: 
     inline SetProducerConsumerThread(
       TSet * benchmarkUnit,
       SetLatencyMeasurements * measurements,
       unsigned int id,
       unsigned int core, 
       const CallArgs & params)
     : base_t(
         benchmarkUnit, measurements, id, core, params)
     { }
   public:
     /// Thread task body. 
     virtual void Task();
  };  
};

} // namespace benchmark
} // namespace embb

#include <embb/benchmark/internal/set_benchmark-inl.h>

#endif /* EMBB_BENCHMARK_CPP_SETS_SET_BENCHMARK_H_ */
