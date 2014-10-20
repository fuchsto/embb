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

#ifndef EMBB_BENCHMARK_CPP_REPORT_H_
#define EMBB_BENCHMARK_CPP_REPORT_H_

#include <embb/benchmark/call_args.h>
#include <string>

namespace embb {
namespace benchmark {

class Report {
protected:
  CallArgs callArgs; 
  ::std::string summary;
  ::std::string summaryHeaders; 

public:
  /**
   * Constructor
   */
  Report(const CallArgs & callArgs);
  /**
   * Print report metrics derived from benchmark measurements to STDOUT.
   */
  virtual void Print() const = 0;
  /**
   * Returns a string containing the benchmark summary as a line
   * in CSV format. The summary contains the metrics printed to STDOUT 
   * in Print().
   */
  virtual const ::std::string & Summary() const {
    return summary; 
  }
  /**
   * Returns a string containing header names for the benchmark 
   * summary columns as a line in CSV format. x
   */
  virtual const ::std::string & SummaryHeaders() const { 
    return summaryHeaders; 
  }
  /**
   * Write all benchmark samples (latencies) to file at given path.
   * The file will contain one line per consumer thread, containing
   * a space-separated list of latencies observed for every dequeued
   * element.
   */
  virtual void WriteSamplesToFile(const ::std::string & filepath) const = 0;
  /**
   * Append benchmark summary as a line in CSV format. The summary
   * contains the metrics printed to STDOUT in Print().
   */
  virtual void WriteSummaryToFile(const ::std::string & filepath, bool writeHeaders) const; 
  /** 
   * Destructor.  
   */
  virtual ~Report() { } 

protected:
  virtual void AppendSummaryValue(
      const ::std::string & header, 
      const ::std::string & value); 
  virtual void AppendSummaryValue(
      const ::std::string & header, 
      size_t value); 
  virtual void AppendSummaryValue(
      const ::std::string & header, 
      double value);
  virtual void AppendOperationLatencyReport(
      const ::std::string & op, 
      const Report & report); 
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_REPORT_H_ */
