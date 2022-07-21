/*
 * Created on Tue Aug 18 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

#include "graalsgx_malloc.h"
#include <assert.h>
#include "Enclave.h"

/**
 * Map reserved memory
 */
void *mmap_resv(void *hint, size_t size, int prot, int flags)
{

    int mprot = 0;
    size_t aligned_size, page_size;
    void *ret = NULL;
    sgx_status_t st = SGX_SUCCESS;

    page_size = getpagesize();
    aligned_size = (size + page_size - 1) & ~(page_size - 1);

    if (aligned_size >= UINT32_MAX)
        return NULL;

    ret = sgx_alloc_rsrv_mem(aligned_size);
    if (ret == NULL)
    {
        printf("mmap_resv(size=%u, aligned size=%lu, prot=0x%x) failed.",
               size, aligned_size, prot);
        return NULL;
    }

    if (prot & MMAP_PROT_READ)
        mprot |= SGX_PROT_READ;
    if (prot & MMAP_PROT_WRITE)
        mprot |= SGX_PROT_WRITE;
    if (prot & MMAP_PROT_EXEC)
        mprot |= SGX_PROT_EXEC;

    printf(">>>>>>>>>>>>>>>>> sgx_tprotect_rsrv_mem ....\n");
    st = sgx_tprotect_rsrv_mem(ret, aligned_size, mprot);
    if (st != SGX_SUCCESS)
    {
        printf("mmap_resv(size=%u, prot=0x%x) failed to set protect.",
               size, prot);
        sgx_free_rsrv_mem(ret, aligned_size);
        return NULL;
    }

    printf("mmap-ed reserved mem\n");
    return ret;
}

/**
 * Unmap reserved memory
 */
void munmap_resv(void *addr, size_t size)
{

    size_t aligned_size, page_size;

    page_size = getpagesize();
    aligned_size = (size + page_size - 1) & ~(page_size - 1);
    sgx_free_rsrv_mem(addr, aligned_size);
}

/**
 * Mprotect reserved memory
 */
int mprotect_resv(void *addr, size_t size, int prot)
{

    int mprot = 0;
    sgx_status_t st = SGX_SUCCESS;

    if (prot & MMAP_PROT_READ)
        mprot |= SGX_PROT_READ;
    if (prot & MMAP_PROT_WRITE)
        mprot |= SGX_PROT_WRITE;
    if (prot & MMAP_PROT_EXEC)
        mprot |= SGX_PROT_EXEC;
    st = sgx_tprotect_rsrv_mem(addr, size, mprot);
    if (st != SGX_SUCCESS)
        printf("mprotect_resv(addr=0x%lx, size=%u, prot=0x%x) failed.",
               addr, size, prot);

    printf("mprotect-ed reserved mem\n");
    return (st == SGX_SUCCESS ? 0 : -1);
}

/**
 * Page aligned malloc
 */
void *pg_aligned_malloc(size_t size)
{
    size_t align = getpagesize();

    //if size is not aligned, it is atmost (align - 1) short to be aligned
    size_t tempSize = size + (align - 1);
    void *ptr = malloc(tempSize);

    void *base = ptr + (align - 1);
    base = (void*)((uintptr_t)base & ~(align - 1));
    return (void *)base;
}

void *temp(size_t align, size_t size)
{
    void *ptr = NULL;

    // We want it to be a power of two since
    // align_up operates on powers of two
    assert((align & (align - 1)) == 0);

    if (align && size)
    {
        /*
         * We know we have to fit an offset value
         * We also allocate extra bytes to ensure we 
         * can meet the alignment
         */
        uint32_t hdr_size = PTR_OFFSET_SZ + (align - 1);
        void *p = malloc(size + hdr_size);
        printf("Unaligned addr: %x\n", p);

        if (p)
        {
            /*
             * Add the offset size to malloc's pointer 
             * (we will always store that)
             * Then align the resulting value to the 
             * target alignment
             */
            ptr = (void *)align_up(((uintptr_t)p + PTR_OFFSET_SZ), align);

            // Calculate the offset and store it
            // behind our aligned pointer
            //*((offset_t *)ptr - 1) = (offset_t)((uintptr_t)ptr - (uintptr_t)p);

        } // else NULL, could not malloc
    }     //else NULL, invalid arguments

    printf("  Aligned addr: %x\n", ptr);
    return ptr;
}