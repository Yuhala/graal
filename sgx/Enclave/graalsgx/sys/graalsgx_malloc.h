/*
 * Created on Tue Aug 18 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

/** 
 * Export some mem management routines
 */
#ifndef GRAALSGX_MALLOC_H
#define GRAALSGX_MALLOC_H

#include <stdlib.h>
#include <stdint.h>
#include "sgx_rsrv_mem_mngr.h"

typedef uint16_t offset_t;
#define PAGE_SHIFT 12
#define PAGE_SIZE 1 << PAGE_SHIFT
#define PTR_OFFSET_SZ 0 //sizeof(offset_t)

#ifndef align_up
#define align_up(num, align) \
    (((num) + ((align)-1)) & ~((align)-1))
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

    void *pg_aligned_malloc(size_t size);
    void *mmap_resv(void *hint, size_t size, int prot, int flags);

#if defined(__cplusplus)
}
#endif

#endif /* GRAALSGX_MALLOC_H */
