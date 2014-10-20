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

#ifndef EMBB_BENCHMARK_CPP_SCENARIO_H_
#define EMBB_BENCHMARK_CPP_SCENARIO_H_

namespace embb {
namespace benchmark { 

class Scenario {
public:
  typedef enum {
    /// t Threads running the same task, allocating and immediately 
    /// deallocating one element. 
    /// ||(Alloc(1), Dealloc(1)){t}
    SCENARIO__ENQUEUE_DEQUEUE_PAIRS = 0, 
    /// One thread allocating all elments in the queue except for 
    /// k elements, followed by k threads allocating and deallocating 
    /// one element. 
    /// Alloc(n-k), ||(Alloc(1), Dealloc(1)){k}
    SCENARIO__ENQUEUE_DEQUEUE_BULK = 1,
    /// t Threads allocating k elements run in parallel to the same 
    /// number of threads deallocating k elements. 
    /// ||(Alloc(k), Dealloc(k)){t}
    SCENARIO__FILL_UP = 2, 
    /// At 50% preallocation, p threads allocating and c threads 
    /// deallocating in parallel
    SCENARIO__RACE = 3, 
    /// p Threads enqueueing and c threads dequeueing in parallel
    SCENARIO__CAPACITY_BUFFER = 4, 
    /// Number of scenarios available in this benchmark. 
    NUM_SCENARIOS, 
    UNDEFINED = 99
  } ScenarioId;

  inline static bool IsProducerConsumerScenario(ScenarioId sId) {
    return (sId == SCENARIO__RACE || sId == SCENARIO__CAPACITY_BUFFER);
  }
  inline static bool IsAllocatorScenario(ScenarioId sId) {
    return !IsProducerConsumerScenario(sId);
  }
};

} // namespace benchmark
} // namespace embb

#endif /* EMBB_BENCHMARK_CPP_SCENARIO_H_ */
