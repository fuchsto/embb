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

#include <embb/benchmark/scenario.h>
#include <embb/benchmark/unit.h>
#include <embb/benchmark/call_args.h>
#include <embb/benchmark/internal/console.h>
#include <embb/base/perf/time_measure.h>

namespace embb {
namespace benchmark {

using internal::Console;

void CallArgs::ParseArgs(int argc, char* argv[]) {

  // Reset all settings: 
  n_threads    = 0; 
  n_producers  = 0;
  n_consumers  = 0;
  n_elements   = 0;
  n_iterations = 1;
  r_prealloc   = 0;
  n_prealloc   = 0;
  n_i_alloc    = 1;
  q_param      = 0;
  exec_count   = 0;
  scenario     = Scenario::UNDEFINED;
  unit_id      = Unit::UNDEFINED;
  timer_type   = embb::base::perf::TimeMeasure::Counter;

  if (argc < 2) {
    Usage();
    return; 
  }

  ::std::string type = argv[1];

  unit_id = Unit::FromUnitName(type);

  if (unit_id == Unit::UNDEFINED) {
    throw InvalidParameterException("Unknown test unit (data structure)");
  }

  if (unit_id == Unit::SELF_TEST && argc == 2) {
    // Self-test requested, no more mandatory parameters
    return;
  }

  // Expecting benchmarked type as first arguments and 
  // at least 2 benchmark parameters each consisting of 
  // 1 flag and 1 parameter value: 
  if (unit_id != Unit::SELF_TEST && argc < (1 + 1 + (2 * 2))) {
    throw InvalidParameterException("Invalid number of arguments");
  }

  for (int flagIdx = 2; flagIdx < argc; flagIdx += 2) {
    int paramIdx = flagIdx + 1;
    // Check if param provided for flag: 
    if (paramIdx < argc) {
      ::std::string flag = argv[flagIdx];
      ::std::string param = argv[paramIdx];

      if (flag == "-k") {
        exec_count = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-n") {
        n_elements = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-p") {
        if (n_threads > 0) {
          throw InvalidParameterException("Cannot set both -t and -p");
        }
        n_producers = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-c") {
        if (n_threads > 0) {
          throw InvalidParameterException("Cannot set both -t and -c");
        }
        n_consumers = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-nc") {
        n_cores = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-t") {
        if (n_consumers > 0 || n_producers > 0) {
          throw InvalidParameterException("Cannot set both -t and -c or -p");
        }
        n_threads = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-s") {
        scenario = static_cast<Scenario::ScenarioId>(atoi(param.c_str()));
      }
      else if (flag == "-i") {
        n_iterations = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-ia") {
        n_i_alloc = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-rpre") {
        r_prealloc = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-npre") {
        n_prealloc = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-q") {
        q_param = atoi(param.c_str());
      }
      else if (flag == "-timer") {
        if (std::string(param.c_str()) == "clock") {
          timer_type = embb::base::perf::TimeMeasure::Clock;
        }
        else {
          timer_type = embb::base::perf::TimeMeasure::Counter;
        }
      }
      else if (flag == "-timerflag") {
        timer_param = static_cast<unsigned int>(atoi(param.c_str()));
      }
      else if (flag == "-data") {
        data_file = param.c_str();
      }
      else if (flag == "-summary") {
        summary_file = param.c_str();
      }
      else {
        ::std::ostringstream ss;
        ss << "Unknown flag '" << flag << "'";
        throw InvalidParameterException(ss.str());
      }
    }
    else {
      throw InvalidParameterException("Parameter missing");
    }
  }
  // Validate params:
  if (IsProducerConsumerBenchmark() && 
      !Scenario::IsProducerConsumerScenario(scenario)) {
    throw InvalidParameterException(
      "Scenario requires -t <nThreads>");
  }
  if (IsAllocatorBenchmark() && 
      !Scenario::IsAllocatorScenario(scenario)) {
    throw InvalidParameterException(
      "Scenario requires -p <nProducers> -c <nConsumers>");
  }
  Print();
}

void CallArgs::Usage() {
  printLn("Usage: embb_benchmark_cpp test | <type> <flags>");
  printLn(" ");
  printLn("  -s          scenario");
  printLn("  -n          container capacity");
  printLn("  -i          number of iterations");
  printLn("  -ia         number of allocations per iteration");
  printLn("  -k          benchmark execution number");
  printLn("  -t          number of threads");
  printLn("  -p          number of producer threads (forbids -t, requires -c)");
  printLn("  -c          number of consumer threads (forbids -t, requires -p)");
  printLn("  -rpre       pre-allocation ratio (percentage)");
  printLn("  -npre       pre-allocation (number of elements)");
  printLn("  -data       measurements output file");
  printLn("  -summary    summary output file");
  printLn("  -timer      select timer type 'clock' or 'counter' (default)");
  printLn("  -timerflag  set timer-specific flag. See output of self test for options");
  printLn(" ");
  printLn("Scenarios:");
  printLn("   0: { Enqueue( n ) }:p || { Dequeue( n ) }:c   - typical continuous stream");
  printLn(" ");
  printLn("   1: { (Enqueue(ia); Dequeue(ia))*i }:t         - enqueue-dequeue pairs standard test");
  printLn(" ");
  printLn("   2: { Enqueue(<prealloc>) };                   - preallocate elements");
  printLn("      { Enqueue(n/2) }:p || { Dequeue( ia ) }:c  - c consumers try to dequeue i elements");
  printLn("                                                 - p prodcuers try to enqueue n/2 elements");
  printLn(" ");
  printLn("   3: { Enqueue(ia) }:p || { Dequeue( ia ) }:c   - c consumers try to dequeue ia element,");
  printLn("                                                 - p prodcuers try to enqueue ia elements");
  printLn("  ");
  printLn("Pool types: ");
  printLn("  treepool         - lock-free - value pool based on a tree structure");
  printLn("  arraypool        - wait-free - value pool based on an index array");
  printLn("  compartmentpool  - wait-free - like array pool, but with thread-specific scan pattern");
  printLn("Scenarios: 0 1 2 3");
  printLn("  ");
  printLn("Queue types: ");
  printLn("   michaelscott    - lock-free - a popular lock-free queue, as a benchmark reference");
  printLn("   michaelscott-ap - lock-free - Michael-Scott queue using array-based pool");
  printLn("   koganpetrank    - wait-free - according to Kogan and Petrank, with hazard pointers");
  printLn("   koganpetrank-pl - wait-free - Kogan-Petrank queue without phase counter");
  printLn("Scenarios: 0 1 2 3 4");
  printLn("  ");
  printLn("Stack types: ");
  printLn("   lockfreestack   - lock-free - Treiber's stack");
  printLn("   simstack        - wait-free - based on the P-SIM universal construction");
  printLn("   simstack-t      - wait-free - with tagged pointers instead of hazard pointers");
  printLn("Scenarios: 0 1 2 3 4");
  printLn("  ");
}

void CallArgs::Print() const {
  if (IsProducerConsumerBenchmark()) {
    Console::WriteValue("Producers", NumProducers());
    Console::WriteValue("Consumers", NumConsumers());
  }
  else {
    Console::WriteValue("Threads", NumThreads());
  }
  Console::WriteValue("Capacity",      n_elements);
  Console::WriteValue("Iterations",    n_iterations);
  Console::WriteValue("Preallocation", r_prealloc, " %");
  Console::WriteValue("            =", n_prealloc, " elements");
  Console::WriteValue("Allocs / It",   n_i_alloc);
}

} // namespace benchmark
} // namespace embb
