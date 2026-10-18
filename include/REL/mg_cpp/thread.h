#ifndef MGCPP_THREAD_H
#define MGCPP_THREAD_H

#include <dolphin.h>

class CThread {
public:
    CThread(void* (*func)(void*), void *param, u32 stackNum, OSPriority priority, u16 attr);
    CThread(void* (*func)(void*), void *param, u32 *stack, u32 stackNum, OSPriority priority, u16 attr);
    ~CThread();
    
    operator OSThread*()
    {
        return &m_osThread;
    }

public:
    u32 getStackRemain();

private:
    void createThread(void* (*func)(void*), void* param, u32 *stack, u32 stackNum, OSPriority priority, u16 attr);
    static void initStack(u32 *stack, u32 stackNum);
    static BOOL checkStackMagic(u32 *stack, int mode);
    

    
private:
    OSThread m_osThread;
    u32 *m_stack;
    static bool s_checkStack;
};

#endif