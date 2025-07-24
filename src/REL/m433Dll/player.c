#include "REL/m433Dll.h"
#include "datanum/effect.h"
#include "datanum/mgconst.h"
#include "datanum/gamemes.h"

#include "game/wipe.h"
#include "game/gamemes.h"

typedef struct PlayerMotData_s {
    u16 index;
    float shift;
    float shiftStart;
    float shiftEnd;
    unsigned int attr;
} PLAYER_MOTDATA;

static OMOBJ *playerObj[GW_PLAYER_MAX+1];
static OMOBJ *ballObj;
static OMOBJ *gameManObj;
static int playerSide[GW_PLAYER_MAX];
static int ballExplodeTimerPrev;
static int ballExplodeTimer;
static int ballSeNo;
static float lbl_1_bss_7C8;


static float playerHeightTbl[10] = {
    130,
    130,
    160,
    130,
    130,
    160,
    180,
    100,
    130,
    130
};

static float playerRadiusTbl[10] = {
    50,
    50,
    50,
    50,
    50,
    50,
    70,
    50,
    50,
    50
};


#define FXID(charBase, no) (charBase+(no))

static int playerFxIdTbl[10][4] = {
    FXID(MSM_SE_VOICE_MARIO, 2),
    FXID(MSM_SE_VOICE_MARIO, 10),
    FXID(MSM_SE_VOICE_MARIO, 4),
    FXID(MSM_SE_VOICE_MARIO, 10),
    
    FXID(MSM_SE_VOICE_LUIGI, 2),
    FXID(MSM_SE_VOICE_LUIGI, 10),
    FXID(MSM_SE_VOICE_LUIGI, 4),
    FXID(MSM_SE_VOICE_LUIGI, 10),
    
    FXID(MSM_SE_VOICE_PEACH, 2),
    FXID(MSM_SE_VOICE_PEACH, 10),
    FXID(MSM_SE_VOICE_PEACH, 4),
    FXID(MSM_SE_VOICE_PEACH, 10),
    
    FXID(MSM_SE_VOICE_YOSHI, 2),
    FXID(MSM_SE_VOICE_YOSHI, 10),
    FXID(MSM_SE_VOICE_YOSHI, 4),
    FXID(MSM_SE_VOICE_YOSHI, 10),
    
    FXID(MSM_SE_VOICE_WARIO, 2),
    FXID(MSM_SE_VOICE_WARIO, 10),
    FXID(MSM_SE_VOICE_WARIO, 4),
    FXID(MSM_SE_VOICE_WARIO, 10),
    
    FXID(MSM_SE_VOICE_DAISY, 2),
    FXID(MSM_SE_VOICE_DAISY, 10),
    FXID(MSM_SE_VOICE_DAISY, 4),
    FXID(MSM_SE_VOICE_DAISY, 10),
    
    FXID(MSM_SE_VOICE_WALUIGI, 2),
    FXID(MSM_SE_VOICE_WALUIGI, 10),
    FXID(MSM_SE_VOICE_WALUIGI, 4),
    FXID(MSM_SE_VOICE_WALUIGI, 10),
    
    FXID(MSM_SE_VOICE_KINOPIO, 2),
    FXID(MSM_SE_VOICE_KINOPIO, 10),
    FXID(MSM_SE_VOICE_KINOPIO, 4),
    FXID(MSM_SE_VOICE_KINOPIO, 10),
    
    FXID(MSM_SE_VOICE_TERESA, 2),
    FXID(MSM_SE_VOICE_TERESA, 10),
    FXID(MSM_SE_VOICE_TERESA, 4),
    FXID(MSM_SE_VOICE_TERESA, 10),
    
    FXID(MSM_SE_VOICE_MINIKOOPAR, 2),
    FXID(MSM_SE_VOICE_MINIKOOPAR, 10),
    FXID(MSM_SE_VOICE_MINIKOOPAR, 4),
    FXID(MSM_SE_VOICE_MINIKOOPAR, 10),
};

#undef FXID

static int playerSpecialMdlTbl[20] = {
    M433_HSF_char_kinopio,
    M433_HSF_char_teresa,
    M433_HSF_char_minikoopa
};

static int playerMotTbl[20] = {
    CHARMOT_HSF_c000m1_300,
    CHARMOT_HSF_c000m1_301,
    CHARMOT_HSF_c000m1_302,
    CHAR_HSF_c000m1_495,
    CHAR_HSF_c000m1_496,
    CHAR_HSF_c000m1_497,
    CHAR_HSF_c000m1_498,
    CHAR_HSF_c000m1_499,
    CHAR_HSF_c000m1_500,
    CHAR_HSF_c000m1_501,
    CHAR_HSF_c000m1_502,
    CHAR_HSF_c000m1_503,
    CHAR_HSF_c000m1_504,
    M433_HSF_playermot_mario,
    CHARMOT_HSF_c000m1_324,
    CHARMOT_HSF_c000m1_423,
    CHARMOT_HSF_c000m1_306,
    CHARMOT_HSF_c000m1_307,
    CHARMOT_HSF_c000m1_356,
    CHARMOT_HSF_c000m1_357
};

static int playerSpecialMotTbl[20] = {
    M433_HSF_char_kinopio_mot1,
    M433_HSF_char_kinopio_mot2,
    M433_HSF_char_kinopio_mot3,
    M433_HSF_char_kinopio_mot4,
    M433_HSF_char_kinopio_mot5,
    M433_HSF_char_kinopio_mot6,
    M433_HSF_char_kinopio_mot7,
    M433_HSF_char_kinopio_mot8,
    M433_HSF_char_kinopio_mot9,
    M433_HSF_char_kinopio_mot10,
    M433_HSF_char_kinopio_mot11,
    M433_HSF_char_kinopio_mot12,
    M433_HSF_char_kinopio_mot13,
    M433_HSF_char_kinopio_mot18,
    M433_HSF_char_kinopio_mot19,
    M433_HSF_char_kinopio_mot20,
    M433_HSF_char_kinopio_mot14,
    M433_HSF_char_kinopio_mot16,
    M433_HSF_char_kinopio_mot15,
    M433_HSF_char_kinopio_mot17
};

static PLAYER_MOTDATA playerMotData[24] = {
    { 0, 0.2f, 0.0f, -1.0f, HU3D_MOTATTR_LOOP },
    { 1, 0.2f, 0.0f, -1.0f, HU3D_MOTATTR_LOOP },
    { 2, 0.2f, 0.0f, -1.0f, HU3D_MOTATTR_LOOP },
    { 3, 0.15f, 0.0f, 0.0f, HU3D_MOTATTR_NONE },
    { 3, 0.02f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
    { 4, 0.02f, 0.2f, -1.0f, HU3D_MOTATTR_NONE },
    { 5, 0.05f, 0.25f, -1.0f, HU3D_MOTATTR_NONE },
    { 6, 0.1f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
    { 7, 0.02f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
    { 11, 0.02f, 0.4f, -1.0f, HU3D_MOTATTR_NONE },
    { 12, 0.02f, 0.36666667f, -1.0f, HU3D_MOTATTR_NONE },
    { 8, 0.01f, 0.2f, -1.0f, HU3D_MOTATTR_NONE },
    { 9, 0.02f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
    { 10, 0.02f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
    { 5, 0.08f, 0.0f, 0.25f, HU3D_MOTATTR_NONE },
    { 11, 0.2f, 0.0f, 0.4f, HU3D_MOTATTR_NONE },
    { 12, 0.08f, 0.0f, 0.3f, HU3D_MOTATTR_NONE },
    { 13, 0.2f, 0.0f, -1.0f, HU3D_MOTATTR_LOOP },
    { 14, 0.2f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
    { 15, 0.2f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
    { 16, 0.2f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
    { 17, 0.2f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
    { 18, 0.2f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
    { 19, 0.2f, 0.0f, -1.0f, HU3D_MOTATTR_NONE },
};

void M433PlayerCreate(OMOBJ *obj);
void M433BallCreate(OMOBJ *obj);
void M433GameManCreate(OMOBJ *obj);

void M433PlayerInit(OMOBJMAN *om)
{
    s16 i, j;
    unsigned int grpMap[GW_PLAYER_MAX];
    unsigned int side[GW_PLAYER_MAX];
    for(i=0; i<GW_PLAYER_MAX; i++) {
        grpMap[i] = GwPlayerConf[i].grp;
        side[i] = i;
    }
    for(i=0; i<GW_PLAYER_MAX-1; i++) {
        for(j=i+1; j<4; j++) {
            if(grpMap[i] > grpMap[j]) {
                int temp = grpMap[i];
                grpMap[i] = grpMap[j];
                grpMap[j] = temp;
                temp = side[i];
                side[i] = side[j];
                side[j] = temp;
            }
        }
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        playerSide[side[i]] = i;
    }
    gameManObj = omAddObjEx(om, 20, 0, 0, 0, M433GameManCreate);
    ballObj = omAddObjEx(om, 21, 32, 0, 0, M433BallCreate);
    omMakeGroupEx(om, 0, 4);
    omGetGroupMemberListEx(om, 0);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        OMOBJ *obj = playerObj[i] = omAddObjEx(om, 40, 2, 20, 0, M433PlayerCreate);
        obj->work[0] = i;
    }
}

void M433PlayerClose(void)
{
    CharModelKill(CHARNO_NONE);
}

typedef struct M433PlayerWork_s {
    int charNo;
    int specialCharNo;
    int moveTime;
    int motId;
    unsigned int motCancelF;
    unsigned int side;
    unsigned int cel;
    unsigned int btnDown;
    unsigned int btn;
    float stkMag;
    float stkAngle;
    BOOL comF;
    BOOL serveF;
    BOOL moveF;
    BOOL serveValidF;
    BOOL comServeF;
    BOOL comTimeServeF;
    BOOL comNearServeF;
    BOOL comMidServeF;
    BOOL comFarServeF;
    BOOL unk50;
    BOOL comReadyServeF;
    BOOL nearBallF;
    BOOL jumpValidF;
    BOOL ballSpikeF;
    BOOL landF;
    BOOL jumpF;
    BOOL comContactF;
    unsigned int event;
    unsigned int mode;
    unsigned int spikeMode;
    unsigned int ballMode;
    unsigned int jumpMode;
    unsigned int winMode;
    unsigned int timer;
    unsigned int serveMode;
    unsigned int maxTime;
    char unk94[4];
    float distLaunch;
    s16 modelTime;
    char unk9E[6];
    float rotY;
    float rotYTarget;
    float rotYSpeed;
    float rotYAccel;
    float scaleMark;
    float scaleMarkTarget;
    float markRotY;
    float comAccuracy;
    char unkC4[8];
    unsigned int comServeMaxTime;
    unsigned int comServeTime;
    unsigned int comJumpMaxTime;
    unsigned int comJumpTime;
    float comAngle;
    float comMag;
    BOOL comWideF;
    BOOL comTargetF;
    BOOL comAllowServeF;
    BOOL comAllowJumpF;
    BOOL comServeCelF;
    unsigned int comJumpMode;
    Vec comTargetPos;
    Vec comJumpTargetPos;
    Vec comServeTargetPos;
    float radius;
    float gravity;
    float shockTimer;
    Vec pos;
    Vec vel;
    Vec targetPos;
    OMOBJ *obj;
} M433_PLAYERWORK;

void M433PlayerModelInit(OMOBJ *obj);
void M433PlayerMain(OMOBJ *obj);
void M433PlayerModelUpdate(OMOBJ *obj);

void M433PlayerMotionSet(OMOBJ *obj, unsigned int motId);
BOOL M433PlayerMotionNextSet(OMOBJ *obj, unsigned int motId);

BOOL M433PlayerMotionShiftNextSet(OMOBJ *obj, unsigned int motId);

BOOL M433PlayerMotionCheck(OMOBJ *obj);
BOOL M433PlayerMotionShiftCheck(OMOBJ *obj);

void M433PlayerCreate(OMOBJ *obj)
{
    int rDist;
    M433_PLAYERWORK *work;
    s16 i;
    unsigned charNo;
    int playerNo;

    float accuracyTbl[4] = { 0.25f, 0.5f, 0.75f, 1.0f };
    obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M433_PLAYERWORK), HU_MEMNUM_OVL);
    work = obj->data;
    memset(work, 0, sizeof(M433_PLAYERWORK));
    playerNo = obj->work[0];
    charNo = GwPlayerConf[playerNo].charNo;
    work->charNo = charNo;
    work->specialCharNo = -1;
    if (charNo >= 7) {
        work->specialCharNo = charNo - 7;
    }
    rDist = work->moveTime = 0;
    work->side = (playerSide[obj->work[0]] >> 1) & 1;
    work->cel = playerSide[obj->work[0]] & 1;
    for(i=0; i<4; i++) {
        if((i != obj->work[0]) && (work->side == ((playerSide[i]>>1)&1))) {
            break;
        }
    }
    work->obj = playerObj[i];
    work->comAccuracy = accuracyTbl[GwPlayerConf[playerNo].dif & 3];
    work->comF = GwPlayerConf[obj->work[0]].type;
    if (work->specialCharNo < 0) {
        obj->mdlId[0] = CharModelCreate(charNo, 4);
        CharModelStepFxSet(charNo, 2);
    }
    else {
        obj->mdlId[0] = Hu3DModelCreateData(playerSpecialMdlTbl[work->specialCharNo]);
    }
    Hu3DModelShadowSet(obj->mdlId[0]);
    obj->mdlId[1] = Hu3DModelCreateData(M433_HSF_s_mark);
    Hu3DModelLayerSet(obj->mdlId[1], 1);
    for(i = 0; i < 20; i++) {
        if (work->specialCharNo < 0) {
            if (DIRNUM(playerMotTbl[i]) != DATA_m433) {
                obj->mtnId[i] = CharMotionCreate(charNo, playerMotTbl[i]);
            }
            else {
                obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], charNo + playerMotTbl[i]);
            }
        }
        else {
            obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], work->specialCharNo + playerSpecialMotTbl[i]);
        }
    }
    if (work->specialCharNo < 0) {
        CharMotionVoiceOnSet(charNo, playerMotTbl[18], 0);
        CharMotionVoiceOnSet(charNo, playerMotTbl[19], 0);
        CharMotionDataClose(charNo);
    }
    else {
        CharNpcDustVoiceOffSet(obj->mdlId[0], obj->mtnId[1], 0);
        CharNpcDustVoiceOffSet(obj->mdlId[0], obj->mtnId[2], 1);
    }
    obj->trans.x = work->pos.x = (0.4f * (700.0f * (obj->work[0] + 1))) - 700.0f;
    obj->trans.y = work->pos.y = 0.0f;
    obj->trans.z = work->pos.z = 0.0f;
    work->vel.x = work->vel.y = work->vel.z = 0.0f;
    work->rotY = work->rotYTarget = 0.0f;
    work->rotYSpeed = 0.0f;
    work->rotYAccel = 0.02f;
    work->radius = playerRadiusTbl[charNo];
    work->motId = -1;
    work->event = -1;
    M433PlayerMotionSet(obj, 0);
    work->mode = 0;
    obj->objFunc = M433PlayerModelInit;
}


void M433PlayerModelInit(OMOBJ *obj)
{
    M433_PLAYERWORK *work = obj->data;
    M433PlayerModelUpdate(obj);
    work->mode = 0;
    obj->objFunc = M433PlayerMain;
}

void M433PlayerWin(OMOBJ *obj);
void M433PlayerComMain(OMOBJ *obj);
void M433PlayerServe(OMOBJ *obj);
void M433PlayerMotionUpdate(OMOBJ *obj);
void M433PlayerSpike(OMOBJ *obj);
void M433PlayerBallCollide(OMOBJ *obj);
void M433PlayerJump(OMOBJ *obj);
void M433PlayerShock(OMOBJ *obj);
void M433PlayerTargetUpdate(OMOBJ *obj);

void M433PlayerMain(OMOBJ *obj)
{
    M433_PLAYERWORK *work = obj->data;
    BOOL cancelF = FALSE;
    M433_PLAYERWORK *other = work->obj->data;
    HuVecF dir;
    float x, y;
    
    work->btnDown = 0;
    work->btn = 0;
    work->stkAngle = 0;
    work->stkMag = 0;
    if(!work->serveF || M433GameWinGet(work->side)) {
        if(M433EventGet(work->side) != work->event) {
            work->event = M433EventGet(work->side);
            switch(M433EventGet(work->side)) {
                case 0:
                    work->comContactF = FALSE;
                    work->vel.x = work->vel.y = work->vel.z = 0;
                    work->mode = 0;
                    work->serveValidF = FALSE;
                    work->comServeF = work->comTimeServeF = FALSE;
                    work->pos.y = 0;
                    if(M433ServeGroupCheck(work->side)) {
                        if(work->cel == M433ServeCelGet(work->side)) {
                            work->pos.z = 0;
                            work->pos.x = 840.00006f;
                            work->mode = 1;
                            work->serveValidF = TRUE;
                        } else {
                            work->pos.z = -105.00001f;
                            work->pos.x = 347.5f;
                        }
                    } else {
                        if(work->cel == M433ServeCelGet(work->side)) {
                            work->pos.z = 140;
                            work->pos.x = 488.5f;
                        } else {
                            work->pos.z = -140;
                            work->pos.x = 347.5f;
                        }
                    }
                    work->rotY = work->rotYTarget = -90.0f;
                    work->rotYSpeed = 1.0f;
                    work->rotYAccel = 1;
                    if(work->side == 0) {
                        work->pos.x *= -1.0f;
                        work->rotY = work->rotYTarget = 90.0f;
                    }
                    work->motId = -1;
                    work->moveF = TRUE;
                    work->timer = work->serveMode = 0;
                    work->ballMode = work->spikeMode = work->jumpMode = 0;
                    break;
                    
                case 1:
                case 2:
                    if(work->mode == 1) {
                        work->moveF = FALSE;
                    }
                    break;
                
                case 3:
                    work->mode = 2;
                    work->ballMode = work->spikeMode = work->jumpMode = 0;
                    work->moveF = FALSE;
                    break;
                    
                case 4:
                case 5:
                    work->mode = 3;
                    break;
                
                case 6:
                    if(M433PointGroupCheck(work->side)) {
                        M433PlayerMotionSet(obj, 22);
                        if(work->specialCharNo >= 0) {
                            HuAudFXPlay(playerFxIdTbl[work->charNo][0]);
                        } else {
                            CharWinLoseVoicePlay(work->charNo, playerMotTbl[18], playerFxIdTbl[0][0]);
                        }
                        VECSubtract(&other->pos, &work->pos, &dir);
                        work->rotYTarget = HuAtan(dir.x, dir.z);
                        work->rotYSpeed = 0.15f;
                        work->rotYAccel = 0;
                    } else {
                        M433PlayerMotionSet(obj, 23);
                    }
                    work->mode = 5;
                    break;
                
                case 8:
                    work->vel.x = work->vel.y = work->vel.z = 0;
                    work->serveF = FALSE;
                    M433PlayerMotionSet(obj, 0);
                    work->mode = 0;
                    break;
                    
                case 7:
                    work->mode = 4;
                    work->timer = work->serveMode = 0;
                    break;
            }
        }
    }
    if(M433GamemesModeGet() == M433_GAMEMES_WAIT) {
        if(work->comF) {
            M433PlayerComMain(obj);
        } else {
            work->btn = HuPadBtn[GwPlayerConf[obj->work[0]].padNo];
            work->btnDown = HuPadBtnDown[GwPlayerConf[obj->work[0]].padNo];
            x = HuPadStkX[GwPlayerConf[obj->work[0]].padNo];
            y = -HuPadStkY[GwPlayerConf[obj->work[0]].padNo];
            work->stkAngle = HuAtan(x, y);
            work->stkMag = 0.015625f*HuMagPoint2D(x, y);
            if(work->stkMag > 1) {
                work->stkMag = 1;
            }
        }
    }
    switch(work->mode) {
        case 0:
            M433PlayerMotionSet(obj, 0);
            cancelF = TRUE;
            break;
        
        case 1:
            M433PlayerServe(obj);
            break;
        
        case 2:
            M433PlayerMotionUpdate(obj);
            M433PlayerSpike(obj);
            M433PlayerBallCollide(obj);
            break;
        
        case 3:
            M433PlayerMotionUpdate(obj);
            M433PlayerJump(obj);
            M433PlayerBallCollide(obj);
            break;
        
        case 4:
            M433PlayerShock(obj);
            break;
        
        case 5:
            work->vel.x = work->vel.y = work->vel.z = 0;
            work->serveF = FALSE;
            if(M433PlayerMotionCheck(obj)) {
                work->serveF = FALSE;
                work->mode = 0;
            }
            break;
    }
    VECAdd(&work->pos, &work->vel, &work->pos);
    if(M433EventGet(work->side) >= 2) {
        M433PlayerTargetUpdate(obj);
    }
    M433PlayerModelUpdate(obj);
    if(M433GamemesModeGet() == M433_GAMEMES_FINISH && cancelF) {
        work->mode = 0;
        obj->objFunc = M433PlayerWin;
    }
}

void M433PlayerWin(OMOBJ *obj)
{
    M433_PLAYERWORK *work = obj->data;
    Vec directPos[2] = { { 130.0f, 0.0f, -50.0f }, { 250.0f, 0.0f, 50.0f } };
    Vec escapePos[2] = { { 150.0f, 0.0f, 100.0f }, { 300.0f, 0.0f, -100.0f } };
    M433_PLAYERWORK *other = work->obj->data;
    M433_PLAYERWORK *workBackup;
    M433_PLAYERWORK *otherBackup;
    HuVecF *pos;
    HuVecF cameraPos;
    HuVecF dir;
    int i;
    M433_PLAYERWORK *temp;
    float dist;
    float outDist[2];
    static unsigned int delayTbl[10] = {
        50,
        50,
        60,
        50,
        50,
        50,
        60,
        30,
        30,
        50
    };
    
    VECAdd(&work->pos, &work->vel, &work->pos);
    M433PlayerTargetUpdate(obj);
    M433PlayerModelUpdate(obj);
    switch(work->mode) {
        case 0:
            M433PlayerMotionSet(obj, 0);
            if(!M433GameFinishEndCheck()) {
                break;
            }
            if(work->cel == 0) {
                BOOL escapeF = TRUE;
                if(work->side == 0) {
                    directPos[0].x *= -1;
                    directPos[1].x *= -1;
                    escapePos[0].x *= -1;
                    escapePos[1].x *= -1;
                }
                workBackup = work;
                otherBackup = other;
                pos = directPos;
                work->winMode = other->winMode = 0;
                if(delayTbl[workBackup->charNo] != delayTbl[otherBackup->charNo]) {
                    if(delayTbl[workBackup->charNo] < delayTbl[otherBackup->charNo]) {
                        temp = workBackup;
                        workBackup = otherBackup;
                        otherBackup = temp;
                    }
                    dist = M433PlaneDistGet(&workBackup->pos, &directPos[0], &otherBackup->pos, &directPos[1]);
                    outDist[0] = 0.8f*(workBackup->radius+otherBackup->radius);
                    if(dist < outDist[0]) {
                        pos = escapePos;
                        work->winMode = other->winMode = 1;
                        OSReport("Escape!\n");
                    } else {
                        escapeF = FALSE;
                        OSReport("Direct!\n");
                    }
                }
                
                if(escapeF) {
                    workBackup = work;
                    otherBackup = other;
                    for(i=0; i<2; i++) {
                        outDist[i] = M433PlaneDistGet(&workBackup->pos, &pos[i&0x1], &otherBackup->pos, &pos[(i+1)&0x1]);
                    }
                    if(outDist[0] < outDist[1]) {
                        temp = workBackup;
                        workBackup = otherBackup;
                        otherBackup = temp;
                    }
                }
                workBackup->targetPos = pos[0];
                workBackup->targetPos.z -= workBackup->radius;
                otherBackup->targetPos = pos[1];
            }
            work->serveF = work->ballSpikeF = work->moveF = FALSE;
            if(work->winMode) {
                work->mode++;
            } else {
                work->mode += 2;
            }
            work->timer = 0;
            break;
        
        case 1:
            if(work->winMode == 1) {
                VECSubtract(&work->targetPos, &work->pos, &dir);
                dir.y = 0;
                dist = VECMag(&dir);
                if(dist < 20) {
                    dist = 0.01f;
                    work->winMode++;
                } else {
                    if(VECMag(&dir) >= 100.0f) {
                        dist = 100;
                    }
                    dist = 0.2f+(0.008f*dist);
                }
                work->stkAngle = HuAtan(dir.x, dir.z);
                work->stkMag = dist;
                M433PlayerMotionUpdate(obj);
            } else if(work->winMode == 2) {
                M433PlayerMotionShiftNextSet(obj, 0);
                if(work->cel == 0 && other->winMode) {
                    if(work->side == 0) {
                        directPos[0].x *= -1;
                        directPos[1].x *= -1;
                    }
                    workBackup = work;
                    otherBackup = other;
                    if(delayTbl[workBackup->charNo] < delayTbl[otherBackup->charNo]) {
                        temp = workBackup;
                        workBackup = otherBackup;
                        otherBackup = temp;
                    }
                    workBackup->targetPos = directPos[0];
                    workBackup->targetPos.z -= workBackup->radius;
                    otherBackup->targetPos = directPos[1];
                    work->winMode = other->winMode = 3;
                }
            }
            if(work->winMode == 3) {
                work->mode++;
                work->timer = 0;
            }
            break;
        
        case 2:
            VECSubtract(&work->targetPos, &work->pos, &dir);
            dir.y = 0;
            dist = VECMag(&dir);
            if(dist < 10) {
                dist = 0.01f;
            } else {
                if(VECMag(&dir) >= 100.0f) {
                    dist = 100;
                }
                dist = 0.2f+(0.008f*dist);
            }
            work->stkAngle = HuAtan(dir.x, dir.z);
            work->stkMag = dist;
            M433PlayerMotionUpdate(obj);
            if(work->stkMag < 0.02f || work->timer > 300.0f) {
                work->rotYTarget = 0;
                work->rotYSpeed = 0.15f;
                work->rotYAccel = 0;
                work->vel.x = work->vel.y = work->vel.z = 0;
                M433GameWinFlagSet(obj->work[0]);
                work->mode++;
            }
            break;
        
        case 3:
            if(M433GameWinModeGet() == M433_WINMODE_NORMAL) {
                if(M433WinGroupCheck(work->side)) {
                    M433PlayerMotionSet(obj, 20);
                    if(work->specialCharNo >= 0) {
                        HuAudFXPlay(playerFxIdTbl[work->charNo][2]);
                    }
                } else {
                    M433PlayerMotionSet(obj, 21);
                    if(work->specialCharNo >= 0) {
                        HuAudFXPlay(playerFxIdTbl[work->charNo][3]);
                    }
                }
                work->mode++;
            } else {
                M433PlayerMotionShiftNextSet(obj, 0);
            }
            break;
        
        case 4:
            if(M433WinGroupCheck(work->side)) {
                cameraPos.x = Center.x + (CZoom * (HuSin(CRot.y) * HuCos(CRot.x)));
                cameraPos.y = Center.y + (CZoom * -HuSin(CRot.x));
                cameraPos.z = Center.z + (CZoom * (HuCos(CRot.y) * HuCos(CRot.x)));
                VECSubtract(&cameraPos, &work->pos, &dir);
                work->rotYTarget = HuAtan(dir.x, dir.z);
            }
            if(M433PlayerMotionCheck(obj)) {
                M433GameWinFlag2Set(obj->work[0]);
                work->mode++;
            }
            break;
    }
    
}

void M433PlayerModelUpdate(OMOBJ *obj)
{
    M433_PLAYERWORK *work = obj->data;
    HuVecF pos;
    work->modelTime++;
    work->timer++;
    obj->trans.x = work->pos.x;
    obj->trans.y = work->pos.y;
    obj->trans.z = work->pos.z;
    work->rotYSpeed += work->rotYAccel;
    if(work->rotYSpeed > 0.45f) {
        work->rotYSpeed = 0.45f;
    }
    work->rotY = M433AngleLerp(work->rotY,work->rotYTarget, work->rotYSpeed);
    obj->rot.y = work->rotY;
    Hu3DModelPosSet(obj->mdlId[1], work->pos.x, 11, work->pos.z);
    work->markRotY += 3.0000002f;
    if(work->markRotY >= 360) {
        work->markRotY -= 360;
    }
    Hu3DModelRotSet(obj->mdlId[1], 0, work->markRotY, 0);
    work->scaleMarkTarget = 0;
    if(M433GamemesModeGet() == M433_GAMEMES_WAIT) {
        if(M433GameEventGet() == 4 || M433GameEventGet() == 3) {
            if(M433ServeSideGet(&pos) == work->side && M433ContactCountGet() != 0 && M433ContactCountGet() < 3 && !work->ballSpikeF && M433ContactCheck(work->side, work->cel)) {
                work->scaleMarkTarget = 1;
            }
        }
        if(work->mode == 1) {
            work->scaleMarkTarget = 1;
        }
    }
    work->scaleMark += 0.1f*(work->scaleMarkTarget-work->scaleMark);
    Hu3DModelScaleSet(obj->mdlId[1], work->scaleMark, 1, work->scaleMark);
    if(work->scaleMarkTarget <= 0.0f && work->scaleMark < 0.1f) {
        Hu3DModelDispOff(obj->mdlId[1]);
        work->scaleMark = 0;
    } else {
        Hu3DModelDispOn(obj->mdlId[1]);
    }
}

void M433ComJumpStart(OMOBJ *obj);
void M433ComMove(OMOBJ *obj);
BOOL M433ComTargetSet(unsigned int side, HuVecF *pos);

void M433PlayerComMain(OMOBJ *obj)
{
    M433_PLAYERWORK *other;
    M433_PLAYERWORK *work = obj->data;
    BOOL moveF = TRUE;
    BOOL temp = 0;
    HuVecF pos;
    HuVecF dir;
    float speed;
    
    other = work->obj->data;
    switch(work->mode) {
        case 0:
            work->comContactF = FALSE;
            M433ComJumpStart(obj);
            break;
        
        case 1:
            if(work->comServeF) {
                work->btnDown |= PAD_BUTTON_A;
                work->comAngle = frandmod(360);
                work->comMag = 0.3f+(0.00040000002f*(work->comAccuracy*frandmod(1000)));
                work->comWideF = FALSE;
                if(frandmod(1000) > (300+(work->comAccuracy*500))) {
                    work->comWideF = TRUE;
                }
            }
            if(M433BallCoordGet(&pos)) {
                if(work->comWideF != 0) {
                    if((fabsf(pos.z) > 1.1f) || (pos.x > 1.1f)) {
                        work->comMag = 0.0f;
                    }
                }
                else {
                    if((fabsf(pos.z) > 0.95f) || (pos.x > 0.95f)) {
                        work->comMag = 0.0f;
                    }
                }
            }
            work->stkAngle = work->comAngle;
            work->stkMag = work->comMag;
            if(work->comTimeServeF) {
                if(work->comServeTime++ >= work->comServeMaxTime) {
                    work->btnDown |= PAD_BUTTON_A;
                }
            } else if(work->ballSpikeF) {
                work->comServeMaxTime = ((60.0f*(0.35f*work->comAccuracy))/1000.0f)*frandmod(1000);
                work->comServeTime = 0;
            }
            M433ComJumpStart(obj);
            break;
        
        case 2:
            work->comAllowJumpF = FALSE;
            if(frandmod(1000) < (work->comAccuracy*300)) {
                work->comAllowJumpF = TRUE;
            }
            if(gameType == 1) {
                switch(ballExplodeTimer) {
                    case 2:
                    case 6:
                        if(frandmod(1000) < (work->comAccuracy*300)) {
                            work->comAllowJumpF = TRUE;
                            break;
                        }
                        break;

                    case 3:
                    case 4:
                        work->comAllowJumpF = FALSE;
                        break;
                }
            }
            M433ComMove(obj);
            if(work->comServeCelF) {
                if(M433GameEventGet() == 2) {
                    work->comServeCelF = FALSE;
                    work->comJumpMode = 0;
                }
                if(M433BallSideGet(&pos) == work->side) {
                    work->comJumpTime++;
                }
                if(M433BallSideGet(&pos) == work->side && work->comJumpMode == 0) {
                    M433BallPointCheck(0.9f, &work->comJumpTargetPos);
                    work->comJumpTargetPos.x = 57.5f;
                    if(work->side == 0) {
                         work->comJumpTargetPos.x *= -1;
                    }
                    work->comJumpMode++;
                }
                if(work->comJumpMode == 1) {
                    if(work->comJumpTime >= 0.5f*work->comJumpMaxTime && M433BallJumpCheck(&work->comJumpTargetPos)) {
                        work->comJumpTargetPos.x = 57.5f;
                        if(work->side == 0) {
                             work->comJumpTargetPos.x *= -1;
                        }
                        work->comJumpMode++;
                    }
                } else if(work->comJumpMode == 2 && (work->comJumpTime >= work->comJumpMaxTime || M433ServeSideGet(&pos) == work->side)) {
                    work->btnDown |= PAD_BUTTON_B;
                    work->comServeCelF = FALSE;
                    work->comJumpMode = 0;
                }
            }
            if(M433BallSideGet(&pos) == work->side && M433BallTimeGet() >= (60*(0.8f-(0.4f*work->comAccuracy))) && work->comContactF) {
                VECSubtract(&pos, &work->pos, &dir);
                dir.y = 0;
                speed = VECMag(&dir);
                if(VECMag(&dir) >= 20) {
                    if(speed >= 50) {
                        speed = 50;
                    }
                    work->stkAngle = HuAtan(dir.x, dir.z);
                    work->stkMag = speed/50;
                }
                if(M433ServeTargetTimeGet() < (0.8f-(0.6f*work->comAccuracy))) {
                    if(work->comNearServeF || (work->comMidServeF && work->comAllowServeF)) {
                        work->btnDown |= PAD_BUTTON_A;
                    }
                }
                moveF = FALSE;
            }
            if(work->comJumpMode) {
                VECSubtract(&work->comJumpTargetPos, &work->pos, &dir);
                dir.y = 0;
                speed = VECMag(&dir);
                if(speed >= 10) {
                    if(speed >= 50) {
                        speed = 50;
                    }
                    work->stkAngle = HuAtan(dir.x, dir.z);
                    work->stkMag = speed/50;
                } else {
                    work->stkAngle = 90;
                    if(work->side) {
                        work->stkAngle *= -1;
                    }
                    work->stkMag = 0.01f;
                }
                moveF = FALSE;
            }
            if(moveF) {
                VECSubtract(&work->comTargetPos, &work->pos, &dir);
                dir.y = 0;
                speed = VECMag(&dir);
                speed -= 100.0f*(1.25f*(2.0f-(1.4f*work->comAccuracy)));
                if(speed >= 10) {
                    if(speed >= 70) {
                        speed = 70;
                    }
                    work->stkAngle = HuAtan(dir.x, dir.z);
                    work->stkMag = speed/70;
                } else {
                    work->stkAngle = 90;
                    if(work->side) {
                        work->stkAngle *= -1;
                    }
                }
            }
            break;
        
        case 3:
            M433ComMove(obj);
            M433ComJumpStart(obj);
            if(M433ServeSideGet(&pos) != work->side) {
                break;
            }
            if(!work->jumpF) {
                if(M433ContactCheck(work->side, work->cel)){ 
                    VECSubtract(&pos, &work->pos, &dir);
                    dir.y = 0.0f;
                    speed = VECMag(&dir);
                    if (speed >= 30.000002f) {
                        if (speed >= 70.0f) {
                            speed = 70.0f;
                        }
                        work->stkMag = speed / 70.0f;
                        work->stkAngle = HuAtan(dir.x, dir.z);
                    }
                } else if(!other->comF) {
                    VECSubtract(&work->comTargetPos, &work->pos, &dir);
                    dir.y = 0;
                    speed = VECMag(&dir);
                    speed -= 100.0f*(1.25f*(2.0f-(1.25f*work->comAccuracy)));
                    if(speed >= 10) {
                        if(speed >= 70) {
                            speed = 70;
                        }
                        work->stkAngle = HuAtan(dir.x, dir.z);
                        work->stkMag = speed/70;
                    } else {
                        work->stkAngle = 90;
                        if(work->side) {
                            work->stkAngle *= -1;
                        }
                    }
                }
                if(((work->comContactF == FALSE && M433GameEventGet() == 3 && work->comAllowJumpF) || (work->comContactF && M433GameEventGet() == 4)) && work->jumpValidF) {
                    work->btnDown |= PAD_BUTTON_B;
                    work->comTargetF = FALSE;
                    if(frandmod(1000) < (int)(300*(work->comAccuracy-0.4f))) {
                        work->comTargetF = M433ComTargetSet(work->side, &work->comServeTargetPos);
                        work->comAngle = 0;
                        work->comMag = 0;
                        work->comWideF = FALSE;
                    }
                    if(!work->comTargetF) {
                        work->comAngle = frandmod(360);
                        work->comMag = 0.3f+(0.00040000002f*(work->comAccuracy*frandmod(1000)));
                        work->comWideF = FALSE;
                        if (frandmod(1000) > (300.0f+(500.0f*work->comAccuracy))) {
                            work->comWideF = TRUE;
                        }
                    }
                }
                if(work->comFarServeF || work->comNearServeF || work->comMidServeF) {
                    work->btnDown |= PAD_BUTTON_A;
                }
            } else {
                if(M433BallCoordGet(&pos) != 0) {
                    if(work->comTargetF) {
                        VECSubtract(&work->comServeTargetPos, &pos, &dir);
                        dir.y = 0.0f;
                        speed = VECMag(&dir);
                        if(speed < 0.05f) {
                            work->comAngle = 0.0f;
                            work->comMag = 0.0f;
                        }
                        else {
                            if(work->side) {
                                dir.x *= -1.0f;
                            }
                            work->comAngle = HuAtan(dir.x, dir.z);
                            work->comMag = 1.0f;
                        }
                    }
                    if(work->comWideF) {
                        if((fabsf(pos.z) > 1.1f) || (pos.x > 1.1f)) {
                            work->comMag = 0.0f;
                        }
                    } else {
                        if((fabsf(pos.z) > 0.95f) || (pos.x > 0.95f)) {
                            work->comMag = 0.0f;
                        }
                    }
                }
                work->stkAngle = work->comAngle;
                work->stkMag = work->comMag;
                if(work->comReadyServeF) {
                    if(work->comServeTime++ >= work->comServeMaxTime) {
                        work->btnDown |= PAD_BUTTON_A;
                    }
                } else {
                    work->comServeMaxTime = ((60.0f*(0.35f*work->comAccuracy))/1000.0f)*frandmod(1000);
                    work->comServeTime = 0;
                }
            }
            
            break;
    }
}

void M433ComMove(OMOBJ *obj)
{
    M433_PLAYERWORK *work = obj->data;
    M433_PLAYERWORK *other = work->obj->data;
    HuVecF pos;
    HuVecF dir;
    float mag, magOther;
    
    if(!other->comF) {
        other->comAccuracy = work->comAccuracy-0.1f;
    }
    if(M433BallSideGet(&pos) == work->side && M433BallAirTimeGet() >= 12.0f) {
        if(!other->comContactF && !work->comContactF || M433BallAirTimeGet() < 1000) {
            if(work->serveF && other->serveF) {
                return;
            }
            VECSubtract(&pos, &work->pos, &dir);
            dir.y = 0;
            mag = VECMag(&dir)-(100*(2*work->comAccuracy));
            VECSubtract(&pos, &other->pos, &dir);
            dir.y = 0;
            magOther = VECMag(&dir)-(100*(2*other->comAccuracy));
            if(work->serveF || work->ballSpikeF) {
                mag = 2000;
            }
            if(other->serveF || other->ballSpikeF) {
                magOther = 2000;
            }
            if(mag >= 300) {
                other->comServeCelF = FALSE;
            }
            if(magOther >= 300) {
                work->comServeCelF = FALSE;
            }
            if(work->comServeCelF) {
                mag += 300;
            }
            if(other->comServeCelF) {
                magOther += 300;
            }
            other->comContactF = work->comContactF = FALSE;
            if(mag <= magOther) {
                work->comContactF = TRUE;
                work->comServeCelF = FALSE;
            } else {
                other->comContactF = TRUE;
                other->comServeCelF = FALSE;
            }
            if(!work->comServeCelF) {
                work->comJumpMode = 0;
            }
            if(!other->comServeCelF) {
                other->comJumpMode = 0;
            }
        } else {
            if(M433ContactCountGet() == 1) {
                work->comContactF = M433ContactCheck(work->side, work->cel) == FALSE;
                other->comContactF = work->comContactF == FALSE;
            }
        }
        
    } else {
        if(M433ServeSideGet(&pos) != work->side) {
            other->comContactF = work->comContactF = FALSE;
        }
    }
}

void M433ComJumpStart(OMOBJ *obj)
{
    M433_PLAYERWORK *work = obj->data;
    BOOL forceJump = FALSE;
    work->comAllowServeF = FALSE;
    if(frandmod(1000) < 600*work->comAccuracy) {
        work->comAllowServeF = TRUE;
    }
    work->comServeCelF = FALSE;
    work->comJumpMode = 0;
    if(work->cel != M433ServeCelGet(work->side) && work->mode) {
        if(gameType == 1 && ballExplodeTimer == 5) {
            forceJump = TRUE;
        }
        if(frandmod(1000) < 300*work->comAccuracy || forceJump) {
            work->comServeCelF = TRUE;
            work->comJumpMaxTime = (((60.0f*(0.4f*work->comAccuracy))/1000.0f) * frandmod(1000))+6;
            work->comJumpTime = 0;
        }
    }
    if(work->cel == M433ServeCelGet(work->side)) {
        work->comTargetPos.z = 140;
        work->comTargetPos.x = 540.2f;
    } else {
        work->comTargetPos.z = -140;
        work->comTargetPos.x = 300.5f;
    }
    if(work->side == 0) {
         work->comTargetPos.x *= -1;
    }
}

BOOL M433ComTargetSet(unsigned int side, HuVecF *pos)
{
    HuVecF posTbl[2];
    HuVecF dir;
    HuVecF ofs;
    HuVecF maxOfs;
    unsigned int j;
    unsigned int i;
    float dist;
    float maxDist;
    float xOfs;
    float zOfs;
    for(j=0, i=0; i<GW_PLAYER_MAX && j<2; i++) {
        M433_PLAYERWORK *work = playerObj[i]->data;
        if(work->side != side) {
            posTbl[j] = work->pos;
            posTbl[j].y = 0;
            j++;
        }
    }
    if(j < 2) {
        return FALSE;
    }
    if(side == 1) {
        posTbl[0].x *= -1;
        posTbl[1].x *= -1;
    }
    xOfs = 47;
    zOfs = 70;
    maxDist = -1;
    ofs.z = -350;
    ofs.y = 0;
    for(i=0; i<=10; i++) {
        ofs.x = 230;
        for(j=0; j<=10; j++) {
            VECSubtract(&ofs, &posTbl[0], &dir);
            dist = VECMag(&dir);
            VECSubtract(&ofs, &posTbl[1], &dir);
            dist *= VECMag(&dir);
            if(dist > maxDist) {
                maxDist = dist;
                maxOfs = ofs;
            }
            ofs.x += xOfs;
        }
        ofs.z += zOfs;
    }
    *pos = maxOfs;
    pos->x -= 465.0f;
    pos->x *= 0.004255319f;
    pos->z *= 0.0028571428f;
    return TRUE;
}

void M433PlayerMotionUpdate(OMOBJ *obj)
{
    M433_PLAYERWORK *work = obj->data;
    BOOL updateRot = TRUE;
    float speed, minSpeed;
    HuVecF pos;
    HuVecF dir;
    if(!work->serveF && !work->ballSpikeF) {
        if(work->stkMag > 0 && !work->moveF) {
            work->rotYTarget = work->stkAngle;
        }
        if(work->stkMag > 0.05f && !work->moveF) {
            work->vel.x = (HuSin(work->stkAngle)*work->stkMag*550)*(1.0f/60.0f);
            work->vel.z = (HuCos(work->stkAngle)*work->stkMag*550)*(1.0f/60.0f);
            if(work->comF) {
                work->rotYSpeed = 0.1f;
            } else {
                work->rotYSpeed = 0.2f;
            }
            work->rotYAccel = 0;
            if(M433PlayerMotionShiftCheck(obj)) {
                minSpeed = 0.5f;
                if(work->motId == 2) {
                    minSpeed = 0.45f;
                }
                if(work->stkMag > minSpeed) {
                    M433PlayerMotionSet(obj, 2);
                    speed = work->stkMag;
                } else {
                    M433PlayerMotionSet(obj, 1);
                    speed = work->stkMag/minSpeed;
                }
                if(work->specialCharNo < 0) {
                    CharMotionSpeedSet(work->charNo, speed);
                } else {
                    Hu3DMotionSpeedSet(obj->mdlId[0], speed);
                }
            }
            if(work->stkMag > 0.5f && ((work->timer+15) & 0xF) == 0) {
                M433SandEffectAdd(&work->pos, 0.2f);
            }
        } else {
            work->vel.x = work->vel.y = work->vel.z = 0;
            if(work->jumpValidF && M433ContactCountGet() >= 2) {
                M433PlayerMotionSet(obj, 16);
            } else if(work->comFarServeF) {
                M433PlayerMotionSet(obj, 15);
            } else if(work->comNearServeF) {
                M433PlayerMotionSet(obj, 14);
            } else {
                M433PlayerMotionShiftNextSet(obj, 0);
            }
            if(updateRot && work->stkMag == 0) {
                M433BallPointCheck(0, &pos);
                VECSubtract(&pos, &work->pos, &dir);
                work->rotYTarget = HuAtan(dir.x, dir.z);
                work->rotYSpeed = 0.1f;
                work->rotYAccel = 0;
            }
        }
    }
}

void M433PlayerServe(OMOBJ *obj)
{
    HuVecF contactPos;
    HuVecF hookPos;
    M433_PLAYERWORK *work = obj->data;
    BOOL flag = work->comTimeServeF;
    int gameFx[3] = {
        MSM_SE_M433_03,
        MSM_SE_M433_04,
        MSM_SE_M433_05
    };
    static char *hookTbl[3] = {
        "itemhook_r",
        "itemhook_R",
        "itemhook_r"
    };
    char *hook;
    
    float gravity;
    float contactDir;
    
    work->comServeF = FALSE;
    work->comTimeServeF = FALSE;
    if(work->specialCharNo < 0) {
        hook = CharModelItemHookGet(work->charNo, CHAR_MODEL2, 0);
        Hu3DModelObjPosGet(obj->mdlId[0], hook, &hookPos);
    } else {
        Hu3DModelObjPosGet(obj->mdlId[0], hookTbl[work->specialCharNo], &hookPos);
    }
    switch(work->serveMode) {
        case 0:
            work->vel.x = work->vel.y = work->vel.z = 0;
            work->serveMode++;
            work->timer = 0;
        case 1:
            work->serveF = FALSE;
            work->serveValidF = TRUE;
            M433PlayerMotionSet(obj, 3);
            M433BallContactSet(&hookPos, 0, work->side, work->cel);
            if(M433EventGet(work->side) != 1) {
                work->timer = 0;
            } else if(!work->moveF) {
                work->comServeF = TRUE;
                if((work->btnDown & PAD_BUTTON_A) || work->timer > 300.0f) {
                    M433PlayerMotionSet(obj, 4);
                    work->serveF = TRUE;
                    work->serveMode++;
                    work->timer = 0;
                }
            }
            break;
        
        case 2:
            if(!work->ballSpikeF) {
                work->vel.x = (2*HuSin(10+(4.285714f*work->timer)))*1.6666667f;
                if(work->side) {
                    work->vel.x *= -1;
                }
            }
            if(work->timer >= 9.0f && work->serveValidF) {
                if(work->side) {
                    contactDir = -1;
                } else {
                    contactDir = 1;
                }
                contactPos.x = obj->trans.x+(250*contactDir);
                contactPos.y = obj->trans.y+80;
                contactPos.z = hookPos.z;
                M433BallContactSet(&contactPos, 1, work->side, work->cel);
                work->serveValidF = FALSE;
            } else if(work->timer >= 28.0f) {
                gravity = 0.5f;
                work->gravity = 2*(-(350-playerHeightTbl[work->charNo]))/(gravity*gravity);
                work->vel.y = (1.0f/60.0f)*(-work->gravity*gravity);
                work->ballSpikeF = TRUE;
                work->serveMode++;
                work->timer = 0;
                M433SandEffectAdd(&work->pos, 0.5f);
            }
            if(!work->serveValidF) {
                M433BallServeSet(work->stkAngle, work->stkMag, work->side, FALSE, &contactPos);
            } else {
                M433BallContactSet(&hookPos, 0, work->side, work->cel);
            }
            break;
        
        case 3:
            work->vel.y += (1.0f/60.0f)*((1.0f/60.0f)*work->gravity);
            if(work->timer > 27.0f && work->timer < 51.0f) {
                work->comTimeServeF = TRUE;
            }
            M433BallServeSet(work->stkAngle, work->stkMag, work->side, work->comTimeServeF, &contactPos);
            if(work->comTimeServeF && (work->btnDown & PAD_BUTTON_A) || (!work->comTimeServeF && flag)) {
                M433PlayerMotionSet(obj, 5);
                if(!work->comTimeServeF) {
                    M433BallTrailStart();
                }
                M433BallContactSet(&contactPos, 2, work->side, work->cel);
                work->comTimeServeF = FALSE;
                work->serveMode = 5;
                work->timer = 0;
                HuAudFXPlay(gameFx[gameType]);
            }
            break;
        
        case 4:
            if(work->timer >= 12.0f) {
                work->ballSpikeF = FALSE;
                work->serveMode = 1;
                work->timer = 0;
            }
            break;
        
        case 5:
            work->vel.y += (1.0f/60.0f)*((1.0f/60.0f)*work->gravity);
            if(work->pos.y <= 0.0f) {
                work->ballSpikeF = FALSE;
                M433SandEffectAdd(&work->pos, 0.5f);
                work->serveF = FALSE;
                work->vel.x = work->vel.y = work->vel.z = 0;
            }
            break;
    }
}

void M433PlayerBallCollide(OMOBJ *obj)
{
    HuVecF pos;
    HuVecF hookPos;
    HuVecF dir;
    float time;
    float dist;
    float angle;

    M433_PLAYERWORK *work = obj->data;
    int contactType[3][3] = { { 3, 4, 6 }, { 3, 4, 6 }, { 3, 4, 7 } };
    int seIdTbl[3] = { MSM_SE_M433_09, MSM_SE_M433_10, MSM_SE_M433_11 };
    int contactSeIdTbl[3] = { MSM_SE_M433_06, MSM_SE_M433_07, MSM_SE_M433_08 };
    M433_PLAYERWORK *other = work->obj->data;
    char *hookR;
    char *hookL;
    static char *hookRTbl[3] = {
        "itemhook_r",
        "itemhook_R",
        "itemhook_r"
    };
    static char *hookLTbl[3] = {
        "itemhook_l",
        "itemhook_L",
        "itemhook_l"
    };
    
    work->comFarServeF = FALSE;
    work->comNearServeF = FALSE;
    work->comMidServeF = FALSE;
    if (work->ballSpikeF == FALSE) {
        hookPos = work->pos;
        if(work->ballMode) {
            if(work->specialCharNo < 0) {
                hookR = CharModelItemHookGet(work->charNo, CHAR_MODEL2, 0);
                hookL = CharModelItemHookGet(work->charNo, CHAR_MODEL2, 1);
            } else {
                hookR = hookRTbl[work->specialCharNo];
                hookL = hookLTbl[work->specialCharNo];
            }
            Hu3DModelObjPosGet(obj->mdlId[0], hookR, &pos);
            Hu3DModelObjPosGet(obj->mdlId[0], hookL, &hookPos);
            VECAdd(&pos, &hookPos, &hookPos);
            VECScale(&hookPos, &hookPos, 0.5f);
        }
        switch (work->ballMode) {
            case 0:
                if ((work->ballMode == 0) && (work->spikeMode == 0) && (work->jumpMode == 0)) {
                    work->serveF = FALSE;
                    work->landF = FALSE;
                    if(work->moveF == FALSE && work->side == M433ServeSideGet(&pos) && M433ContactCheck(work->side, work->cel) && M433ContactCountGet() < 3) {
                        work->targetPos = pos;
                        time = M433ServeTargetTimeGet();
                        if ((time <= 0.8f) && (time >= 0.02f) && (other->landF == FALSE)) {
                            VECSubtract(&pos, &work->pos, &dir);
                            dir.y = 0.0f;
                            dist = VECMag(&dir);
                            work->distLaunch = dist;
                            if (dist <= 200.0f) {
                                if (time >= 0.2f && dist <= 70.0f && time >= (0.2f + (0.001f * dist)) && M433ContactCountGet() >= 1) {
                                    work->comFarServeF = TRUE;
                                }
                                if ((time >= 0.02f) && (dist <= 100.0f) && (time >= (0.02f + (0.001f * dist)))) {
                                    work->comNearServeF = TRUE;
                                }
                                if ((time >= 0.1f) && (time <= 0.3f) && (dist > 100.0f) && (dist <= 200.0f) && (time >= (0.1f+(0.0002f*(dist-100.0f))))) {
                                    work->comMidServeF = TRUE;
                                }
                            }
                        }
                        if (work->btnDown & PAD_BUTTON_A) {
                            work->vel.x = work->vel.y = work->vel.z = 0.0f;
                            if (work->comFarServeF != 0) {
                                work->ballMode = 1;
                                time -= 0.12f;
                                work->distLaunch *= 0.014285714f;
                                M433PlayerMotionSet(obj, 15);
                                M433BallPointCheck(0.0f, &pos);
                            }
                            else if (work->comNearServeF != 0) {
                                work->ballMode = 2;
                                time -= 0.03f;
                                work->distLaunch *= 0.01f;
                                M433PlayerMotionSet(obj, 14);
                                M433BallPointCheck(0.0f, &pos);
                            }
                            else if (work->comMidServeF != 0) {
                                work->ballMode = 3;
                                time -= 0.03f;
                                work->distLaunch *= 0.005f;
                                M433PlayerMotionSet(obj, 7);
                                M433SandEffectAdd(&work->pos, 0.75f);
                                HuAudFXPlay(MSM_SE_M433_15);
                            }
                            else {
                                if (M433ContactCountGet() >= 1) {
                                    M433PlayerMotionSet(obj, 9);
                                }
                                else {
                                    M433PlayerMotionSet(obj, 6);
                                }
                                work->ballMode = 4;
                            }
                            if (work->ballMode < 4) {
                                work->landF = TRUE;
                                if (time < 0.0f) {
                                    time = 0.0f;
                                }
                                work->maxTime = 60.0f * time;
                            }
                            VECSubtract(&pos, &work->pos, &dir);
                            work->rotYTarget = HuAtan(dir.x, dir.z);
                            work->rotYSpeed = 0.2f;
                            work->rotYAccel = 0.0f;
                            work->serveF = TRUE;
                            work->timer = 0;
                        }
                    }
                }
                break;

            case 1:
                if (work->timer >= work->maxTime) {
                    if (work->landF) {
                        if (M433ContactCountGet() >= 2) {
                            pos.x = 350.0f;
                            pos.y = 11.0f;
                            pos.z = 0.0f;
                            if (work->side) {
                                pos.x *= -1.0f;
                            }
                            M433BallTrailStart();
                        }
                        else {
                            pos = other->pos;
                            pos.y = 11.0f;
                            pos.x *= 0.5f;
                        }
                        angle = frandmod(360);
                        dist = (M433BallTrailGet()-1)*3;
                        if(dist < 0) {
                            dist = 0;
                        }
                        dist = 200.0f * (work->distLaunch+dist);
                        pos.x += dist * HuSin(angle);
                        pos.z += dist * HuCos(angle);
                        if (M433ContactCountGet() < 2) {
                            M433StageCollide(&pos, work->side);
                        }
                        M433BallContactSet(&pos, contactType[0][M433ContactCountGet()], work->side, work->cel);
                        M433SandEffectAdd(&work->pos, 0.0f);
                        HuAudFXPlay(contactSeIdTbl[gameType]);
                    }
                    work->landF = FALSE;
                    M433PlayerMotionSet(obj, 9);
                    VECSubtract(&pos, &work->pos, &dir);
                    work->rotYTarget = HuAtan(dir.x, dir.z);
                    work->rotYSpeed = 0.1f;
                    work->maxTime = 100000;
                    work->timer = 0;
                    break;
                }
                if (work->maxTime < 10000) {
                    if(M433BallSlipCheck(0, &hookPos)) {
                        work->maxTime = work->timer;
                    }
                    work->vel.x = 0.07f * (work->targetPos.x - work->pos.x);
                    work->vel.z = 0.07f * (work->targetPos.z - work->pos.z);
                    if (work->landF == FALSE) {
                        work->maxTime = 0;
                    }
                }
                else {
                    work->vel.x *= 0.7f;
                    work->vel.z *= 0.7f;
                    if (M433PlayerMotionCheck(obj)) {
                        work->ballMode = 0;
                    }
                }
                break;
            case 2:
                if (work->timer >= work->maxTime) {
                    if (work->landF) {
                        switch (M433ContactCountGet()) {
                            case 0:
                                pos = other->pos;
                                pos.y = 11.0f;
                                break;
                                
                            case 1:
                                pos = other->pos;
                                pos.y = 11.0f;
                                pos.x *= 0.5f;
                                break;
                                
                            case 2:
                            default:
                                pos.x = 350.0f;
                                pos.y = 11.0f;
                                pos.z = 0.0f;
                                if (work->side) {
                                    pos.x *= -1.0f;
                                }
                                M433BallTrailStart();
                                break;
                        }
                        angle = frandmod(360);
                        dist = (M433BallTrailGet()-1)*3;
                        if(dist < 0) {
                            dist = 0;
                        }
                        dist = 200.0f * (work->distLaunch+dist);
                        pos.x += dist * HuSin(angle);
                        pos.z += dist * HuCos(angle);
                        if (M433ContactCountGet() < 2) {
                            M433StageCollide(&pos, work->side);
                        }
                        M433BallContactSet(&pos, contactType[1][M433ContactCountGet()], work->side, work->cel);
                        M433SandEffectAdd(&work->pos, 0.0f);
                        if (M433ContactCountGet() < 1) {
                            HuAudFXPlay(seIdTbl[gameType]);
                        }
                        else {
                            HuAudFXPlay(contactSeIdTbl[gameType]);
                        }
                    }
                    work->landF = FALSE;
                    M433PlayerMotionSet(obj, 6);
                    VECSubtract(&pos, &work->pos, &dir);
                    work->rotYTarget = HuAtan(dir.x, dir.z);
                    work->rotYSpeed = 0.1f;
                    work->maxTime = 100000;
                    work->timer = 0;
                    break;
                }
                if (work->maxTime < 10000) {
                    if(M433BallSlipCheck(0, &hookPos)) {
                        work->maxTime = work->timer;
                    }
                    work->vel.x = 0.07f * (work->targetPos.x - work->pos.x);
                    work->vel.z = 0.07f * (work->targetPos.z - work->pos.z);
                    if (work->landF == FALSE) {
                        work->maxTime = 0;
                        break;
                    }
                }
                else {
                    work->vel.x *= 0.7f;
                    work->vel.z *= 0.7f;
                    if (M433PlayerMotionCheck(obj)) {
                        work->ballMode = 0;
                        break;
                    }
                }
                break;

            case 3:
                if (work->timer >= work->maxTime) {
                    if (work->landF) {
                        pos = work->pos;
                        pos.y = 11.0f;
                        VECSubtract(&other->pos, &work->pos, &dir);
                        angle = 180.0 + (HuAtan(dir.x, dir.z));
                        angle += frandmod(180) - 90.0f;
                        dist = (M433BallTrailGet()-1)*3;
                        if(dist < 0) {
                            dist = 0;
                        }
                        dist = 200.0f * (work->distLaunch+dist);
                        pos.x += dist * HuSin(angle);
                        pos.z += dist * HuCos(angle);
                        M433StageCollide(&pos, work->side);
                        M433BallContactSet(&pos, contactType[2][M433ContactCountGet()], work->side, work->cel);
                        if (M433ContactCountGet() < 1) {
                            HuAudFXPlay(seIdTbl[gameType]);
                        }
                        else {
                            HuAudFXPlay(contactSeIdTbl[gameType]);
                        }
                    }
                    work->landF = FALSE;
                    work->maxTime = 100000;
                    work->timer = 0;
                    M433SandEffectAdd(&work->pos, 1.3f);
                    break;
                }
                if (work->maxTime < 10000) {
                    work->vel.x = 0.15f * (work->targetPos.x - work->pos.x);
                    work->vel.z = 0.15f * (work->targetPos.z - work->pos.z);
                    break;
                }
                work->vel.x *= 0.95f;
                work->vel.z *= 0.95f;
                if (M433PlayerMotionCheck(obj)) {
                    M433PlayerMotionSet(obj, 8);
                    work->ballMode = 4;
                }
                break;
                
            case 4:
                work->vel.x *= 0.95f;
                work->vel.z *= 0.95f;
                if (M433PlayerMotionCheck(obj)) {
                    work->ballMode = 0;
                }
                break;
        }
    }
}

void M433PlayerJump(OMOBJ *obj)
{
    Vec pos;
    Vec dir;
    float time;
    float gravity;
    M433_PLAYERWORK *work = obj->data;
    int seIdTbl[3] = { MSM_SE_M433_12, MSM_SE_M433_13, MSM_SE_M433_14 };
    M433_PLAYERWORK *other = work->obj->data;
    work->comReadyServeF = FALSE;
    work->jumpValidF = FALSE;
    switch (work->jumpMode) {
        case 0:
            if (work->ballMode == 0 && work->spikeMode == 0 && work->jumpMode == 0) {
                work->serveF = FALSE;
                work->jumpF = FALSE;
                if(work->moveF == FALSE && M433ContactCheck(work->side, work->cel) && M433ContactCountGet() < 3 && work->side == M433ServeSideGet(&pos)) {
                    VECSubtract(&pos, &work->pos, &dir);
                    dir.y = 0.0f;
                    if (VECMag(&dir) > 500.0f) {
                        break;
                    }
                    M433BallPointCheck(0.5f, &pos);
                    pos.y -= playerHeightTbl[work->charNo];
                    VECSubtract(&pos, &work->pos, &dir);
                    dir.y = 0.0f;
                    if (VECMag(&dir) < 200.0f && pos.y >= 50.0f) {
                        work->jumpValidF = TRUE;
                    }
                    if ((work->btnDown & PAD_BUTTON_B) && work->jumpValidF) {
                        work->vel.x = work->vel.y = work->vel.z = 0.0f;
                        M433PlayerMotionSet(obj, 10);
                        work->jumpMode = 1;
                        work->timer = 0;
                        work->serveF = TRUE;
                        if (work->jumpValidF) {
                            gravity = 0.5f;
                            work->gravity = (2.0f * -pos.y) / (gravity * gravity);
                            work->vel.y = (1.0f/60.0f) * (-work->gravity * gravity);
                            VECSubtract(&pos, &work->pos, &dir);
                            dir.y = 0.0f;
                            work->vel.x = (1.0f/60.0f) * (dir.x / 0.5f);
                            work->vel.z = (1.0f/60.0f) * (dir.z / 0.5f);
                            work->rotYTarget = (work->side) ? -90.0f : 90.0f;
                            work->rotYSpeed = 0.2f;
                            work->rotYAccel = 0.0f;
                            work->jumpF = TRUE;
                        }
                        else {
                            work->vel.y = 14.833334f;
                        }
                        work->ballSpikeF = TRUE;
                        M433SandEffectAdd(&work->pos, 0.75f);
                    }
                }
            }
            break;
        case 1:
            work->vel.y += (1.0f/60.0f) * ((1.0f/60.0f) * work->gravity);
            if (work->jumpF != 0) {
                time = ((1.0f/60.0f) * work->timer) - 0.35f;
                if (time >= 0.0f && time < 0.4) {
                    work->comReadyServeF = TRUE;
                    if (work->btnDown & PAD_BUTTON_A) {
                        M433PlayerMotionSet(obj, 11);
                        M433BallServeSet(work->stkAngle, work->stkMag, work->side, work->comReadyServeF, &pos);
                        M433BallContactSet(&pos, 5, work->side, work->cel);
                        work->jumpF = FALSE;
                        work->comReadyServeF = FALSE;
                        HuAudFXPlay(seIdTbl[gameType]);
                    }
                }
                else if (time > 0.0f) {
                    work->jumpF = FALSE;
                    work->comReadyServeF = FALSE;
                }
                M433BallServeSet(work->stkAngle, work->stkMag, work->side, work->comReadyServeF, &pos);
            }
            if (work->pos.y <= 0.0f) {
                M433SandEffectAdd(&work->pos, 0.75f);
                work->serveF = FALSE;
                work->jumpMode = 0;
                work->vel.x = work->vel.y = work->vel.z = 0.0f;
                work->ballSpikeF = FALSE;
                work->jumpF = FALSE;
            }
            break;
    }
}

void M433PlayerSpike(OMOBJ *obj)
{
    HuVecF pos;
    HuVecF dir;
    M433_PLAYERWORK *work = obj->data;
    BOOL jumpF = FALSE;
    int seIdTbl[3] = { MSM_SE_M433_12, MSM_SE_M433_13, MSM_SE_M433_14 };
    int seIdTbl2[3] = { MSM_SE_M433_09, MSM_SE_M433_10, MSM_SE_M433_11 };
    M433_PLAYERWORK *other = work->obj->data;
    work->nearBallF = FALSE;
    switch(work->spikeMode) {
        case 0:
            if ((work->ballMode == 0) && (work->spikeMode == 0) && (work->jumpMode == 0)) {
                work->serveF = FALSE;
                work->jumpF = FALSE;
                if(!work->moveF && M433BallSideGet(&pos) == work->side && !M433ContactGroupCheck(work->side)) {
                    float x = work->pos.x;
                    if(work->side == 0) {
                        x *= -1;
                    }
                    if(x > 230) {
                        break;
                    }
                    if(M433GameEventGet() != 2) {
                        if(M433BallJumpCheck(&pos)) {
                            jumpF = TRUE;
                        }
                        if(jumpF) {
                            x = pos.z-work->pos.z;
                            if(x > 100) {
                                x = 100;
                            } else if(x < -100) {
                                x = -100;
                            } else {
                                work->nearBallF = TRUE;
                            }
                        }
                        if(work->btnDown & PAD_BUTTON_B) {
                            float scale;
                            work->vel.x = work->vel.y = work->vel.z = 0;
                            M433PlayerMotionSet(obj, 10);
                            work->spikeMode = 1;
                            work->timer = 0;
                            work->serveF = TRUE;
                            work->rotYTarget = (work->side) ? -90.0f : 90.0f;
                            work->rotYSpeed = 0.2f;
                            work->rotYAccel = 0;
                            M433SandEffectAdd(&work->pos, 0.75f);
                            scale = 0.5f;
                            work->gravity = (2.0f*-(350-playerHeightTbl[work->charNo]))/(scale*scale);
                            work->vel.y = (1.0f/60.0f)*(-work->gravity*scale);
                            work->vel.x = (1.0f/60.0f)*(-work->pos.x/0.75f);
                            if(jumpF) {
                                work->vel.z = (1.0f/60.0f)*(x/0.5f);
                            }
                            work->ballSpikeF = TRUE;
                            work->nearBallF = FALSE;
                        }
                    }
                }
            }
            break;
        
        case 1:
            work->vel.y += (1.0f/60.0f)*((1.0f/60.0f)*work->gravity);
            if(M433ServeSideGet(&pos) == work->side && M433ContactCountGet() == 0) {
                M433BallPointCheck(0, &pos);
                if(pos.x*work->pos.x > 0 && pos.y > 100) {
                    pos.y = work->pos.y;
                    VECSubtract(&pos, &work->pos, &dir);
                    if(VECMag(&dir) < 60.000004f) {
                        if(frandmod(100) < 30) {
                            M433ServeSideGet(&pos);
                            pos.x *= -0.5f;
                            M433BallContactSet(&pos, 5, work->side, work->cel);
                            HuAudFXPlay(seIdTbl[gameType]);
                        } else {
                            float angle;
                            M433ServeSideGet(&pos);
                            angle = frandmod(360);
                            pos.x += (2*HuSin(angle))*100;
                            pos.z += (2*HuCos(angle))*100;
                            M433StageCollide(&pos, work->side);
                            M433BallContactSet(&pos, 8, work->side, work->cel);
                            HuAudFXPlay(seIdTbl2[gameType]);
                        }
                        M433PlayerMotionSet(obj, 13);
                        other->landF = FALSE;
                    }
                }
            }
            if(work->pos.y <= 0) {
                M433SandEffectAdd(&work->pos, 0.75f);
                work->serveF = FALSE;
                work->spikeMode = 0;
                work->vel.x = work->vel.y = work->vel.z = 0;
                work->ballSpikeF = FALSE;
            }
            break;
    }
}

void M433PlayerShock(OMOBJ *obj)
{
    M433_PLAYERWORK *work = obj->data;
    HuVecF pos;
    HuVecF dir;
    float dist;
    float scale;
    
    work->serveF = FALSE;
    work->moveF = FALSE;
    work->serveValidF = FALSE;
    work->comServeF = FALSE;
    work->comTimeServeF = FALSE;
    work->comNearServeF = FALSE;
    work->comMidServeF = FALSE;
    work->comFarServeF = FALSE;
    work->unk50 = FALSE;
    work->comReadyServeF = FALSE;
    work->nearBallF = FALSE;
    work->jumpValidF = FALSE;
    work->landF = FALSE;
    work->jumpF = FALSE;
    switch(work->serveMode) {
        case 0:
            work->vel.x = work->vel.y = work->vel.z = 0;
            M433BallPointCheck(0, &pos);
            VECSubtract(&work->pos, &pos, &dir);
            if(fabsf(dir.y) < 0.01f) {
                dir.y = 0.01f;
            }
            dist = VECMag(&dir);
            if(dist < 50) {
                dist = 50;
            }
            if(dist > 300) {
                dist = 300;
            }
            dist *= 0.0025f;
            dist = 1-dist;
            dir.y = 0;
            if(dir.x == 0.0f && dir.z == 0.0f) {
                dir.x = 1.0f;
            }
            VECNormalize(&dir, &dir);
            work->rotYTarget = work->rotY = HuAtan(-dir.x, -dir.z);
            scale = 0.4f;
            work->gravity = -3920.0f;
            work->vel.y = (-work->pos.y/scale)-(0.5f*work->gravity*scale);
            work->gravity *= 0.0002777778f;
            work->vel.y *= (1.0f/60.0f);
            work->shockTimer = 60*scale;
            work->vel.x = (100*(dir.x*dist*4))/work->shockTimer;
            work->vel.z = (100*(dir.z*dist*4))/work->shockTimer;
            work->ballSpikeF = TRUE;
            M433PlayerMotionSet(obj, 19);
            work->serveF = FALSE;
            omVibrate(obj->work[0], 12, 4, 2);
            work->serveMode++;
            
        case 1:
            work->vel.y += work->gravity;
            work->shockTimer--;
            if(work->shockTimer >= 0) {
                break;
            }
            work->ballSpikeF = FALSE;
            work->vel.x = work->vel.y = work->vel.z = 0;
            M433PlayerMotionSet(obj, 17);
            work->serveMode++;
            break;
        
        case 2:
            break;
    }
}

void M433PlayerTargetUpdate(OMOBJ *obj)
{
    M433_PLAYERWORK *work;
    M433_PLAYERWORK *other;
    int i, j;
    int side;
    float dist;
    HuVecF dir;
    
    for(side=0; side<2; side++) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            work = playerObj[i]->data;
            for(j=0; j<GW_PLAYER_MAX; j++) {
                if(i == j) {
                    continue;
                }
                other = playerObj[j]->data;
                VECSubtract(&work->pos, &other->pos, &dir);
                dir.y = 0;
                dist = VECMag(&dir);
                if(dist < work->radius+other->radius) {
                    if(dist > 0) {
                        dist = 0.5f*(work->radius+other->radius-dist);
                    } else {
                        dist = 0.5f*(work->radius+other->radius);
                        dir.x = frandmod(1000)-500;
                        dir.z = frandmod(1000)-500;
                    }
                    VECNormalize(&dir, &dir);
                    VECScale(&dir, &dir, dist);
                    VECAdd(&work->pos, &dir, &work->pos);
                    VECSubtract(&other->pos, &dir, &other->pos);
                }
            }
            if(work->pos.y < 0) {
                work->pos.y = 0;
            }
            if(work->pos.z-work->radius < -500) {
                work->pos.z = -500+work->radius;
            } else if(work->pos.z+work->radius > 500) {
                work->pos.z = 500-work->radius;
            }
            if(work->side == 0) {
                work->pos.x *= -1;
            }
            if(work->pos.x-work->radius < 20) {
                work->pos.x = 20+work->radius;
            } else if(work->pos.x+60.000004f > 800) {
                work->pos.x = 740;
            }
            if(work->side == 0) {
                work->pos.x *= -1;
            }
        }
    }
}

void M433PlayerMotionSet(OMOBJ *obj, unsigned int motId)
{
    M433_PLAYERWORK *work = obj->data;
    if(work->motId != motId && motId < 24) {
        float shift = 60*playerMotData[motId].shift;
        if(work->motId < 0) {
            shift = 0;
        }
        work->motId = motId;
        if(work->specialCharNo < 0) {
            CharMotionShiftSet(work->charNo, obj->mtnId[playerMotData[motId].index], 60*playerMotData[motId].shiftStart, shift, playerMotData[motId].attr);
        } else {
            Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[playerMotData[motId].index], 60*playerMotData[motId].shiftStart, shift, playerMotData[motId].attr);
        }
        if(playerMotData[motId].shiftEnd >= 0) {
            Hu3DMotionShiftStartEndSet(obj->mdlId[0], 60*playerMotData[motId].shiftStart, 60*playerMotData[motId].shiftEnd);
        }
        work->motCancelF = FALSE;
    }
}

BOOL M433PlayerMotionCheck(OMOBJ *obj)
{
    M433_PLAYERWORK *work = obj->data;
    BOOL result = FALSE;
    if(work->specialCharNo < 0) {
        if(CharMotionEndCheck(work->charNo) || work->motCancelF) {
            if(CharMotionShiftIDGet(work->charNo) < 0) {
                result = TRUE;
            }
        }
    } else {
        if(Hu3DMotionEndCheck(obj->mdlId[0]) || work->motCancelF) {
            if(Hu3DMotionShiftIDGet(obj->mdlId[0]) < 0) {
                result = TRUE;
            }
        }
    }
    return result;
}

BOOL M433PlayerMotionShiftCheck(OMOBJ *obj)
{
    M433_PLAYERWORK *work = obj->data;
    BOOL result = FALSE;
    if(work->specialCharNo < 0) {
        if(CharMotionShiftIDGet(work->charNo) < 0) {
            result = TRUE;
        }
    } else {
        if(Hu3DMotionShiftIDGet(obj->mdlId[0]) < 0) {
            result = TRUE;
        }
    }
    return result;
}

BOOL M433PlayerMotionNextSet(OMOBJ *obj, unsigned int motId)
{
    BOOL result = FALSE;
    if(M433PlayerMotionCheck(obj)) {
        M433PlayerMotionSet(obj, motId);
        result = TRUE;
    }
    return result;
}

BOOL M433PlayerMotionShiftNextSet(OMOBJ *obj, unsigned int motId)
{
    BOOL result = FALSE;
    if(M433PlayerMotionShiftCheck(obj)) {
        M433PlayerMotionSet(obj, motId);
        result = TRUE;
    }
    return result;
}

typedef struct M433BallTarget_s {
    float radiusMax;
    float radius;
    float scale;
    float maxY;
    float dist;
    float colRadius;
    float colTimeRad;
    float dy;
    float colHeight;
    float colTime;
    float speed;
    HuVecF pos;
    HuVecF posNext;
} M433_BALLTARGET;

typedef struct M433BallWork_s {
    BOOL updateF;
    BOOL killF;
    BOOL ballGroundF;
    BOOL contactF;
    BOOL targetUpdateF;
    BOOL unk14;
    BOOL fire;
    u16 mode;
    char unk1E[2];
    s16 diceUpdateNum;
    int updateTime;
    float diceRot;
    char unk2C[12];
    HuVecF pos;
    HuVecF posNext;
    float ballDist;
    HuVecF ballDir;
    char unk60[20];
    unsigned int type;
    unsigned int contactGrp;
    unsigned int contactCel;
    unsigned int targetTimer;
    float cursorBScale[2];
    float cursorBScaleTarget[2];
    int timer;
    unsigned int cursorAF;
    unsigned int cursorMode;
    unsigned int airTime;
    float cursorAScale;
    float cursorAScaleTarget;
    float cursorARot;
    float trail;
    HuVecF cursorAPos;
    float ballRot;
    HuVecF ballRotAxis;
    Mtx ballMtx;
    float ballSlip;
    float ballScale;
    float ballScaleNew;
    M433_BALLTARGET target;
} M433_BALLWORK;

typedef struct M433ScoreSpr_s {
    s16 num;
    s16 sprId[5];
} M433_SCORESPR;

typedef struct M433ScoreOvlSpr_s {
    s16 sprId[2];
    float sprOfs[2];
    float posX;
    float posY;
    float velX;
    float velY;
    float scaleX;
    float scaleY;
    float tpLvl;
    float zRot;
} M433_SCOREOVL_SPR;

typedef struct M433ScoreSprNum_s {
    int sprId[4];
} M433_SCORESPRNUM;

typedef struct M433ScoreOvl_s {
    int mode;
    unsigned int titleTime;
    unsigned int time;
    unsigned int moveTime;
    unsigned int scorePrev[2];
    unsigned int score[2];
    unsigned int scoreValuePrev;
    unsigned int scoreValue;
    unsigned int winGrp;
    M433_SCOREOVL_SPR scoreOvlSpr[4];
    M433_SCORESPRNUM digitSprId[3][20];
    M433_SCORESPRNUM dashSprId;
} M433_SCOREOVL;

typedef struct M433GameMan_s {
    unsigned int uGameEvent;
    int sWinGrp;
    unsigned int uServeGrp;
    unsigned int uPointGrp;
    unsigned int auServeCel[2];
    unsigned int auEvents[2];
    BOOL abGameWin[2];
    unsigned int auScore[2];
    unsigned int uTime;
    BOOL bPointWin;
    BOOL bContact;
    unsigned int uContactCnt;
    int sGroundGrp;
    unsigned int uContactGrp;
    unsigned int uContactCel;
    unsigned int uTargetGrp;
    unsigned int uGuideTimer;
    M433_SCORESPR scoreSpr[2];
    M433_SCOREOVL scoreOvl;
} M433_GAMEMAN;

void M433SandEffectHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx);

void M433DustEffectHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx);
void M433NumberEffectHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx);

void M433BallMain(OMOBJ *obj);

void M433BallCreate(OMOBJ *obj)
{
    int ballFileTbl[3] = {
        M433_HSF_ball_verydark,
        M433_HSF_bomb_ball,
        M433_HSF_saikoro_ball_dark
    };
    float ballScaleTbl[3] = {
        1.05f,
        2.0f,
        2.0f
    };
    M433_BALLWORK *ball;
    HU3DPARTICLEDATA *effData;
    int i;
    int effMdlId;
    
    obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M433_BALLWORK), HU_MEMNUM_OVL);
    ball = obj->data;
    memset(ball, 0, sizeof(M433_BALLWORK));
    ball->ballScale = 1;
    ball->ballScaleNew = ballScaleTbl[gameType];
    ball->ballSlip = ball->ballScaleNew*30.0;
    obj->mdlId[0] = Hu3DModelCreateData(M433_HSF_ball_normal);
    Hu3DModelShadowSet(obj->mdlId[0]);
    obj->mdlId[1] = Hu3DModelCreateData(M433_HSF_cursor_b);
    Hu3DModelAttrSet(obj->mdlId[1], HU3D_MOTATTR_LOOP);
    obj->mdlId[2] = Hu3DModelCreateData(M433_HSF_cursor_b);
    Hu3DModelAttrSet(obj->mdlId[2], HU3D_MOTATTR_LOOP);
    obj->mdlId[3] = Hu3DModelCreateData(M433_HSF_cursor_a);
    Hu3DModelDispOff(obj->mdlId[3]);
    obj->mdlId[4] = Hu3DModelCreateData(M433_HSF_effect02);
    Hu3DModelDispOff(obj->mdlId[4]);
    obj->mdlId[6] = Hu3DModelCreateData(M433_HSF_effect00);
    Hu3DModelDispOff(obj->mdlId[6]);
    Hu3DModelScaleSet(obj->mdlId[6], 0.75f, 0.75f, 0.75f);
    Hu3DModelLayerSet(obj->mdlId[6], 2);
    obj->mdlId[7] = Hu3DModelCreateData(M433_HSF_effect01);
    Hu3DModelDispOff(obj->mdlId[7]);
    Hu3DModelScaleSet(obj->mdlId[7], 0.75f, 0.75f, 0.75f);
    Hu3DModelLayerSet(obj->mdlId[7], 2);
    obj->mdlId[8] = effMdlId = Hu3DParticleCreate(HuSprAnimRead(HuDataReadNum(EFFECT_ANM_dust, HU_MEMNUM_OVL)), 1);
    Hu3DModelLayerSet(effMdlId, 2);
    Hu3DParticleHookSet(effMdlId, M433DustEffectHook);
    Hu3DModelDispOff(effMdlId);
    obj->mdlId[9] = Hu3DModelCreateData(M433_HSF_fire);
    Hu3DModelDispOff(obj->mdlId[9]);
    Hu3DModelLayerSet(obj->mdlId[9], 2);
    obj->mdlId[10] = effMdlId = Hu3DParticleCreate(HuSprAnimRead(HuDataSelHeapReadNum(M433_ANM_bomb_num, HU_MEMNUM_OVL, HEAP_MODEL)), 1);
    Hu3DModelLayerSet(effMdlId, 2);
    Hu3DParticleHookSet(effMdlId, M433NumberEffectHook);
    Hu3DParticleAnimModeSet(effMdlId, 10);
    Hu3DModelDispOff(effMdlId);
    effData = ((HU3DPARTICLE *)Hu3DData[effMdlId].hookData)->data;
    effData->pos.x = 0;
    effData->pos.y = 400;
    effData->pos.z = -490;
    effData->scale = 150;
    effData->color.r = effData->color.g = effData->color.b = effData->color.a = 255;
    effData->time = 1;
    obj->mdlId[5] = effMdlId = Hu3DParticleCreate(HuSprAnimRead(HuDataSelHeapReadNum(M433_ANM_sand, HU_MEMNUM_OVL, HEAP_MODEL)), 200);
    Hu3DModelLayerSet(effMdlId, 2);
    Hu3DParticleHookSet(effMdlId, M433SandEffectHook);
    effData = ((HU3DPARTICLE *)Hu3DData[effMdlId].hookData)->data;
    for(i=0; i<200; i++, effData++) {
        effData->scale = 0;
        effData->color.a = 0;
        effData->time = 0;
    }
    obj->mdlId[11] = Hu3DModelCreateData(M433_HSF_saikoro_ball);
    Hu3DModelDispOff(obj->mdlId[9]);
    Hu3DModelLayerSet(obj->mdlId[9], 2);
    obj->mdlId[12] = Hu3DModelCreateData(ballFileTbl[gameType]);
    Hu3DModelScaleSet(obj->mdlId[12], ball->ballScale, ball->ballScale, ball->ballScale);
    Hu3DModelAttrSet(obj->mdlId[12], HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(obj->mdlId[12], 0);
    Hu3DModelLayerSet(obj->mdlId[12], 1);
    Hu3DModelShadowSet(obj->mdlId[12]);
    for(i=1; i<19; i++) {
        if(i == 9 && gameType == 0) {
            obj->mdlId[i+12] = Hu3DModelCreateData(M433_HSF_ball_dark);
        } else {
            obj->mdlId[i+12] = Hu3DModelLink(obj->mdlId[12]);
            Hu3DModelLayerSet(obj->mdlId[i+12], 1);
        }
        Hu3DModelShadowSet(obj->mdlId[i+12]);
        Hu3DModelScaleSet(obj->mdlId[i+12], ball->ballScale, ball->ballScale, ball->ballScale);
        Hu3DModelAttrSet(obj->mdlId[i+12], HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(obj->mdlId[i+12], 0);
        if(i == 9) {
            Hu3DModelLayerSet(obj->mdlId[i+12], 2);
        }
    }
    obj->trans.x = ball->pos.x = 0;
    obj->trans.y = ball->pos.y = 0;
    obj->trans.z = ball->pos.z = 0;
    
    ball->ballRot = 0;
    ball->ballRotAxis.x = ball->ballRotAxis.y = 0;
    ball->ballRotAxis.z = 1;
    MTXIdentity(ball->ballMtx);
    ball->mode = 0;
    obj->objFunc = M433BallMain;
}

void M433BallKill(OMOBJ *obj);
BOOL M433BallUpdate(OMOBJ *obj);
void M433BallModelUpdate(OMOBJ *obj);
void M433BallDiceUpdate(OMOBJ *obj);

void M433BallMain(OMOBJ *obj)
{
    HuVecF pos;
    M433_BALLWORK *ball = obj->data;
    int temp = 0;
    int i;
    ball->updateTime++;
    ball->updateF = FALSE;
    switch(ball->mode) {
        case 0:
            break;
            
        case 1:
            ball->ballRot = 0;
            ball->ballRotAxis.x = ball->ballRotAxis.y = 0;
            ball->ballRotAxis.z = 1;
            MTXIdentity(ball->ballMtx);
            ball->updateF = TRUE;
            ball->ballScale = 1;
            break;
        
        case 11:
            if(ball->fire) {
                HuAudFXPlay(MSM_SE_M433_17);
                Hu3DModelDispOn(obj->mdlId[9]);
                Hu3DMotionTimeSet(obj->mdlId[9], 0);
                pos.x = obj->trans.x;
                pos.y = obj->trans.y-75;
                pos.z = obj->trans.z;
                if(pos.y < 0) {
                    pos.y = 0;
                }
                Hu3DModelPosSetV(obj->mdlId[9], &pos);
                ball->cursorBScaleTarget[0] = ball->cursorBScaleTarget[1] = 0;
            }
            ball->fire = FALSE;
            ball->updateF = TRUE;
            ball->killF = TRUE;
            break;
    }
    ball->timer++;
    if(ball->cursorAF) {
        ball->cursorAScaleTarget = 1;
        if(ball->cursorMode == 2) {
            ball->cursorARot += 12.000001f;
            if(ball->cursorARot >= 360) {
                ball->cursorARot -= 360;
            }
            ball->cursorAScaleTarget = 1.5f;
            ball->trail += 0.015000001f;
            Hu3DMotionSpeedSet(obj->mdlId[3], 2.5f);
        } else {
            Hu3DMotionTimeSet(obj->mdlId[3], 0);
            Hu3DMotionSpeedSet(obj->mdlId[3], 0);
        }
        Hu3DModelRotSet(obj->mdlId[3], 0, ball->cursorARot, 0);
        
        Hu3DModelDispOn(obj->mdlId[3]);
        Hu3DModelPosSet(obj->mdlId[3], ball->cursorAPos.x, ball->cursorAPos.y+3, ball->cursorAPos.z);
        ball->airTime++;
        ball->cursorAF = FALSE;
    } else if(ball->cursorMode) {
        ball->cursorMode = 0;
        ball->cursorAScaleTarget = 0;
    }
    ball->cursorAScale += 0.2f*(ball->cursorAScaleTarget-ball->cursorAScale);
    Hu3DModelScaleSet(obj->mdlId[3], ball->cursorAScale, 1, ball->cursorAScale);
    if(ball->cursorAScaleTarget <= 0) {
        if(ball->cursorAScale < 0.1f) {
            Hu3DModelDispOff(obj->mdlId[3]);
            ball->cursorAScale = 0;
        }
    }
    for(i=0; i<2; i++) {
        ball->cursorBScale[i] += 0.3f*(ball->cursorBScaleTarget[i]-ball->cursorBScale[i]);
        if(ball->cursorBScaleTarget[i] > 0) {
            Hu3DModelDispOn(obj->mdlId[i+1]);
        } else if(ball->cursorBScale[i] < 0.1f) {
            Hu3DModelDispOff(obj->mdlId[i+1]);
        }
        Hu3DModelScaleSet(obj->mdlId[i+1], ball->cursorBScale[i], 1, ball->cursorBScale[i]);
    }
    Hu3DParticleAnimModeSet(obj->mdlId[10], ballExplodeTimer);
    if(ball->updateF || ball->killF) {
        Hu3DModelDispOff(obj->mdlId[10]);
    } else if(gameType == 1) {
        Hu3DModelDispOn(obj->mdlId[10]);
    }
    if(gameType == 2 && M433GameEventGet() == 6 && M433GameTimerGet() > 10) {
        M433BallDiceUpdate(obj);
    } else {
        M433BallUpdate(obj);
        M433BallModelUpdate(obj);
        Hu3DModelDispOff(obj->mdlId[11]);
    }
    obj->trans.x = ball->pos.x;
    obj->trans.y = ball->pos.y;
    obj->trans.z = ball->pos.z;
    if(M433GamemesModeGet() >= M433_GAMEMES_FINISH && M433GameFinishEndCheck()) {
        M433DustEffectAdd(&ball->pos);
        Hu3DModelDispOff(obj->mdlId[11]);
        Hu3DModelDispOff(obj->mdlId[0]);
        for(i=0; i<19; i++) {
            Hu3DModelDispOff(obj->mdlId[i+12]);
        }
        obj->objFunc = M433BallKill;
    }
}

void M433BallKill(OMOBJ *obj)
{
    
}

void M433BallTargetSet(M433_BALLTARGET *target, HuVecF *pos, HuVecF *posNext, unsigned int type, float trail);
void M433BallTargetYOfsSet(M433_BALLTARGET *target);
void M433BallTargetRangeSet(M433_BALLTARGET *target);
BOOL M433BallTargetPosGet(M433_BALLTARGET *target, HuVecF *pos, HuVecF *ballDir, float ballDist);

BOOL M433BallUpdate(OMOBJ *obj)
{
    M433_BALLWORK *ball = obj->data;
    static BOOL cursorFlag[9] = {
        FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE
    };
    static float rotSpeed[9] = {
        0, 0.6666667f, 2.1666667f, 2, 1, 7.0000005f, 1.5000001f, 1.5000001f, 3.0000002f
    };
    static float gameRotSpeed[3] = {
        8, 2, 2
    };
    int i;
    Mtx matrix;
    HuVecF dir;
    if(!ball->updateF && !ball->killF && !ball->ballGroundF) {
        if(!ball->targetUpdateF) {
            M433BallTargetSet(&ball->target, &ball->pos, &ball->posNext, ball->type, ball->trail);
            M433BallTargetYOfsSet(&ball->target);
            M433BallTargetRangeSet(&ball->target);
            ball->ballRot = ball->trail*(rotSpeed[ball->type]*gameRotSpeed[gameType]);
            VECSubtract(&ball->posNext, &ball->pos, &dir);
            dir.y = 0;
            if(VECMag(&dir) <= 0) {
                ball->ballRotAxis.x = ball->ballRotAxis.y = 0;
                ball->ballRotAxis.z = 1;
            } else {
                VECNormalize(&dir, &dir);
                ball->ballRotAxis.x = -dir.z;
                ball->ballRotAxis.y = 0;
                ball->ballRotAxis.z = dir.x;
            }
            ball->ballDist = 0;
            i = 0;
            if(ball->cursorBScaleTarget[0] > 0) {
                i++;
            }
            Hu3DModelPosSet(obj->mdlId[i+1], ball->target.posNext.x, ball->target.posNext.y+3, ball->target.posNext.z);
            if(ball->cursorAScale <= 0) {
                ball->timer = 9;
            }
            if(cursorFlag[ball->type]) {
                ball->cursorBScaleTarget[i] = 1;
            }
            ball->cursorBScaleTarget[1-i] = 0;
        }
        ball->targetUpdateF = TRUE;
        if(ball->ballRot != 0.0f) {
            MTXRotAxisDeg(matrix, &ball->ballRotAxis, ball->ballRot);
            MTXConcat(matrix, ball->ballMtx, ball->ballMtx);
            MTXCopy(ball->ballMtx, Hu3DData[obj->mdlId[0]].mtx);
        }
        ball->ballDist += (1.0f/60.0f);
        ball->ballGroundF = M433BallTargetPosGet(&ball->target, &ball->pos, &ball->ballDir, ball->ballDist);
        if(gameType == 2) {
            if((ball->updateTime & 0x7) == 0) {
                ballExplodeTimer++;
                if(ballExplodeTimer > gameBallNumMax) {
                    ballExplodeTimer = 1;
                }
            }
            ballExplodeTimerPrev = ballExplodeTimer;
        }
        if(ball->ballGroundF) {
            ball->cursorBScaleTarget[0] = ball->cursorBScaleTarget[1] = 0;
            if(ball->pos.y <= 12.0f) {
                M433_GAMEMAN *gameman = gameManObj->data;
                ball->targetTimer = 0;
                Hu3DModelPosSetV(obj->mdlId[4], &ball->pos);
                Hu3DMotionTimeSet(obj->mdlId[4], 0);
                if(ball->type == 5 || ball->type == 2) {
                    Hu3DMotionSpeedSet(obj->mdlId[4], 1.5f);
                    M433SandEffectAdd(&ball->pos, 1);
                } else {
                    Hu3DMotionSpeedSet(obj->mdlId[4], 1);
                    M433SandEffectAdd(&ball->pos, 0.5f);
                    ball->pos.y += 11;
                }
                ball->type = 0;
                ball->killF = TRUE;
                gameman->bPointWin = TRUE;
                gameman->sGroundGrp = -1;
                if(fabsf(ball->pos.z) < 350 && fabsf(ball->pos.x) < 700) {
                    gameman->sGroundGrp = 0;
                    if(ball->pos.x >= 0) {
                        gameman->sGroundGrp = 1;
                    }
                }
                for(i=0; i<4; i++) {
                    omObjGetDataAs(playerObj[i], M433_PLAYERWORK)->landF = FALSE;
                }
            }
        }
        ball->ballScale += 0.15f*(ball->ballScaleNew-ball->ballScale);
        if(gameType == 2 && ballSeNo < 0) {
            ballSeNo = HuAudFXPlay(MSM_SE_M433_18);
        }
    } else {
        ball->targetUpdateF = FALSE;
        if(ballSeNo >= 0) {
            HuAudFXStop(ballSeNo);
            ballSeNo = MSM_SENO_NONE;
        }
    }
    return ball->ballGroundF;
}

void M433BallModelUpdate(OMOBJ *obj)
{
    M433_BALLWORK *ball = obj->data;
    int i;
    Mtx ballRot;
    Mtx ballMtx;
    HuVecF ballPos;
    HuVecF posNext;
    static BOOL cursorFlag[9] = {
        FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE
    };
    
    if(gameType == 2 || gameType == 1) {
        ball->targetTimer = 10;
    }
    if(ball->mode == 11) {
        Hu3DModelDispOff(obj->mdlId[0]);
        for(i=0; i<19; i++) {
            Hu3DModelDispOff(obj->mdlId[i+12]);
        }
    } else {
        float speed;
        float trail;
        float time;
        float baseTrail;
        float rot;
        
        if(ball->targetTimer) {
            Hu3DModelDispOff(obj->mdlId[0]);
            baseTrail = trail = ball->target.radius;
            if(!ball->targetUpdateF) {
                speed = 0;
                ballPos = ball->pos;
                MTXCopy(ball->ballMtx, ballMtx);
                MTXIdentity(ballRot);
            } else {
                speed = baseTrail-(0.020000001f*ball->trail);
                if(speed < 0) {
                    speed = 0;
                }
                speed = (trail-speed)/19;
                MTXCopy(ball->ballMtx, ballMtx);
                rot = (ball->ballRot*(1.2f*ball->trail))/19;
                MTXRotAxisDeg(ballRot, &ball->ballRotAxis, -rot);
            }
            for(i=0; i<19; i++) {
                Hu3DModelDispOn(obj->mdlId[i+12]);
                if(ball->targetUpdateF) {
                    M433BallTargetPosGet(&ball->target, &ballPos, &posNext, trail);
                }
                Hu3DModelPosSetV(obj->mdlId[i+12], &ballPos);
                Hu3DModelScaleSet(obj->mdlId[i+12], ball->ballScale, ball->ballScale, ball->ballScale);
                MTXCopy(ballMtx, Hu3DData[obj->mdlId[i+12]].mtx);
                MTXConcat(ballRot, ballMtx, ballMtx);
                trail -= speed;
            }
            ball->target.radius = baseTrail;
        } else {
            Hu3DModelDispOn(obj->mdlId[0]);
            Hu3DModelScaleSet(obj->mdlId[0], ball->ballScale, ball->ballScale, ball->ballScale);
            for(i=0; i<19; i++) {
                Hu3DModelDispOff(obj->mdlId[i+12]);
            }
        }
        if(gameType == 2 || gameType == 1) {
            time = (ballExplodeTimer*2)-1;
            if(gameType == 1) {
                time += 2;
            }
            Hu3DMotionTimeSet(obj->mdlId[0], time);
            for(i=0; i<19; i++) {
                Hu3DMotionTimeSet(obj->mdlId[i+12], time);
            }
        }
    }
}

void M433BallDiceUpdate(OMOBJ *obj)
{
    M433_BALLWORK *ball = obj->data;
    Mtx invCamera;
    Mtx invRot;
    Mtx rot;
    Mtx matrix;
    HuVecF pos;
    HuVecF basePos = { 0, -25, -400 };
    float angle;
    MTXInverse(Hu3DCameraMtx, invCamera);
    if(ball->updateF || ball->killF) {
        ball->updateF = FALSE;
        ball->killF = FALSE;
        ball->ballRot = 2;
        ball->diceRot = 0;
        ball->diceUpdateNum = 0;
    }
    MTXMultVec(invCamera, &basePos, &pos);
    MTXCopy(invCamera, invRot);
    invRot[0][3] = 0;
    invRot[1][3] = 0;
    invRot[2][3] = 0;
    MTXCopy(invRot, matrix);
    ball->diceRot += ball->ballRot;
    MTXRotDeg(rot, 'Y', ball->diceRot);
    MTXConcat(matrix, rot, matrix);
    angle = (1.2f*ball->ballRot)/19;
    MTXRotDeg(rot, 'Y', angle);
    Hu3DModelDispOn(obj->mdlId[11]);
    Hu3DModelPosSetV(obj->mdlId[11], &pos);
    Hu3DModelScaleSet(obj->mdlId[11], 1.2f, 1.2f, 1.2f);
    MTXCopy(matrix, Hu3DData[obj->mdlId[11]].mtx);
    MTXConcat(matrix, rot, matrix);
    Hu3DMotionTimeSet(obj->mdlId[11], (ballExplodeTimer*2)-1);
    ball->diceUpdateNum++;
}

void M433BallTargetSet(M433_BALLTARGET *target, HuVecF *pos, HuVecF *posNext, unsigned int type, float trail)
{
    static HuVecF targetTbl[9] = {
        0, 0, 1,
        0, 1.5, 1,
        242, 0, 1100,
        0, 1.5, 1,
        0, 1.8, 1,
        231, 0, 1500,
        231, 0, 1000,
        0, 1.2, 1,
        0, 0.8, 1
    };
    static float speedTbl[9] = {
        1,
        1,
        1.7,
        1,
        1,
        4,
        2,
        1,
        1
    };
    
    float y;
    float dist;
    float dy;
    HuVecF dir;
    HuVecF pos2;
    
    target->maxY = 0;
    target->scale = 0;
    y = targetTbl[type].y;
    target->radiusMax = y;
    if(target->radiusMax <= 0) {
        if(posNext->x*pos->x < 0 && pos->x != 0.0f && targetTbl[type].x > 0) {
            target->maxY = targetTbl[type].x;
            target->scale = fabsf(pos->x/(posNext->x-pos->x));
            dy = pos->y+(target->scale*(posNext->y-pos->y));
            if(dy < target->maxY) {
                pos2.x = pos->x+(target->scale*(posNext->x-pos->x));
                pos2.y = target->maxY;
                pos2.z = pos->z+(target->scale*(posNext->z-pos->z));
                VECSubtract(posNext, &pos2, &dir);
                dist = VECMag(&dir);
                VECSubtract(&pos2, pos, &dir);
                dist += VECMag(&dir);
            } else {
                VECSubtract(posNext, pos, &dir);
                dist = VECMag(&dir);
            }
        } else {
            VECSubtract(posNext, pos, &dir);
            dist = VECMag(&dir);
        }
        y = dist/(trail*targetTbl[type].z);
        target->radiusMax = y;
    }
    target->radius = 0;
    target->pos = *pos;
    target->posNext = *posNext;
    target->dy = posNext->y-pos->y;
    target->colTime = -980;
    VECSubtract(posNext, pos, &dir);
    dir.y = 0;
    target->dist = VECMag(&dir);
    target->colTimeRad = 0;
    if(y > 0) {
        target->colHeight = (target->dy-(0.5f*(-980.0f*(y*y))))/y;
        target->colRadius = target->dist/y;
    } else {
        target->colHeight = target->dy;
        target->colRadius = target->dist;
    }
    target->speed = speedTbl[type];
}

void M433BallTargetRangeSet(M433_BALLTARGET *target)
{
    float rDist;
    float r;
    float invR;
    float speed;
    float lowR;
    float highR;
    float gravity;
    float y;
    int i;

    if (target->maxY <= 0.0f) {
        return;
    }
    r = target->radiusMax;
    lowR = target->radiusMax;
    highR = 10.0f * target->radiusMax;
    for(i=0; i<16; i++) {
        invR = 1.0f / r;
        rDist = target->dist * invR;
        gravity = (target->dy * invR) - (0.5f * target->colTime * r);
        speed = target->scale * r;
        y = target->pos.y + ((speed * (0.5f * target->colTime * speed)) + (gravity * speed));
        if(target->maxY > y) {
            lowR = r;
        }
        else {
            highR = r;
        }
        r = 0.5f * (lowR + highR);
    }
    invR = 1.0f / r;
    target->radiusMax = r;
    target->colRadius = target->dist * invR;
    target->colHeight = (target->dy * invR) - (0.5f * target->colTime * r);
}

void M433BallTargetYOfsSet(M433_BALLTARGET *target)
{
    float t;
    float invR;
    float speed;
    float lowR;
    float highR;
    float gravity;
    float y;
    float rDist;
    int i;
    if (target->maxY <= 0.0f) {
        return;
    }
    speed = target->scale*target->radiusMax;
    invR = 1/target->radiusMax;
    t = target->colTime;
    lowR = target->colTime;
    highR = target->colTime*target->speed;
    for(i=0; i<16; i++) {
        rDist = target->dist * invR;
        gravity = (target->dy * invR) - (0.5f * t * target->radiusMax);
        y = target->pos.y + ((speed * (0.5f * t * speed)) + (gravity * speed));
        if(target->maxY > y) {
            lowR = t;
        } else {
            highR = t;
        }
        t = 0.5f * (lowR + highR);
    }
    target->colTime = t;
    target->colRadius = target->dist * invR;
    target->colHeight = (target->dy * invR) - (0.5f * target->colTime * target->radiusMax);
    
}

BOOL M433BallTargetPosGet(M433_BALLTARGET *target, HuVecF *pos, HuVecF *ballDir, float ballDist)
{
    HuVecF oldPos;
    HuVecF dir;
    float r;
    float y;

    if (ballDist < 0.0f) {
        ballDist = 0.0f;
    }
    if (ballDist > target->radiusMax) {
        ballDist = target->radiusMax;
    }
    target->radius = ballDist;
    if (target->dist > 0.0f) {
        VECSubtract(&target->posNext, &target->pos, &dir);
        dir.y = 0.0f;
        VECNormalize(&dir, &dir);
    }
    else {
        dir.x = dir.y = dir.z = 0.0f;
    }
    oldPos = *pos;
    r = (ballDist * (0.5f * target->colTimeRad * ballDist)) + (target->colRadius * ballDist);
    y = (ballDist * (0.5f * target->colTime * ballDist)) + (target->colHeight * ballDist);
    pos->x = target->pos.x + (dir.x * r);
    pos->y = target->pos.y + y;
    pos->z = target->pos.z + (dir.z * r);
    VECSubtract(pos, &oldPos, ballDir);
    if (ballDist >= target->radiusMax) {
        return 1;
    }
    return 0;
}

void M433SandEffectHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx)
{
    HU3DPARTICLEDATA *effDataP;
    int i;
    for(effDataP=particleP->data, i=0; i<particleP->maxCnt; i++, effDataP++) {
        if(effDataP->time) {
            VECAdd(&effDataP->pos, &effDataP->vel, &effDataP->pos);
            effDataP->vel.x *= 0.92f;
            effDataP->vel.z *= 0.92f;
            effDataP->vel.y += -0.27222225f;
            effDataP->scale += 2;
            effDataP->color.a *= 0.99f;
            if(effDataP->time < 24.0f) {
                effDataP->color.a = 0.9f*effDataP->color.a;
            }
            if(--effDataP->time == 0) {
                effDataP->time = 0;
                effDataP->scale = 0;
                effDataP->color.a = 0;
            }
        }
    }
    DCStoreRange(particleP->data, particleP->maxCnt*sizeof(HU3DPARTICLEDATA));
}

void M433SandEffectAdd(HuVecF *pos, float speed)
{
    HU3DPARTICLEDATA *effDataP = ((HU3DPARTICLE *)Hu3DData[ballObj->mdlId[5]].hookData)->data;
    unsigned int num = (speed*12)+3;
    int i;
    float radius;
    float angle;
    HuVecF dir;
    for(i=0; i<200; i++, effDataP++) {
        if(effDataP->time) {
            continue;
        }
        effDataP->time = ((frandmod(1000)*0.0006f)+0.6f)*60;
        angle = frandmod(360);
        dir.x = HuSin(angle);
        dir.y = 0;
        dir.z = HuCos(angle);
        radius = frandmod(1000)*0.030000001f;
        effDataP->pos.x = pos->x+(dir.x*radius);
        effDataP->pos.y = (frandmod(1000)*0.030000001f)+pos->y+10;
        effDataP->pos.z = pos->z+(dir.z*radius);
        radius = (frandmod(1000)*0.0016666667f)+0.8333334f;
        radius *= speed+1;
        effDataP->vel.x = dir.x*radius;
        effDataP->vel.z = dir.z*radius;
        effDataP->vel.y = (frandmod(1000)*0.0011666667f)+0.50000006f;
        effDataP->vel.y *= speed+1;
        effDataP->zRot = frandmod(360);
        effDataP->scale = frandmod(30)+60.0f;
        effDataP->color.a = frandmod(100)+120;
        radius = frandmod(1000)*0.001f;
        effDataP->color.r = (radius*20)+235;
        effDataP->color.g = (radius*75)+180;
        effDataP->color.b = (radius*100)+155;
        if(--num < 1) {
            return;
        }
    }
    
}

void M433NumberEffectHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx)
{
    
}

void M433DustEffectAdd(HuVecF *pos)
{
    HU3DPARTICLE *particleP = Hu3DData[ballObj->mdlId[8]].hookData;
    HU3DPARTICLEDATA *effDataP;
    Hu3DModelDispOn(ballObj->mdlId[8]);
    effDataP = particleP->data;
    particleP->dataCnt = 0;
    particleP->emitCnt = 60;
    effDataP->scale = 100;
    effDataP->pos = *pos;
}

void M433DustEffectHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx)
{
    HU3DPARTICLEDATA *effDataP;
    int i;
    int alpha;
    if(particleP->dataCnt == 0) {
        particleP->dataCnt = 1;
        for(effDataP=(HU3DPARTICLEDATA *)&particleP->data, i=0; i<particleP->maxCnt; i++, effDataP++) {
            effDataP->scale = 0;
        }
    }
    alpha = (particleP->emitCnt*255)*(1.0f/60.0f);
    for(effDataP=particleP->data, i=0; i<particleP->maxCnt; i++, effDataP++) {
        effDataP->color.a = alpha;
    }
    if(--particleP->emitCnt == 0) {
        modelP->attr |= HU3D_ATTR_DISPOFF;
    }
    DCStoreRange(particleP->data, particleP->maxCnt*sizeof(HU3DPARTICLEDATA));
}

void M433ScoreCreate(M433_SCORESPR *spr, M433_SCOREOVL *ovl);
void M433ScoreUpdate(M433_SCORESPR *spr, M433_SCOREOVL *ovl);
void M433ScoreNextSet(M433_SCOREOVL *ovl, unsigned int leftScore, unsigned int rightScore, int winGrp);

void M433GameManUpdate(OMOBJ *obj);

void M433GameManCreate(OMOBJ *obj)
{
    M433_GAMEMAN *game;
    obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M433_GAMEMAN), HU_MEMNUM_OVL);
    game = obj->data;
    memset(game, 0, sizeof(M433_GAMEMAN));
    game->uGameEvent = 0;
    game->sWinGrp = -1;
    game->uServeGrp = 0;
    game->auEvents[0] = game->auEvents[1] = 0;
    game->bPointWin = game->bContact = FALSE;
    ballExplodeTimerPrev = 1;
    ballExplodeTimer = 1;
    ballSeNo = MSM_SENO_NONE;
    if(gameType == 1) {
        ballExplodeTimer = 10;
    }
    M433ScoreCreate(&game->scoreSpr[0], &game->scoreOvl);
    obj->objFunc = M433GameManUpdate;
}

void M433GameManDebug(OMOBJ *obj)
{
    M433_GAMEMAN *game = obj->data;
    OSReport("\n");
    OSReport("uGameEvent       %d\n", game->uGameEvent);
    OSReport("sWinGrp          %d\n", game->sWinGrp);
    OSReport("uServeGrp        %d\n", game->uServeGrp);
    OSReport("uPointGrp        %d\n", game->uPointGrp);
    OSReport("auServeCel[2]    %d,%d\n", game->auServeCel[0], game->auServeCel[1]);
    OSReport("auEvents[2]      %d,%d\n", game->auEvents[0], game->auEvents[1]);
    OSReport("uContactCnt      %d\n", game->uContactCnt);
    OSReport("sGroundGrp       %d\n", game->sGroundGrp);
    OSReport("uContactGrp      %d\n", game->uContactGrp);
    OSReport("sContactCel      %d\n", game->uContactCel);
    OSReport("uTargetGrp       %d\n", game->uTargetGrp);
    OSReport("\n");
}


void M433GameManUpdate(OMOBJ *obj)
{
    int winTbl[GW_PLAYER_MAX];
    M433_GAMEMAN *game = obj->data;
    int temp = 0;
    BOOL winMoveF = FALSE;
    BOOL guideMoveF = FALSE;
    BOOL gameWinF = FALSE;
    int timerSpeed = 0;
    int winGrp;
    HuVecF pos;
    
    static HuVecF prevCenter = { 0, 200, 0 };
    static float prevZoom = 1900;
    
    game->uTime++;
    if(M433GamemesModeGet() >= M433_GAMEMES_WAIT) {
        M433ScoreUpdate(&game->scoreSpr[0], &game->scoreOvl);
    }
    if(gameType == 1 && ballExplodeTimer == 0) {
        guideMoveF = TRUE;
    }
    switch(game->uGameEvent) {
        case 0:
            Center.x = prevCenter.x = 0;
            CZoom = prevZoom = 1900;
            if(game->uTime == 4) {
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
            }
            game->uContactCnt = 0;
            game->uContactCel = -1;
            if(game->uTime == 12) {
                M433GuideModeSet(M433_GUIDEMODE_READY);
            }
            if(!WipeCheckEnd() && game->uTime >= 30.0f && M433GamemesModeGet() >= M433_GAMEMES_WAIT) {
                game->uGameEvent = 1;
                game->uTime = 0;
                game->auEvents[0] = game->auEvents[1] = 1;
                game->abGameWin[0] = game->abGameWin[1] = FALSE;
                if(gameType == 1) {
                    ballExplodeTimer = 10;
                }
            }
            break;
        
        case 1:
            if(game->bContact) {
                HuVecF pos;
                prevCenter.x = 150;
                prevZoom = 1700;
                if(M433BallSideGet(&pos) == 0) {
                    prevCenter.x *= -1;
                }
                lbl_1_bss_7C8 = 0;
                if(!guideMoveF) {
                    game->uGuideTimer = 0;
                    game->uContactCel = -1;
                    game->uContactCnt = 0;
                    game->uGameEvent = 2;
                    game->uTime = 0;
                    game->auEvents[0] = game->auEvents[1] = 3;
                    game->abGameWin[0] = game->abGameWin[1] = FALSE;
                    M433GuideModeSet((game->uTargetGrp) ? M433_GUIDEMODE_FACE_RIGHT : M433_GUIDEMODE_FACE_LEFT);
                    
                }
                timerSpeed++;
            }
            break;
        
        case 2:
        case 5:
            if(game->bContact) {
                HuVecF pos;
                prevCenter.x = 100;
                prevZoom = 1700;
                if(M433BallSideGet(&pos) == 0) {
                    prevCenter.x *= -1;
                }
                lbl_1_bss_7C8 = 0;
                if(!guideMoveF) {
                    if(game->uContactCnt) {
                        game->uGuideTimer++;
                        game->uGameEvent = 3;
                        game->auEvents[game->uContactGrp] = 4;
                        if(game->uGuideTimer >= 6 && game->uGuideTimer%3 == 0) {
                            M433GuideModeSet(M433_GUIDEMODE_RUN);
                        }
                    }
                    game->uTime = 0;
                }
                timerSpeed++;
            }
            winMoveF = TRUE;
            break;
        
        case 3:
        case 4:
            if(game->bContact) {
                HuVecF pos;
                if(!guideMoveF) {
                    if(game->uContactGrp == game->uTargetGrp) {
                        game->uGameEvent = 4;
                        game->auEvents[game->uContactGrp] = 5;
                    } else {
                        game->uContactCel = -1;
                        game->uContactCnt = 0;
                        game->uGameEvent = 5;
                        game->auEvents[game->uContactGrp] = 3;
                        M433GuideModeSet((game->uTargetGrp) ? M433_GUIDEMODE_FACE_RIGHT : M433_GUIDEMODE_FACE_LEFT);
                    }
                    game->uTime = 0;
                }
                timerSpeed++;
                if(game->uGameEvent == 5) {
                    prevCenter.x = 200;
                    prevZoom = 1500;
                } else {
                    prevCenter.x = 0;
                    prevZoom = 1900;
                }
                if(M433BallSideGet(&pos) == 0) {
                    prevCenter.x *= -1;
                }
                lbl_1_bss_7C8 = 0;
            }
            winMoveF = TRUE;
            break;
        
        case 6:
            if(game->uTime >= 10000) {
                game->uGameEvent = 0;
                game->uTime = 0;
                game->auEvents[0] = game->auEvents[1] = 0;
                game->abGameWin[0] = game->abGameWin[1] = FALSE;
            } else if(game->uTime >= 120.0f && game->scoreOvl.titleTime == 0) {
                if(game->auScore[game->uPointGrp] >= gameMaxScore) {
                    int winNum;
                    int i;
                    game->sWinGrp = game->uPointGrp;
                    game->uGameEvent = 8;
                    game->auEvents[0] = game->auEvents[1] = 8;
                    game->abGameWin[0] = game->abGameWin[1] = FALSE;
                    winNum = 0;
                    for(i=0; i<GW_PLAYER_MAX; i++) {
                        winTbl[i] = -1;
                        if(game->sWinGrp == omObjGetDataAs(playerObj[i], M433_PLAYERWORK)->side) {
                            winTbl[winNum++] = playerObj[i]->work[0];
                        }
                    }
                    M433GameWinnerSet(M433_WINTYPE_NORMAL, winTbl[0], winTbl[1]);
                    M433GameWinFlagSet(4);
                } else {
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
                    game->uTime = 10000;
                }
            }
            break;
        
        case 7:
        case 8:
        {
            if(0) {
                float temp = 0;
                (void)temp;
            }
        }
            break;
    }
    winGrp = -1;
    if(gameType == 1 && timerSpeed > 0) {
        ballExplodeTimer -= timerSpeed;
        HuAudFXPlay(MSM_SE_M433_16);
        if(guideMoveF && ballExplodeTimer < 0) {
            ballExplodeTimer = 0;
            winMoveF = FALSE;
            gameWinF = TRUE;
            winGrp = 1-game->uContactGrp;
            M433GuideModeSet(M433_GUIDEMODE_READY);
            M433BallExplodeSet();
        }
    }
    if(winGrp < 0 && winMoveF && game->bPointWin) {
        if(game->sGroundGrp >= 0) {
            winGrp = 1-game->sGroundGrp;
            M433GuideModeSet(M433_GUIDEMODE_READY);
        } else {
            winGrp = 1-game->uContactGrp;
            M433GuideModeSet(M433_GUIDEMODE_JUMP);
        }
    }
    if(winGrp >= 0) {
        game->uPointGrp = winGrp;
        game->auScore[winGrp] += ballExplodeTimerPrev;
        if(game->auScore[winGrp] > gameMaxScore) {
            game->auScore[winGrp] = gameMaxScore;
        }
        if(game->uServeGrp != winGrp) {
            game->uServeGrp = winGrp;
            game->auServeCel[game->uServeGrp] = 1-game->auServeCel[game->uServeGrp];
        }
        game->uGameEvent = 6;
        game->auEvents[0] = game->auEvents[1] = 6;
        M433ScoreNextSet(&game->scoreOvl, game->auScore[0], game->auScore[1], winGrp);
        if(gameWinF) {
            game->auEvents[game->uContactGrp] = 7;
            game->abGameWin[game->uContactGrp] = TRUE;
        }
        game->uTime = 0;
    }
    game->bPointWin = FALSE;
    game->bContact = FALSE;
}

static u16 scoreTypeTbl[5] = { 0, 1, 2, 3, 3 };

static int scoreNumFileTbl[2] = { MGCONST_ANM_scoreMedium, MGCONST_ANM_scoreMedium };

static HuVec2F scoreSprOfsTbl[5] = {
    { 0.0f, 0.0f },
    { 0.0f, 0.0f },
    { -23.0f, 0.0f },
    { -11.0f, 0.0f },
    { 11.0f, 0.0f },
};

static HuVec2F scorePosTbl[2] = { { 52.0f, 56.0f }, { 524.0f, 56.0f } };

static s32 scoreNextNumFileTbl[11][4] = {
    { GAMEMES_ANM_number0, 0, 0, 0 },
    { GAMEMES_ANM_number0+1, 0, 0, 0 },
    { GAMEMES_ANM_number0+2, 0, 0, 0 },
    { GAMEMES_ANM_number0+3, 0, 0, 0 },
    { GAMEMES_ANM_number0+4, 0, 0, 0 },
    { GAMEMES_ANM_number0+5, 0, 0, 0 },
    { GAMEMES_ANM_number0+6, 0, 0, 0 },
    { GAMEMES_ANM_number0+7, 0, 0, 0 },
    { GAMEMES_ANM_number0+8, 0, 0, 0 },
    { GAMEMES_ANM_number0+9, 0, 0, 0 },
    { GAMEMES_ANM_fontScoreDash, 0, 0, 0 },
};

void M433ScoreCreate(M433_SCORESPR *spr, M433_SCOREOVL *ovl)
{
    GXColor colorTbl[2] = {
        { 0, 50, 250, 255 },
        { 250, 0, 30, 255 },
    };
    
    int i, j, k;
    for(i=0; i<2; i++, spr++) {
        for(j=0; j<5u; j++) {
            switch(scoreTypeTbl[j]) {
                case 0:
                    spr->sprId[j] = MgScoreWinCreate(60, 40);
                    MgScoreWinPosSet(spr->sprId[j], scorePosTbl[i].x+scoreSprOfsTbl[j].x, scorePosTbl[i].y+scoreSprOfsTbl[j].y);
                    MgScoreWinDispSet(spr->sprId[j], FALSE);
                    MgScoreWinColorSet(spr->sprId[j], colorTbl[i].r, colorTbl[i].g, colorTbl[i].b);
                    break;
                
                case 1:
                    spr->sprId[j] = HUSPR_NONE;
                    break;
                
                case 2:
                    spr->sprId[j] = espEntry(M433_ANM_ball_score, 1, 0);
                    espPriSet(spr->sprId[j], 10);
                    break;
                
                case 3:
                    spr->sprId[j] = espEntry(MGCONST_ANM_scoreMedium, 1, 0);
                    espPriSet(spr->sprId[j], 5);
                    espColorSet(spr->sprId[j], 255, 216, 21);
                    break;
            }
            if(j >= 2) {
                espDispOff(spr->sprId[j]);
                espDrawNoSet(spr->sprId[j], 0);
                espAttrSet(spr->sprId[j], HUSPR_ATTR_NOANIM);
                espAttrSet(spr->sprId[j], HUSPR_ATTR_LINEAR);
                espPosSet(spr->sprId[j], scorePosTbl[i].x+scoreSprOfsTbl[j].x, scorePosTbl[i].y+scoreSprOfsTbl[j].y);
            }
        }
    }
    HuDataDirClose(DATA_mgconst);
    for(i=0; i<3; i++) {
        for(j=0; j<20; j++) {
            for(k=0; k<2; k++) {
                if(scoreNextNumFileTbl[j%10][k]) {
                    ovl->digitSprId[i][j].sprId[k] = espEntry(scoreNextNumFileTbl[j%10][k], 2, 0);
                    espAttrSet(ovl->digitSprId[i][j].sprId[k], HUSPR_ATTR_LINEAR);
                    espDispOff(ovl->digitSprId[i][j].sprId[k]);
                } else {
                    ovl->digitSprId[i][j].sprId[k] = -1;
                }
            }
        }
    }
    for(k=0; k<2; k++) {
        if(scoreNextNumFileTbl[10][k]) {
            ovl->dashSprId.sprId[k] = espEntry(scoreNextNumFileTbl[10][k], 2, 0);
            espAttrSet(ovl->dashSprId.sprId[k], HUSPR_ATTR_LINEAR);
            espDispOff(ovl->dashSprId.sprId[k]);
        } else {
            ovl->dashSprId.sprId[k] = -1;
        }
    }
    for(i=0; i<2; i++) {
        ovl->scoreOvlSpr[i].sprId[0] = -1;
        ovl->scoreOvlSpr[i].sprId[1] = -1;
    }
    ovl->scoreOvlSpr[2].sprId[0] = -1;
    ovl->scoreOvlSpr[2].sprId[1] = -1;
    ovl->scoreOvlSpr[3].sprId[0] = -1;
    ovl->scoreOvlSpr[3].sprId[1] = -1;
    HuDataDirClose(scoreNextNumFileTbl[0][0]);
}

void M433ScoreOvlSprUpdate(M433_SCOREOVL_SPR *ovlspr)
{
    int i;
    for(i=0; i<2; i++) {
        float ofsX, ofsY;
        if(ovlspr->sprId[i] < 0) {        
            continue;
        }
        espDispOn(ovlspr->sprId[i]);
        ofsX = HuCos(ovlspr->zRot)*(ovlspr->sprOfs[i]*ovlspr->scaleX);
        ofsY = HuSin(ovlspr->zRot)*(ovlspr->sprOfs[i]*ovlspr->scaleX);
        espPosSet(ovlspr->sprId[i], ovlspr->posX+ofsX, ovlspr->posY+ofsY);
        espZRotSet(ovlspr->sprId[i], ovlspr->zRot);
        espScaleSet(ovlspr->sprId[i], ovlspr->scaleX, ovlspr->scaleY);
        espTPLvlSet(ovlspr->sprId[i], ovlspr->tpLvl);
    }
}

void M433ScoreOvlValueSet(M433_SCORESPRNUM *sprNum, M433_SCOREOVL_SPR *ovlspr, u32 num)
{
    u32 digit1 = num%10;
    u32 digit2 = num/10;
    if(digit2 == 0) {
        ovlspr->sprId[0] = sprNum[digit1].sprId[0];
        ovlspr->sprId[1] = -1;
        ovlspr->sprOfs[0] = 0;
    } else {
        ovlspr->sprId[0] = sprNum[digit2+10].sprId[0];
        ovlspr->sprId[1] = sprNum[digit1].sprId[0];
        ovlspr->sprOfs[0] = -23.039999f;
        ovlspr->sprOfs[1] = 23.039999f;
    }
    ovlspr->posX = ovlspr->posY = 0;
    ovlspr->velX = ovlspr->velY = 0;
    ovlspr->scaleX = ovlspr->scaleY = 1.5f;
    ovlspr->tpLvl = 1.0f;
    ovlspr->zRot = 0;
}

void M433ScoreUpdate(M433_SCORESPR *spr, M433_SCOREOVL *ovl)
{
    float t;
    float ofsX;
    float radius;
    float velX;
    M433_SCOREOVL_SPR *ovlspr;
    M433_SCORESPRNUM *sprNum;
    M433_SCORESPR *iterSpr;
    int i, j;
    
    
    
    for(iterSpr=spr, i=0; i<2; i++, iterSpr++) {
        MgScoreWinDispSet(iterSpr->sprId[0], TRUE);
        for(j=3; j<5; j++) {
            espDispOn(iterSpr->sprId[j]);
        }
        espBankSet(iterSpr->sprId[3], iterSpr->num/10);
        espBankSet(iterSpr->sprId[4], iterSpr->num%10);
    }
    velX = 2.88f;
    if(ovl->winGrp == 0) {
        ovlspr = &ovl->scoreOvlSpr[0];
    } else {
        ovlspr = &ovl->scoreOvlSpr[1];
        velX *= -1;
    }
    switch(ovl->mode) {
        case 0:
            break;
        
        case 1:
            ovl->scoreValue = ovl->score[ovl->winGrp];
            ovl->scoreValuePrev = ovl->scorePrev[ovl->winGrp]+1;
            M433ScoreOvlValueSet(&ovl->digitSprId[0][0], &ovl->scoreOvlSpr[0], ovl->scorePrev[0]);
            M433ScoreOvlValueSet(&ovl->digitSprId[1][0], &ovl->scoreOvlSpr[1], ovl->scorePrev[1]);
            M433ScoreOvlValueSet(&ovl->dashSprId, &ovl->scoreOvlSpr[2], 0);
            M433ScoreOvlValueSet(&ovl->digitSprId[2][0], &ovl->scoreOvlSpr[3], ovl->scoreValuePrev);
            ofsX = -48.0f;
            ovl->scoreOvlSpr[0].posX = 172.8f;
            ovl->scoreOvlSpr[0].posY = 360.0f - ofsX;
            ovl->scoreOvlSpr[1].posX = 403.19998f;
            ovl->scoreOvlSpr[1].posY = 360.0f - ofsX;
            ovl->scoreOvlSpr[2].posX = 288.0f;
            ovl->scoreOvlSpr[2].posY = 360.0f - ofsX;
            ovl->scoreOvlSpr[0].tpLvl = ovl->scoreOvlSpr[1].tpLvl = ovl->scoreOvlSpr[2].tpLvl = 0.0f;
            ofsX = ofsX / 19.199999f;
            ovl->scoreOvlSpr[0].velY = ofsX;
            ovl->scoreOvlSpr[1].velY = ofsX;
            ovl->scoreOvlSpr[2].velY = ofsX;
            ovl->scoreOvlSpr[3].posX = 806.39996f;
            ovl->scoreOvlSpr[3].posY = 144.0f;
            ovl->scoreOvlSpr[3].scaleX += 0.5f;
            ovl->scoreOvlSpr[3].scaleY -= 0.5f;
            ovl->scoreOvlSpr[3].velX = (403.19998f - ovl->scoreOvlSpr[3].posX) / 24.0f;
            ovl->scoreOvlSpr[3].velY = (360.0f - ovl->scoreOvlSpr[3].posY) / 24.0f;
            if (ovl->winGrp == 0) {
                ovl->scoreOvlSpr[3].posX = -230.40001f;
                ovl->scoreOvlSpr[3].velX *= -1.0f;
            }
            ovl->mode++;
            ovl->time = 0;
            ovl->titleTime = 1;
            break;
            
        case 2:
            t = ovl->scoreOvlSpr[0].tpLvl;
            t += 0.055555556f;
            if (t > 0.95f) {
                t = 1.0f;
            }
            ovl->scoreOvlSpr[2].tpLvl = t;
            ovl->scoreOvlSpr[1].tpLvl = t;
            ovl->scoreOvlSpr[0].tpLvl = t;
            ovl->scoreOvlSpr[0].posY += ovl->scoreOvlSpr[0].velY;
            ovl->scoreOvlSpr[1].posY += ovl->scoreOvlSpr[1].velY;
            ovl->scoreOvlSpr[2].posY += ovl->scoreOvlSpr[2].velY;
            ovl->scoreOvlSpr[3].posX += ovl->scoreOvlSpr[3].velX;
            ovl->scoreOvlSpr[3].posY += ovl->scoreOvlSpr[3].velY;
            if (ovl->time >= 19.199999f) {
                ovlspr->velX = velX;
                ovlspr->velY = -16.0f;
                ovl->mode++;
                ovl->moveTime = 0;
                if(gameType == 2) {
                    HuAudFXPlay(MSM_SE_M433_19);
                }
            }
            break;
        
        case 3:
            spr[ovl->winGrp].num = ovl->scoreValuePrev;
            if (ovl->time <= 24.0f) {
                ovl->scoreOvlSpr[3].posX += ovl->scoreOvlSpr[3].velX;
                ovl->scoreOvlSpr[3].posY += ovl->scoreOvlSpr[3].velY;
            }
            if (ovl->moveTime < 24.0f) {
                ovl->moveTime++;
                t = 33.75f * ovl->moveTime;
                radius = 0.041666668f * ovl->moveTime;
                radius = 0.5f * (1.0f - (radius * radius));
                if (ovl->moveTime == 24.0f) {
                    radius = 0.0f;
                }
                ovl->scoreOvlSpr[3].scaleX = 1.5 + (radius * HuCos(t));
                ovl->scoreOvlSpr[3].scaleY = 1.5 + (radius * HuSin((t - 90.0f)));
            }
            ovlspr->scaleX = ovl->scoreOvlSpr[3].scaleY;
            ovlspr->scaleY = ovl->scoreOvlSpr[3].scaleX;
            ovlspr->posX += ovlspr->velX;
            ovlspr->posY += ovlspr->velY;
            ovlspr->velY += 0.80000013f;
            ovlspr->zRot += 8.333334f;
            ovlspr->tpLvl -= 0.017833335f;
            if(ovl->scoreValuePrev != ovl->scoreValue) {
                ovlspr->tpLvl -= 0.017833335f;
            }
            if (ovlspr->tpLvl < 0.0f) {
                ovlspr->tpLvl = 0;
            }
            if(ovlspr->tpLvl <= 0.0f) {
                ovl->mode = 5;
                ovl->time = 0;
            }
            break;
        
        case 4:
            t = ovl->scoreOvlSpr[3].tpLvl;
            t -= 0.055555556f;
            if(t < 0.05f) {
                t = 0;
                ovl->mode += 10;
                ovl->time = 0;
            }
            ovl->scoreOvlSpr[3].tpLvl = t;
            ovl->scoreOvlSpr[2].tpLvl = t;
            ovl->scoreOvlSpr[1].tpLvl = t;
            ovl->scoreOvlSpr[0].tpLvl = t;
            break;
        
        case 5:
            if(ovl->scoreValuePrev >= ovl->scoreValue) {
                ovl->mode = 4;
                ovl->titleTime = 0;
            } else {
                M433ScoreOvlValueSet(&ovl->digitSprId[ovl->winGrp][0], &ovl->scoreOvlSpr[ovl->winGrp], ovl->scoreValuePrev);
                ovlspr->posX = ovl->scoreOvlSpr[3].posX;
                ovlspr->posY = ovl->scoreOvlSpr[3].posY;
                ovl->scoreValuePrev++;
                M433ScoreOvlValueSet(&ovl->digitSprId[2][0], &ovl->scoreOvlSpr[3], ovl->scoreValuePrev);
                ovl->scoreOvlSpr[3].posX = 806.39996f;
                ovl->scoreOvlSpr[3].posY = 144.0f;
                ovl->scoreOvlSpr[3].scaleX += 0.5f;
                ovl->scoreOvlSpr[3].scaleY -= 0.5f;
                ovl->scoreOvlSpr[3].velX = (403.19998f-ovl->scoreOvlSpr[3].posX)/24.0f;
                ovl->scoreOvlSpr[3].velY = (360.0f-ovl->scoreOvlSpr[3].posY)/24.0f;
                if(ovl->winGrp == 0) {
                    ovl->scoreOvlSpr[3].posX = -230.40001f;
                    ovl->scoreOvlSpr[3].velX *= -1;
                }
                ovl->mode++;
                ovl->time = 0;
            }
            break;
        
        case 6:
            ovl->scoreOvlSpr[3].posX += ovl->scoreOvlSpr[3].velX;
            ovl->scoreOvlSpr[3].posY += ovl->scoreOvlSpr[3].velY;
            if(ovl->time >= 19.199999f) {
                ovlspr->velX = velX;
                ovlspr->velY = -16;
                ovl->mode = 3;
                ovl->moveTime = 0;
                if(gameType == 2) {
                    HuAudFXPlay(MSM_SE_M433_19);
                }
            }
            break;
        
        default:
            for(i=0; i<2; i++) {
                ovl->scoreOvlSpr[i].sprId[0] = -1;
                ovl->scoreOvlSpr[i].sprId[1] = -1;
            }
            ovl->scoreOvlSpr[2].sprId[0] = -1;
            ovl->scoreOvlSpr[2].sprId[1] = -1;
            ovl->scoreOvlSpr[3].sprId[0] = -1;
            ovl->scoreOvlSpr[3].sprId[1] = -1;
            ovl->mode = 0;
            break;
    }
    for(i=0; i<3; i++) {
        sprNum = &ovl->digitSprId[i][0];
        for(j=0; j<20; j++, sprNum++) {
            if(sprNum->sprId[0] >= 0) {
                espDispOff(sprNum->sprId[0]);
            }
            if(sprNum->sprId[1] >= 0) {
                espDispOff(sprNum->sprId[1]);
            }
        }
    }
    sprNum = &ovl->dashSprId;
    if (sprNum->sprId[0] >= 0) {
        espDispOff(sprNum->sprId[0]);
    }
    if (sprNum->sprId[1] >= 0) {
        espDispOff(sprNum->sprId[1]);
    }
    M433ScoreOvlSprUpdate(&ovl->scoreOvlSpr[0]);
    M433ScoreOvlSprUpdate(&ovl->scoreOvlSpr[1]);
    M433ScoreOvlSprUpdate(&ovl->scoreOvlSpr[2]);
    M433ScoreOvlSprUpdate(&ovl->scoreOvlSpr[3]);
    ovl->time++;
}

void M433ScoreNextSet(M433_SCOREOVL *ovl, unsigned int leftScore, unsigned int rightScore, int winGrp)
{
    ovl->mode = 1;
    ovl->scorePrev[0] = ovl->score[0];
    ovl->scorePrev[1] = ovl->score[1];
    ovl->score[0] = leftScore;
    ovl->score[1] = rightScore;
    ovl->winGrp = winGrp;
}

void M433BallContactSet(HuVecF *pos, unsigned int type, unsigned int side, unsigned int cel)
{
    M433_BALLWORK *ball = ballObj->data;
    static int modeTbl[9] = {
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9
    };
    static BOOL contactTbl[9][3] = {
        FALSE, FALSE, FALSE,
        FALSE, FALSE, FALSE,
        TRUE, TRUE, FALSE,
        TRUE, FALSE, TRUE,
        TRUE, FALSE, TRUE,
        TRUE, TRUE, FALSE,
        TRUE, TRUE, FALSE,
        TRUE, FALSE, TRUE,
        TRUE, FALSE, TRUE
    };
    static unsigned int targetTimerTbl[9] = {
        0,
        1,
        2,
        1,
        1,
        2,
        1,
        1,
        1
    };
    ball->mode = 0;
    if(type < 9) {
        ball->mode = modeTbl[type];
        if(contactTbl[type][0]) {
            M433_GAMEMAN *gameman = gameManObj->data;
            gameman->bContact = TRUE;
            gameman->uContactGrp = side;
            gameman->uContactCel = cel;
            gameman->uTargetGrp = side;
            if(contactTbl[type][1]) {
                gameman->uTargetGrp = 1-gameman->uTargetGrp;
            }
            if(contactTbl[type][2]) {
                gameman->uContactCnt++;
            }
        }
    }
    if(type == 5) {
        int mdlNo = 6;
        if(ball->type == 5) {
            mdlNo++;
        }
        Hu3DModelPosSetV(ballObj->mdlId[mdlNo], &ball->pos);
        Hu3DModelDispOn(ballObj->mdlId[mdlNo]);
        Hu3DMotionTimeSet(ballObj->mdlId[mdlNo], 0);
        Hu3DMotionShapeTimeSet(ballObj->mdlId[mdlNo], 0);
    } else if(type != 2) {
        ball->trail = 1;
    }
    ball->contactF = TRUE;
    ball->targetUpdateF = FALSE;
    ball->killF = FALSE;
    ball->ballGroundF = FALSE;
    ball->type = type;
    ball->contactGrp = side;
    ball->contactCel = cel;
    if(targetTimerTbl[type]) {
        ball->targetTimer = targetTimerTbl[type];
    } else if(ball->targetTimer) {
        ball->targetTimer--;
    }
    ball->posNext = *pos;
    if(type == 0) {
        ball->pos = ball->posNext;
    }
    ball->target.radiusMax = 1;
    ball->target.radiusMax = 0;
}

void M433BallExplodeSet(void)
{
    M433_BALLWORK *ball = ballObj->data;
    ball->fire = TRUE;
    ball->mode = 11;
}

void M433StageCollide(HuVecF *pos, unsigned int side)
{
    float border = 44;
    if(pos->z-border < -500) {
        pos->z = -500+border;
    } else if(pos->z+border > 500) {
        pos->z = 500-border;
    }
    if(side == 0) {
        pos->x *= -1;
    }
    if(pos->x-border < 20) {
        pos->x = 20+border;
    } else if(pos->x+border > 800) {
        pos->x = 800-border;
    }
    if(side == 0) {
        pos->x *= -1;
    }
}

void M433BallServeSet(float angle, float mag, unsigned int side, unsigned int scaleF, HuVecF *pos)
{
    M433_BALLWORK *ball = ballObj->data;
    if(mag > 1) {
        mag = 1;
    }
    if(ball->cursorMode == 0) {
        ball->cursorAPos.x = 465;
        ball->cursorAPos.y = 11;
        ball->cursorAPos.z = 0;
        if(side) {
            ball->cursorAPos.x *= -1;
        }
        ball->cursorMode = 1;
        ball->airTime = 0;
        ball->cursorARot = 0;
        ball->timer = 0;
        ball->trail = 1;
    }
    if(scaleF) {
        ball->cursorMode = 2;
        mag *= 1.5f;
    }
    ball->cursorAF = TRUE;
    ball->cursorAPos.x += mag*HuSin(angle)*800*(1.0f/60.0f);
    ball->cursorAPos.z += mag*HuCos(angle)*800*(1.0f/60.0f);
    if(side) {
        ball->cursorAPos.x *= -1;
    }
    if(ball->cursorAPos.x < 230) {
        ball->cursorAPos.x = 230;
    }
    if(side) {
        ball->cursorAPos.x *= -1;
    }
    M433StageCollide(&ball->cursorAPos, 1-side);
    *pos = ball->cursorAPos;
}

BOOL M433BallCoordGet(HuVecF *pos)
{
    M433_BALLWORK *ball = ballObj->data;
    pos->x = pos->y = pos->z = 0;
    if(ball->cursorMode == 0) {
        return FALSE;
    }
    *pos = ball->cursorAPos;
    pos->x = fabsf(pos->x);
    pos->x -= 465;
    pos->x *= 0.004255319f;
    pos->z *= 0.0028571428f;
    return TRUE;
}

void M433BallTrailStart(void)
{
    M433_BALLWORK *ball = ballObj->data;
    ball->trail = 1;
}

float M433BallTrailGet(void)
{
    M433_BALLWORK *ball = ballObj->data;
    return ball->trail;
}

unsigned int M433ServeSideGet(HuVecF *pos)
{
    M433_BALLWORK *ball = ballObj->data;
    *pos = ball->posNext;
    return (ball->posNext.x >= 0) ? 1 : 0;
}

unsigned int M433BallSideGet(HuVecF *pos)
{
    M433_BALLWORK *ball = ballObj->data;
    if(ball->cursorMode) {
        *pos = ball->cursorAPos;
    } else {
        *pos = ball->posNext;
    }
    return (pos->x >= 0) ? 1 : 0;
}

float M433ServeTargetTimeGet(void)
{
    M433_BALLWORK *ball = ballObj->data;
    return ball->target.radiusMax-ball->target.radius;
}

float M433BallTargetTimeGet(void)
{
    M433_BALLWORK *ball = ballObj->data;
    if(ball->cursorMode) {
        return 1.5f;
    } else {
        return ball->target.radiusMax-ball->target.radius;
    }
}

unsigned int M433BallAirTimeGet(void)
{
    M433_BALLWORK *ball = ballObj->data;
    if(ball->cursorMode) {
        return ball->airTime;
    } else {
        return 10000;
    }
}

unsigned int M433BallTimeGet(void)
{
    M433_BALLWORK *ball = ballObj->data;
    return ball->timer;
}

float M433BallPointGet(float y)
{
    M433_BALLWORK *ball = ballObj->data;
    M433_BALLTARGET *target = &ball->target;
    float invT = 1/target->colTime;
    float h = -target->colHeight*invT;
    float maxY = (h*((0.5f*target->colTime)*h))+(target->colHeight*h)+target->pos.y;
    if(y > maxY) {
        return -1;
    }
    h = invT*(-target->colHeight-sqrtf((target->colHeight*target->colHeight)-((2*target->colTime)*(target->pos.y-y))));
    h -= target->radius;
    return h;
}

float M433BallPointCheck(float radius, HuVecF *pos)
{
    M433_BALLWORK *ball = ballObj->data;
    M433_BALLTARGET *target = &ball->target;
    float targetRadius = target->radius;
    float totalRadius;
    float colR;
    float colY;
    HuVecF move;
    HuVecF dir;
    HuVecF tempPos;
    if(radius != 0.0f) {
        totalRadius = targetRadius+radius;
        if(totalRadius < 0) {
            totalRadius = 0;
        }
        if(totalRadius > target->radiusMax) {
            totalRadius = target->radiusMax;
        }
        target->radius = totalRadius;
        if(target->dist > 0) {
            VECSubtract(&target->posNext, &target->pos, &dir);
            dir.y = 0;
            VECNormalize(&dir, &dir);
        } else {
            dir.x = dir.y = dir.z = 0;
        }
        tempPos = *pos;
        colR = (totalRadius*(0.5f*target->colTimeRad*totalRadius))+(target->colRadius*totalRadius);
        colY = (totalRadius*(0.5f*target->colTime*totalRadius))+(target->colHeight*totalRadius);
        pos->x = target->pos.x+(dir.x*colR);
        pos->y = target->pos.y+colY;
        pos->z = target->pos.z+(dir.z*colR);
        VECSubtract(pos, &tempPos, &move);
        totalRadius >= target->radiusMax;
        target->radius = targetRadius;
    } else {
        *pos = ball->pos;
    }
    return targetRadius+radius;
}

static inline float M433BallPointCheck2(float radius, HuVecF *pos)
{
    float colR;
    M433_BALLWORK *ball = ballObj->data;
    M433_BALLTARGET *target = &ball->target;
    float targetRadius = target->radius;
    float totalRadius;
    float colY;
    HuVecF tempPos;
    HuVecF dir;
    HuVecF move;
    M433_BALLTARGET *target2;
    if(radius != 0.0f) {
        totalRadius = targetRadius+radius;
        target2 = target;
        if(totalRadius < 0) {
            totalRadius = 0;
        }
        if(totalRadius > target2->radiusMax) {
            totalRadius = target2->radiusMax;
        }
        target2->radius = totalRadius;
        if(target2->dist > 0) {
            VECSubtract(&target2->posNext, &target2->pos, &dir);
            dir.y = 0;
            VECNormalize(&dir, &dir);
        } else {
            dir.x = dir.y = dir.z = 0;
        }
        tempPos = *pos;
        colR = (totalRadius*(0.5f*target2->colTimeRad*totalRadius))+(target2->colRadius*totalRadius);
        colY = (totalRadius*(0.5f*target2->colTime*totalRadius))+(target2->colHeight*totalRadius);
        pos->x = target2->pos.x+(dir.x*colR);
        pos->y = target2->pos.y+colY;
        pos->z = target2->pos.z+(dir.z*colR);
        VECSubtract(pos, &tempPos, &move);
        totalRadius >= target2->radiusMax;
        target->radius = targetRadius;
    } else {
        *pos = ball->pos;
    }
    return targetRadius+radius;
}

BOOL M433BallJumpCheck(HuVecF *pos)
{
    M433_BALLWORK *ball = ballObj->data;
    HuVecF dir;
    HuVecF ballPos;
    M433BallSideGet(&ballPos);
    VECSubtract(&ballPos, &ball->pos, &dir);
    if(ballPos.x*ball->pos.x >= 0) {
        return FALSE;
    }
    if(ball->pos.x == 0 || dir.x == 0) {
        return FALSE;
    } else {
        float scale = fabsf(ball->pos.x/dir.x);
        VECScale(&dir, &dir, scale);
        VECAdd(&ball->pos, &dir, pos);
        return TRUE;
    }
}

BOOL M433BallSlipCheck(float radius, HuVecF *pos)
{
    HuVecF outDir;
    M433_BALLWORK *ball = ballObj->data;
    HuVecF outPos = { 0, 0, 0 };
    float slip;
    M433BallPointCheck2(radius, &outPos);
    VECSubtract(&outPos, pos, &outDir);
    slip = VECMag(&outDir);
    return slip < ball->ballSlip;
}

unsigned int M433GameEventGet(void)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return gameman->uGameEvent;
}

unsigned int M433GameTimerGet(void)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return gameman->uTime;
}

unsigned int M433EventGet(int side)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return gameman->auEvents[side & 0x1];
}

unsigned int M433GameWinGet(int side)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return gameman->abGameWin[side & 0x1];
}

BOOL M433ServeGroupCheck(int side)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return gameman->uServeGrp == (side & 0x1);
}

unsigned int M433ServeCelGet(int side)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return gameman->auServeCel[side & 0x1];
}

BOOL M433PointGroupCheck(int side)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return gameman->uPointGrp == (side & 0x1);
}

BOOL M433WinGroupCheck(int side)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return gameman->sWinGrp == (side & 0x1);
}

unsigned int M433ContactCountGet(void)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return gameman->uContactCnt;
}

BOOL M433ContactCheck(unsigned int side, unsigned int cel)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return (gameman->uContactGrp == (side & 0x1) && gameman->uContactCel == (cel & 0x1)) == FALSE;
}

BOOL M433ContactGroupCheck(unsigned int side)
{
    M433_GAMEMAN *gameman = gameManObj->data;
    return gameman->uContactGrp == (side & 0x1);
}

float M433CosThetaGet(HuVecF *a, HuVecF *b, HuVecF *dot)
{
    float cosTheta;
    float mag2;
    HuVecF temp;
    VECSubtract(b, a, &temp);
    cosTheta = VECDotProduct(&temp, dot);
    mag2 = -VECSquareMag(dot);
    if(mag2 != 0.0f) {
        cosTheta /= mag2;
    }
    return cosTheta;
}

float M433PlaneDistGet(HuVecF *pos1, HuVecF *point1, HuVecF *pos2, HuVecF *point2)
{
    HuVecF outPos;
    HuVecF ofs1, ofs2;
    VECSubtract(point1, pos1, &ofs1);
    VECSubtract(point2, pos2, &ofs2);
    VECSubtract(&ofs1, &ofs2, &ofs1);
    if(VECMag(&ofs1) <= 0) {
        outPos = *pos1;
    } else {
        float cosTheta = M433CosThetaGet(pos2, pos1, &ofs1);
        if(cosTheta < 0) {
            cosTheta = 0;
        } else if(cosTheta > 1) {
            cosTheta = 1;
        }
        VECScale(&ofs1, &ofs2, cosTheta);
        VECAdd(pos1, &ofs2, &outPos);
    }
    VECSubtract(pos2, &outPos, &ofs1);
    return VECMag(&ofs1);
}