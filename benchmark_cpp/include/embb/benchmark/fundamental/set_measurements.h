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

#ifndef EMBB_BENCHMARK_CPP_FUNDAMENTAL_SET_BENCHMARK_H_
#define EMBB_BENCHMARK_CPP_FUNDAMENTAL_SET_BENCHMARK_H_

#include <embb/benchmark/call_args.h>
#include <embb/benchmark/fundamental/bag_measurements.h>
#include <embb/base/perf/timer.h>

#include <vector>

namespace embb {
namespace benchmark {

/**
 * @brief Measurements defined for data structures implementing
 *        semantics of the fundamental bag data type.
 *        Used as a container for aggregating latencies in a
 *        benchmark run as pairs of start- and end-timestamps.
 */
class SetMeasurements {
protected:
  ::std::vector< LatencyMeasurements > latenciesAdd;
  ::std::vector< LatencyMeasurements > latenciesRemove;
  ::std::vector< LatencyMeasurements > latenciesContains;

  CallArgs args;
  size_t maxAddMeasurements;
  size_t maxRemoveMeasurements;
  size_t maxContainsMeasurements;

public:
  SetMeasurements(const CallArgs & callArgs) 
  : args(callArgs) {
    size_t numOps = args.NumThreads() * 
                          args.NumIterations() * 
                          args.NumAllocsPerIt(); 
    maxAddMeasurements      = numOps; 
    maxRemoveMeasurements   = numOps; 
    maxContainsMeasurements = numOps; 


    for (unsigned int i = 0; i < callArgs.NumThreads(); ++i) {
      LatencyMeasurements addMeasurements(args, maxAddMeasurements);
      LatencyMeasurements removeMeasurements(args, maxRemoveMeasurements);
      LatencyMeasurements containsMeasurements(args, maxContainsMeasurements);

      latenciesAdd.push_back(addMeasurements);
      latenciesRemove.push_back(removeMeasurements);
      latenciesContains.push_back(containsMeasurements);
    }
  }
  virtual ~SetMeasurements() { }

public:

  inline virtual void MeasureAdd(
    unsigned int threadId, 
    embb::base::perf::Timer::timestamp_t start,
    embb::base::perf::Timer::timestamp_t end)
  {
    embb::base::perf::Duration d;
    d.Start = start;
    d.End   = end;
    latenciesAdd[threadId].Add(d);
  }

  inline virtual void MeasureRemove(
    unsigned int threadId, 
    embb::base::perf::Timer::timestamp_t start,
    embb::base::perf::Timer::timestamp_t end)
  {
    embb::base::perf::Duration d;
    d.Start = start;
    d.End = end;
    latenciesRemove[threadId].Add(d);
  }

  inline virtual void MeasureContains(
    unsigned int threadId, 
    embb::base::perf::Timer::timestamp_t start,
    embb::base::perf::Timer::timestamp_t end)
  {
    embb::base::perf::Duration d;
    d.Start = start;
    d.End = end;
    latenciesContains[threadId].Add(d);
  }

  inline virtual const ::std::vector< LatencyMeasurements > & MeasurementsListAdd() const {
    return latenciesAdd;
  }

  inline virtual const ::std::vector< LatencyMeasurements > & MeasurementsListRemove() const {
    return latenciesRemove;
  }

  inline virtual const ::std::vector< LatencyMeasurements > & MeasurementsListContains() const {
    return latenciesContains;
  }
  
  inline virtual size_t NumOperations() const {
    size_t numOps = 0;
    std::vector<LatencyMeasurements>::const_iterator t_la_it;
    std::vector<LatencyMeasurements>::const_iterator t_la_end = latenciesAdd.end(); 
    for (t_la_it = latenciesAdd.begin(); t_la_it != t_la_end; ++t_la_it) {
      numOps += t_la_it->Durations().size();
    }
    std::vector<LatencyMeasurements>::const_iterator t_ra_it;
    std::vector<LatencyMeasurements>::const_iterator t_ra_end = latenciesRemove.end();
    for (t_ra_it = latenciesRemove.begin(); t_ra_it != t_ra_end; ++t_ra_it) {
      numOps += t_ra_it->Durations().size();
    }
    std::vector<LatencyMeasurements>::const_iterator t_ca_it;
    std::vector<LatencyMeasurements>::const_iterator t_ca_end = latenciesContains.end();
    for (t_ca_it = latenciesContains.begin(); t_ca_it != t_ca_end; ++t_ca_it) {
      numOps += t_ca_it->Durations().size();
    }
    return numOps; 
  }

  inline virtual const CallArgs & BenchmarkParameters() const {
    return args;
  }
};

class MultisetMeasurements : public SetMeasurements 
{ 

public:
  MultisetMeasurements(const CallArgs & callArgs)
    : SetMeasurements(callArgs) { 
  }
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_FUNDAMENTAL_SET_BENCHMARK_H_ */
