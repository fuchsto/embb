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

#ifndef EMBB_CONTAINERS_WAIT_FREE_SIM_STACK_H_
#define EMBB_CONTAINERS_WAIT_FREE_SIM_STACK_H_

#include <embb/containers/internal/hazard_pointer.h>
#include <embb/containers/internal/returning_true_iterator.h>
#include <embb/containers/wait_free_compartment_value_pool.h>
#include <embb/containers/indexed_object_pool.h>

#include <embb/base/thread_specific_storage.h>
#include <embb/base/atomic.h>
#include <embb/base/function.h>

#include <embb/base/c/internal/thread_index.h>
#include <embb/base/c/internal/atomic/memory_barrier.h>
#include <embb/base/atomic.h>
#include <embb/containers/internal/cache.h>

#ifndef EMBB_CONTAINERS_VOLATILE
#  define EMBB_CONTAINERS_VOLATILE volatile
#endif
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
class WaitFreeSimStackNode {
private:
  typedef embb::containers::internal::WaitFreeSimStackNode<T> self_t;
public: 
  typedef int index_t;
  typedef struct Element_s{
    EMBB_CONTAINERS_VOLATILE index_t next;
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
 * \tparam LocalPoolSize   Size of thread-specific pool, must be equal to or 
 *                         greater than maximum number of threads
 */
template<
  typename T            = uint32_t,
  T UndefinedValue      = 0xFFFFFFFF,
  size_t LocalPoolSize  = 64,
  class ElementPool     = IndexedObjectPool< 
    EMBB_CONTAINERS_DEPENDANT_TYPENAME internal::WaitFreeSimStackNode<T>::Element, 
    WaitFreeCompartmentValuePool< bool, false, LocalPoolSize > >,
  class StateIndexPool  = WaitFreeCompartmentValuePool< bool, false, LocalPoolSize >
>
class WaitFreeSimStack
{
private: 

  typedef embb::containers::WaitFreeSimStack<
    T, UndefinedValue, LocalPoolSize, ElementPool, StateIndexPool> self_t;

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
    internal::WaitFreeSimStackNode<T> Node_t;
  typedef EMBB_CONTAINERS_DEPENDANT_TYPENAME 
    internal::WaitFreeSimStackNode<T>::Element Element_t;
  typedef EMBB_CONTAINERS_DEPENDANT_TYPENAME 
    internal::WaitFreeSimStackNode<T>::index_t ElementPointer_t;

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
//  ElementPointer_t head;
    typename internal::WaitFreeSimStackNode<T>::index_t head;
    T ret[MAX_THREADS];
  } ObjectStateUnpadded;
  typedef struct ObjectState{
    bitword_t applied;
//  ElementPointer_t head;
    typename internal::WaitFreeSimStackNode<T>::index_t head;
    T ret[MAX_THREADS];
    int32_t pad[EMBB_CONTAINERS_PAD_CACHE(sizeof(ObjectStateUnpadded))];
  } ObjectState;  
  typedef struct StackThreadState {
//  PoolStruct thread_pool;
    EMBB_CONTAINERS_CACHE_ALIGN bitword_t mask;
    bitword_t toggle;
    bitword_t bit;
    ElementPointer_t localObjectStateIndex;
    unsigned int backoff;
  } StackThreadState;
  typedef uint32_t bitfield_t; 
  typedef embb::base::Atomic< bitfield_t > atomic_bitfield_t;
  typedef bool state_t;

private:

#if defined(EMBB_64_BIT_ATOMIC_AVAILABLE)
  static const bitword_t BitWordZero = 0ui64;
  static const bitword_t BitWordOne  = 1ui64;
#else
  static const bitword_t BitWordZero = 0ui32;
  static const bitword_t BitWordOne  = 1ui32;
#endif

private:

  static const int MAX_BACK = 0x000FFF;
  /// Null-pointer representation for hazard pointers
  static const ElementPointer_t UndefinedGuard = 0;
  /// Global stack pointer state
  embb::base::Atomic<ElementPointer_t> stackStateIndex;
  AtomicBitVectorValue atomicTogglesVector;
  /// Capacity of the queue instance. 
  size_t size;
  /// Size of thread-local state pools.
  size_t localPoolSize;
  /// Maximum order for exponential backoff.
  size_t maxBackoff;
  /// Whether exponential backoff will be used on detected contention.
  bool backoffEnabled;
  /// Maximum number of threads accessing this queue instance. 
  size_t numThreads;
  /// Callback instance for release of guarded node indices. 
  embb::base::Function< void, ElementPointer_t > delete_pointer_callback;
  /// Hazard pointer for node index (guards stack top pointer). 
  embb::containers::internal::HazardPointer< ElementPointer_t > hp;
  /// Allocator for object states
  embb::base::Allocator<ObjectState> objectStateAllocator;
  /// Allocator for announced operation arguments (ArgVal)
  embb::base::Allocator<OperationArg> OperationArgAllocator;
  /// Pool for node elements
  ElementPool elementPool;
  /// Pool for indices in object state array
  StateIndexPool stateIndexPool;
  /// Bitset recording which thread already initialized their local state
  uint32_t threadRegistry;
  /// Allocator for the threads' local state (will be replaced 
  /// by thread-specific variable)
  embb::base::Allocator<StackThreadState> stackThreadStateAllocator;
  /// Array of thread-specific states of the stack object
  StackThreadState * threadStates;
  /// Index of initial object state in stack states array
  ElementPointer_t initialStatePointer;
  /// Thread-specific, incremental random seed
  embb::base::ThreadSpecificStorage<long> randomNextTss;

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

  inline static uint32_t __bitSearchFirst32(uint32_t v) {
    // Result of log2(v)
    uint32_t r;
    uint32_t shift;
    r = (v > 0xFFFFu) << 4u;
    v >>= r;
    shift = (v > 0xFFu) << 3u;
    v >>= shift; r |= shift;
    shift = (v > 0xFu) << 2u;
    v >>= shift; r |= shift;
    shift = (v > 0x3u) << 1u;
    v >>= shift; r |= shift;
    r |= (v >> 1u);
    return r;
  }
  inline static uint32_t bitSearchFirst(atomic_uint_t v) {
    uint32_t r = __bitSearchFirst32((uint32_t)v);
#if defined(EMBB_64_BIT_ATOMIC_AVAILABLE)
    return (r == 0u) ? __bitSearchFirst32((uint32_t)(v >> 32u)) + 31u : r;
#else
    return r;
#endif
  }

  inline long Random(void) {
    randomNextTss.Get() = randomNextTss.Get() * 1103515245 + 12345;
    return ((unsigned)(randomNextTss.Get() / 65536) % 32768);
  }
  inline long RandomRange(long low, long high) {
    const unsigned int rand_max = 32767;
    return low + (long)(((double)high) * (Random() / (rand_max + 1.0)));
  }

  void initStackThreadState(
    StackThreadState * threadState,
    unsigned int accessorId) {
    threadState->localObjectStateIndex = 0;
    threadState->mask     = 0;
    threadState->mask    |= (BitWordOne << static_cast<size_t>(accessorId));
    threadState->bit      = 0;
    threadState->bit     ^= (BitWordOne << static_cast<size_t>(accessorId));
    threadState->toggle   = 0;
    threadState->toggle   = static_cast<atomic_uint_t>(
      -static_cast<atomic_int_t>(threadState->mask));
    threadState->backoff  = 1;
    // Initialize thread-specific random seed:
    randomNextTss.Get() = 1;
  }
  
  inline RetVal ApplyOperation(
    StackThreadState * threadState,
    OperationArg      arg,
    unsigned int accessorId) {
    unsigned int numPushOperations;
    bitword_t diffs;
    bitword_t toggles; 
    bitword_t pendingPopOperations;
    ElementPointer_t stackStateIndexNew;
    ElementPointer_t stackStateIndexCurr;
    // Stores pool indices of pushed elements for 
    // rollback. At most MAX_THREADS push operations 
    // have to be reversed.
    ElementPointer_t pushElementIndices[MAX_THREADS];
    ObjectStateUnpadded * localStackState;
    ObjectStateUnpadded * globalStackState;
    // Prepare thread state:
    threadState->bit    ^= (BitWordOne << static_cast<size_t>(accessorId));
    threadState->toggle  = static_cast<atomic_uint_t>(
      -static_cast<atomic_int_t>(threadState->toggle));
    localStackState = (ObjectStateUnpadded *)(
      &stackStates[threadState->localObjectStateIndex]);
    // announce the operation
    operationArgs[accessorId] = arg;
    // Toggle accessorId's bit in atomicTogglesVector, Fetch&Add acts as a 
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
      stackStateIndexCurr = stackStateIndex.Load();
      // read reference of struct ObjectState in a local variable 
      // localStackState
      globalStackState = (ObjectStateUnpadded *)(
        &stackStates[stackStateIndexCurr]);
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
      if (stackStateIndexCurr != stackStateIndex.Load()) {
        continue;
      }
      // Intersection of applied and toggled operations: 
      diffs = localStackState->applied ^ toggles;
      numPushOperations    = 0;
      pendingPopOperations = 0;
      // Apply all operations that have been announced before this point
      while (diffs != BitWordZero) {
        uint32_t threadId = bitSearchFirst(diffs);
        diffs ^= BitWordOne << threadId;
        T announced_arg = operationArgs[threadId];
        if (announced_arg == UndefinedValue) {
          // == POP =======
          // Collect pending pop operatins in bit vector:
          pendingPopOperations |= (BitWordOne << threadId);
        }
        else {
          // == PUSH ======
          // Perform push operation and store pool index of 
          // pushed element for rollback:
          PushOperation(localStackState, threadState, announced_arg, threadId);
          numPushOperations++;
        }
      }     
      // Apply pending pop operations:
      while (pendingPopOperations != BitWordZero) {
        uint32_t threadId = bitSearchFirst(pendingPopOperations);
        pendingPopOperations ^= (BitWordOne << threadId);
        PopOperation(localStackState, threadState, threadId);
      }
      // Update applied operations of local stack state:
      localStackState->applied = toggles;
      // Store index in pool where localStackState will be stored in 
      // new stack pointer's index field:
      stackStateIndexNew = threadState->localObjectStateIndex;
      // Guard index of current object state:
      hp.GuardPointer(0, stackStateIndexCurr);
      if (stackStateIndexCurr == stackStateIndex.Load() &&
          stackStateIndex.CompareAndSwap(
            stackStateIndexCurr, 
            stackStateIndexNew)) {
        bool dummy;
        // Reserve new index from index pool:
        int objectStateIndex = stateIndexPool.Allocate(dummy);
        if (objectStateIndex < 0) {
          EMBB_THROW(embb::base::NoMemoryException,
            "Failed to allocate index for object state");
        }
        threadState->localObjectStateIndex = 
          static_cast<ElementPointer_t>(objectStateIndex);
        // Operation succeeded, reduce backoff limit:
        threadState->backoff = (threadState->backoff >> 1) | 1;
        // Release guard on index of replaced object state:
        hp.GuardPointer(0, UndefinedGuard);
        return localStackState->ret[accessorId];
      }
      else {
        // Release guard on index of current object state:
        hp.GuardPointer(0, UndefinedGuard);
        if (backoffEnabled && threadState->backoff < maxBackoff) {
          threadState->backoff <<= 1;
        }
        // Free elements allocated during failed push operations:
        for (int rb = 0;
             static_cast<unsigned int>(rb) < numPushOperations;
             ++rb) {
          elementPool.Free(pushElementIndices[rb]);
        }
      }
    }
    // Return the value from the state stored at the current object 
    // state index:
    return stackStates[stackStateIndex.Load()].ret[accessorId];
  }

public:

  /**
   * \see stack_concept
   *
   * \notthreadsafe
   *
   * \memory dynamically allocates \c TOOD.
   */
  WaitFreeSimStack(
    size_t size,
    int nThreads = 0,
    bool enableBackoff = false,
    unsigned int threadLocalPoolSize = LocalPoolSize)
  : size(size),
    localPoolSize(threadLocalPoolSize),
    maxBackoff(MAX_BACK),
    backoffEnabled(enableBackoff),
    // Using int for numThreads so compiler warning is raised 
    // when size and numThreads are switched by mistake.
    numThreads(nThreads <= 0
      ? embb::base::Thread::GetThreadsMaxCount()
      : static_cast<size_t>(nThreads)),
    // Extend pool size by thread-local range
    elementPool(
      size + ((nThreads - 1) * threadLocalPoolSize),
      Element_t()),
    stateIndexPool(
      internal::ReturningTrueIterator(0),
      internal::ReturningTrueIterator((localPoolSize * numThreads) + 1)),
      // Disable "this is used in base member initializer" warning.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4355)
#endif
    delete_pointer_callback(*this, &self_t::DeleteNodeCallback),
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    hp(delete_pointer_callback, UndefinedGuard, 1),    
    threadRegistry(0u) 
  {
    if (numThreads > MAX_THREADS) {
      EMBB_THROW(embb::base::ErrorException,
        "Maximum number of accessor thread exceeded");
    }
    if (localPoolSize < numThreads) {
      EMBB_THROW(embb::base::ErrorException,
        "Local pool size must be equal to or greater than number of threads");
    }
    bool flag;
    int initialStateIndex = stateIndexPool.Allocate(flag);
    if (initialStateIndex < 0) {
      EMBB_THROW(embb::base::NoMemoryException,
        "Failed to allocate index for initial object state");
    }
    initialStatePointer =
      static_cast<ElementPointer_t>(initialStateIndex);
    operationArgs = OperationArgAllocator.allocate(
      numThreads);
    stackStates = objectStateAllocator.allocate(
      (localPoolSize * numThreads) + 1);
    threadStates = stackThreadStateAllocator.allocate(
      numThreads);
    // Initialize global stack pointer state:
    stackStateIndex.Store(initialStateIndex);    
    stackStates[initialStateIndex].head    = 0;
    stackStates[initialStateIndex].applied = 0;
    atomicTogglesVector = 0;
  }

  /**
   * Destructor, deallocating memory
   */
  ~WaitFreeSimStack() {
    OperationArgAllocator.deallocate(
      const_cast<OperationArg *>(operationArgs),
      numThreads);
    objectStateAllocator.deallocate(
      const_cast<ObjectState *>(stackStates),
      localPoolSize * numThreads + 1);
  }

  /**
   * Removes an element from the pool and returns the element. 
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
    uint32_t threadBitMask = (1u << tId);
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
   * Adds an element to the pool. 
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
    uint32_t threadBitMask = (BitWordOne << static_cast<uint32_t>(tId));
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

  /**
   * Declares that the calling thread will no longer engage in 
   * operations on this stack instance.
   * 
   * \see stack_concept
   *
   * \waitfree
   */
  void DeactivateCurrentThread() {
    hp.DeactivateCurrentThread();
  }

private:

  /// Return the value removed from the stack
  inline T PopOperation(
    ObjectStateUnpadded * stack,
    StackThreadState * /* threadState */,
    uint32_t accessorId) {
    size_t headCurr = static_cast<size_t>(stack->head);
    if (headCurr != NULL) {      
      Node_t::Element headNode = elementPool[headCurr];
      stack->ret[accessorId] = headNode.value;
      stack->head = headNode.next;
      // Retire the dequeued element node:
      hp.EnqueuePointerForDeletion(headCurr);
    }
    else {
      stack->ret[accessorId] = UndefinedValue;
    }
    return stack->ret[accessorId];
  }

  /// Returns the pool index of the added element 
  /// to allow rollback of push operations.
  inline ElementPointer_t PushOperation(
    ObjectStateUnpadded * stack,
    StackThreadState * /* threadState */,
    T arg,
    uint32_t /* accessorId */) {
    typename embb::containers::internal::WaitFreeSimStackNode<T>::Element n;
    int poolIndex = elementPool.Allocate(n);
    if (poolIndex < 0) {
      EMBB_THROW(embb::base::NoMemoryException,
        "Failed to allocate element for push operation");
    }
    ElementPointer_t elementIndex = static_cast<ElementPointer_t>(poolIndex);
    n.value = arg;
    n.next  = stack->head;
    elementPool[elementIndex] = n;
    stack->head = elementIndex;
    return elementIndex;
  }

  void DeleteNodeCallback(ElementPointer_t releasedNodeIndex) {
    if (releasedNodeIndex == initialStatePointer) {
      // In current pool implementations, the first index 
      // returned from allocation is 0, which is the value of
      // UndefinedGuard. Here, we ensure that the pointer (= index)
      // to the initial stack state cannot be freed:
      return; 
    }
    bool flagExp = true;
    stateIndexPool.Free(flagExp, static_cast<int>(releasedNodeIndex));
  }
};

} // namespace containers
} // namespace embb

#endif /* EMBB_CONTAINERS_WAIT_FREE_SIM_STACK_H_ */
