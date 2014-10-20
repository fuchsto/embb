#ifndef EMBB_BASE_INTERNAL_GUARDED_POINTERS_CALLBACK_INSTANCE_H_
#define EMBB_BASE_INTERNAL_GUARDED_POINTERS_CALLBACK_INSTANCE_H_

#include <cstddef>
#include <cassert>
#include <utility>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>


namespace embb {
namespace base {
namespace internal{ namespace guarded_pointers{

template<typename GuardType>
class CallbackInstance {
public:
  virtual void operator()(GuardType p) const = 0;
  virtual ~CallbackInstance() {}
};

template<class C, typename GuardType>
class CallbackInstanceFromObject : public CallbackInstance<GuardType>
{
private:
  typedef CallbackInstanceFromObject<C, GuardType> self_t;
  /** @brief  Disable default construction */
  CallbackInstanceFromObject() { }
  /** @brief  Disable copy construction */
  CallbackInstanceFromObject(const self_t & other) { }
  /** @brief  Disable assignment */
  self_t & operator=(const self_t & other) { }

public:
  CallbackInstanceFromObject(C& classReference, void (C::*classMethod)(GuardType p))
    : classReference(classReference), classMethod(classMethod) {}
  virtual void operator()(GuardType p) const
  {
    (&classReference->*classMethod)(p);
  }
private:
  C & classReference;
  void (C::*classMethod)(GuardType p);
};

template<typename GuardType>
class CallbackInstanceFromFunction : public CallbackInstance<GuardType>
{
public:
  CallbackInstanceFromFunction(void(*classMethod)(GuardType p))
    : classMethod(classMethod) {}
  virtual void operator()(GuardType p) const
  {
    classMethod(p);
  }
private:
  void(*classMethod)(GuardType p);
};


} //namespace memory_allocation
} //namespace internal
} //namespace base
} //namespace embb



#endif // EMBB_BASE_INTERNAL_GUARDED_POINTERS_CALLBACK_INSTANCE_H_