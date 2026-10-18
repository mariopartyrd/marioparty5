#include "REL/mg_cpp/minigame_core.h"
#include "game/mgdata.h"
#include "game/mg/actman.h"
#include "game/audio.h"
#include "game/wipe.h"
#include "game/charman.h"

static CTickTimer lbl_1_bss_18(120);
static CTickTimer winnerMaxTime(210);
static CTickTimer lbl_1_bss_20(210);


CMinigameCore::CMinigameCore()
    : CSingleton<CMinigameCore>(this)
{
    for(int i=0; i<GW_PLAYER_MAX; i++) {
        m_winnerTbl[i] = -1;
    }
}

CMinigameCore::~CMinigameCore()
{
    if(mgDataTbl[GwSystem.mgNo].type!= MG_TYPE_BATTLE) {
        GetSingletonPtr()->updateWinCoin();
    }
}

void CMinigameCore::setup(int objMax, int omPrio)
{
    OMOBJMAN *objMan = omInitObjMan(objMax, omPrio);
    omGameSysInit(objMan);
    on_setup(objMan);
    omAddObj(objMan, 0, 0, 0, CMinigameCore::ObjUpdate);
    
}

void CMinigameCore::actorSetup()
{
    OMOBJMAN *objMan = MgActorObjectSetup();
    on_setup(objMan);
    omAddObj(objMan, 0, 0, 0, CMinigameCore::ObjUpdate);
}

void CMinigameCore::ObjUpdate(OMOBJ *obj)
{
    if(omSysExitReq && !WipeCheckEnd()) {
        StartFadeOut();
        HuAudFadeOut(1000);
        obj->objFunc = CMinigameCore::MgExit;
    } else {
        CMinigameCore *core = CMinigameCore::GetSingletonPtrRaw();
        core->update();
    }
}

void CMinigameCore::update()
{
    on_update();
    m_timer.update();
}

void CMinigameCore::on_update()
{
    
}

void CMinigameCore::MgExit(OMOBJ *obj)
{
    CMinigameCore *core = CMinigameCore::GetSingletonPtrRaw();
    if(WipeCheckEnd()) {
        core->on_update();
        core->m_timer.update();
        return;
    }
    core->on_close();
    delete core;
    CharModelKill(CHARNO_NONE);
    omOvlReturn(1);
}

void CMinigameCore::AddWinner(int playerNo)
{
    CMinigameCore *core = CMinigameCore::GetSingletonPtr();
    int *tbl = core->m_winnerTbl;
    for(int i=0; i<GW_PLAYER_MAX; i++) {
        if(tbl[i] < 0) {
            tbl[i] = playerNo;
            return;
        }
    }
}

bool CMinigameCore::CheckWinner(int playerNo)
{
    CMinigameCore *core = CMinigameCore::GetSingletonPtr();
    int *tbl = core->m_winnerTbl;
    for(int i=0; i<GW_PLAYER_MAX; i++) {
        if(tbl[i] == playerNo) {
            return true;
        }
    }
    return false;
}

void CMinigameCore::StartStartGameMes()
{
    CMinigameCore *core = CMinigameCore::GetSingletonPtr();
    core->m_gameMesId = GameMesMgStartCreate();
}

void CMinigameCore::StartFinishGameMes()
{
    CMinigameCore *core = CMinigameCore::GetSingletonPtr();
    core->m_gameMesId = GameMesMgFinishCreate();
}

bool CMinigameCore::CheckGameMes()
{
    CMinigameCore *core = CMinigameCore::GetSingletonPtr();
    return GameMesStatGet(GAMEMES_ID_NONE) == 0 && core->m_timer.checkExpire();
}

void CMinigameCore::StartWinnerGameMes()
{
    CMinigameCore *core = CMinigameCore::GetSingletonPtr();
    core->m_timer = winnerMaxTime;
    int *tbl = core->m_winnerTbl;
    
    int firstLose;
    for(firstLose=0; firstLose<GW_PLAYER_MAX; firstLose++) {
        if(tbl[firstLose] < 0) {
            break;
        }
    }
    if(firstLose == 0) {
        GameMesMgDrawCreate();
    } else {
        int charNoTbl[GW_PLAYER_MAX];
        for(int i=1; i<firstLose; i++) {
            for(int j=i; j>=1 && tbl[j-1] > tbl[j]; j--) {
                int tmp = tbl[j-1];
                tbl[j-1] = tbl[j];
                tbl[j] = tmp;
            }                
        }
        for(int i=0; i<GW_PLAYER_MAX; i++) {
            charNoTbl[i] = (tbl[i] < 0) ? CHARNO_NONE : GwPlayerConf[tbl[i]].charNo;
        }
        GameMesMgWinnerCreate(charNoTbl[0], charNoTbl[1], charNoTbl[2], charNoTbl[3]);
    }
}

void CMinigameCore::StartRecordGameMes(unsigned int val)
{
    if(!_CheckFlag(FLAG_MG_PRACTICE)) {
        CMinigameCore *core = CMinigameCore::GetSingletonPtr();
        core->m_gameMesId = GameMesRecordCreate(val);
        HuAudJinglePlay(MSM_STREAM_JNGL_MG_RECORD);
    }
}

void CMinigameCore::StartFadeIn()
{
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREVTYPE, 60);
}

void CMinigameCore::StartFadeOut()
{
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 60);
}

void CMinigameCore::ExitReq()
{
    omSysExitReq = TRUE;
}