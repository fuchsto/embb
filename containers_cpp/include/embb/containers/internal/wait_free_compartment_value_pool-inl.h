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

#ifndef EMBB_CONTAINERS_INTERNAL_WAIT_FREE_COMPARTMENT_VALUE_POOL_INL_H_
#define EMBB_CONTAINERS_INTERNAL_WAIT_FREE_COMPARTMENT_VALUE_POOL_INL_H_

#include <embb/base/c/internal/thread_index.h>
#include <embb/base/thread.h>

#include <iostream>
#include <stdexcept>

namespace embb {
namespace containers {
  
template<typename T, T Undefined, class Allocator, size_t K >
unsigned int WaitFreeCompartmentValuePool<T, Undefined, Allocator, K>::
threadId() {
  // For conversion size32_t <-> unsigned int 
  unsigned int tmpIndexValue;
  if (embb_internal_thread_index(&tmpIndexValue) == EMBB_SUCCESS) {
    return tmpIndexValue;
  }
  EMBB_THROW(embb::base::ErrorException, 
    "Unable to resolve thread ID");
#ifndef EMBB_COMPILER_MSVC
  // To avoid compiler warning
  return static_cast<unsigned int>(-1);
#endif
}

template<typename T, T Undefined, class Allocator, size_t K >
void WaitFreeCompartmentValuePool<T, Undefined, Allocator, K>::
Free(T element, int index) {
  assert(element != Undefined);
  assert(
    index >= 0 && 
    index <= static_cast<int>(allocSize));
  // Just put back the element
  pool[index].Store(element);
}

template<typename T, T Undefined, class Allocator, size_t K >
int WaitFreeCompartmentValuePool<T, Undefined, Allocator, K>::
allocateFrom(
  T & element, 
  size_t poolIdxStart,
  size_t poolIdxEnd)
{
  assert(
    poolIdxStart >= 0 && 
    poolIdxStart <= static_cast<unsigned int>(allocSize));
  assert(
    poolIdxEnd >= 0 && 
    poolIdxEnd <= static_cast<unsigned int>(allocSize));
  
  // Try to allocate from threads pool range first: 
  for (size_t i = poolIdxStart; i != poolIdxEnd; ++i) {
    T expected;
    // if the memory cell is not available, go ahead
    if (Undefined == (expected = pool[i].Load()))
      continue;
    // try to get the memory cell
    if (pool[i].CompareAndSwap(expected, Undefined)) {
      // when the cas was successful, this element is ours
      element = expected;
      return static_cast<int>(i);
    }
  }
  return -1; 
}

template<typename T, T Undefined, class Allocator, size_t K >
int WaitFreeCompartmentValuePool<T, Undefined, Allocator, K>::
Allocate(T & element) {
  int idx; 
  size_t tId = threadId();
  size_t cOffset;

  cOffset = cSplit + (tId * cSize);

  // Try to allocate from threads pool range first:
  idx = allocateFrom(element, cOffset, cOffset + cSize);
  if (idx >= 0) return idx; 
  // Try to allocate from public pool range:  
  idx = allocateFrom(element, 0, cSplit);
  if (idx >= 0) return idx;
  // No element could be aquired: 
  return -1;
}

template<typename T, T Undefined, class Allocator, size_t K >
template<typename RAI>
WaitFreeCompartmentValuePool<T, Undefined, Allocator, K>::
WaitFreeCompartmentValuePool(RAI first, RAI last, int k)
: k(k), 
  cRange(0), 
  cSplit(0), 
  cSize(0), 
  nCompartments(k) {
  size = static_cast<size_t>(std::distance(first, last));

  maxThreads = embb::base::Thread::GetThreadsMaxCount();

  // number of elements in a single compartment, round towards 0:
  cSize = static_cast<size_t>(k);
  // Reserve k * t pool elements for compartments: 
  cRange = maxThreads * cSize;
  if (cRange > size) { 
    EMBB_THROW(embb::base::ErrorException, 
      "Pool capacity must be >= maxThreads");
  }
  // Start index of the compartment range. 
  // Shrink public pool range (cSplit) by size of one 
  // compartment, so every thread can allocate n = size 
  // elements with worst case latency n. 
  cSplit    = size - cSize; 
  allocSize = cSplit + cRange; 
  // use the allocator to allocate array of size size
  pool = allocator.allocate(allocSize);

  size_t i = 0;
  for (RAI curIter(first); i < allocSize; ++curIter) {
    pool[i++] = *curIter;
  }
}

template<typename T, T Undefined, class Allocator, size_t K >
WaitFreeCompartmentValuePool<T, Undefined, Allocator, K>::
~WaitFreeCompartmentValuePool() {
  allocator.deallocate(pool, size);
}

} // namespace containers
} // namespace embb

#endif /* EMBB_CONTAINERS_INTERNAL_WAIT_FREE_COMPARTMENT_VALUE_POOL_INL_H_ */
