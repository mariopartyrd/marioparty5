#include "REL/mg_cpp/sphere.h"


//TODO: Move somewhere else
static inline float getVecMag(HuVecF *vec)
{
    return VECMag(vec);
}

static inline HuVecF *scaleVec(HuVecF *src, HuVecF *dst, float by)
{
    VECScale(src, dst, by);
    return dst;
}

bool CSphere::checkOverlap(CSphere *other)
{
    HuVecF delta;
    VECSubtract(&m_pos, &other->m_pos, &delta);
    float r = getRadius()+other->getRadius();
    return getVecMag(&delta) < r;
}

//Improve applyOfs inlining
bool CSphere::resolveOverlap(CSphere *other, float t, float arg3)
{
    HuVecF delta;
    float r = getRadius()+other->getRadius();
    VECSubtract(&other->m_pos, &m_pos, &delta);
    
    float mag = getVecMag(&delta);
    if(mag >= r) {
        return false;
    } else {
        float scale = t;
        float otherScale = 1-t;
        VECScale(&delta, &delta, ((r-mag)*arg3)/mag);
        HuVecF ofs;
        applyOfs(scaleVec(&delta, &ofs, -scale));
        other->applyOfs(scaleVec(&delta, &ofs, otherScale));
        return true;
    }
}