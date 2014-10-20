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
#include <embb/benchmark/queues/queue_benchmark_runner.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <utility> 
#include <string>
#include <sstream>

namespace embb {
namespace benchmark {

using ::std::min; 
using internal::Console;

MichaelScottQueueTpBenchmarkRunner::
MichaelScottQueueTpBenchmarkRunner(const CallArgs & callArgs) 
: args(callArgs) {
  Console::WriteStep("Preparing unit"); 
  queue = new concrete_queue_t(args.NumElements());
  benchmark = new benchmark_t(queue, args);
}

::std::auto_ptr< embb::benchmark::Report >
MichaelScottQueueTpBenchmarkRunner::Run() {
  Console::WriteHeader("MichaelScottQueue (tree pool)"); 
  Console::WriteStep("Running benchmark"); 

  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;

  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("CreatingReport"); 

  return ::std::auto_ptr< embb::benchmark::Report >(
    new QueueBenchmarkReport(benchmark->Measurements()));
}

MichaelScottQueueApBenchmarkRunner::
MichaelScottQueueApBenchmarkRunner(const CallArgs & callArgs) 
: args(callArgs) {
  Console::WriteStep("Preparing unit"); 

  queue = new concrete_queue_t(
    // queue size
    args.NumElements());

  benchmark = new benchmark_t(queue, args);
}

::std::auto_ptr< embb::benchmark::Report >
MichaelScottQueueApBenchmarkRunner::Run() {
  Console::WriteHeader("MichaelScottQueue (array pool)"); 
  Console::WriteStep("Running benchmark"); 

  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;

  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("CreatingReport"); 

  return ::std::auto_ptr< embb::benchmark::Report >(
    new QueueBenchmarkReport(benchmark->Measurements()));
}

WaitFreeQueueBenchmarkRunner::
WaitFreeQueueBenchmarkRunner(const CallArgs & callArgs) 
: args(callArgs) {
  Console::WriteStep("Preparing unit"); 

  queue = new concrete_queue_t(
    // queue size
    args.NumElements());

  benchmark = new benchmark_t(queue, args);
}

::std::auto_ptr< embb::benchmark::Report >
WaitFreeQueueBenchmarkRunner::Run() {
  Console::WriteHeader("WaitFreeQueue"); 
  Console::WriteStep("Running benchmark"); 
  
  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;
  
  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("Creating report"); 

  Console::WriteValue("Max phase", queue->MaxPhaseUsed());

  ::std::stringstream sps; 
  std::vector< uint32_t > phases = queue->DumpPhases();
  std::vector< uint32_t >::iterator p_it;
  std::vector< uint32_t >::const_iterator p_end = phases.end();
  for (p_it = phases.begin(); p_it != p_end; ++p_it) {
    sps << *p_it << " ";
  }
  Console::WriteValue< const ::std::string & >("Phases", sps.str()); 

  return ::std::auto_ptr< embb::benchmark::Report >(
    new QueueBenchmarkReport(benchmark->Measurements()));
}

WaitFreeQueuePhaselessBenchmarkRunner::
WaitFreeQueuePhaselessBenchmarkRunner(const CallArgs & callArgs) 
: args(callArgs) {
  Console::WriteStep("Preparing unit"); 
  
  queue = new concrete_queue_t(
    // queue size
    args.NumElements());

  benchmark = new benchmark_t(queue, args);
}

::std::auto_ptr< embb::benchmark::Report >
WaitFreeQueuePhaselessBenchmarkRunner::
Run() {
  Console::WriteHeader("WaitFreeQueuePhaseless"); 
  Console::WriteStep("Running benchmark"); 

  Timer runtime;
  benchmark->Run();
  double seconds = runtime.Elapsed() / 1000000.0;
  
  Console::WriteValue<double>("Execution time", seconds, 3, "s");
  Console::WriteStep("Creating report"); 

  return ::std::auto_ptr< embb::benchmark::Report >(
    new QueueBenchmarkReport(benchmark->Measurements()));
}

} // namespace benchmark
} // namespace embb
