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

#include <embb/benchmark/call_args.h>
#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/pools/pool_benchmark_runner.h>
#include <embb/benchmark/pools/pool_latency_measurements.h>
#include <embb/containers/internal/increment_iterator.h>

#include <memory>
#include <vector>
#include <iterator>

namespace embb {
namespace benchmark {

using internal::Console;
using embb::containers::internal::IncrementIterator;

LockFreeTreeValuePoolBenchmarkRunner::
LockFreeTreeValuePoolBenchmarkRunner(const CallArgs & callArgs)  
: args(callArgs), 
  pool(IncrementIterator(0), 
       IncrementIterator(args.NumElements())) {
  benchmark = new benchmark_t(&pool, args);
}

::std::auto_ptr< embb::benchmark::Report >
LockFreeTreeValuePoolBenchmarkRunner::
Run() {
  Console::WriteHeader("LockFreeTreeValuePool"); 

  Timer runtime; 
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0; 

  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("Creating report"); 

  return ::std::auto_ptr< embb::benchmark::Report >(
    new PoolBenchmarkReport(benchmark->Measurements()));
}

WaitFreeArrayValuePoolBenchmarkRunner::
WaitFreeArrayValuePoolBenchmarkRunner(const CallArgs & callArgs) 
: args(callArgs),
  pool(IncrementIterator(0), 
       IncrementIterator(args.NumElements())) {
  benchmark = new benchmark_t(&pool, args);
}

::std::auto_ptr< embb::benchmark::Report >
WaitFreeArrayValuePoolBenchmarkRunner::
Run() {
  Console::WriteHeader("WaitFreeArrayValuePool"); 

  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;

  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("Creating report"); 

  return ::std::auto_ptr< embb::benchmark::Report >(
    new PoolBenchmarkReport(benchmark->Measurements()));
}

WaitFreeCompartmentValuePoolBenchmarkRunner::
WaitFreeCompartmentValuePoolBenchmarkRunner(const CallArgs & callArgs)
: args(callArgs),
  numNodes(
      args.NumElements() + 
      args.QParam() * (embb::base::Thread::GetThreadsMaxCount() - 1)),
  pool(IncrementIterator(0), 
       IncrementIterator(numNodes),
       args.QParam()) {
  benchmark = new benchmark_t(&pool, args, numNodes);
}

::std::auto_ptr< embb::benchmark::Report >
WaitFreeCompartmentValuePoolBenchmarkRunner::
Run() {
  Console::WriteHeader("WaitFreeCompartmentValuePool"); 

  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;

  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("Creating report"); 

  return ::std::auto_ptr< embb::benchmark::Report >(
    new PoolBenchmarkReport(benchmark->Measurements()));
}

} // namespace benchmark
} // namespace embb

