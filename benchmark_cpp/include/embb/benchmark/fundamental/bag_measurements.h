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

#ifndef EMBB_BENCHMARK_CPP_FUNDAMENTAL_BAG_MEASUREMENTS_H_
#define EMBB_BENCHMARK_CPP_FUNDAMENTAL_BAG_MEASUREMENTS_H_

#include <embb/base/perf/timer.h>
#include <embb/benchmark/call_args.h>
#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/latency_measurements.h>

#include <string>
#include <vector>

namespace embb {
namespace benchmark {

/**
 * Measurements defined for data structures implementing semantics of the fundamental 
 * bag data type. 
 * Used as a container for aggregating latencies in a benchmark run as pairs of 
 * start- and end-timestamps.
 */
class BagMeasurements {
protected:
  ::std::vector< LatencyMeasurements > latenciesAdd;
  ::std::vector< LatencyMeasurements > latenciesRemoveAny;
  CallArgs args;
  size_t maxAddMeasurements;
  size_t maxRemoveAnyMeasurements;
    
public:
  BagMeasurements(const CallArgs & callArgs, size_t numOps = 0)
  : args(callArgs) {
    size_t numAdd, numRemoveAny; 
    if (numOps == 0) {
      numOps = args.NumIterations() * 
               args.NumAllocsPerIt(); 
    }
    if (args.IsProducerConsumerBenchmark()) { 
      maxAddMeasurements       = numOps * args.NumProducers(); 
      maxRemoveAnyMeasurements = numOps * args.NumConsumers(); 
      numAdd       = args.NumProducers(); 
      numRemoveAny = args.NumConsumers(); 
    }
    else {
      maxAddMeasurements       = numOps * args.NumThreads(); 
      maxRemoveAnyMeasurements = numOps * args.NumThreads(); 
      numAdd       = args.NumThreads(); 
      numRemoveAny = args.NumThreads(); 
    }
    for (size_t i = 0; i < numAdd; ++i) {
      LatencyMeasurements addMeasurements(args, maxAddMeasurements);
      latenciesAdd.push_back(addMeasurements);
    }
    for (size_t i = 0; i < numRemoveAny; ++i) {
      LatencyMeasurements removeAnyMeasurements(args, maxRemoveAnyMeasurements);
      latenciesRemoveAny.push_back(removeAnyMeasurements);
    }
    internal::Console::WriteValue<double>(
      "@Add", static_cast<double>(maxAddMeasurements),
      " measurements");
    internal::Console::WriteValue<double>(
      "@RemoveAny", static_cast<double>(maxRemoveAnyMeasurements), 
      " measurements");
  }
  virtual ~BagMeasurements() { }

public:
  inline void MeasureAdd(
    size_t threadId,
    embb::base::perf::Timer::timestamp_t start,
    embb::base::perf::Timer::timestamp_t end) {
    if (threadId >= latenciesAdd.size()) {
      throw std::runtime_error("Thread ID out of range"); 
    }
    embb::base::perf::Duration d;
    d.Start = start;
    d.End   = end;
    latenciesAdd[threadId].Add(d);
  }

  inline void MeasureRemoveAny(
    size_t threadId,
    embb::base::perf::Timer::timestamp_t start,
    embb::base::perf::Timer::timestamp_t end) {
    if (threadId > latenciesRemoveAny.size()) {
      throw std::runtime_error("Thread ID out of range"); 
    }
    embb::base::perf::Duration d;
    d.Start = start;
    d.End   = end;
    latenciesRemoveAny[threadId].Add(d);
  }

  inline const ::std::vector< LatencyMeasurements > & MeasurementsListAdd() const {
    return latenciesAdd;
  }

  inline const ::std::vector< LatencyMeasurements > & MeasurementsListRemoveAny() const {
    return latenciesRemoveAny;
  }
  
  virtual size_t NumOperations() const {
    size_t numAddOps       = 0; 
    size_t numRemoveAnyOps = 0; 

    std::vector<LatencyMeasurements>::const_iterator t_la_it;
    std::vector<LatencyMeasurements>::const_iterator t_la_end = latenciesAdd.end(); 
    for (t_la_it = latenciesAdd.begin(); t_la_it != t_la_end; ++t_la_it) {
      numAddOps += t_la_it->Durations().size();
    }
    std::vector<LatencyMeasurements>::const_iterator t_ra_it;
    std::vector<LatencyMeasurements>::const_iterator t_ra_end = latenciesRemoveAny.end();
    for (t_ra_it = latenciesRemoveAny.begin(); t_ra_it != t_ra_end; ++t_ra_it) {
      numRemoveAnyOps += t_ra_it->Durations().size();
    }
    if (numAddOps > maxAddMeasurements) {
      internal::Console::WriteValue("!!! Add", numAddOps, " prognosed maximum exceeded!");
    }
    if (numRemoveAnyOps > maxRemoveAnyMeasurements) {
      internal::Console::WriteValue("!!! RemoveAny", numRemoveAnyOps, " prognosed maximum exceeded!");
    }
    return numAddOps + numRemoveAnyOps; 
  }
  
  inline virtual const CallArgs & BenchmarkParameters() const {
    return args;
  }
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_FUNDAMENTAL_BAG_MEASUREMENTS_H_ */
