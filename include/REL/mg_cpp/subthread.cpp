#include "REL/mg_cpp/subthread.h"
#include "REL/mg_cpp/assert.h"

OSPriority CSubThread::GetStartPriority()
{
    return ::OSGetThreadPriority(::OSGetCurrentThread());
}

CSubThread::CSubThread(u32 stackSize)
    : m_th(CSubThread::ThreadMain, (void *)this, stackSize, GetStartPriority(), (u16)OS_THREAD_ATTR_DETACH),
    m_semaphore(0)
{
}

CSubThread::~CSubThread()
{
    
}

bool CSubThread::isTerminated()
{
    return ::OSIsThreadTerminated(m_th);
}

int CSubThread::wakeupThread(int code)
{
    if(::OSIsThreadTerminated(m_th)) {
        return m_sleepCode;
    } else {
        m_sleepCode = code;
        ::OSResumeThread(m_th);
        OSWaitSemaphore(m_semaphore);
        m_th.getStackRemain();
        ::OSCheckActiveThreads();
        return m_sleepCode;
    }
}

int CSubThread::sleepThread(int code)
{
    ASSERT_LINE(::OSGetCurrentThread() == m_th, 71);
    ASSERT_LINE(!::OSIsThreadTerminated(m_th), 74);
    m_sleepCode = code;
    OSSignalSemaphore(m_semaphore);
    OSSuspendThread(m_th);
    return m_sleepCode;
}

void *CSubThread::ThreadMain(void *ptr)
{
    CSubThread *thread = (CSubThread *)ptr;
    thread->thread_exec(thread->m_sleepCode);
    OSSignalSemaphore(thread->m_semaphore);
    return nullptr;
}