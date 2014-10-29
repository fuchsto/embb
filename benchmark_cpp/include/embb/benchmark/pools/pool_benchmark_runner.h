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

#ifndef EMBB_BENCHMARK_CPP_POOLS_POOL_BENCHMARK_RUNNER_H_
#define EMBB_BENCHMARK_CPP_POOLS_POOL_BENCHMARK_RUNNER_H_

#include <embb/benchmark/call_args.h>
#include <embb/benchmark/report.h>
#include <embb/benchmark/benchmark_runner.h>
#include <embb/benchmark/pools/pool_benchmark.h>
#include <embb/benchmark/pools/pool_benchmark_report.h>

#include <embb/containers/lock_free_tree_value_pool.h>
#include <embb/containers/wait_free_array_value_pool.h>
#include <embb/containers/wait_free_compartment_value_pool.h>

namespace embb {
namespace benchmark {

/**
 * @brief Type adapter class for PoolLatencyBenchmark< CircularObjectPool<...> >
 */
class LockFreeTreeValuePoolBenchmarkRunner: public BenchmarkRunner{
  
public:

  typedef embb::containers::LockFreeTreeValuePool<
    PoolLatencyMeasurements::node_index_t,
    PoolLatencyMeasurements::UndefinedElement > concrete_pool_t;

  typedef PoolBenchmark< concrete_pool_t > benchmark_t;

private: 

  CallArgs        args;
  concrete_pool_t pool;
  benchmark_t *   benchmark;

public:

  LockFreeTreeValuePoolBenchmarkRunner(const CallArgs & args);

  virtual ~LockFreeTreeValuePoolBenchmarkRunner() { 
  }

  virtual ::std::auto_ptr< embb::benchmark::Report > Run();

};

/**
 * @brief Type adapter class for PoolLatencyBenchmark< WaitFreeArrayObjectPool<...> >
 */
class WaitFreeArrayValuePoolBenchmarkRunner : public BenchmarkRunner {

public:
  
  typedef embb::containers::WaitFreeArrayValuePool< 
    PoolLatencyMeasurements::node_index_t,
    PoolLatencyMeasurements::UndefinedElement > concrete_pool_t;

  typedef PoolBenchmark< concrete_pool_t > benchmark_t;

private:

  CallArgs        args;
  concrete_pool_t pool;
  benchmark_t *   benchmark;

public:
  
  WaitFreeArrayValuePoolBenchmarkRunner(const CallArgs & args);

  virtual ~WaitFreeArrayValuePoolBenchmarkRunner() { 
  }

  virtual ::std::auto_ptr< embb::benchmark::Report > Run();

};

/**
* @brief Type adapter class for PoolLatencyBenchmark< WaitFreeArrayObjectPool<...> >
*/
class WaitFreeCompartmentValuePoolBenchmarkRunner : public BenchmarkRunner {

public:

  typedef embb::containers::WaitFreeCompartmentValuePool<
    PoolLatencyMeasurements::node_index_t,
    PoolLatencyMeasurements::UndefinedElement > compartment_pool_t;

  typedef PoolBenchmark< compartment_pool_t > benchmark_t;

private:

  CallArgs           args;
  size_t             numNodes;
  compartment_pool_t pool;
  benchmark_t *      benchmark;

public:

  WaitFreeCompartmentValuePoolBenchmarkRunner(const CallArgs & args);

  virtual ~WaitFreeCompartmentValuePoolBenchmarkRunner() {
  }

  virtual ::std::auto_ptr< embb::benchmark::Report > Run();

};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_POOLS_POOL_BENCHMARK_RUNNER_H_ */
