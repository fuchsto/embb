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

#include <embb/benchmark/scheduling/scheduling_benchmark_report.h>
#include <embb/base/perf/timer.h>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <fstream>

namespace embb {
namespace benchmark {

using embb::base::perf::Timer;
using ::std::min;
using ::std::max;

SchedulingBenchmarkReport::
SchedulingBenchmarkReport(const SchedulingLatencyMeasurements & measurements)
: Report(measurements.BenchmarkParameters()), 
  args(measurements.BenchmarkParameters()),
  highFreqLatencyReport(
    args, 
    measurements.MeasurementsListHighFreq()),
  lowFreqLatencyReport(
    args, 
    measurements.MeasurementsListLowFreq())
{
  throughputTime = Timer::FromInterval(
    min(
      highFreqLatencyReport.EarliestStartTimestamp(),
      lowFreqLatencyReport.EarliestStartTimestamp()),
    max(
      highFreqLatencyReport.LatestEndTimestamp(),
      lowFreqLatencyReport.LatestEndTimestamp()));
  
  double opsPerSec  = static_cast<double>(n_ops) / 
    (throughputTime / 1000000.0f);
  // Headers: 
  this->AppendSummaryValue("throughputTime", throughputTime);
  this->AppendSummaryValue("numOpsTotal",    n_ops);
  this->AppendSummaryValue("opsPerSec",      opsPerSec); 
  this->AppendOperationLatencyReport(
      "lowFreq", 
      lowFreqLatencyReport);
  this->AppendOperationLatencyReport(
      "highFreq", 
      highFreqLatencyReport);
}

void SchedulingBenchmarkReport::
Print() const {
  size_t highFreqOps = highFreqLatencyReport.LatenciesAggregated().size();
  size_t lowFreqOps  = lowFreqLatencyReport.LatenciesAggregated().size();

  std::cout << "HighFreq ------|---------------------------" << std::endl;
  std::cout << "               | " << std::setw(21) << highFreqOps << " ops" << std::endl;
  highFreqLatencyReport.Print();
  
  std::cout << "LowFreq -------|---------------------------" << std::endl;
  std::cout << "               | " << std::setw(21) << lowFreqOps << " ops" << std::endl;
  lowFreqLatencyReport.Print();
}

void SchedulingBenchmarkReport::
WriteSamplesToFile(const ::std::string & filepath) const {
  size_t dotPos = filepath.find_last_of('.'); 
  std::string baseFilename; 
  std::string fileExtension; 
  if (dotPos == std::string::npos) {
    baseFilename  = filepath; 
    fileExtension = ".dat";
  }
  else {
    baseFilename  = filepath.substr(0, dotPos);
    fileExtension = (filepath.substr(dotPos));
  }
  // @TODO
}

} // namespace benchmark
} // namespace embb