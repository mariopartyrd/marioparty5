#include "REL/mg_cpp/timer.h"
#include "REL/mg_cpp/assert.h"

CTimer::CTimer(int type)
{
    switch(type) {
        case MGTIMER_TYPE_NORMAL:
            m_timer = MgTimerCreate(MGTIMER_TYPE_NORMAL);
            break;
        
        case MGTIMER_TYPE_RECORD:
            m_timer = MgTimerCreate(MGTIMER_TYPE_RECORD);
            break;
        
        case MGTIMER_TYPE_SCORE:
            m_timer = MgTimerCreate(MGTIMER_TYPE_SCORE);
            break;
        
        default:
            ASSERT_LINE(false, 30);
            break;
    }
}

CTimer::~CTimer()
{
    MgTimerKill(m_timer);
}

void CTimer::setParam(int maxTime, int endTime, int recordTime)
{
    MgTimerParamSet(m_timer, maxTime, endTime, recordTime);
}

int CTimer::getValue()
{
    return MgTimerValueGet(m_timer);
}

void CTimer::setPos(float posX, float posY)
{
    MgTimerPosSet(m_timer, posX, posY);
}

void CTimer::getPos(float *posX, float *posY)
{
    MgTimerPosGet(m_timer, posX, posY);
}

void CTimer::setModeOn(int offType)
{
    switch(offType) {
        case MGTIMER_OFFTYPE_NORMAL:
            MgTimerModeOnSet(m_timer, MGTIMER_OFFTYPE_NORMAL);
            break;
        
        case MGTIMER_OFFTYPE_FADEOUT:
            MgTimerModeOnSet(m_timer, MGTIMER_OFFTYPE_FADEOUT);
            break;
        
        case MGTIMER_OFFTYPE_FLASH:
            MgTimerModeOnSet(m_timer, MGTIMER_OFFTYPE_FLASH);
            break;
        
        default:
            ASSERT_LINE(false, 72);
            break;
    }
}

void CTimer::setModeOff()
{
    MgTimerModeOffSet(m_timer);
}

void CTimer::stop()
{
    if(!m_timer->stopF) {
        m_timer->stopF = TRUE;
        m_timer->mode = MGTIMER_MODE_ON;
    }
}

void CTimer::reset()
{
    MgTimerKill(m_timer);
    m_timer = MgTimerCreate(MGTIMER_TYPE_NORMAL);
}

bool CTimer::checkDone()
{
    return (bool)(MgTimerDoneCheck(m_timer) != FALSE);
}

void CTimer::setWinColor(u8 r, u8 g, u8 b)
{
    MgTimerWinColorSet(m_timer, r, g, b);
}