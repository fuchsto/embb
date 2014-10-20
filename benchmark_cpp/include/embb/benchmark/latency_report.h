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

#ifndef EMBB_BENCHMARK_CPP_LATENCY_REPORT_H_
#define EMBB_BENCHMARK_CPP_LATENCY_REPORT_H_

#include <embb/benchmark/report.h>
#include <embb/benchmark/latency_measurements.h>

#include <embb/base/perf/timer.h>

#include <string>
#include <vector>
#include <memory>

namespace embb {
namespace benchmark {

/**
 * @brief Utility class for generating metrics from measurements and rendering 
 *        them in several formats. 
 */
class LatencyReport : public Report {
protected:
  unsigned int n_elements;
  double latencyMin;
  double latencyMin95;
  double latencyMax;
  double latencyMax95;
  double latencyMean;
  double latencyMedian;

  ::std::auto_ptr< const ::std::vector< LatencyMeasurements > > measurements;
  ::std::vector< ::std::vector<double> > latencies;
  ::std::vector<double> latenciesAggregated;
  embb::base::perf::Timer::timestamp_t earliestStartTimestamp;
  embb::base::perf::Timer::timestamp_t latestEndTimestamp;
  ::std::string latSummary; 
  ::std::string latSummaryHeaders; 

public:
  /**
   * @brief Construct a report object from given list of sorted latencies.
   *
   * @param [in] latencies  Ascending list of latencies
   */
  LatencyReport(
    const CallArgs & args,
    const ::std::vector< LatencyMeasurements > & measurements);
  virtual ~LatencyReport() { 
    measurements.release();
  }
    
public: 
  inline const ::std::vector< LatencyMeasurements > & Measurements() const {
    return *measurements;
  }
  inline const ::std::vector< ::std::vector<double> > & Latencies() const {
    return latencies; 
  }
  inline const ::std::vector<double> & LatenciesAggregated() const {
    return latenciesAggregated; 
  }
  inline embb::base::perf::Timer::timestamp_t EarliestStartTimestamp() const {
    return earliestStartTimestamp; 
  }
  inline embb::base::perf::Timer::timestamp_t LatestEndTimestamp() const {
    return latestEndTimestamp;
  }

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
  /**
   * Append the benchmark summary to file at given path.
   * The summary contains the metrics printed to STDOUT in Print(). 
   * Appends one line per operator metrics group.
   */
  virtual void WriteSummaryToFile(const ::std::string & filepath, bool writeHeaders) const; 
  /**
   * Returns a string containing the benchmark summary as a single 
   * line in CSV format.
   * The summary contains the metrics printed to STDOUT in Print(). 
   */
  inline virtual const ::std::string & Summary() const { 
    return latSummary; 
  }
  /**
   * Returns a string containing header names for the benchmark 
   * summary columns as a line in CSV format. 
   */
  virtual const ::std::string & SummaryHeaders() const {
    return latSummaryHeaders; 
  }
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_LATENCY_REPORT_H_ */
