#ifndef EMBB_CONTAINERS_INTERNAL_CACHE_H_
#define EMBB_CONTAINERS_INTERNAL_CACHE_H_

#include <embb/containers/internal/flags.h>

// Allow to disable cache-alignment in containers for 
// benchmarks and testing purposes
#if EMBB_CONTAINERS_DISABLE_CACHE_ALIGN
#  define EMBB_CONTAINERS_CACHE_ALIGN
#  define EMBB_CONTAINERS_VAR_ALIGN
#else
#  define EMBB_CONTAINERS_CACHE_ALIGN EMBB_ALIGN(EMBB_CACHE_LINE_SIZE)
#  define EMBB_CONTAINERS_VAR_ALIGN EMBB_ALIGN(16)
#endif

#ifdef EMBB_ARCH_X86_64
#define EMBB_CONTAINERS_PAD_CACHE(A) ((EMBB_CACHE_LINE_SIZE - (A % EMBB_CACHE_LINE_SIZE))/sizeof(int64_t))
#else
#define EMBB_CONTAINERS_PAD_CACHE(A) ((EMBB_CACHE_LINE_SIZE - (A % EMBB_CACHE_LINE_SIZE))/sizeof(int32_t))
#endif

#endif /* EMBB_CONTAINERS_INTERNAL_CACHE_H_ */
