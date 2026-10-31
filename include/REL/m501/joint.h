#ifndef M501_JOINT_H
#define M501_JOINT_H


#include "humath.h"

namespace m501 {

class CJoint {

public:
    CJoint();
    ~CJoint();
    void setSuper(CJoint *super);
    
public:
    void lockMatrix()
    {
        m_lock = true;
    }

    Mtx *getLocalMatrix()
    {
        (void)this;
        return &m_local;
    }
    
    Mtx *getMatrix();
    
    
private:
    Mtx m_local; //0x00
    Mtx m_global; //0x30
    CJoint *m_super; //0x60
    CJoint *m_first; //0x64
    CJoint *m_next; //0x68
    CJoint *m_prev; //0x6C
    bool m_lock; //0x70
};

};

#endif