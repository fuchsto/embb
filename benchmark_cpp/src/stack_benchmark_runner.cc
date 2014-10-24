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

#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/stacks/stack_benchmark_runner.h>
#include <embb/benchmark/stacks/stack_latency_measurements.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <utility> 

namespace embb {
namespace benchmark {

using ::std::min; 
using internal::Console;

LockFreeStackBenchmarkRunner::
LockFreeStackBenchmarkRunner(const CallArgs & callArgs)
: args(callArgs), 
  stack(args.NumElements()) {
  benchmark = new benchmark_t(&stack, args);
}

::std::auto_ptr< embb::benchmark::Report >
LockFreeStackBenchmarkRunner::Run() {
  Console::WriteHeader("LockFreeStack"); 

  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;

  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("Creating report"); 

  return ::std::auto_ptr< embb::benchmark::Report >(
    new StackBenchmarkReport(benchmark->Measurements()));
}

WaitFreeSimStackTaggedBenchmarkRunner::
WaitFreeSimStackTaggedBenchmarkRunner(const CallArgs & callArgs)
: args(callArgs),
  stack(args.NumElements()) {
  benchmark = new benchmark_t(&stack, args);
}

::std::auto_ptr< embb::benchmark::Report >
WaitFreeSimStackTaggedBenchmarkRunner::Run() {
  Console::WriteHeader("WaitFreeSimStackTagged");

  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;

  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("Creating report");

  return ::std::auto_ptr< embb::benchmark::Report >(
    new StackBenchmarkReport(benchmark->Measurements()));
}

WaitFreeSimStackBenchmarkRunner::
WaitFreeSimStackBenchmarkRunner(const CallArgs & callArgs)
: args(callArgs),
stack(args.NumElements()) {
  benchmark = new benchmark_t(&stack, args);
}

::std::auto_ptr< embb::benchmark::Report >
WaitFreeSimStackBenchmarkRunner::Run() {
  Console::WriteHeader("WaitFreeSimStack");

  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;

  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("Creating report");

  return ::std::auto_ptr< embb::benchmark::Report >(
    new StackBenchmarkReport(benchmark->Measurements()));
}

WaitFreeSimStackTpBenchmarkRunner::
WaitFreeSimStackTpBenchmarkRunner(const CallArgs & callArgs)
: args(callArgs),
stack(args.NumElements()) {
  benchmark = new benchmark_t(&stack, args);
}

::std::auto_ptr< embb::benchmark::Report >
WaitFreeSimStackTpBenchmarkRunner::Run() {
  Console::WriteHeader("WaitFreeSimStack (tree pool)");

  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;

  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("Creating report");

  return ::std::auto_ptr< embb::benchmark::Report >(
    new StackBenchmarkReport(benchmark->Measurements()));
}

WaitFreeSimStackApBenchmarkRunner::
WaitFreeSimStackApBenchmarkRunner(const CallArgs & callArgs)
: args(callArgs),
stack(args.NumElements()) {
  benchmark = new benchmark_t(&stack, args);
}

::std::auto_ptr< embb::benchmark::Report >
WaitFreeSimStackApBenchmarkRunner::Run() {
  Console::WriteHeader("WaitFreeSimStack (array pool)");

  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;

  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("Creating report");

  return ::std::auto_ptr< embb::benchmark::Report >(
    new StackBenchmarkReport(benchmark->Measurements()));
}

} // namespace benchmark
} // namespace embb
