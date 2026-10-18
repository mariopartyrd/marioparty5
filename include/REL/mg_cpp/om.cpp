#include "REL/mg_cpp/om.h"
#include "REL/mg_cpp/minigame_core.h"


void *CMinigameCore::operator new(size_t n)
{
    void *ptr = HuMemDirectMallocNum(HEAP_HEAP, n, HU_MEMNUM_OVL);
    ASSERT_LINE(ptr != NULL, 24);
    return ptr;
}

void *CMinigameCore::operator new[](size_t n)
{
    void *ptr = HuMemDirectMallocNum(HEAP_HEAP, n, HU_MEMNUM_OVL);
    ASSERT_LINE(ptr != NULL, 31);
    return ptr;
}

void CMinigameCore::operator delete(void * p)
{
    if(p) {
        HuMemDirectFree(p);
    }
}

void CMinigameCore::operator delete[](void * p)
{
    operator delete(p);
}

COMObj::COMObj(OMOBJ *obj)
{
    obj->data = (void *)this;
    obj->objFunc = COMObj::ObjUpdate;
}

void COMObj::ObjUpdate(OMOBJ *obj)
{
    COMObj *o = (COMObj *)obj->data;
    o->on_update(obj);
}

void COMObj::ObjDelete(OMOBJ *obj)
{
    COMObj *o = (COMObj *)obj->data;
    delete o;
    omDelObj(HuPrcCurrentGet(), obj);
}

void COMObj::ObjDelete(OMOBJMAN *objman, OMOBJ *obj)
{
    COMObj *o = (COMObj *)obj->data;
    delete o;
    omDelObj(objman, obj);
}