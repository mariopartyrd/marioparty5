#include "game/memory.h"
#include "game/init.h"
#include "dolphin/os.h"

static u32 HeapSizeTbl[HEAP_MAX] = { 0x200000, 0xC0000, 0xB00000, 0x580000, 0 };
static void *HeapTbl[HEAP_MAX];

void HuMemInitAll(void)
{
    s32 i;
    void *ptr;
    u32 free_size;
    for(i=0; i<4; i++) {
        ptr = OSAlloc(HeapSizeTbl[i]);
        if(ptr == NULL) {
            OSReport("HuMem> Failed OSAlloc Size:%d(left:%x)\n", HeapSizeTbl[i], OSCheckHeap(currentHeapHandle));
            return;
        }
        HeapTbl[i] = HuMemInit(ptr, HeapSizeTbl[i]);
    }
    free_size = OSCheckHeap(currentHeapHandle);
    OSReport("HuMem> left memory space %dKB(%d)\n", free_size/1024, free_size);
    ptr = OSAlloc(free_size);
    if(ptr == NULL) {
        OSReport("HuMem> Failed OSAlloc left space\n");
        return;
    }
    HeapTbl[4] = HuMemInit(ptr, free_size);
    HeapSizeTbl[4] = free_size;
}

void *HuMemInit(void *ptr, s32 size)
{
    return HuMemHeapInit(ptr, size);
}

void HuMemDCFlushAll()
{
    HuMemDCFlush(2);
    HuMemDCFlush(0);
}

void HuMemDCFlush(HEAPID heap)
{
    DCFlushRangeNoSync(HeapTbl[heap], HeapSizeTbl[heap]);
}

void *HuMemDirectMalloc(HEAPID heap, s32 size)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    size = OSRoundUp32B(size);
    return HuMemMemoryAlloc(HeapTbl[heap], size, retaddr);
}

void *HuMemDirectMallocNum(HEAPID heap, s32 size, u32 num)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    size = OSRoundUp32B(size);
    return HuMemMemoryAllocNum(HeapTbl[heap], size, num, retaddr);
}

void *HuMemDirectTailMalloc(HEAPID heap, s32 size)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    size = OSRoundUp32B(size);
    return HuMemTailMemoryAlloc(HeapTbl[heap], size, retaddr);
}

void HuMemDirectFree(void *ptr)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    HuMemMemoryFree(ptr, retaddr);
}

void HuMemDirectFreeNum(HEAPID heap, u32 num)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    HuMemMemoryFreeNum(HeapTbl[heap], num, retaddr);
}

s32 HuMemUsedMallocSizeGet(HEAPID heap)
{
    return HuMemUsedMemorySizeGet(HeapTbl[heap]);
}

s32 HuMemUsedMallocBlockGet(HEAPID heap)
{
    return HuMemUsedMemoryBlockGet(HeapTbl[heap]);
}

u32 HuMemHeapSizeGet(HEAPID heap)
{
    return HeapSizeTbl[heap];
}

void *HuMemHeapPtrGet(HEAPID heap)
{
    return HeapTbl[heap];
}