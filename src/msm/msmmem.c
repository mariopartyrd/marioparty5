#include "msm/msmmem.h"

#include <stdint.h>

typedef struct MSMBlock_s {
    struct MSMBlock_s* prev;
    struct MSMBlock_s* next;
    u32 freeSize;
    u32 size;
    void* ptr;
#if UINTPTR_MAX <= 0xFFFFFFFFu
    char pad[12];
#endif
    // msmMemAlloc() reserves exactly 0x20 bytes of header per block
    // (allocSize = size + 0x20) and hands out (block + 1), so this struct must
    // stay 32 bytes wide. On a 64-bit host the three pointers already fill it,
    // which is why the padding is 32-bit only -- keeping it there would push
    // the user pointer 12 bytes past what the allocator accounted for.
} MSMBLOCK;

typedef struct MSMMem_s {
    void *ptr;
    u32 size;
    MSMBLOCK *head;
    MSMBLOCK first;
} MSM_MEM;

static MSM_MEM mem;

void msmMemFree(void* ptr) {
    MSMBLOCK* block;
    MSMBLOCK* blockPrev;
    MSMBLOCK* blockNext;
    MSMBLOCK* blockHead;
    MSMBLOCK* base;
    
    base = ptr;

    block = &base[-1];
    blockPrev = block->prev;
    blockNext = block->next;
    if ((uintptr_t)mem.ptr > (uintptr_t)block ||
        ((uintptr_t)mem.ptr + mem.size) <= (uintptr_t)block) {
        return;
    }

    if ((blockPrev->next != block) || (block->ptr != ptr) || (blockNext && (blockNext->prev != block))) {
        return;
    }
    
    blockPrev->size += block->freeSize + block->size;
    blockPrev->next = blockNext;
    blockHead = mem.head;
    if ((blockHead == block) || (blockHead->size < blockPrev->size)) {
        mem.head = blockPrev;
    }
    if (blockNext) {
        blockNext->prev = blockPrev;
        if (mem.head->size < blockNext->size) {
            mem.head = blockNext;
        }
    }
}

void* msmMemAlloc(u32 size) {
    s32 alignOfs;
    u32 freeSize;
    u32 allocSize;
    MSMBLOCK* block;
    MSMBLOCK* blockPrev;
    MSMBLOCK* blockNext;

    allocSize = size + 0x20;
    alignOfs = allocSize & 0x1F;
    if (alignOfs) {
        allocSize += 0x20 - alignOfs;
    }
    if (mem.head->size >= allocSize) {
        blockPrev = mem.head;
    } else {
        blockPrev = &mem.first;
        
       do {
            if (blockPrev->size >= allocSize) break;
            blockPrev = blockPrev->next;
        } while (blockPrev);
        if (!blockPrev) {
            return NULL;
        }
    }
    
    freeSize = blockPrev->freeSize;
    if (freeSize != 0) {
        freeSize -= 0x20;
    }
    block = (void*)((uintptr_t)blockPrev->ptr + (freeSize));
    blockNext = blockPrev->next;
    if ((uintptr_t)mem.ptr > (uintptr_t)block ||
        ((uintptr_t)mem.ptr + mem.size) <= (uintptr_t)block) {
        return NULL;
    }
    block->freeSize = allocSize;
    block->size = blockPrev->size - allocSize;
    block->ptr = (block + 1);
    block->prev = blockPrev;
    block->next = blockNext;
    mem.head = block;
    blockPrev->size = 0;
    blockPrev->next = block;
    if (blockNext) {
        blockNext->prev = block;
        if (mem.head->size < blockNext->size) {
            mem.head = blockNext;
        }
    }
    return block->ptr;
}

void msmMemInit(void* ptr, u32 size) {
    MSMBLOCK* block;
    uintptr_t base;
    u32 ofs;

    // Round the heap base up to 32 bytes, then take what is left of the heap,
    // also rounded down to 32. Done in uintptr_t because the original 32-bit
    // casts discarded the top half of a host pointer.
    //
    // Written the long way round on purpose: the round-up is a conditional
    // rather than (base + 0x1F) & ~0x1F, and the size is computed by reading
    // mem.ptr back rather than from a local. Both are what the original
    // compiles to; the tidier forms are shorter and do not match.
    base = (uintptr_t)ptr;
    ofs = (u32)(base & 0x1F);
    mem.ptr = (void*)(base + (ofs ? 0x20 - ofs : 0));
    mem.size = (u32)(((base + size) - (uintptr_t)mem.ptr) & ~(uintptr_t)0x1F);

    block = &mem.first;
    block->freeSize = 0;
    block->size = mem.size;
    block->ptr = mem.ptr;
    block->prev = NULL;
    block->next = NULL;
    mem.head = &mem.first;
}
