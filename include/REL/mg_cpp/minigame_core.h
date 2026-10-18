#ifndef MGCPP_MINIGAME_CORE_H
#define MGCPP_MINIGAME_CORE_H

#include "REL/mg_cpp/singleton.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/gamemes.h"

class CMinigameCoreDummy {
    int unk0;
};

class CTickTimer {
public:
    CTickTimer()
    {
        m_tick = 0;
    }
    
    CTickTimer(int time)
    {
        m_tick = time;
    }
    
    bool update()
    {
        return m_tick > 0 && --m_tick == 0;
    }
    
    bool checkExpire()
    {
        return m_tick <= 0;
    }
private:
    
    int m_tick;
};

class CMinigameCore : private CSingleton<CMinigameCore>, private CMinigameCoreDummy {

public:
    CMinigameCore();
    virtual ~CMinigameCore();
    virtual void on_setup(OMOBJMAN *objMan) = 0;
    virtual void on_update();
    virtual void on_close() = 0;
    
public:
    void setup(int objMax, int omPrio);
    void actorSetup();
    void update();

private:
    static void ObjUpdate(OMOBJ *obj);
    static void MgExit(OMOBJ *obj);
public:
    static void StartFadeIn();
    static void StartFadeOut();
    static void AddWinner(int playerNo);
    static bool CheckWinner(int playerNo);
    static void StartStartGameMes();
    static void StartFinishGameMes();
    static bool CheckGameMes();
    static void StartWinnerGameMes();
    static void StartRecordGameMes(unsigned int val);
    static void ExitReq();
    
private:
    void setBonusCoin(int playerNo) {
        if(playerNo >= 0) {
            GWMgCoinBonusSet(playerNo, 10);
        }
    }

    
public:
    void updateWinCoin(void)
    {
        for(int i=0; i<GW_PLAYER_MAX; i++) {
            setBonusCoin(m_winnerTbl[i]);
        }
    }
    

public:
    static void* operator new(size_t sz);
    static void* operator new[](size_t sz);
    static void operator delete(void *p);
    static void operator delete[](void *p);
public:
    int m_winnerTbl[GW_PLAYER_MAX];
    int m_gameMesId;
    CTickTimer m_timer;

private:
};


#endif