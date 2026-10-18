#ifndef MGCPP_TIMER_H
#define MGCPP_TIMER_H

#include "game/mg/timer.h"

class CTimer {
public:
    CTimer(int type);
    ~CTimer();
    void setParam(int maxTime, int endTime, int recordTime);
    int getValue();
    void setPos(float posX, float posY);
    void getPos(float *posX, float *posY);
    void setModeOn(int offType);
    void setModeOff();
    void stop();
    void reset();
    bool checkDone();
    void setWinColor(u8 r, u8 g, u8 b);

private:
    MGTIMER *m_timer;
};

#endif