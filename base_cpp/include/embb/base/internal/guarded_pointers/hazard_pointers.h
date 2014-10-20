#ifndef EMBB_BASE_INTERNAL_GUARDED_POINTERS_HAZARD_POINTERS_H_
#define EMBB_BASE_INTERNAL_GUARDED_POINTERS_HAZARD_POINTERS_H_

#include <cstddef>
#include <cassert>
#include <utility>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <list>
#include <vector>
#include <algorithm> 

#include <embb/base/internal/guarded_pointers/i_guarded_pointers.h>
#include <embb/base/atomic.h>
#include <embb/base/thread_specific_storage.h>
#include <embb/base/thread.h>

namespace embb {
namespace base {
namespace internal{
namespace guarded_pointers {

#define RETIRE_THRESHOLD 1.25

template< typename GuardType, unsigned int GuardsPerThread >
class HazardPointer
{

private:

  embb::base::Atomic< bool > active;
  embb::base::Atomic< GuardType > guarded_pointer[GuardsPerThread];

  // TODO, find replacement for that... std::list allocates memory
  ::std::list< GuardType > retired;
  int retiredCounter;

public:

  ::std::list< GuardType >& getRetired()
  {
    return retired;
  }

  void setRetired(::std::list<GuardType> & other)
  {
    retired.clear();
    typename ::std::list<GuardType>::iterator it;
    typename ::std::list<GuardType>::const_iterator end = other.end();
    for (it = other.begin(); it != end; ++it) {
      retired.push_back(*it); 
    }
  }

  int& getRetiredCounter()
  {
    return retiredCounter;
  }

  bool tryReserve()
  {
    bool expected = false;
    return active.CompareAndSwap(expected, true);
  }

  void deactivate()
  {
    active = false;
  }

  GuardType getGuard(int pos) const
  {
    return guarded_pointer[pos];
  }

  bool isActive() const
  {
    return active;
  }

  void release(int guardNumber, GuardType)
  {
    // Deactivate the guard with the given number: 
    guarded_pointer[guardNumber] = (GuardType)NULL;
  }

  void addRetired(GuardType pointerToGuard)
  {
    retired.push_back(pointerToGuard);
    retiredCounter++;
  }

  void guardPointer(int guardNumber, GuardType pointerToGuard)
  {
    guarded_pointer[guardNumber] = pointerToGuard;
  }

  HazardPointer() :
    active(false), retiredCounter(0)
  {
    for (int i = 0; i != GuardsPerThread; ++i)
    {
      guarded_pointer[i] = (GuardType)NULL;
    }
  }

  ~HazardPointer() 
  { 
    // Do not delete guarded_pointer array, as 
    // HazardPointers::callBackForPointerIsReleasedEvent 
    // is solely responsible for freeing guarded 
    // elements. 
  }

private:
  /*
  HazardPointer(const HazardPointer &)
  { }

  const HazardPointer & operator=(const HazardPointer &)
  { }
  */
};

template< typename GuardType, int GuardsPerThread >
class HazardPointers : public IGuardedPointers< GuardType >
{

private:

  // Array of hazard pointers... its size is max size of threads
  HazardPointer< GuardType, GuardsPerThread >* hazard_pointer_list;

  unsigned int hazard_list_size;
  int active_hazard_pointers;

  HazardPointer< GuardType, GuardsPerThread >* acquire()
  {
    for (unsigned int i = 0; i != hazard_list_size; ++i)
    {
      if (hazard_pointer_list[i].tryReserve())
      {
        active_hazard_pointers++;
        return &hazard_pointer_list[i];
      }
    }

    // We ran out of hazard entries... should not happen!
    assert(false);
    return NULL;
  }

  embb::base::ThreadSpecificStorage< HazardPointer< GuardType, GuardsPerThread > * >
    thread_local_storage_hazard_pointer;

  HazardPointer< GuardType, GuardsPerThread >* hazardPointerForLocalThread()
  {
    HazardPointer< GuardType, GuardsPerThread >* hp = 
      thread_local_storage_hazard_pointer.Get();

    if (hp == NULL)
    {
      HazardPointer< GuardType, GuardsPerThread >* ptr(acquire());
      thread_local_storage_hazard_pointer.Get() = ptr;
      hp = ptr; 
    }

    return hp;
  }

public:

  virtual void DeactivateCurrentThread()
  {
    // Set active to false...
    // now the other threads will take care of this threads left
    // nodes...
    hazardPointerForLocalThread()->deactivate();
    active_hazard_pointers--;

    // Reset thread local storage to null...
    HazardPointer< GuardType, GuardsPerThread > * null_hp(NULL);
    thread_local_storage_hazard_pointer.Get() = null_hp;

    if (active_hazard_pointers == 0) {
      Finalize(); 
    }
  }

  virtual ~HazardPointers() 
  { }

  void Finalize() {
    // Call delete-callback on every unguarded, non-deleted 
    // guard element. 
    // Note that no hazard pointer should be active at this 
    // point if every thread called DeactivateCurrentThread() 
    // on completion. 
    for (unsigned int i = 0; i != hazard_list_size; ++i)
    {
      if (!hazard_pointer_list[i].isActive())
      {
        for (int pos = 0; pos != GuardsPerThread; ++pos)
        {
          GuardType guard = hazard_pointer_list[i].getGuard(pos);
          if (SentinelValue() == guard) continue;
          // Active guard found, call release: 
          this->callBackForPointerIsReleasedEvent(guard);
        }
      }
    }
  }

  HazardPointers(CallbackInstance<GuardType>& cb) :
		IGuardedPointers<GuardType>(cb), 
    hazard_list_size(Thread::GetThreadsMaxCount()), 
    active_hazard_pointers(0)
  {
    // Create array of hazard pointers
    hazard_pointer_list = new HazardPointer< GuardType, GuardsPerThread >[hazard_list_size];
  }

  virtual void GuardPointer(int guardNumberForThisThread, GuardType pointerToGuard)
  {
    hazardPointerForLocalThread()->guardPointer(guardNumberForThisThread, pointerToGuard);
  }

  virtual void EnqueuePointerForDeletion(GuardType pointerToGuard)
  {
    hazardPointerForLocalThread()->addRetired(pointerToGuard);
    if (isThresholdExceeded())
    {
      scan();
      // The original Paper suggest to here call a method Helpscan...
      // However, that makes no sense, as no thread will set its 
    }
  }

  virtual void ReleaseGuard(int guardNumberForThisThread, GuardType pointerToGuard) {
    hazardPointerForLocalThread()->release(guardNumberForThisThread, pointerToGuard);
  }

  virtual int GetGuardsPerThread()
  {
    return GuardsPerThread;
  }

  virtual GuardType SentinelValue() const {
    // @TODO, so far this just checks that GuardType 
    // can be casted to NULL at compile time. 
    return static_cast<GuardType>(NULL);
  }

private:

  bool isThresholdExceeded() {
    double retiredCounterLocThread =
      (double)hazardPointerForLocalThread()->getRetiredCounter();
    return retiredCounterLocThread >=
      RETIRE_THRESHOLD *
      active_hazard_pointers *
      GuardsPerThread;
  }

  void helpScan()
  {
    for (unsigned int i = 0; i != hazard_list_size; ++i)
    {
      // try to find non active lists...
      if (!hazard_pointer_list[i].isActive() && hazard_pointer_list[i].tryReserve())
      {
        // here: grab retired things, first check if there are any...
        if (hazard_pointer_list[i].getRetiredCount() > 0)
        {
          // if we have retired things, add them to our list...
          for (int pos = 0; pos != GuardsPerThread; ++pos)
          {
            GuardType guard = hazard_pointer_list[i].getGuard(pos);
            if (NULL == guard) continue;

            // remove retired pointer from the other list ("dead" thread)
            hazard_pointer_list[i].guardPointer(pos, NULL);
            hazard_pointer_list[i]->getRetiredCounter()--;

            // add it to our list...
            hazardPointerForLocalThread()->addRetired(guard);

            // if our list exceeds the threshold, scan again..
            if (isThresholdExceeded())
            {
              scan();
            }
          }
        }

        //we are done, mark it deactivated again
        hazard_pointer_list[i].deactivate();
      }
    }
  }

  void scan()
  {
    // Filter active hazard pointers to temporary list: 
    ::std::vector<GuardType> found_guards;
    for (unsigned int i = 0; i != hazard_list_size; ++i)
    {
      if (hazard_pointer_list[i].isActive())
      {
        for (int pos = 0; pos != GuardsPerThread; ++pos)
        {
          GuardType guard = hazard_pointer_list[i].getGuard(pos);
          if (SentinelValue() == guard) continue;
          found_guards.push_back(guard);
        }
      }
    }
    // Sort for later std::binary_search: 
    ::std::sort(found_guards.begin(), found_guards.end());

    ::std::list<GuardType> & retired = hazardPointerForLocalThread()->getRetired();

    // List to contain all non-erased hazard pointers: 
    ::std::list<GuardType> retired_new;
    // List to contain all erased hazard pointers: 
    ::std::list<GuardType> retired_erase;

    // Iterate over set intersection of active guards and retired list: 
    typename ::std::list<GuardType>::iterator retired_it = retired.begin();
    for (; retired_it != retired.end(); ++retired_it) {
      // O(Log2 n)
      if (false == ::std::binary_search(found_guards.begin(), found_guards.end(), *retired_it)) {
        // Locally retired hazard pointer is inactive, release it:
        retired_erase.push_back(*retired_it);
      }
      else {
        // Locally retired hazard pointer is still active, 
        // remember this one for later: 
        retired_new.push_back(*retired_it);
      }
    }

    // O(n), as std::list operator= implements clear & copy: 
    hazardPointerForLocalThread()->setRetired(retired_new);

    // Erase all pointers marked for erase: 
    typename ::std::list<GuardType>::iterator erase_it = retired_erase.begin();
    for (; erase_it != retired_erase.end(); ++erase_it) {
      this->callBackForPointerIsReleasedEvent(*erase_it);
      hazardPointerForLocalThread()->getRetiredCounter()--;
    }
  }

};

} //namespace memory_allocation
} //namespace internal
} //namespace base
} //namespace embb

#endif // EMBB_BASE_INTERNAL_GUARDED_POINTERS_HAZARD_POINTERS_H_
