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
  typedef uint32_t index_t;

  typedef struct Element_s {
    EMBB_CONTAINERS_VOLATILE struct WaitFreeSimStackNode<T>::Element_s * next;
    T value;
  } Element;
};

} // namespace internal

template<
  typename T             = uint32_t,
  T UndefinedValue       = 0xFFFFFFFF,
  typename IndexPool     = embb::containers::WaitFreeCompartmentValuePool<bool, false, 64>,
  class ElementAllocator = embb::base::Allocator< EMBB_CONTAINERS_DEPENDANT_TYPENAME internal::WaitFreeSimStackNode<T>::Element >,
  class ValueAllocator   = embb::base::Allocator<T>
>
class WaitFreeSimStack
{
private: 
  typedef embb::containers::WaitFreeSimStack<T, UndefinedValue, ValuePool, ElementAllocator> self_t;
  static const unsigned int MAX_THREADS = 32;

private:
  typedef EMBB_CONTAINERS_DEPENDANT_TYPEDEF T Object;
  typedef EMBB_CONTAINERS_DEPENDANT_TYPEDEF T RetVal;
  typedef EMBB_CONTAINERS_DEPENDANT_TYPEDEF T OperationDesc;
  typedef EMBB_CONTAINERS_DEPENDANT_TYPEDEF internal::WaitFreeSimStackNode<T> Node_t;
  typedef EMBB_CONTAINERS_DEPENDANT_TYPENAME internal::WaitFreeSimStackNode<T>::Element Element_t;
  typedef uint32_t index_t;
  typedef uint32_t atomic_int_t;
  typedef uint32_t bitword_t;
  typedef embb::base::Atomic<bitword_t> AtomicBitVectorValue;

  typedef struct ObjectStateUnpadded {
    bitword_t applied;
    typename embb::containers::internal::WaitFreeSimStackNode<T>::Element * head;
    T ret[MAX_THREADS];
  } ObjectStateUnpadded;
  typedef struct ObjectState {
    bitword_t applied;
    typename embb::containers::internal::WaitFreeSimStackNode<T>::Element * head;
    T ret[MAX_THREADS];
    int32_t pad[EMBB_CONTAINERS_PAD_CACHE(sizeof(ObjectStateUnpadded))];
  } ObjectState;
  
  static const bitword_t BitWordZero = 0; // 0L for x64
  static const bitword_t BitWordOne  = 1; // 1L for x64
  static const atomic_int_t UndefinedGuard = 0xFFFFFFFF;

  inline static uint32_t __bitSearchFirst32(uint32_t v) {
    // Result of log2(v)
    register uint32_t r; 
    register uint32_t shift;
    r = (v > 0xFFFF) << 4; 
    v >>= r;
    shift = (v > 0xFF) << 3; v >>= shift; r |= shift;
    shift = (v > 0xF)  << 2; v >>= shift; r |= shift;
    shift = (v > 0x3)  << 1; v >>= shift; r |= shift;
    r |= (v >> 1);
    return r;
  }
  inline static uint32_t bitSearchFirst(uint64_t v) {
    uint32_t r = __bitSearchFirst32((uint32_t)v);
    return (r == 0) ? __bitSearchFirst32((uint32_t)(v >> 32)) + 31 : r;
  }
  inline static int bitSearchFirst(atomic_int_t B) {
    for (int b = 0; b < static_cast<int>(MAX_THREADS); ++b) {
      if (B & (BitWordOne << b)) {
        return b; 
      }
    }
    return -1;
  }
  
  /// Allocator for memory used for initial element nodes. 
  ValueAllocator valueAllocator;
  /// Allocator for object states
  embb::base::Allocator<ObjectState> objectStateAllocator;
  /// Allocator for announced operation arguments (ArgVal)
  embb::base::Allocator<OperationDesc> operationDescAllocator;
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

  typedef atomic_int_t pointer_t;

  typedef struct StackThreadState {
    PoolStruct thread_pool;
    EMBB_CONTAINERS_CACHE_ALIGN bitword_t mask;
    bitword_t toggle;
    bitword_t bit;
    unsigned int local_index;
    unsigned int backoff;
  } StackThreadState;
  typedef uint32_t bitfield_t; 
  typedef embb::base::Atomic< bitfield_t > atomic_bitfield_t;
  typedef bool state_t;
  typedef union int_aligned32_t {
    int32_t v ;
    char pad[EMBB_CONTAINERS_CACHE_LINE_SIZE];
  } int_aligned32_t;
  typedef union int_aligned64_t {
    EMBB_CONTAINERS_CACHE_ALIGN int64_t v;
    char pad[EMBB_CONTAINERS_CACHE_LINE_SIZE];
  } int_aligned64_t;
  
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

private:
  // Shared variables
  static const int MAX_BACK = 200;
  // Shared stack pointer state
  embb::base::Atomic<atomic_int_t> stackPointer;
  AtomicBitVectorValue atomicTogglesVector;
  /// Capacity of the queue instance. 
  size_t size;
  size_t localPoolSize;
  size_t maxBackoff;
  bool backoffEnabled;
  /// Callback instance for release of guarded node indices. 
  embb::base::Function< void, uint32_t > delete_pointer_callback;
  /// Hazard pointer for node index (guards stack top pointer). 
  embb::containers::internal::HazardPointer< uint32_t > hp;
  /// Maximum number of threads accessing this queue instance. 
  size_t numThreads;
  /// Total capacity of underlying element pools:
  size_t poolCapacity; 
  /// Bitset recording which thread already initialized their local state
  uint32_t threadRegistry;
  /// Allocator for the threads' local state (will be replaced 
  /// by thread-specific variable)
  embb::base::Allocator<StackThreadState> stackThreadStateAllocator;
  StackThreadState * threadStates;
  /// Pool for indexed elements:
  IndexPool indexPool;

private:
  // Shared variables
  EMBB_CONTAINERS_CACHE_ALIGN EMBB_CONTAINERS_VOLATILE OperationDesc * announce;
  EMBB_CONTAINERS_CACHE_ALIGN EMBB_CONTAINERS_VOLATILE ObjectState * stackPool;

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

  void initStackThreadState(
    StackThreadState * threadState,
    unsigned int accessorId)
  {
    threadState->local_index = 0;
    threadState->mask        = 0;
    threadState->mask       |= (1L << accessorId);
    threadState->bit         = 0;
    threadState->bit        ^= (1L << accessorId);
    threadState->toggle      = 0;
    threadState->toggle      = -threadState->mask;
    threadState->backoff     = 1;
    // Initialize thread-specific pool range:
    init_pool(&threadState->thread_pool, sizeof(Element_t));
    // Initialize thread-specific random seed:
    randomNextTss.Get() = 1;
  }
 
  inline RetVal ApplyOperation(
    StackThreadState * threadState,
    OperationDesc arg,
    unsigned int accessorId) 
  {
    bitword_t diffs;
    bitword_t toggles; 
    bitword_t pendingPopOperations;
    pointer_t stackPointerNew;
    pointer_t stackPointerCurr;
    ObjectStateUnpadded * localStackState;
    ObjectStateUnpadded * globalStackState;
    unsigned int numPushOperations;
    OperationDesc tmp_arg;

    threadState->bit    ^= (1L << accessorId);
    threadState->toggle  = -threadState->toggle;
    localStackState = (ObjectStateUnpadded *)(
      &stackPool[(accessorId * localPoolSize) + threadState->local_index]);
    // announce the operation
    announce[accessorId] = arg;
    // toggle accessorId's bit in atomicTogglesVector, Fetch&Add acts as a 
    // full write-barrier
    atomicTogglesVector.FetchAndAdd(threadState->toggle);
    // Random backoff if enabled:
    if (backoffEnabled) {
      EMBB_CONTAINERS_VOLATILE unsigned int k;
      unsigned int backoff_limit = 1000;
      if (RandomRange(1, numThreads) > 1) {
        for (k = 0; k < backoff_limit; k++) {
          ;
        }
      }
    }
    for (int spin = 0; spin < 2; ++spin) {
      // read reference to struct ObjectState
      stackPointerCurr = stackPointer.Load();
      // read reference of struct ObjectState in a local variable localStackState
      globalStackState = (ObjectStateUnpadded *)&stackPool[stackPointerCurr.struct_data.index];
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
      if (stackPointerCurr != stackPointer.Load()) {
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
        T announced_arg = announce[threadId];
        if (announced_arg == UndefinedValue) {
          // == POP =======
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
      // Store index in pool where localStackState will be stored in 
      // new stack pointer's index field:
//    stackPointerNew = (localPoolSize * accessorId) + threadState->local_index;
      bool dummy = false;
      stackPointerNew = indexPool.allocate(dummy);
//    hp.GuardPointer(0, stackPointerNew);
      if (stackPointerCurr == stackPointer.Load() &&
        stackPointer.CompareAndSwap(
            stackPointerCurr, 
            stackPointerNew)) {
        threadState->local_index = (threadState->local_index + 1) % localPoolSize;
        threadState->backoff     = (threadState->backoff >> 1) | 1;
        return localStackState->ret[accessorId];
      }
      else {
        if (backoffEnabled && threadState->backoff < maxBackoff) {
          threadState->backoff <<= 1;
        }
        rollback(&threadState->thread_pool, numPushOperations);
      }
    }
//  hp.GuardPointer(0, UndefinedGuard);
    // Return the value found in the record stored:
    return stackPool[stackPointer.Load()].ret[accessorId];
  }

public:
  WaitFreeSimStack(
    size_t size, 
    int nThreads       = 0, 
    bool enableBackoff = false) 
  : size(size),
//  localPoolSize(16),
    maxBackoff(MAX_BACK),
    backoffEnabled(enableBackoff),
    delete_pointer_callback(*this, &self_t::DeleteNodeCallback),
    hp(delete_pointer_callback, UndefinedValue, 2),
    // Using int for numThreads so compiler warning is raised 
    // when size and numThreads are switched by mistake. 
    numThreads(nThreads > 0 
      ? static_cast<size_t>(nThreads) 
      : embb::base::Thread::GetThreadsMaxCount()),
    poolCapacity(hp.GetRetiredListMaxSize() * numThreads + size + 1)
    threadRegistry(0u), 
    indexPool(
      internal::ReturningTrueIterator(0), 
      internal::ReturningTrueIterator(poolCapacity)
  {
    if (numThreads > MAX_THREADS) {
      EMBB_THROW(embb::base::ErrorException,
        "Maximum number of accessor thread exceeded");
    }
    maxBackoff   = numThreads * 100;
    announce     = operationDescAllocator.allocate(numThreads);
    stackPool    = objectStateAllocator.allocate(poolCapacity);
    threadStates = stackThreadStateAllocator.allocate(numThreads);
    // Initialize global stack pointer state: 
    bool dummy = false;
    int sentinelIndex = indexPool.allocate(dummy);
    stackPointer.Store(sentinelIndex);
    atomicTogglesVector = 0;
    stackPool[sentinelIndex].head = 0;
    stackPool[sentinelIndex].applied = 0;
  }
  ~WaitFreeSimStack() {
    operationDescAllocator.deallocate(
      const_cast<OperationDesc *>(announce),
      numThreads);
    objectStateAllocator.deallocate(
      const_cast<ObjectState *>(stackPool),
      poolCapacity);
  }

private:
  inline void PopOperation(
    ObjectStateUnpadded * stack,
    int accessorId)
  {
    if (stack->head != NULL) {
      stack->ret[accessorId] = stack->head->value;
      stack->head = const_cast<Element_t *>(stack->head->next);
    }
    else {
      stack->ret[accessorId] = UndefinedValue;
    }
  }

public:
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

private:
  inline void PushOperation(
    StackThreadState * threadState,
    ObjectStateUnpadded * stack,
    T arg) 
  {
    typename embb::containers::internal::WaitFreeSimStackNode<T>::Element * n;
    n           = (Element_t *)(alloc_obj(&threadState->thread_pool));
    n->value    = arg;
    n->next     = const_cast< Element_t * >(stack->head);
    stack->head = n;
  }

public:
  bool TryPush(const T & element) {
    unsigned int tId;
    if (!queryThreadId(tId)) {
      EMBB_THROW(embb::base::ErrorException,
        "TryPush: Invalid thread ID");
    }

    StackThreadState * threadState = &threadStates[tId];

    uint32_t threadBitMask = (1 << tId);
    if ((threadRegistry & threadBitMask) == 0) {
      // Initialize local state for this thread
      initStackThreadState(threadState, tId);
      threadRegistry |= threadBitMask;
    }
    
    ApplyOperation(threadState, element, tId);
    return true; 
  }

  inline size_t GetCapacity() {
    return size;
  }

  void DeactivateCurrentThread() { 
  }

  void DeleteNodeCallback(index_t /* releasedNodeIndex */) {
  }
};

}
}

#endif /* EMBB_CONTAINERS_WAIT_FREE_SIM_STACK_H_ */
