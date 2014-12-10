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

#ifndef EMBB_CONTAINERS_PRIMITIVES_LLX_SCX_H_
#define EMBB_CONTAINERS_PRIMITIVES_LLX_SCX_H_

#include <embb/base/thread.h>
#include <embb/base/atomic.h>
#include <vector>

/** 
 * Implementation of the LLX/STX primitive as presented in 
 * "Pragmatic Primitives for Non-blocking Data Structures" 
 * (Brown et al., 2013).
 */

namespace embb { 
namespace containers {
namespace primitives {

template<typename T>
class LlxScx {

private:

  typedef LlxScx self_t;

  /**
   * Possible states of an LLX/SCX operation.
   */
  typedef enum {
    InProgress = 0,
    Comitted,
    Aborted
  } OperationState;
  
  /**
   * RAII-style implementation of a simplified hazard 
   * pointer scheme. SmartHazardPointer instances should 
   * reside only on the stack, never the heap, and T* 
   * values should reside only in the heap. A NodePtr can 
   * be automatically converted to a Node*, but an explicit 
   * constructor is needed to go the other way.
   *
   * Example:
   *   SmartHazardPointer<Node> shp = SmartHazardPointer(
   *     nodePool−>allocate());
   *   // assign via default copy constructor:
   *   *shp = Node(...); 
   */
  template <typename T>
  class SmartHazardPointer {
  private:
    static HazardPointerTable * table;
    T * ptr;

  public:
    SmartHazardPointer(T ** node) {
      while (true) {
        ptr = *node;
        table->add(ptr);
        // Full fence:
        embb_atomic_memory_barrier();
        T * reread = *node;
        // @TODO: Prove practical wait-freedom
        if (read == reread) {
          return;
        }
      }
    }
    SmartHazardPointer(T * node) {
      ptr = node;
    }
    /**
     * Dereference.
     */
    T * operator->() {
      return ptr;
    }
    /**
     * Dereference lvalue.
     */
    T & operator*() {
      return *ptr;
    }
    /**
     * Equality test with regular pointer
     */
    bool operator==(T * const other) {
      return ptr == other;
    }
    /**
     * Equality test with another SmartHazardPointer
     */
    bool operator==(const SmartHazardPointer & other) {
      return this->ptr == other->ptr;
    }
    /**
     * Destructor, retires hazard pointer.
     */
    ~SmartHazardPointer() {
      table->remove(ptr);
    }
    /**
     * Conversion to regular pointer.
     */
    operator T*() {
      return ptr;
    }
  };

public:
  
  /**
   * An SCX record contains enough information to allow any
   * process to complete a pending SCX operation.
   */
  template<
    typename T,
    class ConcreteDataRecord >
  class ScxRecord {

  public:

    inline ScxRecord(OperationState operationState = InProgress) :
      state(operationState) {
    }

    /**
     * Tentatively performs an SCX (extended store-conditional)
     * operation on this SCX record.
     * Returns true if the given value has been stored 
     * successfully, otherwise false.
     * 
     * Preconditions: 
     * - For each r in data_records, the calling thread has 
     *   performed an invocation Ir of r.LLX linked to this SCX
     *   record.
     * - The given value is not the initial state of the field
     *   referenced by fieldIndex.
     * - For each r in data_records, no 
     *   SCX(data_records',finalize_range',fieldIndex,newValue)
     *   was linearized before Ir was linearized.
     */
    bool TrySCX(T newValue, unsigned int fieldIndex) {
      
    }

    /**
     * Performs a VLX (extended validate link) operation on this 
     * SCX record. 
     * Returns true if the calling thread's link obtained by its
     * most recent invocation of SCX is still valid.
     *
     * Precondition: 
     * - for each data record r in data_records, the active thread
     *   has performed and r.LLX linked to this VLX.
     */
    bool VLX() {
      // @TODO
      return true;
    }

    inline OperationState State() const {
      return state;
    }

    inline void SetState(OperationState newState) {
      state = newState;
    }

  private:

    /**
     * Returns true if helped operation has been completed.
     */
    bool Help() {
      // We ensure that an SCX S does not change a data record 
      // while it is frozen for another SCX S'. Instead, S uses 
      // the information in the SCX record of S' to help S'
      // complete, so that the data record can be unfrozen.

      // Freeze all data records in data_records to protect their 
      // mutable fields from being changed by other SCXs:
      for (unsigned int fieldIdx = 0;
           fieldIdx < MutableFieldContainer::Size();
           ++fieldIdx) {
        DataRecord * r    = data_records[fieldIdx];
        // pointer indexed by r in this->info_fields:
        ScxRecord * rinfo = &info_fields[fieldIdx];
        if (!r->Info().CompareAndSwap(rinfo, this)) {
          if (r.Info().Load() != this) {
            // could not freeze r because it is frozen for 
            // another SCX:
            if (all_frozen) {
              // SCX already completed:
              return true;
            }
            // atomically unfreeze all nodes frozen for this SCX:
            state = Aborted;
            return false;
          }
        }
      }
      // finished freezing data records
      assert(state == InProgress || state == Comitted);
      all_frozen = true;
      for (unsigned int fieldRangeIdx = finalize_range.first;
           fieldRangeIdx < finalize_range.second;
           ++fieldRangeIdx) {
        data_records[fieldRangeIdx].MarkForFinalize();
      }
      T expectedOldValue = old;
      field_index.CompareAndSwap(expectedOldValue, newValue);
      // finalize all r in data_records within finalize range, 
      // unfreeze all r in data_records outside of finalize range. 
      // Linearization point of this operation.
      state = Comitted;
      return true;
    }

  private:
    // Note: 
    // data_records 'V', finalize_range 'R', field_index 'fld' 
    // and 'new' store the arguments of the SCX operation that 
    // created the SCX record.

    /**
     * Sequence of data records. 
     * Named 'V' in original publication.
     */
    ::std::vector<ConcreteDataRecord> data_records;
    /**
     * Subsequence (index range) of data_records to be finalized.
     * Named 'R' in original publication.
     */
    ::std::pair<size_t, size_t> finalize_range;
    /**
     * Pointer (pool index) to a mutable field of a data record 
     * in data_records.
     * Named 'fld' in original publication.
     */
    embb::base::Atomic<size_t> field_index;
    /**
     * Value to be written in field referenced by field_index.
     */
    T new;
    /**
     * Value previously read from field referenced by field_index.
     */
    T old;
    /**
     * Current state of this SCX record.
     */
    OperationState state;
    /**
     * Whether all fields are currently frozen, initially false. 
     * Set to true after all data records in data_records V have 
     * been frozen for the SCX.
     */
    bool all_frozen;
    /**
     * Sequence of references (indices) read from the 'info' field 
     * of each element in data_records.
     */
    ::std::vector<size_t> info_fields;
  }; /* class ScxRecord */
  
  /**
   * User-defined data record.
   * Mutable fields must each be contained in a single word.
   * Size of immutable fields is arbitrary.
   */
  template<
    typename T,
    typename MutableFieldsContainer,
    typename ImmutableFieldsContainer >
  class DataRecord {

  private:
    typedef DataRecord self_t;
    typedef struct LlxResult_s {
      DataRecord * DataRecord;
      ScxRecord * ScxRecord;
      MutableFieldsContainer MutableFields;
    } LlxResult;

    /**
     * The dummy SCX record always has state = Aborted.
     */
    static const ScxRecord dummy_scx(Aborted);

  private:
    unsigned int ThreadId() {
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

  public:
    /**
     * Constructor.
     */
    DataRecord(
      unsigned int maxThreads = 
        embb::base::Thread::GetThreadsMaxCount()) :
      max_threads(maxThreads),
      info(&dummy_scx),
      marked(false) {
      llx_results = static_cast<LlxResult *>(
        embb::base::Allocation::AllocateCacheAligned(
          sizeof(LlxResult) * max_threads));
    }

    /**
     * Destructor.
     */
    ~DataRecord() {
      embb::base::Allocation::FreeAligned(llx_results);
    }
    
    /**
     * Tentatively performs an LLX (extended load-linked) 
     * operation on this data record.
     * Returns true and stores result in given reference
     * variable on success, otherwise returns false.
     */
    bool TryLLX(MutableFieldsContainer & mutableFields) {
      bool           marked_1   = marked;
      ScxRecord *    curr_info  = info;
      OperationState curr_state = curr_info->State();
      bool           marked_2   = marked;
      if (curr_state == Aborted ||
          (curr_state == Comitted && !marked_2) {
        // read mutable fields into local variables:
        MutableFieldsContainer mutable_fields_local(
          mutable_fields);
        if (info == &curr_info) {
          // store <r, curr_info, mutable_fields_local> in
          // the thread-specific table:
          LlxResult llx_result;
          llx_result.DataRecord    = this;
          llx_result.ScxRecord     = curr_info;
          llx_result.MutableFields = mutable_fields_local;
          llx_results[ThreadId()]  = llx_result;
          // Set return value:
          mutableFields = mutable_fields_local;
        }
      }
      if (curr_info->State() == Comitted || 
          (curr_info->State() == InProgress && 
           curr_info->Help()) && 
           marked) {
        // Successfully completed active SCX:
        return true;
      }
      if (info->State() == InProgress) {
        // Help active SCX:
        info->Help();
      }
      return false;
    }

    /**
     * When an SCX accesses a data record, it changes 
     * the info field of the data record to point to 
     * its SCX record.
     */
    void Access(const ScxRecord & scx) {
      info = &scx;
    }
   
    /**
     * While this SCX is active, the info field acts as
     * a kind of lock on the data record, granting
     * exclusive access to this SCX, rather than to a
     * process.
     * A data record r is frozen for an SCX-record U if
     * r.info points to U and either U.state is InProgress,
     * or U.state is Committed and r is marked.
     * While a data record r is frozen for an SCX record U,
     * a mutable field f of r can be changed only if f is
     * the field pointed to by U.fld, and it can only be
     * changed by a process helping the SCX that created U.
     */
    inline bool IsFrozenFor(const ScxRecord & scx) const {
      return info == &scx &&
             (scx.State() == InProgress ||
              (scx.State() == Comitted && marked));
    }

    /**
     * Returns the pointer to the SCX record holding 
     * exclusive access to this data record.
     */
    inline embb::base::Atomic<ScxRecord *> & const Info() {
      return info;
    }

    /** 
     * Mark this data record for finalizing.
     */
    inline void MarkForFinalize() {
      marked = true;
    }

  private:
    /**
     * Maximum number of threads operating on any data 
     * record instance.
     */
    unsigned int max_threads;
    
    /**
     * Maps thread id to result from latest LLX performed 
     * by the respective thread.
     */
    LlxResult * llx_results;
    
    /**
     * Pointer to SCX record that describes the last 
     * SCX that accessed this data record, initialized 
     * with dummy SCX record.
     */
    embb::base::Atomic<ScxRecord *> info;
    
    /**
     * Marked flag, used to finalize this data record.
     * The marked bit is initially False and only ever 
     * changes from alse to true.     
     */
    bool marked;
    
    /**
     * User-defined mutable fields, accessible using the 
     * random-access operator[](int fieldIndex).
     */
    MutableFieldsContainer mutable_fields;
    
    /**
     * User-defined immutable fields, each contained in a 
     * single word and accessible using the random-access
     * operator[](int fieldIndex).
     */
    ImmutableFieldsContainer immutable_fields;

  private:
    /**
     * To prevent default- and copy-construction.
     */
    DataRecord();
    DataRecord(const DataRecord &);
    DataRecord & operator=(const DataRecord &);
  }; /* class DataRecord */
};

} // namespace primitives
} // namespace containers
} // namespace embb

#endif /* EMBB_CONTAINERS_PRIMITIVES_LLX_SCX_H_ */