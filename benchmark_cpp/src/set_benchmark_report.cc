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

#if 0

#include <embb/benchmarksets/set_benchmark_report.h>

#include <iomanip>
#include <sstream>
#include <fstream>


namespace embb {
namespace benchmark {

using embb::base::perf::Timer;

SetBenchmarkReport::
SetBenchmarkReport(const SetLatencyMeasurements & measurements)
: Report(measurements.BenchmarkParameters()), 
  args(measurements.BenchmarkParameters()),
  removeLatencyReport(
    args, 
    measurements.MeasurementsListRemove()),
  addLatencyReport(
    args,
    measurements.MeasurementsListAdd()),
  containsLatencyReport(
    args, 
    measurements.MeasurementsListContains()),
  n_ops(measurements.NumOperations())
{
  throughputTime = Timer::FromInterval(
    addLatencyReport.EarliestStartTimestamp(),
    removeLatencyReport.LatestEndTimestamp());

  double opsPerSec  = static_cast<double>(n_ops) / 
    (throughputTime / 1000000.0f);
  this->AppendSummaryValue("throughputTime", throughputTime);
  this->AppendSummaryValue("numOpsTotal",    n_ops);
  this->AppendSummaryValue("opsPerSec",      opsPerSec); 
  this->AppendOperationLatencyReport(
      "remove", 
      removeLatencyReport);
  this->AppendOperationLatencyReport(
      "add", 
      addLatencyReport); 
  this->AppendOperationLatencyReport(
      "contains", 
      containsLatencyReport);
}

void SetBenchmarkReport::
Print() const {
  double opsPerSec = static_cast<double>(n_ops) / 
    (throughputTime / 1000000.0f); 
  unsigned int prec = 3;
  size_t numRemoveOps   = removeLatencyReport.LatenciesAggregated().size();
  size_t numAddOps      = addLatencyReport.LatenciesAggregated().size();
  size_t numContainsOps = containsLatencyReport.LatenciesAggregated().size();

  ::std::cout << "Add -----------|---------------------------" << std::endl;
  ::std::cout << "               | " << std::setw(21) << numAddOps << " ops" << std::endl;
  
  addLatencyReport.Print();
  
  ::std::cout << "Remove --------|---------------------------" << std::endl;
  ::std::cout << "               | " << std::setw(21) << numRemoveOps << " ops" << std::endl;
  removeLatencyReport.Print();
  
  ::std::cout << "Contains ------|---------------------------" << std::endl;
  ::std::cout << "               | " << std::setw(21) << numContainsOps << " ops" << std::endl;
  containsLatencyReport.Print();

  ::std::cout << "Total ---------|---------------------------" << std::endl;
  ::std::cout << "               | Operations " 
            << std::setw(7 + prec) << n_ops << std::endl;
  ::std::cout << "               | Time       " 
            << std::fixed << std::setprecision(prec) << std::setw(7 + prec) 
            << throughputTime / 1000.0f << " ms" << std::endl;
  ::std::cout << "               | Throughput " 
            << std::setw(7 + prec) << static_cast<int>(opsPerSec) << " e/s" 
            << std::endl;
}

void SetBenchmarkReport::
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
  addLatencyReport.WriteSamplesToFile(
    baseFilename + ".Add" + fileExtension);
  removeLatencyReport.WriteSamplesToFile(
    baseFilename + ".Remove" + fileExtension);
  containsLatencyReport.WriteSamplesToFile(
    baseFilename + ".Contains" + fileExtension);
}

} // namespace benchmark
} // namespace embb

#endif