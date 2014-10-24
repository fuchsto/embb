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

#ifndef EMBB_CONTAINERS_INTERNAL_INDEXED_OBJECT_POOL_INL_H_
#define EMBB_CONTAINERS_INTERNAL_INDEXED_OBJECT_POOL_INL_H_

#include <embb/containers/internal/returning_true_iterator.h>

namespace embb {
namespace containers {

template<typename T, class IndexPool, class Allocator>
IndexedObjectPool<T, IndexPool, Allocator>::
IndexedObjectPool()
{ }

template<typename T, class IndexPool, class Allocator>
template<typename RAI>
IndexedObjectPool<T, IndexPool, Allocator>::
IndexedObjectPool(RAI first, RAI last) : 
  size(static_cast<size_t>(std::distance(first, last))) {
  indexPool = new IndexPool(
    internal::ReturningTrueIterator(0),
    internal::ReturningTrueIterator(size));
  // use the allocator to allocate array of size dist
  elements = allocator.allocate(size);
  // fill element pool with elements from the iteration
  int i = 0;
  for (RAI curIter(first); curIter != last; ++curIter, ++i) {
    // assign element from iteration
    elements[i] = *curIter;
  }
}

template<typename T, class IndexPool, class Allocator>
IndexedObjectPool<T, IndexPool, Allocator >::
IndexedObjectPool(size_t size, const T & defaultInstance) :
  size(size) {
  indexPool = new IndexPool(
    internal::ReturningTrueIterator(0),
    internal::ReturningTrueIterator(size));
  // use the allocator to allocate array of size dist
  elements = allocator.allocate(size);
  // fill element pool with elements from the iteration
  for (size_t i = 0; i < size; ++i) {
    // initialize element from default constructor and 
    // assignment operator
    elements[i] = defaultInstance;
  }
}

template<typename T, class IndexPool, class Allocator>
IndexedObjectPool<T, IndexPool, Allocator >::
~IndexedObjectPool()
{
  allocator.deallocate(elements, (size_t)size);
}

template<typename T, class IndexPool, class Allocator>
int IndexedObjectPool<T, IndexPool, Allocator >::
Allocate(T & element) {
  // Reserve a pool index:
  bool reservedFlag; 
  int index = indexPool->Allocate(reservedFlag);
  // Assign element to be allocated at pool index. 
  // Index returned from index pool is -1 if no index 
  // is available. 
  if (index >= 0) {
    element = elements[index];
  }
  return index; 
}

template<typename T, class IndexPool, class Allocator>
void IndexedObjectPool<T, IndexPool, Allocator >::
Free(int elementIndex) {
  // Call the referenced element's destructor:
  elements[elementIndex].~T();
  // Release index of the element for reuse:
  indexPool->Free(true, elementIndex);
}

template<typename T, class IndexPool, class Allocator>
T & IndexedObjectPool<T, IndexPool, Allocator >::
operator[](size_t elementIndex) {
  return elements[elementIndex];
}

} // namespace containers
} // namespace embb

#endif /* EMBB_CONTAINERS_INTERNAL_INDEXED_OBJECT_POOL_INL_H_ */
