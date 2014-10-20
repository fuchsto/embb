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

#ifndef EMBB_BENCHMARK_CPP_STACKS_STACK_BENCHMARK_H_
#define EMBB_BENCHMARK_CPP_STACKS_STACK_BENCHMARK_H_

#include <embb/benchmark/call_args.h>
#include <embb/benchmark/report.h>
#include <embb/benchmark/stacks/stack_latency_measurements.h>
#include <embb/benchmark/internal/producer_consumer_thread.h>
#include <embb/base/perf/timer.h>
#include <embb/base/thread.h>

#include <vector>
#include <algorithm> // std::min

namespace embb {
namespace benchmark {

using ::std::min;

template< typename TStack >
class StackBenchmark {
private:
  typedef StackBenchmark<TStack> self_t;
public:   
  typedef StackLatencyMeasurements measurements_t; 
  typedef StackLatencyMeasurements::element_t element_t; 
  
private:
  TStack * const   s;
  const CallArgs & callArgs;   
  size_t           n_nodes;
  measurements_t   measurements;
    
private:
  // Disable default construction. 
  StackBenchmark() { }
  // Disable copy construction. 
  StackBenchmark(const self_t & other) { }
  // Disable assignment. 
  self_t & operator=(const self_t & other) { }
  
public: 
  StackBenchmark(TStack * stack, const CallArgs & args);
  virtual ~StackBenchmark();

public:
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
  void RunScenario_4_CapacityBuffer();
  
protected:
  class StackProducerConsumerThread : 
  public internal::ProducerConsumerThread< 
    TStack, 
    StackLatencyMeasurements > {
   private:
     typedef typename internal::ProducerConsumerThread<
       TStack,
       StackLatencyMeasurements > base_t; 
   public: 
     inline StackProducerConsumerThread(
       TStack * benchmarkUnit, 
       StackLatencyMeasurements * measurements,
       unsigned int id,
       unsigned int core,
       const CallArgs & params) 
     : base_t(
         benchmarkUnit, measurements, id, core, params) 
     { 
       this->NumProduceElements(params.NumAllocsPerIt());
       this->NumConsumeElements(params.NumAllocsPerIt());
     }
   protected:
    /// Thread task body. 
    virtual void Task();
  };
  
  /* Benchmark thread definition specialized for 
   * enqueue operations.
   */
  class ProducerThread : public StackProducerConsumerThread {
  private:
    int n_sleep_ms;
  public:
    ProducerThread(
      TStack * queue, StackLatencyMeasurements * measurements,
      unsigned int id, unsigned int core, const CallArgs & callArgs, 
      size_t num_enqueues, int nSleepMs = 0) :
      StackProducerConsumerThread(
        queue, measurements, id, core, callArgs), 
      n_sleep_ms(nSleepMs) {
        this->NumProduceElements(num_enqueues);
        this->NumConsumeElements(0);
      }
    /// The producer threads task loop. 
    virtual void Task(); 
  }; 

  /* Benchmark thread definition specialized for 
   * dequeue operations. 
   * Measurements element latency (buffer latency) if 
   * possible for the queue element type. 
   */
  class ConsumerThread : public StackProducerConsumerThread {
  public:
    ConsumerThread(
      TStack * queue, StackLatencyMeasurements * measurements,
      unsigned int id, unsigned int core, const CallArgs & callArgs, 
      size_t num_dequeues) :
      StackProducerConsumerThread(
        queue, measurements, id, core, callArgs) {  
        this->NumProduceElements(0);
        this->NumConsumeElements(num_dequeues);
      }
    /// Complete the thread task and deactivate the thread. 
    virtual void Join() {
      this->RequestStop(); 
      StackProducerConsumerThread::Join();
    }
    /// The consumer threads task loop. 
    virtual void Task(); 
  }; 

protected:
  ::std::vector<ProducerThread *> producers;
  ::std::vector<ConsumerThread *> consumers;
};

} // namespace benchmark
} // namespace embb

#include <embb/benchmark/internal/stack_benchmark-inl.h>

#endif // EMBB_BENCHMARK_CPP_STACKS_STACK_BENCHMARK_H_
