#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <stdint.h>
#include <stddef.h>

/* Macro for alignment */
#define MEM_ALIGN(addr, align) (((addr) + ((align)-1)) & ~((align)-1))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * @brief Memory Configuration used during init API
 */
typedef struct DPC_ObjectDetection_MemCfg_t
{
    /*! @brief   Start address of memory provided by the application
     *           from which DPC will allocate.
     */
    void *addr;

    /*! @brief   Size limit of memory allowed to be consumed by the DPC */
    uint32_t size;
} DPC_ObjectDetection_MemCfg;

/*
 * @brief Memory pool object to manage memory based on @ref DPC_ObjectDetection_MemCfg_t.
 */
typedef struct MemPoolObj_t
{
    /*! @brief Memory configuration */
    DPC_ObjectDetection_MemCfg cfg;

    /*! @brief   Pool running adress.*/
    uintptr_t currAddr;

    /*! @brief   Pool max address. This pool allows setting address to desired
     *           (e.g for rewinding purposes), so having a running maximum
     *           helps in finding max pool usage
     */
    uintptr_t maxCurrAddr;
} MemPoolObj;

/* Function prototypes */
void DPC_ObjDet_MemPoolReset(MemPoolObj *pool);
static void DPC_ObjDet_MemPoolSet(MemPoolObj *pool, void *addr);
static void *DPC_ObjDet_MemPoolGet(MemPoolObj *pool);
uint32_t DPC_ObjDet_MemPoolGetMaxUsage(MemPoolObj *pool);
void *DPC_ObjDet_MemPoolAlloc(MemPoolObj *pool, uint32_t size, uint8_t align);

#endif /* MEM_POOL_H */
