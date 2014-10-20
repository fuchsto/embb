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

#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/latency_report.h>
#include <embb/benchmark/latency_measurements.h>
#include <embb/base/perf/timer.h>
#include <embb/base/perf/duration.h>

#include <algorithm> 
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <math.h>

namespace embb {
namespace benchmark {

using embb::base::perf::Timer;
using embb::base::perf::Duration;

LatencyReport::LatencyReport(
  const CallArgs & args, 
  const ::std::vector< LatencyMeasurements > & measurementsList) 
: Report(args), 
  measurements(&measurementsList),
  earliestStartTimestamp(Timer::TimestampInfinity()), 
  latestEndTimestamp(Timer::TimestampNegInfinity())
{
  ::std::vector< LatencyMeasurements >::const_iterator c_it;
  ::std::vector< LatencyMeasurements >::const_iterator c_end = measurements->end();
  // Transform list of operation time intervals 
  //   [ threadId -> (timestamp, timestamp)[] ] 
  // to list of operation latencies: 
  //   [ threadId -> double[] ]: 
  for (c_it = measurements->begin(); c_it != c_end; ++c_it) {
    ::std::vector< double > opLatencies;
    ::std::vector<Duration>::const_iterator d_it; 
    ::std::vector<Duration>::const_iterator d_end = c_it->Durations().end();
    for (d_it = c_it->Durations().begin(); d_it != d_end; ++d_it) {
      opLatencies.push_back(Timer::FromInterval(d_it->Start, d_it->End));
      // Scan for earliest start timestamp and latest end timestamp
      // for throughput:  
      
      if (d_it->Start < earliestStartTimestamp) {
        earliestStartTimestamp = d_it->Start; 
      }
      if (d_it->End > latestEndTimestamp) {
        latestEndTimestamp = d_it->End;
      }      
    }
    latencies.push_back(opLatencies);
    // Aggregate list of latencies per thread  
    //   [ threadId -> double[] ] 
    // into list of all latencies: 
    //   double[]
    latenciesAggregated.insert(
      latenciesAggregated.begin(),
      opLatencies.begin(),
      opLatencies.end());
  }  
  // Sort latencies for median: 
  ::std::sort(latenciesAggregated.begin(), latenciesAggregated.end());

  ::std::vector<double>::const_iterator it;
  ::std::vector<double>::const_iterator const end = latenciesAggregated.end();
  latencyMean = 0;
  unsigned int idx = 0;
  double latencyLast = 0;
  for (it = latenciesAggregated.begin(); it != end; ++it, ++idx) {
    if (latencyLast > *it) {
      throw ::std::runtime_error("Latency list is not sorted");
    }
    latencyLast = *it;
    latencyMean += *it; 
  }
  if (latenciesAggregated.size() > 10) {
    // Find element at lower 5% margin: 
    size_t indexMin95 = static_cast<size_t>(
      round(static_cast<float>(latenciesAggregated.size()) * 0.05));
    // Find element at lower 5% margin: 
    size_t indexMax95 = static_cast<size_t>(
      round(static_cast<float>(latenciesAggregated.size()) * 0.95));
    // Measurement at lower 5% margin: 
    latencyMin95 = latenciesAggregated[indexMin95];
    // Measurement at upper 5% margin: 
    latencyMax95 = latenciesAggregated[indexMax95];
    // Measurement in median of all latencies: 
    latencyMedian = latenciesAggregated[latenciesAggregated.size() / 2];
    latencyMin = (latenciesAggregated.size() > 0) ? latenciesAggregated.front() : 0;
    latencyMax = (latenciesAggregated.size() > 0) ? latenciesAggregated.back() : 0;
    latencyMean /= static_cast<double>(latenciesAggregated.size());
  }
  else {
    latencyMin    = 0;
    latencyMax    = 0;
    latencyMin95  = 0;
    latencyMax95  = 0;
    latencyMedian = 0; 
    latencyMean   = 0; 
  }
  unsigned int prec = 4;
  std::stringstream ss; 
  size_t numOps = latenciesAggregated.size(); 
  double throughputTime = Timer::FromInterval(
    earliestStartTimestamp, 
    latestEndTimestamp); 
  double opsPerSec  = static_cast<double>(numOps) / 
    (throughputTime / 1000000.0f);

  ss << 
    std::fixed << std::setprecision(prec) << latencyMin     << "," <<
    std::fixed << std::setprecision(prec) << latencyMin95   << "," <<
    std::fixed << std::setprecision(prec) << latencyMax     << "," <<
    std::fixed << std::setprecision(prec) << latencyMax95   << "," <<
    std::fixed << std::setprecision(prec) << latencyMean    << "," <<
    std::fixed << std::setprecision(prec) << latencyMedian  << "," << 
    std::fixed << std::setprecision(prec) << throughputTime << "," <<
    std::fixed << std::setprecision(prec) << numOps         << "," <<
    std::fixed << std::setprecision(prec) << opsPerSec      << ","; 
  latSummary = ss.str();
}

void LatencyReport::Print() const { 
  unsigned int digits = 6; 
  unsigned int prec   = 3; 
  size_t numOps = latenciesAggregated.size(); 
  double throughputTime = Timer::FromInterval(
    earliestStartTimestamp, 
    latestEndTimestamp); 
  double opsPerSec  = static_cast<double>(numOps) / 
    (throughputTime / 1000000.0f);
  // Print summary to STDOUT: 
  std::cout <<
    "               " <<
    "|         Min |       Min 95 |         Mean " << 
    "|       Median |       Max 95 |          Max " << 
    "| Operations |         Time | Throughput |" << ::std::endl <<
    "               |" <<
    std::fixed << std::setprecision(prec) << std::setw(digits + prec) << latencyMin    << " us | " <<
    std::fixed << std::setprecision(prec) << std::setw(digits + prec) << latencyMin95  << " us | " <<
    std::fixed << std::setprecision(prec) << std::setw(digits + prec) << latencyMean   << " us | " <<
    std::fixed << std::setprecision(prec) << std::setw(digits + prec) << latencyMedian << " us | " <<
    std::fixed << std::setprecision(prec) << std::setw(digits + prec) << latencyMax95  << " us | " <<
    std::fixed << std::setprecision(prec) << std::setw(digits + prec) << latencyMax    << " us | " <<
    std::fixed << std::setw(digits + prec + 1) << numOps << " | " <<
    std::fixed << std::setprecision(prec) << std::setw(digits + prec) << throughputTime / 1000.0f << " ms" << " | " <<
    std::setw(digits + prec + 1) << static_cast<int>(opsPerSec) << " |" << std::endl;
}

void LatencyReport::WriteSamplesToFile(const ::std::string & filepath) const {
  ::std::cout << "-> " << filepath << ::std::endl;
  // Write output in format similar to: 
  // 
  //  T0-S,             T0-E,           T1-S,             T1-E, ...
  //  <time Start op1>, <time End op1>, <time start op1>, <time end op1>, ...
  //  <time Start op2>, <time End op2>, <time start op2>, <time end op2>, ...
  //  ...
  ::std::ofstream file;
  file.open(filepath.c_str(), ::std::ofstream::out | ::std::ofstream::app);
  ::std::vector< LatencyMeasurements >::const_iterator c_it;
  ::std::vector< LatencyMeasurements >::const_iterator c_end = measurements->end();
  // Maximum amount of samples per consumer. Used 
  // to fill up all rows with 0 to uniform amount of 
  // samples. 
  size_t max_len = 0;
  for (c_it = measurements->begin(); c_it != c_end; ++c_it) {
    size_t len = c_it->Durations().size();
    if (max_len < len) {
      max_len = len; 
    }
  }
  // Iterate over threads. Can't avoid cross-iterating here, 
  // as we need threads in columns (observation factors). Also, 
  // CSV must not be wider than 256 columns. Transposing in a 
  // later step would take longer than just cross-iterating here.
  for (unsigned int threadCol = 0; threadCol < measurements->size(); ++threadCol) { 
    // Write column headers in first row: 
    file << "'T" << threadCol << "-S','T" << threadCol << "-E',"; 
  }
  file << std::endl; 
  for (unsigned int row = 0; row < max_len; ++row) { 
    // Write one row with data from all threads in this index. 
    // One measurement expands to two columns, the start and end 
    // timestamps. 
    for (unsigned int threadCol = 0; threadCol < measurements->size(); ++threadCol) { 
      // Pad all threads to same amount of measurements
      if (measurements->at(threadCol).Durations().size() <= row) { 
        file << "0,0,"; 
      }
      else {
        file << (measurements->at(threadCol).Durations().at(row).Start) << ","
             << (measurements->at(threadCol).Durations().at(row).End) << ",";
      }
    }
    file << std::endl;
  }
  file.close();
}

void LatencyReport::WriteSummaryToFile(const ::std::string & filepath, bool) const {
  ::std::ofstream file;
  file.open(filepath.c_str(), ::std::ofstream::out | ::std::ofstream::app);
  file << Summary() << ::std::endl;
  file.close();
}

} // namespace benchmark
} // namespace embb
