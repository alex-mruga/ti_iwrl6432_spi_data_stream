/*!
* @file mem_pool.c
*
* This file implements Memory Pool management functions.
* @copyright Copyright (C) 2022-24 Texas Instruments Incorporated
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
*
*   Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the
*   distribution.
*
*   Neither the name of Texas Instruments Incorporated nor the names of
*   its contributors may be used to endorse or promote products derived
*   from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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