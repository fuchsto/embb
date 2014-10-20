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

#ifndef EMBB_BENCHMARK_CPP_SCHEDULING_SCHEDULING_LATENCY_MEASURES_H_
#define EMBB_BENCHMARK_CPP_SCHEDULING_SCHEDULING_LATENCY_MEASURES_H_

#include <embb/benchmark/call_args.h>
#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/latency_measurements.h>
#include <embb/base/perf/timer.h>

#include <utility>
#include <vector>

namespace embb {
namespace benchmark {

class SchedulingLatencyMeasurements {
protected:
  ::std::vector< LatencyMeasurements > latenciesHighFreq;
  ::std::vector< LatencyMeasurements > latenciesLowFreq;

  typedef embb::base::perf::Timer::timestamp_t timestamp_t; 

public:
  typedef struct {
    int          threadId;
    int          elementIndex;
    timestamp_t  timestamp;
  } element_t;
  typedef ::std::vector< element_t > element_pool_t;
  
protected:
  CallArgs args;
  size_t maxHighFreqMeasurements; 
  size_t maxLowFreqMeasurements; 
  
public:
  SchedulingLatencyMeasurements(const CallArgs & callArgs) :
    args(callArgs) {
    maxHighFreqMeasurements = callArgs.NumElements(); 
    maxLowFreqMeasurements  = callArgs.NumElements(); 
    for (unsigned int i = 0; i < args.NumThreads(); ++i) {
      LatencyMeasurements highFreqMeasurements(args, maxHighFreqMeasurements);
      LatencyMeasurements lowFreqMeasurements(args, maxLowFreqMeasurements);
      latenciesHighFreq.push_back(highFreqMeasurements);
      latenciesLowFreq.push_back(lowFreqMeasurements);
    }
    internal::Console::WriteValue<double>(
      "@LowFreq", maxLowFreqMeasurements, 
      " measurements");
    internal::Console::WriteValue<double>(
      "@HighFreq", maxHighFreqMeasurements, 
      " measurements");
  }
  virtual ~SchedulingLatencyMeasurements() { }

public:
  inline void MeasureLowFreq(
    unsigned int threadId,
    embb::base::perf::Timer::timestamp_t start,
    embb::base::perf::Timer::timestamp_t end) {
    if (threadId >= args.NumThreads()) {
      throw std::runtime_error("Thread ID out of range"); 
    }
    embb::base::perf::Duration d;
    d.Start = start;
    d.End   = end;
    latenciesLowFreq[threadId].Add(d);
  }

  inline void MeasureHighFreq(
    unsigned int threadId,
    embb::base::perf::Timer::timestamp_t start,
    embb::base::perf::Timer::timestamp_t end) {
    if (threadId >= args.NumThreads()) {
      throw std::runtime_error("Thread ID out of range"); 
    }
    embb::base::perf::Duration d;
    d.Start = start;
    d.End   = end;
    latenciesHighFreq[threadId].Add(d);
  }

  inline const ::std::vector< LatencyMeasurements > & MeasurementsListLowFreq() const {
    return latenciesLowFreq;
  }

  inline const ::std::vector< LatencyMeasurements > & MeasurementsListHighFreq() const {
    return latenciesHighFreq;
  }

  inline virtual const CallArgs & BenchmarkParameters() const {
    return args;
  }
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_SCHEDULING_SCHEDULING_LATENCY_MEASURES_H_ */
