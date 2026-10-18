#ifndef MGCPP_SPHERE_H
#define MGCPP_SPHERE_H

#include "humath.h"

struct CSphere {
    float getRadius() { return m_radius; }
    bool checkOverlap(CSphere *other);
    bool resolveOverlap(CSphere *other, float t, float arg3);
    
    void applyOfs(HuVecF *ofs)
    {
        VECAdd(ofs, &m_pos, &m_pos);
    }
    
    HuVecF m_pos;
    float m_radius;
};

#endif