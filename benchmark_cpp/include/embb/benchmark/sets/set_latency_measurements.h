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

#ifndef EMBB_BENCHMARK_CPP_SETS_SET_LATENCY_MEASUREMENTS_H_
#define EMBB_BENCHMARK_CPP_SETS_SET_LATENCY_MEASUREMENTS_H_

#include <embb/benchmark/call_args.h>
#include <embb/benchmark/fundamental/set_measurements.h>
#include <embb/base/perf/timer.h>
#include <embb/base/perf/duration.h>
#include <utility>
#include <vector>

namespace embb {
namespace benchmark {

/**
 * @brief Latency benchmark implementation for data structures
 *        modified by allocators (e.g. map, set, pool, tree).
 */
class SetLatencyMeasurements : public SetMeasurements {
private:
  typedef embb::base::perf::Timer::timestamp_t timestamp_t; 

public:
  typedef struct {
    int          allocatorId;
    int          poolIndex;
    int          nodeIndex;  
    timestamp_t  timestamp;
  } node_t;
  
  typedef int node_index_t;
  typedef ::std::vector< node_t >       node_pool_t;
  typedef ::std::vector< node_index_t > node_indices_t; 

public:
  static const node_index_t UndefinedElement = static_cast<node_index_t>(-1);
  
protected:
  size_t n_allocators;

public:
  /**
   * Creates a new instance of LatencyBenchmark for operations of pools.
   */
  SetLatencyMeasurements(const CallArgs & callArgs) :
    SetMeasurements(callArgs),
    n_allocators(callArgs.NumProducers()) { 
  }

  virtual ~SetLatencyMeasurements() { }

public:
  size_t NumAllocators() const {
    return n_allocators;
  }
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_SETS_SET_LATENCY_MEASUREMENTS_H_ */
