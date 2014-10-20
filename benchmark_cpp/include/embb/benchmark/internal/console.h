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

#ifndef EMBB_BENCHMARK_CPP_INTERNAL_CONSOLE_H_
#define EMBB_BENCHMARK_CPP_INTERNAL_CONSOLE_H_

#include <string>
#include <iostream>
#include <iomanip>

namespace embb {
namespace benchmark {
namespace internal {

class Console {
public: 
  static inline void WriteHeader(
      const ::std::string & header) { 
    ::std::cout << "===============| " << header
                << ::std::endl; 
  }
  static inline void WriteStep(
      const ::std::string & step) { 
    ::std::cout << "             --| " << step
                << ::std::endl; 
  }
  /// Status
  static inline void WriteStatus(
      const ::std::string & status) { 
    ::std::cout << "               | " << status 
                << ::std::endl; 
  }
  static inline void WriteStatus(
      const ::std::string & group, 
      const ::std::string & status) { 
    ::std::cout << ::std::setw(14) << group <<  " | " << status 
                << ::std::endl; 
  }
  /// Values
  template< typename T >
  static inline void WriteValue(
      const ::std::string & name, 
      T value,
      unsigned int prec, 
      const ::std::string & unit = "") {
    ::std::cout << ::std::setw(14) << name << " | "
                << ::std::setprecision(prec) << value << unit
                << ::std::endl; 
  }
  template< typename T >
  static inline void WriteValue(
      const ::std::string & name, 
      T value,
      const ::std::string & unit = "") {
    ::std::cout << ::std::setw(14) << name << " | "
                << value << unit
                << ::std::endl; 
  }
  /// Grouped values
  template< typename T >
  static inline void WriteValue(
      const ::std::string & group, 
      const ::std::string & name, 
      T value, 
      unsigned int prec,
      const ::std::string & unit = "") {
    ::std::cout << ::std::setw(14) << group <<  " | "
                << ::std::setw(10) << name
                << ::std::setprecision(prec) << value << unit
                << ::std::endl; 
  }
  /// Grouped values
  template< typename T >
  static inline void WriteValue(
      const ::std::string & group, 
      const ::std::string & name, 
      T value, 
      const ::std::string & unit = "") {
    ::std::cout << ::std::setw(14) << group <<  " | "
                << ::std::setw(10) << name
                << value << unit
                << ::std::endl; 
  }
};

} // namespace internal
} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_INTERNAL_CONSOLE_H_ */
