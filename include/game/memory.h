#ifndef _GAME_MEMORY_H
#define _GAME_MEMORY_H

#include "dolphin/types.h"

#define HuRestMemGet(heap) (HuMemHeapSizeGet((heap))-HuMemUsedMallocSizeGet((heap)))

#define HU_MEMNUM_OVL 0x10000000

typedef enum HeapID_s {
    HEAP_HEAP,
    HEAP_SOUND,
    HEAP_MODEL,
    HEAP_DVD,
    HEAP_SPACE,
    HEAP_MAX
} HEAPID;


void HuMemInitAll(void);
void *HuMemInit(void *ptr, s32 size);
void HuMemDCFlushAll();
void HuMemDCFlush(HEAPID heap);
void *HuMemDirectMalloc(HEAPID heap, s32 size);
void *HuMemDirectMallocNum(HEAPID heap, s32 size, u32 num);
void *HuMemDirectTailMalloc(HEAPID heap, s32 size);
void HuMemDirectFree(void *ptr);
void HuMemDirectFreeNum(HEAPID heap, u32 num);
s32 HuMemUsedMallocSizeGet(HEAPID heap);
s32 HuMemUsedMallocBlockGet(HEAPID heap);
u32 HuMemHeapSizeGet(HEAPID heap);
void *HuMemHeapPtrGet(HEAPID heap);

void *HuMemHeapInit(void *heap, s32 size);
void *HuMemMemoryAlloc(void *heap, s32 size, u32 retAddr);
void *HuMemMemoryAllocNum(void *heap, s32 size, u32 num, u32 retAddr);
void *HuMemTailMemoryAlloc(void *heap, s32 size, u32 retAddr);

void HuMemMemoryFree(void *heap, u32 retAddr);
void HuMemMemoryFreeNum(void *heap, u32 num, u32 retAddr);
s32 HuMemUsedMemorySizeGet(void *heap);
s32 HuMemUsedMemoryBlockGet(void *heap);
s32 HuMemMemorySizeGet(void *heap);
s32 HuMemMemoryAllocSizeGet(s32 size);
void HuMemHeapDump(void *heap, s16 status);

#endif