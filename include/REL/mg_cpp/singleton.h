#ifndef MGCPP_SINGLETON_H
#define MGCPP_SINGLETON_H

#include "REL/mg_cpp/assert.h"


template <typename T>
class CSingleton {
public:
    CSingleton(T *ptr)
    {
        SetSingletonPtr(ptr);
    }
    
    ~CSingleton()
    {
        ResetSingletonPtr();
    }
    
    static T *GetSingletonPtrRaw(void)
    {
        return ms_pointer;
    }
    
    static T *GetSingletonPtr(void)
    {
        ASSERT_LINE(ms_pointer != 0, 39);
        return ms_pointer;
    }

    static void SetSingletonPtr(T *ptr)
    {
        ASSERT_LINE(ms_pointer == 0, 53);
        ms_pointer = ptr;
    }
        
    static void ResetSingletonPtr(void)
    {
        ASSERT_LINE(ms_pointer != 0, 61);
        ms_pointer = 0;
    }
    
private:
    static T *ms_pointer;
};

#endif