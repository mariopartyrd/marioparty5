#include "REL/mg_cpp/handle.h"
#include "REL/mg_cpp/assert.h"

CHandleMan::CHandleMan(unsigned int n)
{
    ASSERT_LINE(n <= MAXCOUNT, 68);
    m_count = n;
    m_head = new CHandleData[n+1];
    m_tail = m_head+n;
    m_tail->next = m_tail;
    m_tail->prev = m_tail;
    for(int i=1; i<n; i++) {
        m_head[i].next = m_head+i+1;
        m_head[i].prev = m_head+i-1;
        m_head[i].ptr = 0;
        m_head[i].ref.d.index = i;
        m_head[i].ref.d.count = 0;
    }
    m_head[1].prev = m_head+0;
    m_head[n-1].next =  m_head+0;
    m_head[0].next =  m_head+1;
    m_head[0].prev = m_head+n-1;
    
}

CHandleMan::~CHandleMan()
{
    delete m_head;
}

CHandle CHandleMan::createHandle(void *ptr)
{
    CHandleData *handle = m_head->next;
    if(handle == m_head) {
        return NULL_HANDLE;
    }
    handle->next->prev = handle->prev;
    handle->prev->next = handle->next;
    handle->next = m_tail->next;
    handle->prev = m_tail;
    m_tail->next->prev = handle;
    m_tail->next = handle;
    handle->ptr = ptr;
    handle->ref.d.count++;
    return handle->ref.handle;
}

void *CHandleMan::deleteHandle(CHandle handle)
{
    CHandleRef ref;
    ref.handle = handle;
    if(handle == NULL_HANDLE) {
        return nullptr;
    }
    if(ref.d.index >= m_count) {
        return nullptr;
    }
    CHandleData *handleData = &m_head[ref.d.index];
    if(handleData->ref.handle != handle) {
        return nullptr;
    }
    void *ptr = handleData->ptr;
    handleData->ptr = NULL;
    handleData->next->prev = handleData->prev;
    handleData->prev->next = handleData->next;
    handleData->next = m_head;
    handleData->prev = m_head->prev;
    m_head->prev->next = handleData;
    m_head->prev = handleData;
    return ptr;
}

void *CHandleMan::getPtr(CHandle handle)
{
    CHandleRef ref;
    ref.handle = handle;
    if(handle == NULL_HANDLE) {
        return nullptr;
    }
    if(ref.d.index >= m_count) {
        return nullptr;
    }
    CHandleData *handleData = &m_head[ref.d.index];
    if(handleData->ref.handle != handle) {
        return nullptr;
    }
    return handleData->ptr;
}

unsigned int CHandleMan::getNum(CHandleData *node)
{
    unsigned int num = 0;
    for(CHandleData *handle=node->next; handle != (CHandleData *)node; handle=handle->next, num++);
    return num;
}

unsigned int CHandleMan::getTailNum(void)
{
    return CHandleMan::getNum(m_tail);
}

unsigned int CHandleMan::getHeadNum(void)
{
    return CHandleMan::getNum(m_head);
}