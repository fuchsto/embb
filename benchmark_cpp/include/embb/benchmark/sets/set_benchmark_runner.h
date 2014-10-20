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

#ifndef EMBB_BENCHMARK_CPP_SETS_SET_BENCHMARK_RUNNER_H_
#define EMBB_BENCHMARK_CPP_SETS_SET_BENCHMARK_RUNNER_H_

#include <embb/benchmark/report.h>
#include <embb/benchmark/call_args.h>
#include <embb/benchmark/benchmark_runner.h>
#include <embb/benchmark/sets/set_benchmark.h>
#include <embb/benchmark/sets/set_benchmark_report.h>

#if 0

#include <embb/containers/harris_list_set.h>

namespace embb {
namespace benchmark {

/**
 * Type adapter class for PoolLatencyBenchmark< CircularObjectPool<...> >
 */
class HarrisListSetBenchmarkRunner: public BenchmarkRunner{
  
public:
  typedef embb::containers::HarrisListSet< SetLatencyMeasurements::node_index_t >
    concrete_set_t;
  typedef SetBenchmark< concrete_set_t > benchmark_t;

private: 
  CallArgs         args;
  concrete_set_t * set;
  benchmark_t *    benchmark;

public:
  HarrisListSetBenchmarkRunner(const CallArgs & args);
  virtual ~HarrisListSetBenchmarkRunner() { }
  virtual ::std::auto_ptr< embb::benchmark::Report > Run();

};

} // namespace benchmark
} // namespace embb

#endif

#endif /* EMBB_BENCHMARK_CPP_SETS_SET_BENCHMARK_RUNNER_H_ */
