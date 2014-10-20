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

#ifndef EMBB_BENCHMARK_CPP_QUEUES_QUEUE_BENCHMARK_H_
#define EMBB_BENCHMARK_CPP_QUEUES_QUEUE_BENCHMARK_H_

#include <embb/benchmark/call_args.h>
#include <embb/benchmark/internal/producer_consumer_thread.h>
#include <embb/benchmark/report.h>
#include <embb/base/perf/timer.h>
#include <embb/benchmark/queues/queue_latency_measurements.h>
#include <embb/base/thread.h>

#include <vector>
#include <algorithm>

namespace embb {
namespace benchmark {

using ::std::min;

template< typename TQueue >
class QueueBenchmark {
public:
  typedef QueueLatencyMeasurements measurements_t; 
  typedef QueueLatencyMeasurements::element_t element_t;

private: 
  typedef QueueBenchmark<TQueue> self_t;

private: 
  TQueue * const   q;
  const CallArgs & callArgs;
  measurements_t   measurements;
    
private:
  // Disable default construction. 
  QueueBenchmark() { }
  // Disable copy construction. 
  QueueBenchmark(const self_t & other) { }
  // Disable assignment. 
  self_t & operator=(const self_t & other) { }

public: 
  QueueBenchmark(TQueue * queue, const CallArgs & params);
  virtual ~QueueBenchmark();

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
  class QueueProducerConsumerThread : 
  public internal::ProducerConsumerThread< 
    TQueue, 
    QueueLatencyMeasurements > {
   private:
     typedef typename internal::ProducerConsumerThread<
       TQueue,
       QueueLatencyMeasurements > base_t; 
   public: 
     inline QueueProducerConsumerThread(
       TQueue * benchmarkUnit, 
       QueueLatencyMeasurements * measurements,
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
  class ProducerThread : public QueueProducerConsumerThread {
  private:
    int n_sleep_ms;
  public:
    ProducerThread(
      TQueue * queue, QueueLatencyMeasurements * measurements,
      unsigned int id, unsigned int core, const CallArgs & callArgs,
      size_t num_enqueues, int nSleepMs = 0) :
      QueueProducerConsumerThread(
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
  class ConsumerThread : public QueueProducerConsumerThread {
  public:
    ConsumerThread(
      TQueue * queue, QueueLatencyMeasurements * measurements,
      unsigned int id, unsigned int core, const CallArgs & callArgs,
      size_t num_dequeues) :
      QueueProducerConsumerThread(
        queue, measurements, id, core, callArgs) {
        this->NumProduceElements(0);
        this->NumConsumeElements(num_dequeues);
      }
    /// Complete the thread task and deactivate the thread. 
    virtual void Join() {
      this->RequestStop(); 
      QueueProducerConsumerThread::Join();
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

#include <embb/benchmark/internal/queue_benchmark-inl.h>

#endif /* EMBB_BENCHMARK_CPP_QUEUES_QUEUE_BENCHMARK_H_ */
