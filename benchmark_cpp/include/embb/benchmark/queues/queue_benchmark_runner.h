
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

#ifndef EMBB_BENCHMARK_CPP_QUEUES_QUEUE_BENCHMARK_RUNNER_H_
#define EMBB_BENCHMARK_CPP_QUEUES_QUEUE_BENCHMARK_RUNNER_H_

#include <embb/benchmark/queues/queue_benchmark.h>
#include <embb/benchmark/queues/queue_benchmark_report.h>
#include <embb/benchmark/latency_report.h>
#include <embb/benchmark/benchmark_runner.h>
#include <embb/containers/wait_free_array_value_pool.h>
#include <embb/containers/lock_free_tree_value_pool.h>
#include <embb/containers/lock_free_mpmc_queue.h>
#include <embb/containers/wait_free_queue.h>
#include <embb/containers/wait_free_phaseless_queue.h>

#include <memory> 
#include <vector>

namespace embb {
namespace benchmark {

using ::std::min;

/**
 * @brief Type adapter class for QueueLatencyBenchmark< MichaelScottQueue<...> >
 * 
 * Uses tree-based value pool. 
 */
class MichaelScottQueueTpBenchmarkRunner : public BenchmarkRunner {
private:
  typedef QueueLatencyMeasurements::element_t element_t;

public:
  typedef embb::containers::LockFreeMPMCQueue< element_t > concrete_queue_t;
  typedef QueueBenchmark< concrete_queue_t > benchmark_t;

private:
  concrete_queue_t * queue;
  benchmark_t *      benchmark;
  CallArgs           args;

public:
  MichaelScottQueueTpBenchmarkRunner(const CallArgs & args);
  virtual ~MichaelScottQueueTpBenchmarkRunner() { }
  virtual ::std::auto_ptr< embb::benchmark::Report > Run();
};

/**
 * @brief Type adapter class for QueueLatencyBenchmark< MichaelScottQueue<...> >
 * 
 * Uses array-based value pool. 
 */
class MichaelScottQueueApBenchmarkRunner : public BenchmarkRunner {
private:
  typedef QueueLatencyMeasurements::element_t element_t;

public:
  typedef embb::containers::LockFreeMPMCQueue< element_t > concrete_queue_t;
  typedef QueueBenchmark< concrete_queue_t > benchmark_t;

private:
  concrete_queue_t * queue;
  benchmark_t *      benchmark;
  CallArgs           args;

public:
  MichaelScottQueueApBenchmarkRunner(const CallArgs & args);
  virtual ~MichaelScottQueueApBenchmarkRunner() { }
  virtual ::std::auto_ptr< embb::benchmark::Report > Run();
};

/**
 * @brief Type adapter class for QueueLatencyBenchmark< WaitFreeQueue<...> >
 */
class WaitFreeQueueBenchmarkRunner : public BenchmarkRunner {
private:
  typedef QueueLatencyMeasurements::element_t element_t;

public:
  typedef embb::containers::WaitFreeQueue< element_t > concrete_queue_t;
  typedef QueueBenchmark< concrete_queue_t > benchmark_t;

private:
  concrete_queue_t * queue;
  benchmark_t *      benchmark;
  CallArgs           args;

public:
  WaitFreeQueueBenchmarkRunner(const CallArgs & args);
  virtual ~WaitFreeQueueBenchmarkRunner() { }
  virtual ::std::auto_ptr< embb::benchmark::Report > Run();
};

/**
 * @brief Type adapter class for QueueLatencyBenchmark< WaitFreePhaselessQueue<...> >
 */
class WaitFreeQueuePhaselessBenchmarkRunner : public BenchmarkRunner {
private:
  typedef QueueLatencyMeasurements::element_t element_t;

public:
  typedef embb::containers::WaitFreePhaselessQueue< element_t > concrete_queue_t;
  typedef QueueBenchmark< concrete_queue_t > benchmark_t;

private:
  concrete_queue_t * queue;
  benchmark_t *      benchmark;
  CallArgs           args;

public:
  WaitFreeQueuePhaselessBenchmarkRunner(const CallArgs & args);
  virtual ~WaitFreeQueuePhaselessBenchmarkRunner() { }
  virtual ::std::auto_ptr< embb::benchmark::Report > Run();
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_QUEUES_QUEUE_BENCHMARK_RUNNER_H_ */
