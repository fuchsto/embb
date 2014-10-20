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

#ifndef EMBB_BENCHMARK_CPP_SCHEDULING_WORK_STEALING_BENCHMARK_H_
#define EMBB_BENCHMARK_CPP_SCHEDULING_WORK_STEALING_BENCHMARK_H_

#include <embb/base/perf/timer.h>
#include <embb/base/thread.h>
#include <embb/benchmark/call_args.h>
#include <embb/benchmark/report.h>
#include <embb/benchmark/scheduling/scheduling_latency_measurements.h>

#include <mtapi.h>
#include <embb/mtapi/c/mtapi.h>

namespace embb {
namespace benchmark {

class WorkStealingBenchmark {
public: 
  typedef SchedulingLatencyMeasurements measurements_t;
  typedef SchedulingLatencyMeasurements::element_t element_t;

private:
   typedef WorkStealingBenchmark self_t;
  
private:
  CallArgs        args;
  measurements_t  measurements;

private:
  static unsigned int        lowFreqPriority;
  static unsigned int        highFreqPriority;
  static mtapi_action_hndl_t lowFreqAction;
  static mtapi_job_hndl_t    lowFreqJob;
  static mtapi_action_hndl_t highFreqAction;
  static mtapi_job_hndl_t    highFreqJob;

  static void lowFreqFunction(
    const void * args,
    mtapi_size_t args_size,
    void* result_buffer,
    mtapi_size_t result_buffer_size,
    const void* node_local_data,
    mtapi_size_t node_local_data_size,
    mtapi_task_context_t * context);

  static void highFreqFunction(
    const void * args,
    mtapi_size_t args_size,
    void* result_buffer,
    mtapi_size_t result_buffer_size,
    const void* node_local_data,
    mtapi_size_t node_local_data_size,
    mtapi_task_context_t * context);

  static void highFreqCallback(void * pBuffer, long bufferLen);
  static void lowFreqCallback(void * pBuffer, long bufferLen);

private:
  SchedulingLatencyMeasurements::element_t * elements;

  self_t & operator=(const self_t &); 
  
public:  
   WorkStealingBenchmark(const CallArgs & args);
   ~WorkStealingBenchmark();
  
  /// Starts the benchmark. Selects specific scenario 
  /// implementation to run from settings in call arguments. 
  static void MtapiInit(const CallArgs & args);
  void Run();
  inline const measurements_t & Measurements() const {
    return measurements;
  }

private:
  void RunScenario_20_HighFreqVsBulk();
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_SCHEDULING_WORK_STEALING_BENCHMARK_H_ */
