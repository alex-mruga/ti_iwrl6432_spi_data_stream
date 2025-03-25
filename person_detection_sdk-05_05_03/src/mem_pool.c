/*!
* @file mem_pool.c
*
* This file implements Memory Pool management functions.
*
*/
#include <stdint.h>
#include <stddef.h>

#include "system.h"
#include "mem_pool.h"

void DPC_ObjDet_MemPoolReset(MemPoolObj *pool) {
    pool->currAddr = (uintptr_t)pool->cfg.addr;
    pool->maxCurrAddr = pool->currAddr;
}

static void DPC_ObjDet_MemPoolSet(MemPoolObj *pool, void *addr) {
    pool->currAddr = (uintptr_t)addr;
    pool->maxCurrAddr = MAX(pool->currAddr, pool->maxCurrAddr);
}

static void *DPC_ObjDet_MemPoolGet(MemPoolObj *pool) {
    return((void *)pool->currAddr);
}

uint32_t DPC_ObjDet_MemPoolGetMaxUsage(MemPoolObj *pool) {
    return((uint32_t)(pool->maxCurrAddr - (uintptr_t)pool->cfg.addr));
}

void *DPC_ObjDet_MemPoolAlloc(MemPoolObj *pool,
                              uint32_t size,
                              uint8_t align) {
    void *retAddr = NULL;
    uintptr_t addr;

    addr = MEM_ALIGN(pool->currAddr, align);
    if ((addr + size) <= ((uintptr_t)pool->cfg.addr + pool->cfg.size))
    {
        retAddr = (void *)addr;
        pool->currAddr = addr + size;
        pool->maxCurrAddr = MAX(pool->currAddr, pool->maxCurrAddr);
    }

    return(retAddr);
}