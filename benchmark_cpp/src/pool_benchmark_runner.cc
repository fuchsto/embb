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

#include <memory>
#include <vector>
#include <iterator>

namespace embb {
namespace benchmark {

using internal::Console;

LockFreeTreeValuePoolBenchmarkRunner::
LockFreeTreeValuePoolBenchmarkRunner(const CallArgs & callArgs)  
: args(callArgs) {
  Console::WriteStep("Preparing unit"); 
  
  PoolLatencyMeasurements::node_indices_t elements;
  for (unsigned int node_index = 0; node_index < args.NumElements(); ++node_index) {
    elements.push_back(
      static_cast< PoolLatencyMeasurements::node_index_t >(node_index));
  }
  pool      = new concrete_pool_t(elements.begin(), elements.end());
  benchmark = new benchmark_t(pool, args);
}

::std::auto_ptr< embb::benchmark::Report >
LockFreeTreeValuePoolBenchmarkRunner::
Run() {
  Console::WriteHeader("LockFreeTreeValuePool"); 
  Console::WriteStep("Running benchmark"); 

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
: args(callArgs) {
  Console::WriteStep("Preparing unit"); 

  PoolLatencyMeasurements::node_indices_t elements;
  for (unsigned int node_index = 0; node_index < args.NumElements(); ++node_index) {
    elements.push_back(
      static_cast< PoolLatencyMeasurements::node_index_t >(node_index));
  }
  pool      = new concrete_pool_t(elements.begin(), elements.end());
  benchmark = new benchmark_t(pool, args);
}

::std::auto_ptr< embb::benchmark::Report >
WaitFreeArrayValuePoolBenchmarkRunner::
Run() {
  Console::WriteHeader("WaitFreeArrayValuePool"); 
  Console::WriteStep("Running benchmark"); 

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
: args(callArgs) {
  Console::WriteStep("Preparing unit"); 
  unsigned int nThreads = embb::base::Thread::GetThreadsMaxCount(); 
  int k = args.QParam();
  if (k <= 0) { 
    k = 5; 
  }
  size_t cRange = (args.NumElements() * static_cast<size_t>(k)) / 100; 
  size_t cSize  = cRange / nThreads;
  cRange = cSize * nThreads; 
  size_t nTotal = args.NumElements() +
    ((nThreads - 1) * cSize); 
  PoolLatencyMeasurements::node_indices_t elements;
  for (size_t node_index = 0; node_index <= nTotal; ++node_index) {
    elements.push_back(
      static_cast< PoolLatencyMeasurements::node_index_t >(node_index));
  }

  PoolLatencyMeasurements::node_indices_t::iterator end = elements.begin(); 
  std::advance(end, args.NumElements());
  pool = new compartment_pool_t(elements.begin(), 
                                end, 
                                static_cast<int>(cSize));
  benchmark = new benchmark_t(pool, args, nTotal);
}

::std::auto_ptr< embb::benchmark::Report >
WaitFreeCompartmentValuePoolBenchmarkRunner::
Run() {
  Console::WriteHeader("WaitFreeCompartmentValuePool"); 
  Console::WriteStep("Running benchmark"); 

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
