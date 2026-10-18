#ifndef MGCPP_MT19937_H
#define MGCPP_MT19937_H

#include <dolphin/types.h>

class CMT19937 {
public:
    CMT19937() {
        setSeed(5489);
    }
    void setSeed(u32 seed);
    void setSeedMulti(u32 *idxTbl, u32 idxNum);
    u32 random();
    s32 irandom();
    float frandom();
    float frandom2();
    float frandomMid();
    double drandom();
    float randomMantissa();
    
private:
    enum { STATE_SIZE = 624, PERIOD=397, DIFF=STATE_SIZE-PERIOD };
    u32 m_state[STATE_SIZE];
    u32 m_stateOfs;
};

#endif