#ifndef MGCPP_SUBTHREAD_H
#define MGCPP_SUBTHREAD_H

#include "REL/mg_cpp/thread.h"
#include "REL/mg_cpp/semaphore.h"

class CSubThread {
public:
    CSubThread(u32 stackSize);
    virtual ~CSubThread();
    
public:
    virtual void thread_exec(u32 code) = 0;
public:
    bool isTerminated();
    int wakeupThread(int code);
    int sleepThread(int code);
    static void *ThreadMain(void *ptr);
    static OSPriority GetStartPriority();

private:
    int m_4;
    CThread m_th;
    CSemaphore m_semaphore;
    int m_sleepCode;
    
};

#endif