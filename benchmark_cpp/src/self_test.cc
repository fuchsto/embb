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

#include <embb/benchmark/internal/self_test.h>
#include <embb/base/perf/timer.h>

#include <iostream>
#include <iomanip>
#include <cmath> // INFINITY

#if defined(EMBB_COMPILER_MSVC)
// Sleep for Win32
#define SleepMilliseconds(ms) Sleep(static_cast<DWORD>(ms))
#else
// Sleep for POSIX
#include <unistd.h>
#define SleepMilliseconds(ms) usleep(ms * 1000)
#endif

namespace embb {
namespace benchmark {
namespace internal {

using embb::base::perf::Timer;

void SelfTest::RunGranularityTest(int samples) {
// {{{
  ::std::cout << "   -- Testing granularity" << ::std::endl; 
  double granularityMin = INFINITY; 
  double granularityMax = -1; 
  for (int i = 0; i < samples; ++i) {
    Timer t;
    Timer::timestamp_t start = t.Start();
    Timer::timestamp_t end   = Timer::Now();
    double elapsed  = Timer::FromInterval(start, end);
    if (elapsed < granularityMin) { 
      granularityMin = elapsed; 
    }
    if (elapsed > granularityMax) { 
      granularityMax = elapsed; 
    }
  }
  ::std::cout << "     Min: " <<  ::std::setprecision(2) 
              << granularityMin << "us " 
              << "     Max: " <<  ::std::setprecision(2) 
              << granularityMax << "us " 
              << ::std::endl; 
// }}}
}

void SelfTest::RunPrecisionTest(int samples, int mSecs) {
// {{{
  double precisionLower = INFINITY; 
  double precisionUpper = 0.0f; 
  int uSecs = mSecs * 1000; 
  ::std::cout << "   -- Testing " << samples << " samples of " 
              << mSecs << "ms = " 
              << uSecs << "us ... " 
              << ::std::endl;
  for (int i = 0; i < samples; ++i) {
    Timer t;

    SleepMilliseconds(mSecs);

    Timer::timestamp_t start = t.Start();
    Timer::timestamp_t end   = Timer::Now();
    double elapsed  = Timer::FromInterval(start, end);
    double accuracy = fabs(static_cast<double>(uSecs) - elapsed); 
    if (accuracy < precisionLower) {
      precisionLower = accuracy;
    }
    if (accuracy > precisionUpper) {
      precisionUpper = accuracy;
    }
    ::std::cout << "      " << ::std::setw(2) << i+1 << "/" << samples << ": " 
                << end << " - " << start << " = " << (end - start)
                << " -> " 
                << ::std::fixed << ::std::setw(12) << ::std::setprecision(2) << elapsed << "us "
                << " -> accuracy: " 
                << ::std::fixed << ::std::setw(6) << ::std::setprecision(2) << accuracy << "us"
                << ::std::endl;
  }
  double precision = precisionUpper - precisionLower; 
  ::std::cout << "----> Precision: " 
              << ::std::fixed << ::std::setprecision(3) << precisionLower << "us - "
              << ::std::fixed << ::std::setprecision(3) << precisionUpper << "us = "
              << ::std::fixed << ::std::setprecision(3) << precision << "us " << ::std::endl
              << ::std::endl;
}

void SelfTest::Run() { 
  ::std::cout << "===== " << Timer::TimerName() << ::std::endl;
  RunGranularityTest(500); 
  RunPrecisionTest(50,    10); 
  RunPrecisionTest(50,   100); 
  RunPrecisionTest(10,   500); 
  RunPrecisionTest( 5,  1000); 
  RunPrecisionTest( 3, 10000); 
}

} // namespace internal
} // namespace benchmark
} // namespace embb
