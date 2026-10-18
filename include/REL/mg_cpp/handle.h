#ifndef MGCPP_HANDLE_H
#define MGCPP_HANDLE_H

#include "game/hu3d.h"

typedef unsigned int CHandle;

union CHandleRef {
    struct {
        unsigned int index : 20;
        unsigned int count : 12;
    } d;
    CHandle handle;
};

struct CHandleData {
    CHandleData *next;
    CHandleData *prev;
    void *ptr;
    CHandleRef ref;
};

class CHandleMan {
public:
    enum { NULL_HANDLE=0, MAXCOUNT = 4096 };
    
public:
    CHandleMan(unsigned int n = MAXCOUNT);
    ~CHandleMan();
    CHandle createHandle(void *ptr);
    void *deleteHandle(CHandle handle);
    void *getPtr(CHandle handle);
    
    unsigned int getTailNum(void);
    unsigned int getHeadNum(void);
    
private:
    static unsigned int getNum(CHandleData *node);
    
private:
    CHandleData *m_head;
    CHandleData *m_tail;
    int m_count;
};

#endif