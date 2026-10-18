#include "REL/mg_cpp/thread.h"
#include "REL/mg_cpp/assert.h"
#include "stdio.h"

bool CThread::s_checkStack;

CThread::CThread(void* (*func)(void*), void *param, u32 stackSize, OSPriority priority, u16 attr)
{
    u32 stackNum = stackSize/4;
    u32 *stack = new u32[stackNum];
    m_stack = stack;
    createThread(func, param, stack, stackNum, priority, attr);
}

CThread::CThread(void* (*func)(void*), void *param, u32 *stack, u32 stackSize, OSPriority priority, u16 attr)
{
    m_stack = nullptr;
    u32 stackNum = stackSize/4;
    createThread(func, param, stack, stackNum, priority, attr);
}

void CThread::createThread(void* (*func)(void*), void* param, u32 *stack, u32 stackNum, OSPriority priority, u16 attr)
{
    ASSERT_LINE(priority >= OS_PRIORITY_MIN && priority <= OS_PRIORITY_MAX, 82);
    initStack(stack, stackNum);
    checkStackMagic(stack, 0);
    bool result = ::OSCreateThread(&m_osThread, func, param, &stack[stackNum], stackNum*sizeof(u32), priority, attr);
    
    if(!result) {
        OSPanic(__FILE__, 96, "OSCreateThread() failed.");
    }
}

CThread::~CThread()
{
    if(!::OSIsThreadTerminated(&m_osThread)) {
        ::OSCancelThread(&m_osThread);
    }
    if(s_checkStack) {
        u32 stackSize = m_osThread.stackBase-(u8 *)m_osThread.stackEnd;
        u32 stackRemain = getStackRemain();
        printf("delete CThread@%p\nstack remain/total = 0x%08x/0x%08x (%f%%)\n", this, stackRemain, stackSize, (stackRemain*100.0)/stackSize);
    }
    delete[] m_stack;
}

u32 CThread::getStackRemain()
{
    u32 *stackEnd = m_osThread.stackEnd;
    if(checkStackMagic(stackEnd, 1)) {
        OSPanic(__FILE__, 127, "stack overflow\n");
    }
    u32 *stack = stackEnd;
    u32 *stackBase = (u32 *)m_osThread.stackBase;
    while(stack < stackBase) {
        if(*stack != 0xDEADBABE) {
            break;
        }
        stack++;
    }
    return (u32)stack-(u32)stackEnd;
}

BOOL CThread::checkStackMagic(u32 *stack, int mode)
{
    u32 *magicEnd = &stack[8];
    switch(mode) {
        case 0:
            while(stack != magicEnd) {
                *stack++ = 0xDEADBABE;
            }
            break;
        
        case 1:
            while(stack != magicEnd) {
                if(*stack++ != 0xDEADBABE) {
                    return TRUE;
                }
            }
            break;
    }
    return FALSE;
}

void CThread::initStack(u32 *stack, u32 stackNum)
{
    if(CThread::s_checkStack) {
        u32 *stackEnd = &stack[stackNum];
        while(stack != stackEnd) {
            *stack++ = 0xDEADBABE;
        }
    }
}
