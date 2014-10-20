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

#ifndef EMBB_BENCHMARK_CPP_LATENCY_MEASUREMENTS_H_
#define EMBB_BENCHMARK_CPP_LATENCY_MEASUREMENTS_H_

#include <embb/benchmark/call_args.h>
#include <embb/base/perf/duration.h>

#include <utility>
#include <vector>

namespace embb {
namespace benchmark {

/**
 * @brief Generic latency measures base class.
 *        Used as a container for aggregating latencies in a
 *        benchmark run as pairs of start- and end-timestamps.
 */
class LatencyMeasurements {

private:
  typedef LatencyMeasurements self_t;

protected:
  CallArgs args;
  size_t   max_measurements;
  /// List of measured durations for a specific operation
  //  as pairs of start- and end-timestamps.
  ::std::vector< embb::base::perf::Duration > durations;

public:
  /// Creates a new instance of a generic latency benchmark
  /// for a given number of container elements.
  LatencyMeasurements(const CallArgs & callArgs, size_t maxMeasurements) :
    args(callArgs),
    max_measurements(maxMeasurements) {
    durations.reserve(max_measurements);
  }

public:
  inline const ::std::vector< embb::base::perf::Duration > & Durations() const {
    return durations;
  }
  inline void Add(const embb::base::perf::Duration & d) {
    durations.push_back(d);
  }
  size_t MaxMeasurements() const {
    return max_measurements;
  }
  inline const CallArgs & BenchmarkParameters() const {
    return args;
  }

};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_LATENCY_MEASUREMENTS_H_ */
