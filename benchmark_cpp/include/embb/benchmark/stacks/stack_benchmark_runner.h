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

#ifndef EMBB_BENCHMARK_CPP_STACKS_STACK_BENCHMARK_RUNNER_H_
#define EMBB_BENCHMARK_CPP_STACKS_STACK_BENCHMARK_RUNNER_H_

#include <embb/benchmark/report.h>
#include <embb/benchmark/call_args.h>
#include <embb/benchmark/benchmark_runner.h>
#include <embb/benchmark/stacks/stack_benchmark.h>
#include <embb/benchmark/stacks/stack_benchmark_report.h>

#include <embb/containers/lock_free_stack.h>
#include <embb/containers/wait_free_sim_stack.h>
#include <embb/containers/wait_free_sim_stack_tagged.h>
#include <embb/containers/indexed_object_pool.h>

#include <memory>

namespace embb {
namespace benchmark {

/**
 * Type adapter class for StackBenchmark< LockFreeStack<...> >
 */
class LockFreeStackBenchmarkRunner : public BenchmarkRunner {
private:
  typedef StackLatencyMeasurements::element_t element_t;

public:
  typedef embb::containers::LockFreeStack< element_t > concrete_stack_t;
  typedef StackBenchmark< concrete_stack_t > benchmark_t;

private:  
  CallArgs         args;
  concrete_stack_t stack;
  benchmark_t *    benchmark;

public:
  LockFreeStackBenchmarkRunner(const CallArgs & args);
  virtual ~LockFreeStackBenchmarkRunner() { }
  virtual ::std::auto_ptr< embb::benchmark::Report > Run();
};

/**
 * Type adapter class for StackBenchmark< WaitFreeSimStackTagged<...> >
 */
class WaitFreeSimStackTaggedBenchmarkRunner : public BenchmarkRunner {
private:
  typedef StackLatencyMeasurements::element_t element_t;

public:
  typedef embb::containers::WaitFreeSimStackTagged< element_t > concrete_stack_t;
  typedef StackBenchmark< concrete_stack_t > benchmark_t;

private:  
  CallArgs         args;
  concrete_stack_t stack;
  benchmark_t *    benchmark;

public:
  WaitFreeSimStackTaggedBenchmarkRunner(const CallArgs & args);
  virtual ~WaitFreeSimStackTaggedBenchmarkRunner() { }
  virtual ::std::auto_ptr< embb::benchmark::Report > Run();
};

/**
 * Type adapter class for StackBenchmark< WaitFreeSimStack<...> >
 */
class WaitFreeSimStackBenchmarkRunner : public BenchmarkRunner {
private:
  typedef StackLatencyMeasurements::element_t element_t;

public:
  typedef embb::containers::WaitFreeSimStack< element_t > concrete_stack_t;
  typedef StackBenchmark< concrete_stack_t > benchmark_t;

private:
  CallArgs         args;
  concrete_stack_t stack;
  benchmark_t *    benchmark;

public:
  WaitFreeSimStackBenchmarkRunner(const CallArgs & args);
  virtual ~WaitFreeSimStackBenchmarkRunner() { }
  virtual ::std::auto_ptr< embb::benchmark::Report > Run();
};

/**
 * Type adapter class for StackBenchmark< WaitFreeSimStack<...> >
 */
class WaitFreeSimStackTpBenchmarkRunner : public BenchmarkRunner {
private:
  typedef StackLatencyMeasurements::element_t element_t;

public:
  typedef embb::containers::WaitFreeSimStack<
    element_t,
    0xFFFFFFFF,
    64,
    embb::containers::IndexedObjectPool<
      EMBB_CONTAINERS_DEPENDANT_TYPENAME 
      embb::containers::internal::WaitFreeSimStackNode<element_t>::Element,
      embb::containers::LockFreeTreeValuePool< bool, false >
  >,
  embb::containers::LockFreeTreeValuePool< bool, false > > concrete_stack_t;
  typedef StackBenchmark< concrete_stack_t > benchmark_t;

private:
  CallArgs         args;
  concrete_stack_t stack;
  benchmark_t *    benchmark;

public:
  WaitFreeSimStackTpBenchmarkRunner(const CallArgs & args);
  virtual ~WaitFreeSimStackTpBenchmarkRunner() { }
  virtual ::std::auto_ptr< embb::benchmark::Report > Run();
};

/**
 * Type adapter class for StackBenchmark< WaitFreeSimStack<...> >
 */
class WaitFreeSimStackApBenchmarkRunner : public BenchmarkRunner {
private:
  typedef StackLatencyMeasurements::element_t element_t;

public:
  typedef embb::containers::WaitFreeSimStack<
    element_t,
    0xFFFFFFFF,
    64,
    embb::containers::IndexedObjectPool<
      EMBB_CONTAINERS_DEPENDANT_TYPENAME 
      embb::containers::internal::WaitFreeSimStackNode<element_t>::Element,
      embb::containers::WaitFreeArrayValuePool< bool, false >
  >,
  embb::containers::WaitFreeArrayValuePool< bool, false > > concrete_stack_t;
  typedef StackBenchmark< concrete_stack_t > benchmark_t;

private:
  CallArgs         args;
  concrete_stack_t stack;
  benchmark_t *    benchmark;

public:
  WaitFreeSimStackApBenchmarkRunner(const CallArgs & args);
  virtual ~WaitFreeSimStackApBenchmarkRunner() { }
  virtual ::std::auto_ptr< embb::benchmark::Report > Run();
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_STACKS_STACK_BENCHMARK_RUNNER_H_ */
