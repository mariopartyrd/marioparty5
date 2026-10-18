#include "REL/mg_cpp/mt19937.h"

void CMT19937::setSeed(u32 seed)
{
    m_state[0] = seed;
    for(m_stateOfs=1; m_stateOfs<STATE_SIZE; m_stateOfs++) {
        m_state[m_stateOfs] = m_stateOfs+((m_state[m_stateOfs-1]^(m_state[m_stateOfs-1] >> 30))*1812433253);
    }
}

void CMT19937::setSeedMulti(u32 *idxTbl, u32 idxNum)
{
    u32 stateOfs;
    u32 i;
    u32 idxOfs;
    setSeed(19650218);
    for(stateOfs=1, idxOfs=0, i=((idxNum<STATE_SIZE) ? STATE_SIZE : idxNum); i!=0; i--) {
        m_state[stateOfs] = idxTbl[idxOfs]+(m_state[stateOfs]^((m_state[stateOfs-1]^(m_state[stateOfs-1] >> 30))*1664525))+idxOfs;
        stateOfs++;
        idxOfs++;
        if(stateOfs >= STATE_SIZE) {
            m_state[0] = m_state[STATE_SIZE-1];
            stateOfs = 1;
        }
        if(idxOfs >= idxNum) {
            idxOfs = 0;
        }
    }
    for(i=STATE_SIZE-1; i!=0; i--) {
        m_state[stateOfs] = (m_state[stateOfs]^((m_state[stateOfs-1]^(m_state[stateOfs-1] >> 30))*1566083941))-stateOfs;
        stateOfs++;
        if(stateOfs >= STATE_SIZE) {
            m_state[0] = m_state[STATE_SIZE-1];
            stateOfs = 1;
        }
    }
    m_state[0] = 0x80000000;
}

u32 CMT19937::random()
{
    static const u32 maskTbl[2] = { 0, 0x9908B0DF };
    u32 temp;
    u32 i;
    if(m_stateOfs >= STATE_SIZE) {
        for(i=0; i<DIFF; i++) {
            temp = (m_state[i] & 0x80000000)|(m_state[i+1] & 0x7FFFFFFF);
            m_state[i] = maskTbl[temp & 0x1]^((temp >> 1)^m_state[i+PERIOD]);
        }
        for(; i<STATE_SIZE-1; i++) {
            temp = (m_state[i] & 0x80000000)|(m_state[i+1] & 0x7FFFFFFF);
            m_state[i] = maskTbl[temp & 0x1]^((temp >> 1)^m_state[i-DIFF]);
        }
        temp = (m_state[STATE_SIZE-1] & 0x80000000)|(m_state[0] & 0x7FFFFFFF);
        m_state[STATE_SIZE-1] = (temp >> 1)^m_state[PERIOD-1]^maskTbl[temp & 0x1];
        m_stateOfs = 0;
    }
    temp = m_state[m_stateOfs++];
    temp ^= (temp >> 11);
    temp ^= ((temp << 7) & 0x9D2C5680);
    temp ^= (temp << 15) & 0xEFC60000;
    temp ^= temp >> 18;
    return temp;
}

s32 CMT19937::irandom()
{
    return random() >> 1;
}

float CMT19937::frandom()
{
    return random()*2.3283064e-10f;
}

float CMT19937::frandom2()
{
    return random()*2.3283064e-10f;
}

float CMT19937::frandomMid()
{
    return (random()+0.5f)*2.3283064e-10f;
}

double CMT19937::drandom()
{
    u32 a = random() >> 5;
    u32 b = random() >> 6;
    return ((a*67108864.0)+b)*1.1102230246251565e-16;
}

float CMT19937::randomMantissa()
{
    u32 temp = random() >> 9;
    return temp*1.1920929e-7f;
}