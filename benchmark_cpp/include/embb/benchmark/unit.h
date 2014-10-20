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

#ifndef EMBB_BENCHMARK_CPP_UNIT_H_
#define EMBB_BENCHMARK_CPP_UNIT_H_

#include <string>

namespace embb {
namespace benchmark { 

class Unit {
public:
  typedef enum {
    UNDEFINED                  = 0,
    SELF_TEST                  = 1,
		MICHAEL_SCOTT_QUEUE_AP     = 2, 
		MICHAEL_SCOTT_QUEUE_TP     = 3, 
    KOGAN_PETRANK_QUEUE        = 4, 
    KOGAN_PETRANK_QUEUE_PL     = 5, 
    LOCKFREE_TREE_POOL         = 6, 
    WAITFREE_ARRAY_POOL        = 7, 
    WAITFREE_COMPARTMENT_POOL  = 8,
    WAIT_FREE_SIM_STACK        = 9,
    LOCK_FREE_STACK            = 10,
    WAIT_FREE_SIM_STACK_TAGGED = 11,
    HARRIS_LIST_SET            = 12
  } UnitId;

  inline static UnitId FromUnitName(const ::std::string & name) {
    if (name == "test") {
      return Unit::SELF_TEST;
    }
    if (name == "michaelscott") {
      return Unit::MICHAEL_SCOTT_QUEUE_TP;
    }
    if (name == "michaelscott-ap") {
      return Unit::MICHAEL_SCOTT_QUEUE_AP;
    }
    if (name == "koganpetrank") {
      return Unit::KOGAN_PETRANK_QUEUE;
    }
    if (name == "koganpetrank-pl") {
      return Unit::KOGAN_PETRANK_QUEUE_PL;
    }
    if (name == "treepool") {
      return Unit::LOCKFREE_TREE_POOL;
    }
    if (name == "arraypool") {
      return Unit::WAITFREE_ARRAY_POOL;
    }
    if (name == "compartmentpool") {
      return Unit::WAITFREE_COMPARTMENT_POOL;
    }
    if (name == "harrislistset") {
      return Unit::HARRIS_LIST_SET;
    }
    if (name == "simstack-t") {
      return Unit::WAIT_FREE_SIM_STACK_TAGGED;
    }
    if (name == "lockfreestack") {
      return Unit::LOCK_FREE_STACK;
    }
    return Unit::UNDEFINED;
  }

};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_UNIT_H_ */
