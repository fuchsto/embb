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

#ifndef EMBB_BASE_CPP_PERF_PERFORMANCE_MONITOR_H_
#define EMBB_BASE_CPP_PERF_PERFORMANCE_MONITOR_H_

#include <papi.h>
#include <embb/base/perf/performance_metrics.h>

namespace embb {
namespace base {
namespace perf {

/**
 * @brief Performance monitoring of procedures. 
 * 
 * Utility class to measure quantitative performance metrics 
 * secondary to latency, such as MFlops and FLPins. 
 * Wrapper for calls to the PAPI library. 
 */
class PerformanceMonitor
{
 private:
  PerformanceMetrics metrics; 

 public: 
  /**
   * @brief  Constructs a new PerformanceMonitor object 
   */
  PerformanceMonitor() { 
  }
  /**
   * @brief  Copy constructor
   */
  PerformanceMonitor(const PerformanceMonitor & other)
    : metrics(other.metrics)
  { }
  /**
   * @brief  Assignment operator. 
   */
  PerformanceMonitor & operator=(const PerformanceMonitor & rhs) {
    if (this != &rhs) {
      metrics = rhs.metrics; 
    }
    return *this; 
  }
  /**
   * @brief  Start a new performance measurement. 
   * 
   * Called before the procedure to be monitored. 
   */
  void Start() {
    if (PAPI_flops( 
          &metrics.real_time, 
          &metrics.proc_time, 
          &metrics.flpins, 
          &metrics.mflops ) < PAPI_OK) {
      throw ::std::runtime_error("PAPI_flops failed"); 
    }
  }
  /**
   * @brief  Stop monitoring and collect performance metrics. 
   */
  const PerformanceMetrics & CollectMetrics() {
    if (PAPI_flops( 
          &metrics.real_time, 
          &metrics.proc_time, 
          &metrics.flpins, 
          &metrics.mflops ) < PAPI_OK) {
      throw ::std::runtime_error("PAPI_flops failed"); 
    }
  }

};

} // namespace perf
} // namespace base
} // namespace embb

#endif // EMBB_BASE_CPP_PERF_PERFORMANCE_MONITOR_H_

