#ifndef EMBB_BASE_INTERNAL_GUARDED_POINTERS_I_GUARDED_POINTERS_H_
#define EMBB_BASE_INTERNAL_GUARDED_POINTERS_I_GUARDED_POINTERS_H_

#include <cstddef>
#include <cassert>
#include <utility>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <embb/base/internal/guarded_pointers/callback_instance.h>

namespace embb {
namespace base {
namespace internal{ 
namespace guarded_pointers{

template< typename GuardType >
class IGuardedPointers
{

private: 

  typedef IGuardedPointers<GuardType> self_t; 

  /** @brief  Disable default construction */
  IGuardedPointers() { }
  /** @brief  Disable copy construction */
  IGuardedPointers(const self_t & other) { }
  /** @brief  Disable assignment */
  self_t & operator=(const self_t & other) { }

public:

  CallbackInstance<GuardType>& callBackForPointerIsReleasedEvent;

  IGuardedPointers(CallbackInstance<GuardType>& callBackForPointerIsReleasedEvent) :
    callBackForPointerIsReleasedEvent(callBackForPointerIsReleasedEvent)
  {}

  virtual ~IGuardedPointers() {}

  virtual void GuardPointer(int guardNumberForThisThread, GuardType pointerToGuard) = 0;

  virtual void EnqueuePointerForDeletion(GuardType pointerToGuard) = 0;
  virtual void ReleaseGuard(int guardNumberForThisThread, GuardType pointerToGuard) = 0; 
  virtual void DeactivateCurrentThread() = 0;  
  virtual int GetGuardsPerThread() = 0; 
  virtual GuardType SentinelValue() const = 0; 

};

} //namespace memory_allocation
} //namespace internal
} //namespace base
} //namespace embb



#endif // EMBB_BASE_INTERNAL_GUARDED_POINTERS_I_GUARDED_POINTERS_H_
