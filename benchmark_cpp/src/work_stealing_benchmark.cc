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

#include <embb/benchmark/benchmark.h>
#include <embb/benchmark/scenario.h>
#include <embb/benchmark/call_args.h>
#include <embb/benchmark/internal/console.h>
#include <embb/benchmark/internal/util.h>
#include <embb/benchmark/scheduling/work_stealing_benchmark.h>
#include <embb/base/perf/timer.h>

#include <mtapi.h>
#include <embb/mtapi/c/mtapi.h>

#include <vector>
#include <iostream>
#include <stdexcept>

namespace embb {
namespace benchmark {

using embb::base::perf::Timer;

WorkStealingBenchmark::
WorkStealingBenchmark(const CallArgs & callArgs)
: args(callArgs), measurements(callArgs) {

}

WorkStealingBenchmark::
  ~WorkStealingBenchmark()
{
}

void WorkStealingBenchmark::
Run() {
  MtapiInit(args); 
}

void WorkStealingBenchmark::
MtapiInit(const CallArgs & args) {
  mtapi_info_t info;
  mtapi_status_t status;

  mtapi_node_attributes_t nodeAttr;
  mtapi_nodeattr_init(&nodeAttr, &status);
  mtapi_nodeattr_set(&nodeAttr,
    MTAPI_NODE_MAX_PRIORITIES,
    MTAPI_ATTRIBUTE_VALUE(3),
    MTAPI_ATTRIBUTE_POINTER_AS_VALUE,
    &status);
  mtapi_nodeattr_set(&nodeAttr,
    MTAPI_NODE_MAX_TASKS,
    MTAPI_ATTRIBUTE_VALUE(10000),
    MTAPI_ATTRIBUTE_POINTER_AS_VALUE,
    &status);
  mtapi_nodeattr_set(&nodeAttr,
    MTAPI_NODE_NUMCORES,
    MTAPI_ATTRIBUTE_VALUE(args.NumCores()),
    MTAPI_ATTRIBUTE_POINTER_AS_VALUE,
    &status);

  mtapi_initialize(1, 1, &nodeAttr, &info, &status);

  lowFreqAction = mtapi_action_create(
    1,
    lowFreqFunction,
    MTAPI_NULL, 0, MTAPI_NULL, &status);
  lowFreqJob = mtapi_job_get(1, 1, &status);

  highFreqAction = mtapi_action_create(
    2,
    highFreqFunction,
    MTAPI_NULL, 0, MTAPI_NULL, &status);
  highFreqJob = mtapi_job_get(2, 1, &status);
}

/*
void WorkStealingBenchmark::
LowFreqProcess() {

  mtapi_group_hndl_t group;
  mtapi_task_attributes_t ta;

  unsigned int bidx = 0;
  mtapi_status_t status;
  mtapi_taskattr_init(&ta, &status);
  mtapi_taskattr_set(&ta, MTAPI_TASK_PRIORITY, MTAPI_ATTRIBUTE_VALUE(1), MTAPI_ATTRIBUTE_POINTER_AS_VALUE, &status);
  group = mtapi_group_create(MTAPI_GROUP_ID_NONE, MTAPI_NULL, &status);
  
  for (unsigned int i = 0; i < args.NumElements(); i += 16)
  {
    element_pool[i] = i;

    mtapi_task_start(
      MTAPI_TASK_ID_NONE, 
      smallJob, 
      &element_pool[i], 
      sizeof(element_pool_t), 
      MTAPI_NULL, 
      0, 
      &ta, 
      group,
      &status);
    bidx++;
  }
  
  mtapi_group_wait_all(group, MTAPI_INFINITE, &status);
}
*/

void WorkStealingBenchmark::
lowFreqFunction(
  const void * /* args */,
  mtapi_size_t /* args_size */,
  void *       /* result_buffer */,
  mtapi_size_t /* result_buffer_size */,
  const void * /* node_local_data */,
  mtapi_size_t /* node_local_data_size */,
  mtapi_task_context_t * context)
{
  mtapi_status_t status;
  mtapi_uint_t core = mtapi_context_corenum_get(context, &status);
}

void WorkStealingBenchmark::
highFreqFunction(
  const void *           /* args */,
  mtapi_size_t           /* args_size */,
  void *                 /* result_buffer */,
  mtapi_size_t           /* result_buffer_size */,
  const void *           /* node_local_data */,
  mtapi_size_t           /* node_local_data_size */,
  mtapi_task_context_t * context)
{
  mtapi_status_t status;
  mtapi_uint_t core = mtapi_context_corenum_get(context, &status);
}

void WorkStealingBenchmark::
lowFreqCallback(void *pBuffer, long bufferLen)
{
  mtapi_task_attributes_t ta;
  mtapi_status_t status;
  mtapi_task_hndl_t task;
  mtapi_taskattr_init(&ta, &status);

  mtapi_taskattr_set(
    &ta,
    MTAPI_TASK_PRIORITY,
    MTAPI_ATTRIBUTE_VALUE(lowFreqPriority),
    MTAPI_ATTRIBUTE_POINTER_AS_VALUE,
    &status);

  task = mtapi_task_start(
    MTAPI_TASK_ID_NONE,
    lowFreqJob,
    MTAPI_NULL, 0,
    pBuffer, bufferLen,
    &ta,
    MTAPI_GROUP_NONE,
    &status);

  mtapi_task_wait(task, MTAPI_INFINITE, &status);
}

void WorkStealingBenchmark::
highFreqCallback(void *pBuffer, long bufferLen)
{
  mtapi_task_attributes_t ta;
  mtapi_status_t status;
  mtapi_task_hndl_t task;
  mtapi_taskattr_init(&ta, &status);

  mtapi_taskattr_set(
    &ta,
    MTAPI_TASK_PRIORITY,
    MTAPI_ATTRIBUTE_VALUE(highFreqPriority),
    MTAPI_ATTRIBUTE_POINTER_AS_VALUE,
    &status);

  task = mtapi_task_start(
    MTAPI_TASK_ID_NONE,
    highFreqJob,
    MTAPI_NULL, 0,
    pBuffer, bufferLen,
    &ta,
    MTAPI_GROUP_NONE,
    &status);

  mtapi_task_wait(task, MTAPI_INFINITE, &status);
}

unsigned int WorkStealingBenchmark::lowFreqPriority  = 0; 
unsigned int WorkStealingBenchmark::highFreqPriority = 2; 

mtapi_job_hndl_t WorkStealingBenchmark::lowFreqJob;
mtapi_job_hndl_t WorkStealingBenchmark::highFreqJob;

mtapi_action_hndl_t WorkStealingBenchmark::lowFreqAction;
mtapi_action_hndl_t WorkStealingBenchmark::highFreqAction;

} // namespace benchmark
} // namespace embb