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

#ifndef EMBB_BENCHMARK_CPP_CALL_ARGS_H_
#define EMBB_BENCHMARK_CPP_CALL_ARGS_H_

#include <embb/benchmark/unit.h>
#include <embb/benchmark/scenario.h>
#include <embb/base/perf/timer.h>
#include <embb/base/perf/time_measure.h>

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

namespace embb {
namespace benchmark {

class CallArgs {
private: 
  size_t n_threads; 
  size_t n_cores; 
  size_t n_producers;
  size_t n_consumers;
  size_t n_elements;
  size_t n_iterations; 
  size_t r_prealloc; 
  size_t n_prealloc; 
  size_t n_i_alloc; 
  size_t n_alloc; 
  size_t exec_count; 
  int q_param; 
  Scenario::ScenarioId scenario; 
  Unit::UnitId  unit_id;
  ::std::string summary_file; 
  ::std::string data_file; 
  embb::base::perf::TimeMeasure::MeasureMode timer_type; 
  unsigned int timer_param; 

public: 
  inline bool IsProducerConsumerBenchmark() const {
    return (n_producers > 0 || n_consumers > 0);
  }
  inline bool IsAllocatorBenchmark() const {
    return (n_threads > 0);
  }

public: 
  class InvalidParameterException : public ::std::runtime_error {
  public:
    InvalidParameterException(const ::std::string & msg) : ::std::runtime_error(msg)
    { }
  };

  CallArgs() :
    n_threads(0),
    n_cores(1), 
    n_producers(0),
    n_consumers(0),
    n_elements(0),
    n_iterations(1),
    r_prealloc(0), 
    n_prealloc(0), 
    n_i_alloc(1), 
    n_alloc(1), 
    exec_count(0),
    q_param(0), 
    scenario(Scenario::UNDEFINED),
    unit_id(Unit::UNDEFINED), 
    timer_type(embb::base::perf::TimeMeasure::Counter),
    timer_param(0)
  { }

  CallArgs(int argc, char* argv[]) {
    ParseArgs(argc, argv);
  }

  inline size_t NumThreads() const {
    return (n_threads == 0)
      ? n_consumers + n_producers
      : n_threads; 
  }

  inline size_t NumProducers() const {
    return n_producers; 
  }

  inline size_t NumConsumers() const {
    return n_consumers; 
  }

  inline size_t NumCores() const {
    return n_cores;
  }

  inline size_t NumElements() const {
    return n_elements;
  }

  inline size_t NumIterations() const {
    return n_iterations; 
  }

  inline size_t RPrealloc() const {
    return r_prealloc;  
  }

  inline size_t NPrealloc() const {
    if (n_prealloc > 0) {
      return n_prealloc;  
    }
    if (RPrealloc() > 0) {
      return (NumElements() * RPrealloc()) / 100;
    }
    return 0; 
  }

  inline size_t NumAllocsPerIt() const {
    return n_i_alloc;  
  }

  /// Identifying number of benchmark execution 
  /// within a test series. 
  inline size_t ExecutionCount() const {
    return exec_count;
  }

  inline bool WriteToSummaryFile() const {
    return summary_file.length() > 0; 
  }

  inline bool WriteToDataFile() const {
    return data_file.length() > 0; 
  }

  inline const ::std::string & SummaryFile() const {
    return summary_file;
  }

  inline const ::std::string & DataFile() const {
    return data_file;
  }

  inline Unit::UnitId UnitId() const {
    return unit_id;
  }

  inline Scenario::ScenarioId ScenarioId() const {
    return scenario; 
  }

  inline int QParam() const {
    return q_param;  
  }

  inline embb::base::perf::TimeMeasure::MeasureMode TimerType() const {
    return timer_type;
  }

  inline unsigned int TimerParam() const {
    return timer_param;
  }

public:
  void ParseArgs(int argc, char* argv[]);
  void Print() const;
  static void Usage();

private:
  static void printLn(const ::std::string & line) {
    ::std::cout << line << ::std::endl;
  }

};

} // namespace benchmark
} // namespace embb

#endif // EMBB_BENCHMARK_CPP_CALL_ARGS_H_
