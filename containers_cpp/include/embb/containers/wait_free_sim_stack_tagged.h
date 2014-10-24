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

#ifndef EMBB_CONTAINERS_WAIT_FREE_SIM_STACK_TAGGED_H_
#define EMBB_CONTAINERS_WAIT_FREE_SIM_STACK_TAGGED_H_

#include <embb/containers/internal/hazard_pointer.h>
#include <embb/containers/internal/returning_true_iterator.h>
#include <embb/containers/wait_free_array_value_pool.h>

#include <embb/base/thread_specific_storage.h>
#include <embb/base/atomic.h>
#include <embb/base/function.h>

#include <embb/base/c/internal/thread_index.h>
#include <embb/base/c/internal/atomic/memory_barrier.h>
#include <embb/base/atomic.h>
#include <embb/containers/internal/cache.h> 

#ifdef EMBB_COMPILER_MSVC
#  define EMBB_CONTAINERS_DEPENDANT_TYPEDEF typename
#  define EMBB_CONTAINERS_DEPENDANT_TYPENAME typename
#else
#  define EMBB_CONTAINERS_DEPENDANT_TYPEDEF
#  define EMBB_CONTAINERS_DEPENDANT_TYPENAME typename
#endif

namespace embb {
namespace containers {

namespace internal {

/**
 * Utility class implementing the queues node data type.
 */
template<typename T>
class WaitFreeSimStackTaggedNode {
private:
  typedef embb::containers::internal::WaitFreeSimStackTaggedNode<T> self_t;
public: 
  typedef uint32_t index_t;

  typedef struct Element_s {
    EMBB_CONTAINERS_VOLATILE struct WaitFreeSimStackTaggedNode<T>::Element_s * next;
    T value;
  } Element;
};

} // namespace internal

/**
 * Wait-Free Stack using elimination and exponential back-off
 *
 * \tparam T               Type of elements contained in the stack
 * \tparam UndefinedValue  Element value representing and undefined state, 
 *                         returned as result of Push
 */
template<
  typename T             = uint32_t,
  T UndefinedValue       = 0xFFFFFFFF,
  class ElementAllocator = embb::base::Allocator< 
    EMBB_CONTAINERS_DEPENDANT_TYPENAME internal::WaitFreeSimStackTaggedNode<T>::Element >
>
class WaitFreeSimStackTagged
{
private: 
  typedef embb::containers::WaitFreeSimStackTagged<T, UndefinedValue, ElementAllocator> self_t;
#if defined(EMBB_64_BIT_ATOMIC_AVAILABLE)
  static const unsigned int MAX_THREADS = 64;
#else
  static const unsigned int MAX_THREADS = 32;
#endif

private:
  typedef EMBB_CONTAINERS_DEPENDANT_TYPEDEF T Object;
  typedef EMBB_CONTAINERS_DEPENDANT_TYPEDEF T RetVal;
  typedef EMBB_CONTAINERS_DEPENDANT_TYPEDEF T OperationArg;
  typedef EMBB_CONTAINERS_DEPENDANT_TYPEDEF 
    internal::WaitFreeSimStackTaggedNode<T> Node_t;
  typedef EMBB_CONTAINERS_DEPENDANT_TYPENAME 
    internal::WaitFreeSimStackTaggedNode<T>::Element Element_t;
#if defined(EMBB_64_BIT_ATOMIC_AVAILABLE)
  typedef int64_t atomic_int_t;
  typedef uint64_t atomic_uint_t;
  typedef uint64_t bitword_t;
#else
  typedef int32_t atomic_int_t;
  typedef uint32_t atomic_uint_t;
  typedef uint32_t bitword_t;
#endif
  typedef embb::base::Atomic<bitword_t> AtomicBitVectorValue;

  typedef struct ObjectStateUnpadded {
    bitword_t applied;
    typename internal::WaitFreeSimStackTaggedNode<T>::Element * head;
    T ret[MAX_THREADS];
  } ObjectStateUnpadded;
  typedef struct ObjectState {
    bitword_t applied;
    typename internal::WaitFreeSimStackTaggedNode<T>::Element * head;
    T ret[MAX_THREADS];
    int32_t pad[EMBB_CONTAINERS_PAD_CACHE(sizeof(ObjectStateUnpadded))];
  } ObjectState;
  
#if defined(EMBB_64_BIT_ATOMIC_AVAILABLE)
  static const bitword_t BitWordZero = 0ui64;
  static const bitword_t BitWordOne  = 1ui64;
#else
  static const bitword_t BitWordZero = 0ui32;
  static const bitword_t BitWordOne  = 1ui32;
#endif
    
  /// Allocator for object states
  embb::base::Allocator<ObjectState> objectStateAllocator;
  /// Allocator for announced operation arguments (ArgVal)
  embb::base::Allocator<OperationArg> operationDescAllocator;
  /// Allocator for node elements
  ElementAllocator elementAllocator;

  // Thread-local pool
  typedef struct PoolStructUnpadded {
    void * p[4096];
    int index;
    int obj_size;
  } PoolStructUnpadded;
  typedef struct PoolStruct {
    void * p[4096];
    int index;
    int obj_size;
  int32_t align[EMBB_CONTAINERS_PAD_CACHE(sizeof(PoolStructUnpadded))];
  } PoolStruct;
  inline void init_pool(PoolStruct * pool, int obj_size) {
    int i;
    pool->obj_size = obj_size;
    pool->index = 0;
    Element_t * nodes = elementAllocator.allocate(4096);
    for (i = 0; i < 4096; i++) {
      pool->p[i] = &nodes[i];
    }
  }
  inline void * alloc_obj(PoolStruct *pool) {
    if (pool->index == 4096) {
      int size = pool->obj_size;
      init_pool(pool, size);
    }
    return pool->p[pool->index++];
  }
  inline void free_obj(PoolStruct * pool, void * obj) {
    if (pool->index > 0)
      pool->p[--pool->index] = obj;
  }
  inline void rollback(PoolStruct * pool, int num_objs) {
    if (pool->index - num_objs >= 0)
      pool->index -= num_objs;
    else
      pool->index = 0;
  }

  typedef union pointer_t {
    struct StructData{
      atomic_uint_t seq   : 8;
      atomic_uint_t index : 24;
    } struct_data;
    atomic_uint_t raw_data;
  } pointer_t;
  // Disable "structure was padded due to __declspec(align())" warning.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4324)
#endif
  typedef struct StackThreadState {
    PoolStruct thread_pool;
    EMBB_CONTAINERS_CACHE_ALIGN bitword_t mask;
    bitword_t toggle;
    bitword_t bit;
    unsigned int localObjectStateIndex;
    unsigned int backoff;
  } StackThreadState;
#ifdef _MSC_VER
#pragma warning(pop)
#endif
  typedef uint32_t bitfield_t; 
  typedef embb::base::Atomic< bitfield_t > atomic_bitfield_t;
  typedef bool state_t;
  typedef union int_aligned32_t {
    int32_t v ;
    char pad[EMBB_CACHE_LINE_SIZE];
  } int_aligned32_t;
  typedef union int_aligned64_t {
    EMBB_CONTAINERS_CACHE_ALIGN int64_t v;
    char pad[EMBB_CACHE_LINE_SIZE];
  } int_aligned64_t;
  
private:

  static const int MAX_BACK = 0x0000FFFFu;
  // Shared stack pointer state
  embb::base::Atomic<atomic_uint_t> stackPointer;
  AtomicBitVectorValue atomicTogglesVector;
  /// Capacity of the queue instance. 
  size_t size;
  size_t localPoolSize;
  size_t maxBackoff;
  bool backoffEnabled;
  /// Maximum number of threads accessing this queue instance. 
  size_t numThreads;
  /// Bitset recording which thread already initialized their local state
  uint32_t threadRegistry;
  /// Allocator for the threads' local state (will be replaced 
  /// by thread-specific variable)
  embb::base::Allocator<StackThreadState> stackThreadStateAllocator;
  StackThreadState * threadStates;

private:
  // Disable "structure was padded due to __declspec(align())" warning.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4324)
#endif
  EMBB_CONTAINERS_CACHE_ALIGN EMBB_CONTAINERS_VOLATILE OperationArg *
    operationArgs;
  EMBB_CONTAINERS_CACHE_ALIGN EMBB_CONTAINERS_VOLATILE ObjectState *
    stackStates;
#ifdef _MSC_VER
#pragma warning(pop)
#endif

private:

  bool queryThreadId(unsigned int & retIndexValue) {
    unsigned int tmpIndexValue; // For conversion size32_t <-> unsigned int 
    if (embb_internal_thread_index(&tmpIndexValue) == EMBB_SUCCESS) { 
      // Fail if thread index is not in range of number of accessors: 
      if (tmpIndexValue < numThreads) {
        retIndexValue = tmpIndexValue; 
        return true;      }
      // @TODO: Map thread id to accessor id range. 
      return false; 
    }
    retIndexValue = UndefinedValue;
    return false; 
  }
  
  // In Numerical Recipes in C: The Art of Scientific Computing 
  embb::base::ThreadSpecificStorage<long> randomNextTss;
  inline long Random(void) {
    randomNextTss.Get() = randomNextTss.Get() * 1103515245 + 12345;
    return((unsigned)(randomNextTss.Get() / 65536) % 32768);
  }
  inline long RandomRange(long low, long high) {
    const unsigned int rand_max = 32767;
    return low + (long)(((double)high) * (Random() / (rand_max + 1.0)));
  }

  inline static int bitSearchFirst(bitword_t B) {
    for (int b = 0; b < static_cast<int>(MAX_THREADS); ++b) {
      if (B & (BitWordOne << b)) {
        return b;
      }
    }
    return -1;
  }

  void initStackThreadState(
    StackThreadState * threadState,
    unsigned int accessorId)
  {
    threadState->localObjectStateIndex = 0;
    threadState->mask     = 0;
    threadState->mask    |= (BitWordOne << static_cast<size_t>(accessorId));
    threadState->bit      = 0;
    threadState->bit     ^= (BitWordOne << static_cast<size_t>(accessorId));
    threadState->toggle   = 0;
    threadState->toggle   = ~threadState->mask + 1; // 2s complement negation
    threadState->backoff  = 1;
    // Initialize thread-specific pool range:
    init_pool(&threadState->thread_pool, sizeof(Element_t));
    // Initialize thread-specific random seed:
    randomNextTss.Get() = 1;
  }
  
  inline RetVal ApplyOperation(
    StackThreadState * threadState,
    OperationArg arg,
    unsigned int accessorId) {
    int numPushOperations;
    bitword_t diffs;
    bitword_t toggles; 
    bitword_t pendingPopOperations;
    pointer_t stackPointerNew;
    pointer_t stackPointerCurr;
    // This thread's local stack object state 
    ObjectStateUnpadded * localStackState;
    // The current global stack object state
    ObjectStateUnpadded * globalStackState;
    
    threadState->bit    ^= (BitWordOne << accessorId);
    threadState->toggle  = ~threadState->toggle + 1; // 2s complement negation
    localStackState = (ObjectStateUnpadded *)(
      &stackStates[(accessorId * localPoolSize) + 
      threadState->localObjectStateIndex]);
    // announce the operation
    operationArgs[accessorId] = arg;
    // toggle accessorId's bit in atomicTogglesVector, Fetch&Add acts as a 
    // full write-barrier
    atomicTogglesVector.FetchAndAdd(threadState->toggle);
    for (int spin = 0; spin < 2; ++spin) {
      // Random backoff if enabled:
      if (backoffEnabled) {
        EMBB_CONTAINERS_VOLATILE int k;
        EMBB_CONTAINERS_VOLATILE long backoff_limit = RandomRange(
          static_cast<long>(threadState->backoff >> 1u),
          static_cast<long>(threadState->backoff));
        for (k = 0; k < backoff_limit; k++) {
          ;
        }
      }
      // read reference to struct ObjectState
      stackPointerCurr.raw_data = stackPointer.Load();
      // read reference of struct ObjectState in a local variable localStackState
      globalStackState = (ObjectStateUnpadded *)&(
        stackStates[stackPointerCurr.struct_data.index]);
      // Get all applied operations at this point: 
      diffs = globalStackState->applied;
      // Determine the set of active processes
      diffs ^= threadState->bit;
      // If this operation has already been applied, return
      if ((diffs >> accessorId) & 1) {
        break;
      }
      // Copy global state to local state:
      *localStackState = *globalStackState;
      toggles = atomicTogglesVector.Load(); 
      if (stackPointerCurr.raw_data != stackPointer.Load()) {
        continue;
      }
      // Intersection of applied and toggled operations: 
      diffs = localStackState->applied ^ toggles;
      numPushOperations    = 0;
      pendingPopOperations = 0;
      // Apply all operations that have been announced before this point
      while (diffs != BitWordZero) {
        int threadId = bitSearchFirst(diffs);
        diffs ^= BitWordOne << threadId;
        T announced_arg = operationArgs[threadId];
        if (announced_arg == UndefinedValue) {
          // == POP =======
          // Collect pending pop operatins in bit vector:
          pendingPopOperations |= (BitWordOne << threadId);
        }
        else {
          // == PUSH ======
          PushOperation(threadState, localStackState, announced_arg);
          numPushOperations++;
        }
      }      
      // Apply pending pop operations:
      while (pendingPopOperations != BitWordZero) {
        int threadId = bitSearchFirst(pendingPopOperations);
        pendingPopOperations ^= (BitWordOne << threadId);
        PopOperation(localStackState, threadId);
      }      
      // Update applied operations of local stack state:
      localStackState->applied = toggles;
      // Increase timestamp
      stackPointerNew.struct_data.seq = stackPointerCurr.struct_data.seq + 1;
      // Store index in pool where localStackState will be stored in 
      // new stack pointer's index field:
      stackPointerNew.struct_data.index = 
        (localPoolSize * accessorId) + threadState->localObjectStateIndex;
      if (stackPointerCurr.raw_data == stackPointer.Load() &&
        stackPointer.CompareAndSwap(
            stackPointerCurr.raw_data, 
            stackPointerNew.raw_data)) {
        threadState->localObjectStateIndex = 
          (threadState->localObjectStateIndex + 1) % localPoolSize;
        // Operation succeeded, reduce backoff limit:
        threadState->backoff = (threadState->backoff >> 1) | 1;
        return localStackState->ret[accessorId];
      }
      else {
        if (backoffEnabled && threadState->backoff < maxBackoff) {
          // Operation failed, reduce backoff limit:
          threadState->backoff <<= 1;
        }
        rollback(&threadState->thread_pool, numPushOperations);
      }
    }
    // Return the value from the state stored at the current object 
    // state index:
    pointer_t sp;
    sp.raw_data = stackPointer.Load();
    return stackStates[sp.struct_data.index].ret[accessorId];
  }

public:

  /**
   * \see stack_concept
   *
   * \notthreadsafe
   *
   * \memory dynamically allocates \c TOOD.
   */
  WaitFreeSimStackTagged(size_t size, int nThreads = 0, bool enableBackoff = false) :
    size(size),
    localPoolSize(64),
    maxBackoff(MAX_BACK),
    backoffEnabled(enableBackoff),
    numThreads(static_cast<size_t>(nThreads)),
    threadRegistry(0u) {
    if (numThreads == 0) {
      numThreads = embb::base::Thread::GetThreadsMaxCount();
    }
    if (numThreads > MAX_THREADS) {
      EMBB_THROW(embb::base::ErrorException,
        "Maximum number of accessor thread exceeded");
    }
    maxBackoff = numThreads * 100;    
    operationArgs = operationDescAllocator.allocate(
      numThreads);
    stackStates  = objectStateAllocator.allocate(
      (localPoolSize * numThreads) + 1);
    threadStates = stackThreadStateAllocator.allocate(
      numThreads);
    // Initialize global stack pointer state: 
    pointer_t sp;
    sp.raw_data = stackPointer.Load();
    sp.struct_data.index = localPoolSize * numThreads;
    sp.struct_data.seq = 0;
    stackPointer.Store(sp.raw_data);
    atomicTogglesVector = 0;
    stackStates[localPoolSize * numThreads].head    = NULL;
    stackStates[localPoolSize * numThreads].applied = 0;
  }

  /**
   * Destructor, deallocating memory
   */
  ~WaitFreeSimStackTagged() {
    operationDescAllocator.deallocate(
      const_cast<OperationArg *>(operationArgs),
      numThreads);
    objectStateAllocator.deallocate(
      const_cast<ObjectState *>(stackStates),
      localPoolSize * numThreads + 1);
  }

  /**
   * Removes an element from the stack and returns the element. 
   * 
   * \see stack_concept
   *
   * \waitfree
   */
  bool TryPop(T & retValue) {
    unsigned int tId;
    if (!queryThreadId(tId)) {
      EMBB_THROW(embb::base::ErrorException,
        "TryPop: Invalid thread ID");
    }
    StackThreadState * threadState = &threadStates[tId];
    int threadBitMask = (1 << tId);
    if ((threadRegistry & threadBitMask) == 0) {
      // Initialize local state for this thread
      initStackThreadState(threadState, tId);
      threadRegistry |= threadBitMask;
    }
    retValue = ApplyOperation(threadState, UndefinedValue, tId);
    if (retValue == UndefinedValue) {
      return false;
    }
    return true; 
  }
  
  /**
   * Adds an element to the stack. 
   * 
   * \see stack_concept
   *
   * \waitfree
   */
  bool TryPush(const T & element) {
    unsigned int tId;
    if (!queryThreadId(tId)) {
      EMBB_THROW(embb::base::ErrorException,
        "TryPush: Invalid thread ID");
    }
    StackThreadState * threadState = &threadStates[tId];
    int threadBitMask = (1 << tId);
    if ((threadRegistry & threadBitMask) == 0) {
      // Initialize local state for this thread
      initStackThreadState(threadState, tId);
      threadRegistry |= threadBitMask;
    }    
    ApplyOperation(threadState, element, tId);
    return true; 
  }

  /**
   * Returns the stack instance's capacity.
   * 
   * \see stack_concept
   *
   * \waitfree
   */
  inline size_t GetCapacity() {
    return size;
  }

private:

  inline void PushOperation(
    StackThreadState * threadState,
    ObjectStateUnpadded * stack,
    T arg) {
    typename embb::containers::internal::WaitFreeSimStackTaggedNode<T>::Element * n;
    n = (Element_t *)(alloc_obj(&threadState->thread_pool));
    n->value = arg;
    n->next = const_cast< Element_t * >(stack->head);
    stack->head = n;
  }

  inline void PopOperation(
    ObjectStateUnpadded * stack,
    int accessorId) {
    if (stack->head != NULL) {
      stack->ret[accessorId] = stack->head->value;
      stack->head = const_cast<Element_t *>(stack->head->next);
    }
    else {
      stack->ret[accessorId] = UndefinedValue;
    }
  }

};

}
}

#endif /* EMBB_CONTAINERS_WAIT_FREE_SIM_STACK_TAGGED_H_ */
