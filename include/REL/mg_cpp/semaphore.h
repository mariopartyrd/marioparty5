#ifndef MGCPP_SEMAPHORE_H
#define MGCPP_SEMAPHORE_H

#include <dolphin.h>

class CSemaphore {
public:
    CSemaphore(s32 count)
    {
        ::OSInitSemaphore(&m_osSemaphore, count);
    }
    
    operator OSSemaphore*()
    {
        return &m_osSemaphore;
    }

    
private:
    OSSemaphore m_osSemaphore;
};

#endif