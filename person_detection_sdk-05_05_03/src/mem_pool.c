#include "mem_pool.h"

/**
 *  @b Description
 *  @n
 *      Utility function for reseting memory pool.
 *
 *  @param[in]  pool Handle to pool object.
 *
 *  \ingroup DPC_OBJDET__INTERNAL_FUNCTION
 *
 *  @retval
 *      none.
 */
void DPC_ObjDet_MemPoolReset(MemPoolObj *pool)
{
    pool->currAddr = (uintptr_t)pool->cfg.addr;
    pool->maxCurrAddr = pool->currAddr;
}


/**
 *  @b Description
 *  @n
 *      Utility function for setting memory pool to desired address in the pool.
 *      Helps to rewind for example.
 *
 *  @param[in]  pool Handle to pool object.
 *  @param[in]  addr Address to assign to the pool's current address.
 *
 *  \ingroup DPC_OBJDET__INTERNAL_FUNCTION
 *
 *  @retval
 *      None
 */
static void DPC_ObjDet_MemPoolSet(MemPoolObj *pool, void *addr)
{
    pool->currAddr = (uintptr_t)addr;
    pool->maxCurrAddr = MAX(pool->currAddr, pool->maxCurrAddr);
}

/**
 *  @b Description
 *  @n
 *      Utility function for getting memory pool current address.
 *
 *  @param[in]  pool Handle to pool object.
 *
 *  \ingroup DPC_OBJDET__INTERNAL_FUNCTION
 *
 *  @retval
 *      pointer to current address of the pool (from which next allocation will
 *      allocate to the desired alignment).
 */
static void *DPC_ObjDet_MemPoolGet(MemPoolObj *pool)
{
    return((void *)pool->currAddr);
}

/**
 *  @b Description
 *  @n
 *      Utility function for getting maximum memory pool usage.
 *
 *  @param[in]  pool Handle to pool object.
 *
 *  \ingroup DPC_OBJDET__INTERNAL_FUNCTION
 *
 *  @retval
 *      Amount of pool used in bytes.
 */
uint32_t DPC_ObjDet_MemPoolGetMaxUsage(MemPoolObj *pool)
{
    return((uint32_t)(pool->maxCurrAddr - (uintptr_t)pool->cfg.addr));
}

/**
 *  @b Description
 *  @n
 *      Utility function for allocating from a static memory pool.
 *
 *  @param[in]  pool Handle to pool object.
 *  @param[in]  size Size in bytes to be allocated.
 *  @param[in]  align Alignment in bytes
 *
 *  \ingroup DPC_OBJDET__INTERNAL_FUNCTION
 *
 *  @retval
 *      pointer to beginning of allocated block. NULL indicates could not
 *      allocate.
 */
void *DPC_ObjDet_MemPoolAlloc(MemPoolObj *pool,
                              uint32_t size,
                              uint8_t align)
{
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