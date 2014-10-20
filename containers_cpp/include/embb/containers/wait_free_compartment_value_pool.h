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

#ifndef EMBB_CONTAINERS_WAIT_FREE_COMPARTMENT_VALUE_POOL_H_
#define EMBB_CONTAINERS_WAIT_FREE_COMPARTMENT_VALUE_POOL_H_

#include <embb/base/atomic.h>
#include <embb/base/memory_allocation.h>

namespace embb {
namespace containers {

/**
 * \defgroup CPP_DATA_VALUE_POOL Value Pools
 * Different Value Pools
 *
 * \ingroup CPP_DATA
 *
 */

/**
 * Wait-Free Value Pool using Array Construction
 *
 * \concept{value_pool_concept}
 *
 * \ingroup CPP_DATA_VALUE_POOL
 *
 * \tparam Allocator the allocator used to allocate the pool array
 *
 */
template<typename T,
  T Undefined,
  class Allocator = embb::base::Allocator< embb::base::Atomic<T> >,
  size_t K = 64 >
class WaitFreeCompartmentValuePool {

private:

  static unsigned int threadId();

private:

  int k; 
  
  unsigned int maxThreads; 
  unsigned int * threadCompartmentIds;

  /// Number of elements in all compartments
  size_t cRange; 
  /// Start index of the compartment range
  size_t cSplit; 
  /// Number of elements in a single compartment
  size_t cSize;
  int nCompartments; 

  size_t size;
  size_t allocSize;
  embb::base::Atomic<T> * pool;
  Allocator allocator;

  // Prevent default constructor
  WaitFreeCompartmentValuePool();

  // Prevent copy-construction
  WaitFreeCompartmentValuePool(const WaitFreeCompartmentValuePool&);

  // Prevent assignment
  WaitFreeCompartmentValuePool& operator=(const WaitFreeCompartmentValuePool&);
  
private: 

  int allocateFrom(
    T & element,
    size_t poolIdxStart,
    size_t poolIdxEnd
    );

public:

  /**
   * Return element and index to the pool.
   *
   * \see value_pool_concept
   *
   * \waitfree
   */
  void Free(
    T element,
    /**< [IN,OUT] element, must be previously acquired by allocate */
    int index
    /**< [IN,OUT] index, as previously acquired by allocate */
    );

  /**
   * Request element and index from pool.
   *
   * \return index of element
   *
   * \see value_pool_concept
   *
   * \waitfree
   */
  int Allocate(
    T & element
    /**< [IN,OUT] the aquired element */
    );

  /**
   * \see value_pool_concept
   *
   * \notthreadsafe
   *
   * \memory dynamically allocates \c n*sizeof(embb::base::Atomic<T>) bytes,
   *         where \c n is the number of elements of the pool.
   */
  template<typename RAI>
  WaitFreeCompartmentValuePool(
    RAI first,
     /**< [IN] first iterator to elements the pool is filled with */
    RAI last, 
     /**< [IN] last iterator to elements the pool is filled with */
    int k =  static_cast<int>(K)
     /**< [IN] setting for thread specific pool range differring from default */
    );

  /**
   * Destructor, deallocating memory
   */
  ~WaitFreeCompartmentValuePool();
};
} // namespace containers
} // namespace embb

#include <embb/containers/internal/wait_free_compartment_value_pool-inl.h>

#endif /* EMBB_CONTAINERS_WAIT_FREE_COMPARTMENT_VALUE_POOL_H_ */
