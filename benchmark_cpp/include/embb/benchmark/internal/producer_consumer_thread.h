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

#ifndef EMBB_BASE_CPP_BENCHMARK_INTERNAL_PRODUCER_CONSUMER_THREAD_H_
#define EMBB_BASE_CPP_BENCHMARK_INTERNAL_PRODUCER_CONSUMER_THREAD_H_

#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/call_args.h>
#include <embb/base/core_set.h>
#include <embb/base/thread.h>

namespace embb {
namespace benchmark {
namespace internal {

template< 
  typename TUnit, 
  typename TMeasurements >
class ProducerConsumerThread {
private:
  typedef ProducerConsumerThread< TUnit, TMeasurements > self_t; 

protected: 
  TUnit * const        unit;
  TMeasurements *      measurements;
  unsigned int         accessor_id;
  unsigned int         core_affinity;
  bool                 running; 
  bool                 stopped;
  CallArgs             callArgs; 
  embb::base::Thread * thread;
  size_t               n_iterations; 
  size_t               n_produce_elements; 
  size_t               n_consume_elements; 
  /// Whether failed producing operation leads to a clean 
  /// exit instead of an error
  bool quit_on_failed_produce; 
  /// Whether failed consuming operation leads to a clean 
  /// exit instead of an error
  bool quit_on_failed_consume; 

 private: 
  /// Disable default construction. 
  ProducerConsumerThread() { }
  /// Disable copy construction. 
  ProducerConsumerThread(const ProducerConsumerThread & other) { }
  /// Disable assignment. 
  ProducerConsumerThread & operator=(
    const ProducerConsumerThread & other) {
    return *this;
  }    

protected: 
  /// Return pointer to benchmark measurements container. 
  inline TMeasurements & Measurements() {
    return *measurements;
  }
  /// Amount of elements this task handler is configured 
  /// to produce per producer iteration. 
  inline virtual size_t NumProduceElements() const {
    return n_produce_elements;
  }
  /// Amount of elements this task handler is configured 
  /// to consume per consumer iteration. 
  inline virtual size_t NumConsumeElements() const {
    return n_consume_elements;
  }
  /// Amount of iterations of the task body. 
  inline virtual size_t NumIterations() const {
    return n_iterations;
  }
  /// The implementation instance under test. 
  inline TUnit & Unit() {
    return *unit; 
  }
  /// Call adapter for member function.
  template<class TClass, typename TFunc>
  struct MemberCall {
    TFunc f_;
    TClass * self_;
    MemberCall(TFunc f, TClass* self) : f_(f), self_(self) { }
    void operator()() { (self_->*f_)(); }
  };

public:    
  /// Constructor initializing the amount of elements 
  /// to produce and consumer in every iteration from 
  /// callArgs.NumAllocPerIt(). 
  inline ProducerConsumerThread(
    TUnit * benchmarkUnit,
    TMeasurements * measurements,
    unsigned int id,
    unsigned int coreAffinity, 
    const CallArgs & params) 
  : unit(benchmarkUnit),
    measurements(measurements),
    accessor_id(id),
    core_affinity(coreAffinity),
    running(false),
    stopped(false), 
    callArgs(params),
    thread(NULL), 
    n_iterations(params.NumIterations()), 
    n_produce_elements(params.NumAllocsPerIt()),
    n_consume_elements(params.NumAllocsPerIt()),
    quit_on_failed_produce(false), 
    quit_on_failed_consume(false)
  {
    // Pin thread to core indicated by thread id: 
    embb::base::CoreSet coreSet;
    // Empty core set: 
    coreSet.Reset(false); 
    // Add single core id:
    coreAffinity %= embb::base::CoreSet::CountAvailable();
    coreSet.Add(coreAffinity);
    MemberCall<ProducerConsumerThread, void(ProducerConsumerThread::*)()> 
      start(&ProducerConsumerThread::TaskWrapper, this);
    // Run thread on specified core:
    thread = new embb::base::Thread(coreSet, start);
  }
  virtual ~ProducerConsumerThread() { }
  /// Return the accessor id this instance has been 
  /// initialized with. 
  inline virtual unsigned int Id() const {
    return accessor_id;
  }
  /// Return the core id set as affinity for this thread.
  inline virtual unsigned int Core() const {
    return core_affinity;
  }
  /// Whether this task handler is running. 
  inline virtual bool IsRunning() const {
    return running; 
  }
  /// Whether this task handler has received a 
  /// stop request. 
  inline virtual bool IsStopped() const {
    return stopped;
  }
  /// Set number of elements to produce. 
  inline virtual void NumProduceElements(size_t n) {
    n_produce_elements = n; 
  }
  /// Set number of elements to consume. 
  inline virtual void NumConsumeElements(size_t n) {
    n_consume_elements = n;
  }
  /// Set number of iterations
  inline virtual void NumIterations(size_t n) {
    n_iterations = n; 
  }
  /// Whether failed consuming operation (allocation) 
  /// is accectable
  inline virtual void QuitOnFailedConsume(bool doQuit) {
    quit_on_failed_consume = doQuit; 
  }
  inline virtual bool QuitOnFailedConsume() const { 
    return quit_on_failed_consume; 
  }
  /// Whether failed producing operation (allocation) 
  /// is accectable 
  inline virtual void QuitOnFailedProduce(bool doQuit) {
    quit_on_failed_produce = doQuit; 
  }
  inline virtual bool QuitOnFailedProduce() const { 
    return quit_on_failed_produce; 
  }
 
protected: 
  /// Thread task execution wrapper. 
  virtual void TaskWrapper();
  /// Thread task body. 
  virtual void Task() { }
 
public: 
  /// Enters the task body loop. 
  virtual void Run() {
    running = true; 
  }
  /// Complete the thread task and deactivate the thread.   
  virtual void Join() {
    thread->Join();
  }
  /// Request to stop the thread. 
  /// Current iteration will be completed, the thread will 
  /// return at the next possible time. 
  virtual void RequestStop() { 
    stopped = true; 
  }
};

} // namespace internal
} // namespace benchmark
} // namespace embb

#include <embb/benchmark/internal/producer_consumer_thread-inl.h>

#endif /* EMBB_BASE_CPP_BENCHMARK_INTERNAL_PRODUCER_CONSUMER_THREAD_H_ */
