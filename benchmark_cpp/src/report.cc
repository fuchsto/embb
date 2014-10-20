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

#include <embb/benchmark/call_args.h>
#include <embb/benchmark/report.h>
#include <embb/base/perf/timer.h>
#include <embb/base/perf/duration.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace embb {
namespace benchmark {

Report::
Report(const CallArgs & callArgs) 
: callArgs(callArgs) {
  summaryHeaders.append("type,scenario,numElements,numThreads,numProducers,numConsumers,"); 
  summaryHeaders.append("numIterations,numAllocsPerIt,rPrealloc,nPrealloc,");
}

void Report::
WriteSummaryToFile(const ::std::string & filepath, bool writeHeaders) const {
  ::std::ofstream file;
  file.open(filepath.c_str(), ::std::ofstream::out | ::std::ofstream::app);
  if (writeHeaders) {
    file << summaryHeaders << std::endl;
  }
  file << callArgs.UnitId()         << ","
       << callArgs.ScenarioId()     << ","
       << callArgs.NumElements()    << ","
       << callArgs.NumThreads()     << ","
       << callArgs.NumProducers()   << ","
       << callArgs.NumConsumers()   << ","
       << callArgs.NumIterations()  << ","
       << callArgs.NumAllocsPerIt() << ","
       << callArgs.RPrealloc()      << ","
       << callArgs.NPrealloc()      << ","
       << Summary() 
       << ::std::endl;
  file.close();
}

#if 0
  template<typename... Args> inline void AppendSummaryHeaders(Args&&... headers)
  virtual void AppendSummaryHeaders(const ::std::string & headers...) {
    pass(AppendSummaryHeader(headers)... ); 
  }
#endif

void Report::
AppendSummaryValue(
  const ::std::string & header, 
  const ::std::string & value) {
  summaryHeaders.append(header); 
  summaryHeaders.append(","); 
  summary.append(value); 
  summary.append(","); 
}

void Report::
AppendSummaryValue(
const ::std::string & header,
size_t value) {
  ::std::stringstream ss;
  ss << value;
  AppendSummaryValue(header, ss.str());
}

void Report::
AppendSummaryValue(
  const ::std::string & header, 
  double value) {
  ::std::stringstream ss;
  ss << ::std::fixed << std::setprecision(4) << value; 
  AppendSummaryValue(header, ss.str());
}

void Report::
AppendOperationLatencyReport(
  const ::std::string & op, 
  const Report & report) {
  ::std::stringstream ss; 
  ss << op << "LatMin,"  << op << "LatMin95," << op << "LatMax," << op << "LatMax95,"
     << op << "LatMean," << op << "LatMedian,"
     << op << "ThroughputTime," << op << "NumOps," << op << "OpsPerSec,";
  summaryHeaders.append(ss.str()); 
  summary.append(report.Summary()); 
}

} // namespace benchmark
} // namespace embb
