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
  scenario     = 0;
  exec_count   = 0;
  benchmark    = UNDEFINED;
  timer_type   = embb::base::perf::TimeMeasure::Counter;

  if (argc < 2) {
    throw InvalidParameterException("Invalid number of arguments");
  }

  ::std::string type = argv[1];
  int firstFlagIndex = 2;

  if (type == "test") {
    // Self-test requested. 
    // Ignore other parameters: 
    benchmark = SELF_TEST;
    if (argc == 2) {
      return;
    }
  }

  // Expecting benchmarked type as first arguments and 
  // at least 2 benchmark parameters each consisting of 
  // 1 flag and 1 parameter value: 
  if (benchmark != SELF_TEST && argc < (1 + 1 + (2 * 2))) {
    throw InvalidParameterException("Invalid number of arguments");
  }

  else if (type == "michaelscott") {
    benchmark = MICHAEL_SCOTT_QUEUE_TP;
  }
  else if (type == "michaelscott-ap") {
    benchmark = MICHAEL_SCOTT_QUEUE_AP;
  }
  else if (type == "koganpetrank") {
    benchmark = KOGAN_PETRANK_QUEUE;
  }
  else if (type == "koganpetrank-pl") {
    benchmark = KOGAN_PETRANK_QUEUE_PL;
  }
  else if (type == "treepool") {
    benchmark = LOCKFREE_TREE_POOL;
  }
  else if (type == "arraypool") {
    benchmark = WAITFREE_ARRAY_POOL;
  }
  else if (type == "compartmentpool") {
    benchmark = WAITFREE_COMPARTMENT_POOL;
  }
  else if (type == "harrislistset") {
    benchmark = HARRIS_LIST_SET;
  }
  else if (type == "simstack-t") {
    benchmark = WAIT_FREE_SIM_STACK_TAGGED;
  }
  else if (type == "lockfreestack") {
    benchmark = LOCK_FREE_STACK;
  }
  
  for (int flagIdx = firstFlagIndex; flagIdx < argc; flagIdx += 2) {
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
        scenario = atoi(param.c_str());
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
}

void CallArgs::Usage() {
  printLn("Usage: embb_base_cpp_benchmark test | <type> <type parameters> -n <elements> -f <output file>");
  printLn("  ");
  printLn("  -s                 scenario");
  printLn("  -n                 num element");
  printLn("  -i                 num iterations");
  printLn("  -k                 benchmark execution number");
  printLn("  -data              measurements output file");
  printLn("  -summary           summary output file");
  printLn("  ");
  printLn("Queue types: ");
  printLn("  michaelscott     - lock-free - a popular lock-free queue, as a benchmark reference");
  printLn("  koganpetrank     - wait-free - according to Kogan and Petrank, with hazard pointers");
  printLn("  koganpetrank-pl  - wait-free - Kogan-Petrank queue without phase");
  printLn(" ");
  printLn("  -p                 num producers");
  printLn("  -c                 num consumers");
  printLn("Queue scenarios: ");
  printLn("   - 0: { Enqueue( n ) }:p || { Dequeue( n ) }:c   - typical continuous stream");
  printLn("   - 1: { (Enqueue(ia); Dequeue(ia))*i }:t         - enqueue-dequeue pairs standard test");
  printLn("  ");
  printLn("x    2: { Enqueue(n/2) };                          - fill queue up to half capacity");
  printLn("        { Enqueue(n/2) }:p || { Dequeue( i ) }:c   - c consumers try to dequeue i elements");
  printLn("                                                   - p prodcuers try to enqueue n/2 elements");
  printLn("                                                   - evaluate for p < c, p > c, i = n");
  printLn("x    3: { Enqueue(n/2) };                          - fill queue up to half capacity");
  printLn("        { random(Enqueue(1) | Dequeue(1)) * i }:t  - random decision for enqueue or dequeue");
  printLn("  ");
  printLn("Stack types: ");
  printLn("  freelist         - lock-free - from the IBM freelist");
  printLn("  waitfreelist     - wait-free - wait-free simulation of the freelist algorithm");
  printLn("  eliminationstack - wait-free - wait-free simulation the lock-free elimination stack");
  printLn("  ");
  printLn("  -p                 num producers");
  printLn("  -c                 num consumers");
  printLn("Stack scenarios: ");
  printLn("x    0: { Push( n ) }:p || { Pop( n ) }:c          - typical messaging with decaying messages");
  printLn("  ");
  printLn("x    1: { Push(n/2) };                             - fill queue up to half capacity");
  printLn("        { Push(n/2) }:p || { Pop( i ) }:c          - c consumers try to dequeue i elements");
  printLn("                                                   - p prodcuers try to enqueue n/2 elements");
  printLn("                                                   - evaluate for p < c and p > c");
  printLn("                                                   - evaluate for p < c, p > c, i = n");
  printLn("  ");
  printLn("Pool types: ");
  printLn("  treepool         - lock-free - value pool based on a tree structure");
  printLn("  arraypool        - wait-free - value pool based on an index array");
  printLn("  compartmentpool  - wait-free - like array pool, but with thread-specific scan pattern");
  printLn("  -t                 num threads");
  printLn("  -ia                num allocations per iteration");
  printLn("  -r                 pre-allocation ratio (decimal part)");
  printLn("Pool scenarios: ");
  printLn("  ");
  printLn("   - 0: enq-deq pairs");
  printLn("        { Alloc(r*n) }; ");
  printLn("        { ||(Alloc(ia); Dealloc(ia))*i   }:t ");
  printLn("  ");
  printLn("   - 1: enq-deq bulk");
  printLn("        { (Alloc(r*n);Sleep())*forever } || { (Alloc(ia); Dealloc(ia))*i }:t ");
  printLn("  ");
  printLn("   - 2: fill-up");
  printLn("        { ||(Alloc(n/t) }:t ");
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
  Console::WriteValue("Preallocation", r_prealloc, "%");
  Console::WriteValue("Allocs / It",   n_i_alloc);
  Console::WriteValue("Scenario",      scenario);
  Console::WriteValue("Execution No.", exec_count);
}

} // namespace benchmark
} // namespace embb