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

#include <embb/benchmark/unit.h>
#include <embb/benchmark/call_args.h>
#include <embb/benchmark/report.h>
#include <embb/benchmark/benchmark_runner.h>
#include <embb/benchmark/internal/self_test.h>
#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/queues/queue_benchmark_runner.h>
#include <embb/benchmark/queues/queue_benchmark_report.h>
#include <embb/benchmark/pools/pool_benchmark_runner.h>
#include <embb/benchmark/pools/pool_benchmark_report.h>
#include <embb/benchmark/stacks/stack_benchmark_runner.h>
#include <embb/benchmark/stacks/stack_benchmark_report.h>
#include <embb/benchmark/sets/set_benchmark_runner.h>
#include <embb/benchmark/sets/set_benchmark_report.h>
#include <embb/base/perf/timer.h>
#include <embb/base/thread.h>

#include <memory>
#include <iostream>
#include <iomanip>
#include <string>

using embb::base::perf::Timer;
using embb::benchmark::internal::Console;
using namespace embb::benchmark;

int confirmExit() {
  ::std::cout << ::std::endl;
  ::std::cout << "DONE" << ::std::endl;
  return 0;
}

void runBenchmark(BenchmarkRunner & benchmark, CallArgs & params) {
  ::std::auto_ptr< Report > report = benchmark.Run();
  report->Print();
  if (params.WriteToSummaryFile()) {
    report->WriteSummaryToFile(params.SummaryFile(), 
                               params.ExecutionCount() == 0);
  }
  if (params.WriteToDataFile()) {
    report->WriteSamplesToFile(params.DataFile());
  }
}

int main(int argc, char* argv[])
{
  CallArgs params;
 
#if !defined(NDEBUG)
  std::cout << "!!!! Running a debug build !!!!!!!!!!!!!!!!!!!!!!!" << std::endl; 
#endif

  try {
    params.ParseArgs(argc, argv);
  }
  catch (::std::runtime_error & e) {
    ::std::cerr << "Failed: " << e.what() << ::std::endl;
    return confirmExit();
  }

  Timer::Calibrate(params.TimerType(), 
                   params.TimerParam());
  
  try {
    embb::base::Thread::SetThreadsMaxCount(
      static_cast<unsigned int>(1 + params.NumThreads()));

    Console::WriteValue("EMBB threads", embb::base::Thread::GetThreadsMaxCount());

    if (params.UnitId() == Unit::SELF_TEST) {
      embb::benchmark::internal::SelfTest::Run(); 
      return confirmExit();
    }
    
    else if (params.UnitId() == Unit::MICHAEL_SCOTT_QUEUE_TP) {
      MichaelScottQueueTpBenchmarkRunner benchmark(params);
      runBenchmark(benchmark, params);
    }
    else if (params.UnitId() == Unit::MICHAEL_SCOTT_QUEUE_AP) {
      MichaelScottQueueApBenchmarkRunner benchmark(params);
      runBenchmark(benchmark, params);
    }
    else if (params.UnitId() == Unit::KOGAN_PETRANK_QUEUE) {
      WaitFreeQueueBenchmarkRunner benchmark(params);
      runBenchmark(benchmark, params);
    }
    else if (params.UnitId() == Unit::KOGAN_PETRANK_QUEUE_PL) {
      WaitFreeQueuePhaselessBenchmarkRunner benchmark(params);
      runBenchmark(benchmark, params);
    }
    else if (params.UnitId() == Unit::WAITFREE_COMPARTMENT_POOL) {
      WaitFreeCompartmentValuePoolBenchmarkRunner benchmark(params);
      runBenchmark(benchmark, params);
    }
    else if (params.UnitId() == Unit::WAITFREE_ARRAY_POOL) {
      WaitFreeArrayValuePoolBenchmarkRunner benchmark(params);
      runBenchmark(benchmark, params);
    }
    else if (params.UnitId() == Unit::LOCKFREE_TREE_POOL) {
      LockFreeTreeValuePoolBenchmarkRunner benchmark(params);
      runBenchmark(benchmark, params);
    }
    else if (params.UnitId() == Unit::WAIT_FREE_SIM_STACK_TAGGED) {
      WaitFreeSimStackTaggedBenchmarkRunner benchmark(params);
      runBenchmark(benchmark, params);
    }
    else if (params.UnitId() == Unit::LOCK_FREE_STACK) {
      LockFreeStackBenchmarkRunner benchmark(params);
      runBenchmark(benchmark, params);
    }
  }
  catch (embb::base::Exception & embbe) { 
    ::std::cerr << "EMBB exception caught: " << embbe.What() << ::std::endl;
    return 1; 
  }
  catch (::std::runtime_error & re) {
    ::std::cerr << "Runtime error caught: " << re.what() << ::std::endl;
    return 2; 
  }
  catch (::std::exception & e) {
    ::std::cerr << "Exception caught: " << e.what() << ::std::endl;
    return 3; 
  }

  return confirmExit();
}
