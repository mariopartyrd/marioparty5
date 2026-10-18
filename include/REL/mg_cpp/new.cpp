#include "REL/mg_cpp/assert.h"
#include "string.h"
#include "game/memory.h"

void * operator new(size_t n)
{
    void *ptr = HuMemDirectMallocNum(HEAP_HEAP, n, HU_MEMNUM_OVL);
    ASSERT_LINE(ptr != NULL, 23);
    return ptr;
}

void * operator new[](size_t n)
{
    return operator new(n);
}

void operator delete(void * p)
{
    if(p) {
        HuMemDirectFree(p);
    }
}

void operator delete[](void * p)
{
    operator delete(p);
}

