#include "game/board/gate.h"
#include "game/board/model.h"
#include "game/board/masu.h"
#include "game/board/camera.h"
#include "game/board/window.h"
#include "game/board/player.h"
#include "game/board/coin.h"

#include "messnum/board_gate.h"

#define GATE_MAX 8

typedef struct GateBattanWork_s {
    unsigned doneF : 1;
    int playerNo;
    int mode;
    MBMODELID modelId;
    s16 masuId;
    float rotY;
    float masuAngle;
    float targetAngle;
    int no;
    s16 time;
    s16 maxTime;
    OMOBJ *obj;
} GATEBATTAN_WORK;

typedef struct GateDossunWork_s {
    unsigned doneF : 1;
    unsigned downF : 1;
    int playerNo;
    int mode;
    MBMODELID modelId;
    s16 masuId;
    s16 masuIdEnd;
    float rotY;
    int no;
    s16 time;
    s16 maxTime;
    OMOBJ *obj;
} GATEDOSSUN_WORK;

static GATEDOSSUN_WORK *gateDossunWorkP[GATE_MAX];
static GATEBATTAN_WORK *gateBattanWorkP[GATE_MAX];

static int gateDossunNum;
static int gateBattanNum;
static HUPROCESS *gateDossunProc;
static HUPROCESS *gateBattanProc;

static int dossunMotFileTbl[] = {
    BOARD_HSF_dossunIdle,
    HU_DATANUM_NONE,
};

static int battanMotFileTbl[] = {
    BOARD_HSF_battanIdle,
    BOARD_HSF_battanMove,
    HU_DATANUM_NONE,
};

void MBGateInit(void)
{
    gateBattanNum = 0;
    gateDossunNum = 0;
    memset(gateBattanWorkP, 0, GATE_MAX*sizeof(GATEBATTAN_WORK *));
    memset(gateDossunWorkP, 0, GATE_MAX*sizeof(GATEDOSSUN_WORK *));
}

void MBGateClose(void)
{
    int i;
    for(i=0; i<gateBattanNum; i++) {
        MBGateBattanKill(i);
    }
    for(i=0; i<gateDossunNum; i++) {
        MBGateDossunKill(i);
    }
}

void MBGateCreate(void)
{
    int i;
    for(i=0; i<MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
        MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, i+1);
        if(masuP->flag & MASU_FLAG_BATTAN) {
            MBGateBattanCreate(i+1);
        }
        if(masuP->flag & MASU_FLAG_DOSSUN) {
            MBGateDossunCreate(i+1);
        }
    }
}

static void GateBattanObjExec(OMOBJ *obj);

void MBGateBattanCreate(int masuId)
{
    GATEBATTAN_WORK *work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(GATEBATTAN_WORK), HU_MEMNUM_OVL);
    
    HuVecF startPos;
    HuVecF endPos;
    HuVecF pos;
    HuVecF dir;
    s16 path[MASU_LINK_MAX];
    
    gateBattanWorkP[gateBattanNum] = work;
    memset(work, 0, sizeof(GATEBATTAN_WORK));
    work->modelId = MBModelCreate(BOARD_HSF_battan, battanMotFileTbl, FALSE);
    MBMotionNoSet(work->modelId, 1, HU3D_MOTATTR_LOOP);
    work->no = gateBattanNum;
    work->masuId = masuId;
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &startPos);
    MBMasuPathNoLoopCheck(masuId, path);
    
    MBMasuPosGet(MASU_LAYER_DEFAULT, path[0], &endPos);
    
    VECSubtract(&endPos, &startPos, &dir);
    work->rotY = work->masuAngle = HuAtan(-dir.x, -dir.z);
    VECScale(&dir, &dir, 0.5f);
    VECAdd(&startPos, &dir, &pos);
    MBModelPosSetV(work->modelId, &pos);
    MBModelRotSet(work->modelId, 0, work->rotY, 0);
    work->obj = MBAddObj(260, 0, 0, GateBattanObjExec);
    work->obj->work[0] = (u32)work;
    gateBattanNum++;
}

void MBGateBattanKill(int no)
{
    GATEBATTAN_WORK *work = gateBattanWorkP[no];
    MBModelKill(work->modelId);
    HuMemDirectFree(work);
    gateBattanWorkP[no] = NULL;
}

s16 MBGateMasuExec(int playerNo, s16 masuId, s16 masuNext)
{
    s16 path[MASU_LINK_MAX];
    
    int no;
    void *work;
    
    
    if(MBMasuFlagGet(MASU_LAYER_DEFAULT, masuId) & MASU_FLAG_BATTAN) {
        MBMasuPathNoLoopCheck(masuId, path);
        no = MBGateBattanSearch(masuId);
        work = gateBattanWorkP[no];
        if(masuNext == path[0] && !MBGateBattanExec(playerNo, masuId)) {
            masuNext = path[1];
        }
    } else if(MBMasuFlagGet(MASU_LAYER_DEFAULT, masuId) & MASU_FLAG_DOSSUN) {
        MBMasuPathNoLoopCheck(masuId, path);
        no = MBGateDossunSearch(masuId);
        work = gateDossunWorkP[no];
        if(masuNext == path[0] && !MBGateDossunExec(playerNo, masuId)) {
            masuNext = path[1];
        }
    }
    return masuNext;
}

int MBGateBattanSearch(s16 masuId)
{
    int i;
    for(i=0; i<gateBattanNum; i++) {
        GATEBATTAN_WORK *work = gateBattanWorkP[i];
        if(work->masuId == masuId) {
            break;
        }
    }
    if(i < gateBattanNum) {
        return i;
    }
    return -1;
}

static void GateBattanProcExec(void);
static void GateBattanProcDestroy(void);

typedef struct GateBattanProcWork_s {
    int playerNo;
    int result;
} GATEBATTAN_PROCWORK;

int MBGateBattanExec(int playerNo, int masuId)
{
    GATEBATTAN_PROCWORK *procWork;
    MBWalkNumDispSet(playerNo, FALSE);
    gateBattanProc = MBPrcCreate(GateBattanProcExec, 8202, 16384);
    procWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(GATEBATTAN_PROCWORK), HU_MEMNUM_OVL);
    gateBattanProc->property = procWork;
    MBPrcDestructorSet(gateBattanProc, GateBattanProcDestroy);
    procWork->playerNo = playerNo;
    procWork->result = 1;
    while(gateBattanProc) {
        HuPrcVSleep();
    }
    MBWalkNumDispSet(playerNo, TRUE);
    return procWork->result;
}

static void GateBattanMoveStart(GATEBATTAN_WORK *work, int playerNo);
static BOOL GateBattanMoveCheck(GATEBATTAN_WORK *work);

static void GateBattanProcExec(void)
{
    GATEBATTAN_PROCWORK *procWork = HuPrcCurrentGet()->property;
    int playerNo = procWork->playerNo;
    int no = MBGateBattanSearch(GwPlayer[playerNo].masuId);
    GATEBATTAN_WORK *work = gateBattanWorkP[no];
    
    HuVecF modelPos;
    HuVecF playerPos;
    HuVecF dir;
    
    float angle;
    MBModelPosGet(work->modelId, &modelPos);
    MBPlayerPosGet(playerNo, &playerPos);
    VECSubtract(&modelPos, &playerPos, &dir);
    angle = 90-HuAtan(dir.z, dir.x);
    MBPlayerRotateStart(playerNo, angle, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBPlayerMotIdleSet(playerNo);
    MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
    MBCameraMotionWait();
    MBCameraFocusSet(MB_MODEL_NONE);
    if(MBPlayerCoinGet(playerNo) < 10) {
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_GATE_BATTAN_COIN, HUWIN_SPEAKER_BATTAN);
        MBTopWinWait();
        MBTopWinKill();
        procWork->result = 0;
    } else {
        MBAudFXPlay(MSM_SE_GUIDE_02);
        MBWinCreateChoice(MBWIN_TYPE_EVENT, MESS_BOARD_GATE_BATTAN_CHOICE, HUWIN_SPEAKER_BATTAN, 0);
        MBTopWinAttrReset(HUWIN_ATTR_NOCANCEL);
        if(GwPlayer[playerNo].comF) {
            MBComChoiceSetLeft();
        }
        MBTopWinWait();
        if(MBWinLastChoiceGet() == 1 || MBWinLastChoiceGet() == -1) {
            MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_GATE_BATTAN_NO, HUWIN_SPEAKER_BATTAN);
            MBTopWinWait();
            MBTopWinKill();
            procWork->result = 0;
        } else {
            MBCoinAddDispExec(playerNo, -10, TRUE);
            MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_GATE_BATTAN_YES, HUWIN_SPEAKER_BATTAN);
            MBTopWinWait();
            MBTopWinKill();
            GateBattanMoveStart(work, playerNo);
            while(!GateBattanMoveCheck(work)) {
                HuPrcVSleep();
            }
        }
    }
    MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_WALK);
    MBCameraMotionWait();
    HuPrcEnd();
}

static void GateBattanProcDestroy(void)
{
    HuMemDirectFree(gateBattanProc->property);
    gateBattanProc = NULL;
}

static void GateBattanObjExec(OMOBJ *obj)
{
    GATEBATTAN_WORK *work = (GATEBATTAN_WORK *)obj->work[0];
    float weight;
    if(MBKillCheck()) {
        MBDelObj(obj);
        return;
    }
    switch(work->mode) {
        case 0:
            break;
            
        case 1:
            if(work->time > work->maxTime) {
                work->doneF = TRUE;
                work->mode = 2;
                MBMotionShiftSet(work->modelId, 1, 0, 10, HU3D_MOTATTR_LOOP);
            } else {
                weight = ((float)work->time++)/work->maxTime;
                obj->trans.x = obj->rot.x+(weight*(obj->scale.x-obj->rot.x));
                obj->trans.y = obj->rot.y+(weight*(obj->scale.y-obj->rot.y));
                obj->trans.z = obj->rot.z+(weight*(obj->scale.z-obj->rot.z));
                MBModelPosSetV(work->modelId, &obj->trans);
                MBAngleMoveTo(&work->rotY, work->targetAngle, 0.1f);
                MBModelRotSet(work->modelId, 0, work->rotY, 0);
            }
            break;
        
        case 2:
            if(work->masuId != GwPlayer[work->playerNo].masuId) {
                work->mode = 3;
                work->time = 0;
                work->maxTime = 120;
                work->targetAngle = 180+work->targetAngle;
                MBMotionShiftSet(work->modelId, 2, 0, 5, HU3D_MOTATTR_LOOP);
            }
            break;
        
        case 3:
            if(work->time > work->maxTime) {
                if(MBAngleMoveTo(&work->rotY, work->masuAngle, 0.2f)) {
                    work->mode = 0;
                    MBMotionShiftSet(work->modelId, 1, 0, 5, HU3D_MOTATTR_LOOP);
                }
                MBModelRotSet(work->modelId, 0, work->rotY, 0);
            } else {
                weight = ((float)work->time++)/work->maxTime;
                obj->trans.x = obj->scale.x+(weight*(obj->rot.x-obj->scale.x));
                obj->trans.y = obj->scale.y+(weight*(obj->rot.y-obj->scale.y));
                obj->trans.z = obj->scale.z+(weight*(obj->rot.z-obj->scale.z));
                MBModelPosSetV(work->modelId, &obj->trans);
                MBAngleMoveTo(&work->rotY, work->targetAngle, 0.1f);
                MBModelRotSet(work->modelId, 0, work->rotY, 0);
            }
            break;
    }
}

static void GateBattanMoveStart(GATEBATTAN_WORK *work, int playerNo)
{
    s16 path[MASU_LINK_MAX];
    HuVecF masuPos;
    HuVecF dir;
    HuVecF startPos;
    HuVecF endPos;
    
    work->doneF = FALSE;
    work->mode = 1;
    work->time = 0;
    work->maxTime = 120;
    work->playerNo = playerNo;
    MBMotionShiftSet(work->modelId, 2, 0, 5, HU3D_MOTATTR_LOOP);
    MBModelPosGet(work->modelId, &work->obj->rot);
    MBMasuPosGet(MASU_LAYER_DEFAULT, work->masuId, &masuPos);
    MBMasuPathNoLoopCheck(work->masuId, path);
    MBMasuPosGet(MASU_LAYER_DEFAULT, path[0], &startPos);
    MBMasuPosGet(MASU_LAYER_DEFAULT, path[1], &endPos);
    VECSubtract(&startPos, &masuPos, &startPos);
    VECSubtract(&endPos, &masuPos, &endPos);
    VECScale(&startPos, &startPos, 0.5f);
    VECScale(&endPos, &endPos, 0.5f);
    work->obj->scale.x = masuPos.x+startPos.x+endPos.x;
    work->obj->scale.y = masuPos.y+startPos.y+endPos.y;
    work->obj->scale.z = masuPos.z+startPos.z+endPos.z;
    VECSubtract(&work->obj->scale, &work->obj->rot, &dir);
    work->targetAngle = HuAtan(dir.x, dir.z);
    MBAudFXPlay(MSM_SE_BD3_13);
}

static BOOL GateBattanMoveCheck(GATEBATTAN_WORK *work)
{
    return work->doneF;
}

static void GateDossunObjExec(OMOBJ *obj);

void MBGateDossunCreate(int masuId)
{
    GATEDOSSUN_WORK *work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(GATEDOSSUN_WORK), HU_MEMNUM_OVL);
    
    HuVecF startPos;
    HuVecF endPos;
    HuVecF pos;
    HuVecF dir;
    s16 path[MASU_LINK_MAX];
    
    gateDossunWorkP[gateDossunNum] = work;
    memset(work, 0, sizeof(GATEDOSSUN_WORK));
    work->modelId = MBModelCreate(BOARD_HSF_dossun, dossunMotFileTbl, FALSE);
    MBMotionNoSet(work->modelId, 1, HU3D_MOTATTR_LOOP);
    work->no = gateDossunNum;
    work->masuId = masuId;
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &startPos);
    MBMasuPathNoLoopCheck(masuId, path);
    
    MBMasuPosGet(MASU_LAYER_DEFAULT, path[0], &endPos);
    work->masuIdEnd = path[0];
    work->downF = startPos.y > endPos.y;
    
    VECSubtract(&endPos, &startPos, &dir);
    work->rotY = HuAtan(-dir.x, -dir.z);
    VECScale(&dir, &dir, 0.5f);
    VECAdd(&startPos, &dir, &pos);
    MBModelPosSet(work->modelId, pos.x, startPos.y, pos.z);
    MBModelRotSet(work->modelId, 0, work->rotY, 0);
    work->obj = MBAddObj(260, 0, 0, GateDossunObjExec);
    work->obj->work[0] = (u32)work;
    gateDossunNum++;
}

void MBGateDossunKill(int no)
{
    GATEDOSSUN_WORK *work = gateDossunWorkP[no];
    MBModelKill(work->modelId);
    HuMemDirectFree(work);
    gateDossunWorkP[no] = NULL;
}

int MBGateDossunSearch(s16 masuId)
{
    int i;
    for(i=0; i<gateDossunNum; i++) {
        GATEDOSSUN_WORK *work = gateDossunWorkP[i];
        if(work->masuId == masuId) {
            break;
        }
    }
    if(i < gateDossunNum) {
        return i;
    }
    return -1;
}

static void GateDossunProcExec(void);
static void GateDossunProcDestroy(void);

typedef struct GateDossunProcWork_s {
    int playerNo;
    int result;
} GATEDOSSUN_PROCWORK;

int MBGateDossunExec(int playerNo, int masuId)
{
    GATEDOSSUN_PROCWORK *procWork;
    MBWalkNumDispSet(playerNo, FALSE);
    gateDossunProc = MBPrcCreate(GateDossunProcExec, 8202, 16384);
    procWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(GATEDOSSUN_PROCWORK), HU_MEMNUM_OVL);
    gateDossunProc->property = procWork;
    MBPrcDestructorSet(gateDossunProc, GateDossunProcDestroy);
    procWork->playerNo = playerNo;
    procWork->result = 1;
    while(gateDossunProc) {
        HuPrcVSleep();
    }
    MBWalkNumDispSet(playerNo, TRUE);
    return procWork->result;
}

static void GateDossunMoveStart(GATEDOSSUN_WORK *work, int playerNo);
static BOOL GateDossunMoveCheck(GATEDOSSUN_WORK *work);


static void GateDossunProcExec(void)
{
    GATEDOSSUN_PROCWORK *procWork = HuPrcCurrentGet()->property;
    int playerNo = procWork->playerNo;
    int no = MBGateDossunSearch(GwPlayer[playerNo].masuId);
    GATEDOSSUN_WORK *work = gateDossunWorkP[no];
    
    HuVecF modelPos;
    HuVecF playerPos;
    HuVecF pos;
    
    HuVecF dir;
    
    int i;
    float weight;
    
    float angle;
    
    MBModelPosGet(work->modelId, &modelPos);
    MBPlayerPosGet(playerNo, &playerPos);
    VECSubtract(&modelPos, &playerPos, &dir);
    angle = 90-HuAtan(dir.z, dir.x);
    MBPlayerRotateStart(playerNo, angle, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBPlayerMotIdleSet(playerNo);
    MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
    MBCameraMotionWait();
    MBCameraFocusSet(MB_MODEL_NONE);
    if(MBPlayerCoinGet(playerNo) < 10) {
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_GATE_DOSSUN_COIN, HUWIN_SPEAKER_DOSSUN);
        MBTopWinWait();
        MBTopWinKill();
        procWork->result = 0;
    } else {
        MBAudFXPlay(MSM_SE_GUIDE_04);
        if(!work->downF) {
            MBWinCreateChoice(MBWIN_TYPE_EVENT, MESS_BOARD_GATE_DOSSUN_CHOICE_UP, HUWIN_SPEAKER_DOSSUN, 0);
        } else {
            MBWinCreateChoice(MBWIN_TYPE_EVENT, MESS_BOARD_GATE_DOSSUN_CHOICE_DOWN, HUWIN_SPEAKER_DOSSUN, 0);
        }
        MBTopWinAttrReset(HUWIN_ATTR_NOCANCEL);
        if(GwPlayer[playerNo].comF) {
            MBComChoiceSetLeft();
        }
        MBTopWinWait();
        if(MBWinLastChoiceGet() == 1 || MBWinLastChoiceGet() == -1) {
            MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_GATE_DOSSUN_NO, HUWIN_SPEAKER_DOSSUN);
            MBTopWinWait();
            MBTopWinKill();
            procWork->result = 0;
        } else {
            MBCoinAddDispExec(playerNo, -10, TRUE);
            MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_GATE_DOSSUN_YES, HUWIN_SPEAKER_DOSSUN);
            MBTopWinWait();
            MBTopWinKill();
            MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_JUMP, 0, 4, HU3D_MOTATTR_NONE);
            for(i=0; i<30u; i++) {
                weight = i/30.0f;
                pos.x = playerPos.x+(weight*(modelPos.x-playerPos.x));
                pos.y = playerPos.y+(weight*((modelPos.y+200)-playerPos.y))+(100*(1.5*HuSin(weight*180)));
                pos.z = playerPos.z+(weight*(modelPos.z-playerPos.z));
                MBPlayerPosSetV(playerNo, &pos);
                HuPrcVSleep();
            }
            MBPlayerMotIdleSet(playerNo);
            HuPrcSleep(30);
            MBCameraFocusPlayerSet(playerNo);
            GateDossunMoveStart(work, playerNo);
            while(!GateDossunMoveCheck(work)) {
                HuPrcVSleep();
            }
        }
    }
    MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_WALK);
    MBCameraMotionWait();
    HuPrcEnd();
}

static void GateDossunProcDestroy(void)
{
    HuMemDirectFree(gateDossunProc->property);
    gateDossunProc = NULL;
}

static void GateDossunObjExec(OMOBJ *obj)
{
    GATEDOSSUN_WORK *work = (GATEDOSSUN_WORK *)obj->work[0];
    float posY;
    float weight;
    if(MBKillCheck()) {
        MBDelObj(obj);
        return;
    }
    switch(work->mode) {
        case 0:
            break;
        
        case 1:
            if(work->time > work->maxTime) {
                work->doneF = TRUE;
                work->mode = 2;
            } else {
                weight = HuSin(((float)work->time++/work->maxTime)*90);
                posY = obj->rot.y+(weight*(obj->scale.y-obj->rot.y));
                MBModelPosSet(work->modelId, obj->rot.x, posY, obj->rot.z);
                MBPlayerPosSet(work->playerNo, obj->rot.x, posY+200, obj->rot.z);
            }
            break;
        
        case 2:
            if(work->masuId != GwPlayer[work->playerNo].masuId) {
                work->mode = 3;
                work->time = 0;
                work->maxTime = 120;
            }
            break;
        
        case 3:
            if(work->time > work->maxTime) {
                work->mode = 0;
            } else {
                weight = (float)work->time++/work->maxTime;
                posY = obj->scale.y+(weight*(obj->rot.y-obj->scale.y));
                MBModelPosSet(work->modelId, obj->rot.x, posY, obj->rot.z);
            }
            break;
    }
}


static void GateDossunMoveStart(GATEDOSSUN_WORK *work, int playerNo)
{
    s16 path[MASU_LINK_MAX];
    work->doneF = FALSE;
    work->mode = 1;
    work->time = 0;
    work->maxTime = 30;
    work->playerNo = playerNo;
    MBModelPosGet(work->modelId, &work->obj->rot);
    MBMasuPathNoLoopCheck(work->masuId, path);
    MBMasuPosGet(MASU_LAYER_DEFAULT, path[0], &work->obj->scale);
}

static BOOL GateDossunMoveCheck(GATEDOSSUN_WORK *work)
{
    return work->doneF;
}