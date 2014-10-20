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

#ifndef EMBB_BENCHMARK_CPP_QUEUES_QUEUE_BENCHMARK_REPORT_H_
#define EMBB_BENCHMARK_CPP_QUEUES_QUEUE_BENCHMARK_REPORT_H_

#include <embb/benchmark/report.h>
#include <embb/benchmark/call_args.h>
#include <embb/benchmark/latency_report.h>
#include <embb/benchmark/queues/queue_latency_measurements.h>
#include <embb/base/perf/timer.h>

#include <string>

namespace embb {
namespace benchmark {

class QueueBenchmarkReport : public Report {
protected:
  CallArgs args; 
  LatencyReport removeAnyLatencyReport;
  LatencyReport addLatencyReport;
  LatencyReport bufLatencyReport;
  size_t n_ops;
  double throughputTime;

public:
  QueueBenchmarkReport(const QueueLatencyMeasurements & measurements);
  virtual ~QueueBenchmarkReport() { }

public:
  /**
   * Print report stats to STDOUT.
   */
  virtual void Print() const;

  /**
   * Write all benchmark samples (latencies) to file at given path.
   * The file will contain one line per consumer thread, containing
   * a space-separated list of latencies observed for every dequeued
   * element.
   */
  virtual void WriteSamplesToFile(const ::std::string & filepath) const;
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_QUEUES_QUEUE_BENCHMARK_REPORT_H_ */
