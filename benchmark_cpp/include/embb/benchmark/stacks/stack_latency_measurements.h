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

#ifndef EMBB_BENCHMARK_CPP_STACKS_STACK_LATENCY_MEASUREMENTS_H_
#define EMBB_BENCHMARK_CPP_STACKS_STACK_LATENCY_MEASUREMENTS_H_

#include <embb/benchmark/call_args.h>
#include <embb/benchmark/fundamental/bag_measurements.h>
#include <embb/base/perf/timer.h>
#include <embb/base/perf/duration.h>

#include <utility>
#include <vector>
#include <stdint.h>

namespace embb {
namespace benchmark {

/**
 * Latency benchmark implementation for data structures
 * modified by producers and consumers (e.g. queue).
 */
class StackLatencyMeasurements : public BagMeasurements {
private:
  typedef embb::base::perf::Timer::timestamp_t timestamp_t;

public:
  typedef uint32_t element_t;

public:
  /**
   * Creates a new instance of LatencyBenchmark for
   * operations of producers and consumers (i.e. queues).
   */
  StackLatencyMeasurements(const CallArgs & callArgs) :
    BagMeasurements(callArgs) {
  }
  virtual ~StackLatencyMeasurements() { }
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_STACKS_STACK_LATENCY_MEASUREMENTS_H_ */
