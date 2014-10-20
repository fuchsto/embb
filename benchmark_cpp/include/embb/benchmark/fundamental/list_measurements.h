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

#ifndef EMBB_BENCHMARK_CPP_FUNDAMENTAL_LIST_MEASUREMENTS_H_
#define EMBB_BENCHMARK_CPP_FUNDAMENTAL_LIST_MEASUREMENTS_H_

#include <embb/benchmark/latency_measurements.h>
#include <embb/benchmark/internal/call_args.h>
#include <embb/base/perf/timer.h>

#include <vector>

namespace embb {
namespace benchmark {

/**
 * Measurements defined for data structures implementing semantics of the 
 * fundamental list data type.
 * Used as a container for aggregating latencies in a benchmark run as 
 * pairs of start- and end-timestamps.
 */
class ListMeasurements {
protected:
  internal::CallArgs args;
  ::std::vector< LatencyMeasurements > latenciesAddFront;
  ::std::vector< LatencyMeasurements > latenciesAddBack;
  ::std::vector< LatencyMeasurements > latenciesRemoveFront;
  ::std::vector< LatencyMeasurements > latenciesRemoveBack;
  ::std::vector< LatencyMeasurements > latenciesElementAt;
  unsigned int maxAddFrontMeasurements;
  unsigned int maxRemoveFrontMeasurements;
  unsigned int maxAddBackMeasurements;
  unsigned int maxRemoveBackMeasurements;
  unsigned int maxElementAtMeasurements;

public:
  ListMeasurements(const internal::CallArgs & callArgs) :
    args(callArgs) {
    unsigned int numOps = args.NumThreads() * 
                          args.NumIterations() * 
                          args.NumAllocsPerIt(); 
    maxAddFrontMeasurements    = numOps; 
    maxRemoveFrontMeasurements = numOps; 
    maxAddBackMeasurements     = numOps; 
    maxRemoveBackMeasurements  = numOps; 
    maxElementAtMeasurements   = numOps; 
    for (unsigned int i = 0; i < args.NumThreads(); ++i) {
      LatencyMeasurements addFrontMeasurements(args, maxAddFrontMeasurements);
      LatencyMeasurements removeFrontMeasurements(args, maxRemoveFrontMeasurements);
      LatencyMeasurements addBackMeasurements(args, maxAddBackMeasurements);
      LatencyMeasurements removeBackMeasurements(args, maxRemoveBackMeasurements);
      LatencyMeasurements elementAtMeasurements(args, maxElementAtMeasurements);

      latenciesAddFront.push_back(addFrontMeasurements);
      latenciesRemoveFront.push_back(removeFrontMeasurements);
      latenciesAddBack.push_back(addBackMeasurements);
      latenciesRemoveBack.push_back(removeBackMeasurements);
      latenciesElementAt.push_back(elementAtMeasurements);
    }
  }

public:
  inline void MeasureAddFront(unsigned int threadId, const embb::base::perf::Duration & d) {
    latenciesAddFront[threadId].Add(d);
  }
  inline virtual const internal::CallArgs & BenchmarkParameters() const {
    return args;
  }
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_FUNDAMENTAL_LIST_MEASUREMENTS_H_ */
