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

#ifndef EMBB_CONTAINERS_WAIT_FREE_PHASELESS_QUEUE_H_
#define EMBB_CONTAINERS_WAIT_FREE_PHASELESS_QUEUE_H_

#include <embb/containers/internal/flags.h>

#include <embb/containers/wait_free_array_value_pool.h>
#include <embb/containers/indexed_object_pool.h>

#if EMBB_CONTAINERS_USE_HP_OLD
#include <embb/base/internal/guarded_pointers/i_guarded_pointers.h>
#include <embb/base/internal/guarded_pointers/hazard_pointers.h>
#else
#include <embb/containers/internal/hazard_pointer.h>
#endif

#include <embb/containers/internal/returning_true_iterator.h>

#include <embb/base/atomic.h>
#include <embb/base/function.h>

#include <embb/base/c/internal/thread_index.h>

#include <stdlib.h>

#include <embb/containers/internal/cache.h>

namespace embb {
namespace containers {

namespace internal {

/**
 * @brief Utility class implementing the queues node data type.
 */
template<typename T>
class WaitFreePhaselessQueueNode {
private:
  typedef WaitFreePhaselessQueueNode<T> self_t;

public: 
  typedef uint32_t index_t;
  static const index_t UndefinedIndex;

private:
  T value;                              ///< Node value
  index_t enq_aid;                      ///< Enqeue accessor id
  embb::base::Atomic<index_t> next_idx; ///< Pool-index of next Node in list (atomic). -1 for none. 
  embb::base::Atomic<index_t> deq_aid;  ///< Dequeue accessor id (atomic). -1 for none. 
  
public:
  WaitFreePhaselessQueueNode() :
    enq_aid(UndefinedIndex) {
    next_idx.Store(UndefinedIndex);
    deq_aid.Store(UndefinedIndex);
  }
  WaitFreePhaselessQueueNode(const self_t & other) :
    value(other.value),
    enq_aid(other.enq_aid) {
    next_idx.Store(other.next_idx.Load());
    deq_aid.Store(other.deq_aid.Load());
  }
  self_t & operator=(const self_t & other) {
    if (this != &other) {
      next_idx.Store(other.next_idx.Load());
      deq_aid.Store(other.deq_aid.Load());
      value   = other.value;
      enq_aid = other.enq_aid;
    }
    return *this;
  }
  /**
   * @brief Constructs a new instance of Node.
   *
   * @param [in]  val     Value to be contained in the Node instance.
   * @param [in]  enqAid  Enqueuer accessor id.
   *
   * @progress wait-free
   */
  WaitFreePhaselessQueueNode(T const val, index_t enqAid) :
    value(val), enq_aid(enqAid) {
    next_idx.Store(UndefinedIndex);
    deq_aid.Store(UndefinedIndex);
  }
  inline T Value() const {
    return value;
  }
  inline index_t NextPoolIdx() const {
    return next_idx.Load();
  }
  /**
   * @brief Set pointer to next Node element via CAS.
   *
   * @param [in]  nodePtrExpected  expected current pointer value.
   * @param [in]  nodePtr  New pointer value to set.
   *
   * @progress wait-free
   *
   * @return true if new pointer value could be set.
   */
  inline bool CASNext(index_t expNextIdx, index_t newNextIdx) {
    return next_idx.CompareAndSwap(expNextIdx, newNextIdx);
  }
  inline bool NextIsNull() const {
    return next_idx.Load() == UndefinedIndex;
  }
  inline index_t EnqueueAID() const {
    return enq_aid;
  }
  inline embb::base::Atomic<index_t> & DequeueAID() {
    return deq_aid;
  }
};

/// Using maximum value of OperationDesc::NodeIndex (30 bit) to represent 'undefined'. 
template<typename T>
const typename WaitFreePhaselessQueueNode<T>::index_t
WaitFreePhaselessQueueNode<T>::UndefinedIndex = (0x3FFFFFFF);

} // namespace internal

/**
 * @brief Wait-free queue for multiple enqueuers and dequeuers.
 *
 * @tparam  T Generic type of elements in the queue.
 *
 * @progress wait-free
 */
template<
  typename T,
  class NodeAllocator = embb::base::Allocator< internal::WaitFreePhaselessQueueNode<T> >,
  class OpAllocator   = embb::base::Allocator< embb::base::Atomic<uint32_t> >, 
  class NodePool      = IndexedObjectPool< 
    internal::WaitFreePhaselessQueueNode<T>, 
    WaitFreeArrayValuePool<bool, false> >
>
class WaitFreePhaselessQueue
{
private:
  typedef internal::WaitFreePhaselessQueueNode<T> Node_t;
  typedef typename internal::WaitFreePhaselessQueueNode<T>::index_t index_t;
  typedef WaitFreePhaselessQueue<T, NodeAllocator, OpAllocator, NodePool> self_t;

private:
  /// Maximum size of queue. Using maximum value of 
  /// OperationDesc::NodeIndex 30 bit) minus one element 
  /// required for sentinel node. 
  static const index_t QUEUE_SIZE_MAX = static_cast<index_t>(0x3FFFFFFE);
  /// Number of guards per thread. 
  static const size_t num_guards = 2; 
  /// Null-pointer for hazard pointers
  static const index_t UndefinedGuard = 0; 
  /**
   * Helper class for operation descriptions. 
   * Used instead of bit-field struct for portability. 
   */
  class OperationDesc {
  private: 
    static const index_t PENDING_FLAG_MASK = 0x80000000; ///< Bit 32
    static const index_t ENQUEUE_FLAG_MASK = 0x40000000; ///< Bit 31
    static const index_t NODE_INDEX_MASK   = 0x3FFFFFFF; ///< Bit 30..1

    OperationDesc() { }

  public:
    bool Pending;
    bool Enqueue;
    index_t NodeIndex;
    index_t Raw;

  public:
    /**
     * @brief Converts state to binary value { pending:1, enqueue:1, nodeIndex:30 }.
     */
    OperationDesc(
      bool pending,
      bool enqueue,
      index_t nodeIndex) :
      Pending(pending),
      Enqueue(enqueue),
      NodeIndex(nodeIndex),
      Raw(0) { 
      index_t nodeIndexMask = NodeIndex & NODE_INDEX_MASK;

      if (Pending) {
        Raw |= PENDING_FLAG_MASK;
      }
      if (Enqueue) {
        Raw |= ENQUEUE_FLAG_MASK;
      }
      Raw |= nodeIndexMask;
    }
    /**
     * @brief Expects binary value { pending:1, enqueue:1, nodeIndex:30 }. 
     */
    OperationDesc(index_t raw) : Raw(raw) {
      Pending   = (raw & PENDING_FLAG_MASK) ? true : false;
      Enqueue   = (raw & ENQUEUE_FLAG_MASK) ? true : false;
      NodeIndex = (raw & NODE_INDEX_MASK);
    }    
  };
  // Disable "structure was padded due to __declspec(align())" warning.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4324)
#endif
  /// Index of head node in node pool. 
  EMBB_CONTAINERS_CACHE_ALIGN embb::base::Atomic<index_t> headIdx;
  /// Index of tail node in node pool. 
  EMBB_CONTAINERS_CACHE_ALIGN embb::base::Atomic<index_t> tailIdx;
  /// Capacity of the queue instance.
#ifdef _MSC_VER
#pragma warning(pop)
#endif
  size_t size;

#if EMBB_CONTAINERS_USE_HP_OLD
  typedef embb::base::internal::guarded_pointers::CallbackInstanceFromObject<self_t, index_t>
    hp_callback_t;
  /// Callback instance for release of guarded node indices. 
  hp_callback_t delete_pointer_callback;
  /// Hazard pointer for node index (guards stack top pointer). 
  embb::base::internal::guarded_pointers::HazardPointers<index_t, num_guards> hp;
#else
  /// Callback instance for release of guarded node indices. 
  embb::base::Function< void, index_t > delete_pointer_callback;
  /// Hazard pointer for node index (guards stack top pointer). 
  embb::containers::internal::HazardPointer< index_t > hp;
#endif

  Node_t nullNode;
  /// Maximum number of threads accessing this queue instance. 
  size_t num_states;
  /// Capacity of the node pool, respective to overhead due to 
  /// hazard pointers. 
  size_t node_pool_size;
  /// Stores one state for every concurrent accessor on the queue. 
  /// Size: Two for every concurrent accessor (needed for swapping). 
  embb::base::Atomic<index_t> * operationDescriptions;
  /// Pool for element nodes in the queue. 
  NodePool      nodePool;
  /// Allocator for memory used for operation descriptions. 
  OpAllocator   operationDescriptionAllocator;
  /// Allocator for memorory used for initial element nodes. 
  NodeAllocator nodeAllocator;
  /// Index of sentinel node, stored for release in destructor. 
  index_t       sentinelNodeIndex;

private:
  /**
   * Resolves thread index usable as accessor id.
   * Returns true if thread index could be resolved, false otherwise.
   * A call could fail, e.g. if there have been created more
   * threads than initially configured in the embb runtime.
   * @param [out]  retIndexValue  Value of thread index value. 
   */
  bool loadAccessorThreadIndex(index_t & retIndexValue) {
    unsigned int tmpIndexValue; // For conversion size32_t <-> unsigned int 
    if (embb_internal_thread_index(&tmpIndexValue) == EMBB_SUCCESS) {
      // Fail if thread index is not in range of number of accessors: 
      if (tmpIndexValue < num_states) {
        retIndexValue = tmpIndexValue;
        return true;
      }
      // @TODO: Map thread id to accessor id range. 
      return false;
    }
    retIndexValue = Node_t::UndefinedIndex;
    return false;
  }
  inline size_t retiredListMaxSize(size_t nThreads) {
    return static_cast<size_t>(1.25 *
      static_cast<double>(nThreads) *
      static_cast<double>(num_guards)) + 1;
  }
  
public:
  /**
   * @brief Constructs a new instance of WaitFreeQueue.
   *
   * @param [in]  size  Size of the queue as maximum number of elements.
   * @param [in]  numThreads  Number of threads operating on this queue instance.
   *
   * @throws  embb::base::NoMemoryException
   *
   * \waitfree
   */
  WaitFreePhaselessQueue(size_t size, int numThreads = 0) :
    size(size),
// Disable "this is used in base member initializer" warning.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4355)
#endif
    delete_pointer_callback(*this, &self_t::DeleteNodeCallback),
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#if EMBB_CONTAINERS_USE_HP_OLD
    hp(delete_pointer_callback),
#else
    hp(delete_pointer_callback, UndefinedGuard, 2),
#endif
    // Using int for numThreads so compiler warning is 
    // raised when size and numThreads are switched by 
    // mistake. 
    num_states(numThreads <= 0
        ? embb::base::Thread::GetThreadsMaxCount()
        : static_cast<size_t>(numThreads)),
    // Node pool size, with respect to the maximum number of 
    // retired nodes not eligible for reuse due to hazard pointers: 
    node_pool_size(
      // numThreads caused trouble here
      retiredListMaxSize(num_states) * num_states + 
      size + 1),
    nodePool(node_pool_size, nullNode)
  {
    assert(sizeof(index_t) == 4); 
    if (size > QUEUE_SIZE_MAX) {
      EMBB_THROW(embb::base::NoMemoryException,
        "Maximum size of queue exceeded");
    }
    // Allocate sentinel node: 
    Node_t sentinelNode;
    int sentinelNodePoolIndex = nodePool.Allocate(sentinelNode);
    if (sentinelNodePoolIndex < 0) {
      EMBB_THROW(embb::base::NoMemoryException,
        "Allocation of sentinel node failed");
    }
    sentinelNodeIndex = static_cast<index_t>(sentinelNodePoolIndex);
    // Guard sentinel node from deletion for the 
    // life time of the queue: 
    hp.GuardPointer(0, sentinelNodeIndex);
    headIdx.Store(sentinelNodeIndex);
    tailIdx.Store(sentinelNodeIndex);
    // State of the queue is one operation description per queue accessor. 
    // Initialize clear state: Null-operarion for every accessor. 
    operationDescriptions = operationDescriptionAllocator.allocate(num_states);
    for (size_t accessorId = 0; accessorId < num_states; ++accessorId) {
      OperationDesc op(
        false,                 // nonpending
        true,                  // enqueue, should not matter
        Node_t::UndefinedIndex // node index
      );
      // As all operation descriptions are the same, 
      // we do not have to map accessor ids to operation 
      // pool indices. Every accessor will randomly grab 
      // an operation pool element and stick to it, as 
      // a threads accessor id will not change. 
      operationDescriptions[accessorId].Store(op.Raw);
    }
  }

  /** 
   * Destructor. Frees underlying data structures. 
   * Caller has to ensure there are no other accessors depending on 
   * this instance. 
   */
  ~WaitFreePhaselessQueue() {
    // Dequeue until queue is empty: 
    T val;
    while (TryDequeue(val)) {}
    hp.GuardPointer(0, UndefinedGuard); 
    // Delete internally managed memory regions: 
    operationDescriptionAllocator.deallocate(
      operationDescriptions, 
      num_states);
  }

  /**
   * Adds an element to the queue.
   * @param [in]  element  The element to add. 
   * @throws  embb::base::ErrorException
   *
   * \waitfree
   */
  bool TryEnqueue(T const & element) {
    index_t accessorId = Node_t::UndefinedIndex; 
    if (!loadAccessorThreadIndex(accessorId)) {
      EMBB_THROW(embb::base::ErrorException, 
        "Invalid thread ID");
    }
    // Register new node in pool: 
    Node_t poolNode;
    int nodeIndex = nodePool.Allocate(poolNode);
    if (nodeIndex < 0) {
      return false; // Queue is at capacity
    }
    // Guard index of node to be enqueued until enqueue
    // operation has been completed. No need to re-validate 
    // this guard: The node index has not been enqueued yet
    // and cannot be deleted in between:
    hp.GuardPointer(0, static_cast<index_t>(nodeIndex));
    // Initialize node in pool: 
    Node_t newNode(element, accessorId);
    nodePool[static_cast<index_t>(nodeIndex)] = newNode;    
    OperationDesc enqOp(
      true,    // pending
      true,    // enqueue
      static_cast<index_t>(nodeIndex)
    );
    operationDescriptions[accessorId].Store(enqOp.Raw);    
    Help();
    HelpFinishEnqueue(); 
    // Release guard: 
    hp.GuardPointer(0, UndefinedGuard);
    return true; 
  }

  /**
   * @brief Dequeue an element from the queue. Returns false when called on 
   *        empty queue.
   * @param [out]  retElement  The element dequeued, or default(T) if called 
   *                           on empty queue. 
   * @throws  embb::base::ErrorException
   * \waitfree
   */
  bool TryDequeue(T & retElement) {
    index_t accessorId = static_cast<index_t>(-1);
    if (!loadAccessorThreadIndex(accessorId)) {
      EMBB_THROW(embb::base::ErrorException,
        "Invalid thread ID");
    }
    
    OperationDesc curOp(operationDescriptions[accessorId].Load());
    // Assert that current operation of this accessor is completed: 
    assert(!curOp.Pending);

    // Register new operation description for CAS: 
    OperationDesc newOp(
      true,                   // pending
      false,                  // dequeue
      Node_t::UndefinedIndex  // node index
    );
    if (!operationDescriptions[accessorId].CompareAndSwap(
      curOp.Raw,
      newOp.Raw)) {
      // The threads own operation has changed, 
      // should not happen. 
      assert(false); 
    }    
    Help();
    HelpFinishDequeue();
    // Accessor's operation Idx might have changed in 
    // between. 
    curOp = OperationDesc(operationDescriptions[accessorId].Load());
    index_t nodeIdx = curOp.NodeIndex;
    Node_t & node   = nodePool[nodeIdx];
    if (nodeIdx == Node_t::UndefinedIndex) {
      // Allow dequeueing from empty queue, but 
      // return false: 
      retElement = T();
      return false;
    }
    index_t deqNodeIdx = node.NextPoolIdx();
    retElement = nodePool[deqNodeIdx].Value();
    // Node value returned is safe, release guard on 
    // dequeued node index: 
    hp.EnqueuePointerForDeletion(nodeIdx);
    return true; 
  }
  
  /**
   * @brief Announce that the current thread will no longer perform operations 
   *        on this queue. 
   * Releases hazard pointers guarded by the current thread. 
   * \waitfree
   */
  void DeactivateCurrentThread() {
    hp.DeactivateCurrentThread();
  }

  /**
   * @brief Returns the maximum size for instances of this queue.
   * \waitfree
   */
  inline size_t GetCapacity() {
    return size;
  }

private: 
  /**
   * @brief Help progressing pending enqueue operations of given accessors.
   * @param [in]  accessorId  Accessor id of operations to help.
   * \waitfree
   */
  void HelpEnqueue(unsigned int accessorId) {
    while (IsPending(accessorId)) {
      index_t lastIdx   = tailIdx.Load();
      Node_t & lastNode = nodePool[lastIdx];
      index_t nextIdx   = lastNode.NextPoolIdx();
//    hp.GuardPointer(0, lastIdx);
      // Last node still is tail: 
      if (lastIdx == tailIdx.Load()) {
        // tail.next is null (no pending enqueue on tail): 
        if (lastNode.NextIsNull()) {
          // Apply enqueue. 
          // No other accessor helped this enqueue operation yet: 
          if (IsPending(accessorId)) {
            // Set next-pointer of last element in list
            // (last.next.CAS(next, state[aid].node)): 
            OperationDesc opDesc(operationDescriptions[accessorId].Load());
            if (lastNode.CASNext(nextIdx, opDesc.NodeIndex)) {
              HelpFinishEnqueue();
              return;
            }
          }
        }
        else {
          // Some enqueue operation in progress
          HelpFinishEnqueue();
        }
      }
      else {
//      hp.ReleaseGuard(0, lastIdx);
      }
    }
  }

  /**
   * @brief Help finishing pending enqueue operations of arbitrary accessors,
   *        including own pending enqueue operations.
   * \waitfree
   */
  void HelpFinishEnqueue() {
    // Load node pointed at by tail: 
    index_t lastIdx   = tailIdx.Load();
    Node_t & lastNode = nodePool[lastIdx];
    // This node is NEXT of tail, but not tail => unfinished ENQ
    
    if (!lastNode.NextIsNull()) {
      index_t nextIdx = lastNode.NextPoolIdx();
//    hp.GuardPointer(0, nextIdx);
      Node_t & nextNode = nodePool[nextIdx];
      // Load accessor id from last (non-tail) element in list: 
      index_t helpAID   = nextNode.EnqueueAID();
      // Load operation for accessor that started the unfinished enqueue: 
      OperationDesc helpOp(operationDescriptions[helpAID].Load());      
      // tail index still points at last node: 
      // (last == tail && state[aid].node == next)
      if (lastIdx == tailIdx.Load() &&
        helpOp.NodeIndex == nextIdx)
      {
        OperationDesc newOp(
          false,   // set to nonpending
          true,    // enqueue
          nextIdx  // node index == helpOp.NodeIndex
        );
        operationDescriptions[helpAID].CompareAndSwap(
          helpOp.Raw,
          newOp.Raw);
        // Update tail pointer: 
        tailIdx.CompareAndSwap(lastIdx, nextIdx);
      }
//    hp.ReleaseGuard(0, nextIdx);
    }
  }

  /**
   * @brief Help progressing pending dequeue operations of given accessor,
   *        including own pending dequeue operations.
   * @param [in]  accessorId  Accessor id of operations to help.
   * \waitfree
   */
  void HelpDequeue(index_t accessorId) {
    while (IsPending(accessorId)) {
      index_t firstIdx = headIdx.Load();

//    hp.GuardPointer(0, firstIdx);

      index_t lastIdx  = tailIdx.Load();
      Node_t & first   = nodePool[firstIdx];

      if (firstIdx != headIdx.Load()) {
        // Head pointer changed by concurrent enqueue        
        // Release guard: 
//      hp.ReleaseGuard(0, firstIdx);        
        // Retry:
        continue;
      }
      if (firstIdx == lastIdx) {
        // Queue might be empty
        if (first.NextIsNull()) {
          // Queue is empty          
          // Release guard: 
//        hp.ReleaseGuard(0, firstIdx);
          OperationDesc curOp(operationDescriptions[accessorId].Load());
          if (lastIdx == tailIdx.Load() && IsPending(accessorId)) {
            OperationDesc newOp(
              false,                 // Set nonpending state
              false, 
              Node_t::UndefinedIndex // Leave undefined, to signal failed dequeue
            );
            // CAS without check as possibly another accessor 
            // already helped this dequeue operation.
            operationDescriptions[accessorId].CompareAndSwap(curOp.Raw, newOp.Raw);
          }
        }
        else {
          // Head is advanced because of unfinished enqueue, so 
          // help other enqueue first: 
          HelpFinishEnqueue();
        }
      }
      else {
        // Queue is not empty
        OperationDesc curOp(operationDescriptions[accessorId].Load());
        index_t nodeIdx = curOp.NodeIndex;
        if (!IsPending(accessorId)) {
          // Accessor not pending because another 
          // thread completed this operation already, done. 
          break; 
        }        
        if (firstIdx == headIdx.Load() && nodeIdx != firstIdx) {
          // Guard dequeue index until dequeue operation is
          // completed (see Dequeue()):
          hp.GuardPointer(0, firstIdx); // <<< o/s

          OperationDesc newOp(
            true, 
            false, 
            firstIdx // Set node index
          );
          if (!operationDescriptions[accessorId].CompareAndSwap(curOp.Raw, newOp.Raw)) {
            // This loop is wait-free as every accessor's operation 
            // will be pending for a limited time only, so continue 
            // and retry is okay. 

            // This CAS can only have failed because another 
            // thread completed this dequeue operation in the
            // meantime. 

            // Release guard: 
            // hp.ReleaseGuard(0, static_cast<index_t>(firstIdx));

            // Retry:
            continue;
          }
        }
        // The following CAS also happens if 
        //
        //   firstIdx != headIdx.Load() || nodeIdx == firstIdx
        // 
        // In this case, HelpFinishDequeue will complete the dequeue. 
        index_t curDeqAID = Node_t::UndefinedIndex;
        // Register this accessor as dequeuer of this node. 
        // If this CAS fails, another accessor is already (perhaps 
        // helping) dequeueing this node winning this CAS. In this 
        // case this dequeue operation is just ignored. 
        if (!first.DequeueAID().CompareAndSwap(curDeqAID, accessorId)) {
          // Lost CAS to helping accessor
        }
        HelpFinishDequeue();
      }
    } // while pending
  }

  /**
   * @brief Help finishing pending dequeue operations of arbitrary accessors, 
   *        including own pending dequeue operations.
   * \waitfree
   */
  void HelpFinishDequeue() {
    index_t firstIdx   = headIdx.Load();
    Node_t & first     = nodePool[firstIdx];
    index_t nextIdx    = first.NextPoolIdx();
//  hp.GuardPointer(1, nextIdx);
    index_t accessorId = first.DequeueAID().Load();
    if (accessorId != Node_t::UndefinedIndex) {
      OperationDesc curOp(operationDescriptions[accessorId].Load());
      if (firstIdx == headIdx.Load() && nextIdx != Node_t::UndefinedIndex) {
        // Guard dequeue index until dequeue operation is
        // completed (see Dequeue()). Possibly already guarded 
        // in HelpDequeue(), but guarding twice is okay:       
        hp.GuardPointer(0, firstIdx); // <<< o/s

        // Set state of helped operation to NONPENDING: 
        OperationDesc newOp(
          false, // nonpending
          false, // dequeue
          curOp.NodeIndex
        );
        // CAS without check as possibly another accessor 
        // already helped this dequeue operation.
        operationDescriptions[accessorId].CompareAndSwap(curOp.Raw, newOp.Raw);
        headIdx.CompareAndSwap(firstIdx, nextIdx);
      }
    }
    // Release guard: 
//  hp.ReleaseGuard(1, nextIdx);
  }

  /**
   * @brief Help finishing pending operations of arbitrary accessors, including
   *        own pending operations.
   * Post-condition: Pending operations of any other accessor have been helped
   *                 before helping the accessors own operation.
   * \waitfree
   */
  void Help() {
    // Fairness guarantee in every thread: 
    //  "Every other thread will help my operation 
    //   before helping its own operation"
    // Number of total operations in operation description buffer 
    index_t numHelpOps    = static_cast<index_t>(num_states);
    index_t ownAccessorId = Node_t::UndefinedIndex;
    loadAccessorThreadIndex(ownAccessorId);
    // Start helping accessor with next id, ensuring that 
    // own accessor id will be used in last iteration of 
    // help loop: 
    index_t startAccessorId = (ownAccessorId + 1) % num_states;
    for (unsigned int accessorId = startAccessorId; 
         numHelpOps > 0;
         ++accessorId, --numHelpOps)
    {
      OperationDesc desc(
        operationDescriptions[accessorId % num_states].Load());

      if (desc.Pending) {
        if (desc.Enqueue) {
          HelpEnqueue(accessorId % num_states);
        }
        else {
          HelpDequeue(accessorId % num_states);
        }
      }
    }
  }

  /**
   * @brief Whether the accessor with given id is pending.
   * @return  True if the given accessor has a pending operation.
   * @param [in]  accessorId Id of the accessor
   * \waitfree
   */
  inline bool IsPending(index_t accessorId) const {
    OperationDesc opDesc(operationDescriptions[accessorId].Load()); 
    return opDesc.Pending;
  }

  /// Whether the node with given index is in the process of being 
  /// dequeued. Prevents reclamation of node id when it cannot be 
  /// guarded by hazard pointers, between HelpDequeue and HelpFinishDequeue. 
  bool NodeIsPendingDequeue(index_t nodeId) {
    index_t ownAccessorId = Node_t::UndefinedIndex;
    if (!loadAccessorThreadIndex(ownAccessorId)) {
      EMBB_THROW(embb::base::ErrorException, "Invalid thread ID");
    }
    index_t startAccessorId = (ownAccessorId + 1) % num_states;
    for (unsigned int accessorId = startAccessorId;
         accessorId != ownAccessorId;
         ++accessorId)
    {
      OperationDesc desc(
        operationDescriptions[accessorId % num_states].Load());
      if (desc.NodeIndex == nodeId && 
          desc.Pending   == true) {
        return true; 
      }
    }
    return false; 
  }

  /**
   * @brief Callback used for deallocating a node index from hazard 
   *        pointers. Frees the associated node in the pool.
   * @param [in]  releasedNodeIndex  Index of the node to release. 
   * \waitfree
   */
  void DeleteNodeCallback(index_t releasedNodeIndex) {
    if (sentinelNodeIndex == releasedNodeIndex
//      || NodeIsPendingDequeue(releasedNodeIndex)
    ) {
      return;
    }
    nodePool.Free(static_cast<int>(releasedNodeIndex));
  }
};

} // namespace containers
} // namespace embb

#endif /* EMBB_CONTAINERS_WAIT_FREE_PHASELESS_QUEUE_H_ */
