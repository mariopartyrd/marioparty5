#include "game/board/mgcall.h"
#include "game/board/status.h"
#include "game/board/guide.h"
#include "game/board/masu.h"
#include "game/board/model.h"
#include "game/board/camera.h"
#include "game/board/tutorial.h"
#include "game/board/audio.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/board/pad.h"

#include "game/mgdata.h"
#include "game/wipe.h"
#include "game/esprite.h"

#include "datanum/bbattle.h"
#include "messnum/board_battle.h"
#include "messnum/mgname.h"
#include "messnum/board_tutorial.h"

static const HuVec2F StatusPos4PBase[GW_PLAYER_MAX] = {
    { 176, 184 },
    { 400, 184 },
    { 176, 296 },
    { 400, 296 },
};

static const HuVec2F StatusPos1Vs3Base[GW_PLAYER_MAX] = {
    { 176, 240 },
    { 400, 160 },
    { 400, 240 },
    { 400, 320 },
};

static const HuVec2F StatusPos2Vs2Base[GW_PLAYER_MAX] = {
    { 176, 200 },
    { 176, 280 },
    { 400, 200 },
    { 400, 280 },
};

static const HuVec2F StatusPosKettou[2] = {
    { 152, 240 },
    { 424, 240 }
};

static OMOBJ *MgListObj[4];

static HuVec2F MgStatusPos1Vs3[GW_PLAYER_MAX];
static HuVec2F MgStatusPos2Vs2[GW_PLAYER_MAX];
static HuVec2F MgStatusPos4P[GW_PLAYER_MAX];


static s16 MgPlayListKettou[14];
static s16 MgPlayList1Vs3[3];
static s16 MgPlayList2Vs2[3];
static s16 MgPlayListBattle[2];
static s16 MgPlayList4P[8];

static s16 MgPlayListOfsTbl[MG_TYPE_MAX];

static unsigned int MgCallDir = -1;

static s16 *MgCallPlayTbl[MG_TYPE_MAX] = {
    MgPlayList4P,
    MgPlayList1Vs3,
    MgPlayList2Vs2,
    MgPlayListBattle,
    NULL,
    NULL,
    MgPlayListKettou,
    NULL,
    NULL,
};

static int MgCallPlayNumTbl[MG_TYPE_MAX] = {
    8,
    3,
    3,
    2,
    0,
    0,
    14,
    0,
    0
};

static int MgCallTypeFileTbl[4] = {
    BOARD_ANM_mgCall4P,
    BOARD_ANM_mgCall1Vs3,
    BOARD_ANM_mgCall2Vs2,
    BOARD_ANM_mgCallKettou
};

static s16 MgCallFocusNo;
static HUPROCESS *MgCallProc;

void MBMgCallInit(void)
{
    int i;
    for(i=0; i<MG_TYPE_MAX; i++) {
        MgPlayListOfsTbl[i] = 0;
    }
    memset(&MgPlayList4P[0], 0, sizeof(MgPlayList4P));
    memset(&MgPlayList1Vs3[0], 0, sizeof(MgPlayList1Vs3));
    memset(&MgPlayList2Vs2[0], 0, sizeof(MgPlayList1Vs3));
    memset(&MgPlayListBattle[0], 0, sizeof(MgPlayListBattle));
    memset(&MgPlayListKettou[0], 0, sizeof(MgPlayListKettou));
    memcpy(&MgStatusPos4P[0], &StatusPos4PBase[0], GW_PLAYER_MAX*sizeof(HuVec2F));
    memcpy(&MgStatusPos2Vs2[0], &StatusPos2Vs2Base[0], GW_PLAYER_MAX*sizeof(HuVec2F));
    memcpy(&MgStatusPos1Vs3[0], &StatusPos1Vs3Base[0], GW_PLAYER_MAX*sizeof(HuVec2F));
}

static void MgCallMain(void);
static void MgCallKill(void);

void MBMgCallExec(void)
{
    MgCallProc = MBPrcCreate(MgCallMain, 8198, 24576);
    MBPrcDestructorSet(MgCallProc, MgCallKill);
    while(MgCallProc) {
        HuPrcVSleep();
    }
}

typedef struct MgCallWork_s {
    int type;
    s16 sprId[7];
    s16 unk12[2];
    MBMODELID guideMdlId;
    MBMODELID battleMdlId[3];
} MGCALL_WORK;

static int MgPlayerGroupUpdate(int *colorIn);

static int MgCallBattleExec(MGCALL_WORK *workP);
static int MgListExec(int type);
static void MgCallNext(int type, int no);

static void MgCallMain(void)
{
    Mtx explodeMtx;
    MGCALL_WORK work;
    HuVecF pos;
    HuVecF explodePos2D;
    HuVecF explodePos3D;
    HuVec2F statusPos;
    
    MGCALL_WORK *workP = &work;
    int i;
    int no;
    BOOL battleF = FALSE;
    MBCAMERA *cameraP = MBCameraGet();
    int masuId;
    
    float scale;
    float weight;
    
    for(i=0; i<7; i++) {
        workP->sprId[i] = -1;
    }
    for(i=0; i<3; i++) {
        workP->battleMdlId[i] = MB_MODEL_NONE;
    }
    if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            MBStatusCapsuleDispSet(i, FALSE);
        }
    }
    if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
        workP->guideMdlId = MBGuideCreateDirect();
        masuId = MBMasuFlagFind(MASU_LAYER_DEFAULT, MASU_FLAG_START, MASU_FLAG_START);
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
        pos.y += 250;
        MBModelPosSet(workP->guideMdlId, pos.x, pos.y, pos.z);
        MBCameraSkipSet(FALSE);
        MBCameraModelViewSet(workP->guideMdlId, NULL, NULL, 1800, -1, 21);
        MBCameraMotionWait();
        MBCameraFocusSet(MB_MODEL_NONE);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
        }
        WipeWait();
    } else {
        workP->guideMdlId = MB_MODEL_NONE;
    }
    MBAudFXPlay(MSM_SE_BOARD_107);
    if(!GWTeamFGet()) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            MBStatusPosOnGet(i, (HuVecF *)&statusPos);
            MBStatusMoveSet(i, (HuVecF *)&statusPos, (HuVecF *)&MgStatusPos4P[i], STATUS_MOVE_SIN, 15);
        }
    } else {
        for(i=0; i<2; i++) {
            MBStatusPosOnGet(i, (HuVecF *)&statusPos);
            MBStatusNoMoveSet(i, (HuVecF *)&statusPos, (HuVecF *)&StatusPosKettou[i], STATUS_MOVE_SIN, 15);
        }
    }
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    workP->type = MgPlayerGroupUpdate(NULL);
    workP->sprId[2] = espEntry(BOARD_ANM_mgCallVs, 100, 0);
    espPosSet(workP->sprId[2], 288, 240);
    MBAudFXPlay(MSM_SE_BOARD_108);
    for(i=0; i<30; i++) {
        weight = i/30.0f;
        scale = 1.5+HuSin(weight*360);
        if(weight < 0.2f) {
            scale *= (weight/0.2f);
        }
        espScaleSet(workP->sprId[2], scale, scale);
        HuPrcVSleep();
    }
    HuPrcSleep(15);
    workP->sprId[0] = espEntry(MgCallTypeFileTbl[workP->type], 100, 0);
    espPosSet(workP->sprId[0], 288, -32);
    espDrawNoSet(workP->sprId[0], 32);
    for(i=0; i<30u; i++) {
        weight = i/30.0f;
        espPosSet(workP->sprId[0], 288, 88-(120*HuSin((1-weight)*90.0f)));
        HuPrcVSleep();
    }
    if(!_CheckFlag(FLAG_MG_CIRCUIT) && workP->type == 0 && frandmod(100) < 10 && MBPlayerMaxCoinGet() >= 10) {
        battleF = TRUE;
        no = MgCallBattleExec(workP);
    } else {
        for(i=0; i<60; i++) {
            weight = i/60.0f;
            scale = 1.5f+(0.8f*HuSin(weight*(720+(720*weight))));
            espScaleSet(workP->sprId[2], scale, scale);
            HuPrcVSleep();
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            MBStatusPosGet(i, (HuVecF *)&statusPos);
            if(statusPos.x  >= 288) {
                statusPos.x = 704;
            } else {
                statusPos.x = -128;
            }
            MBStatusMoveSet(i, NULL, (HuVecF *)&statusPos, STATUS_MOVE_SIN, 15);
        }
        workP->sprId[3] = MBModelCreate(BOARD_HSF_effExplode, NULL, FALSE);
        MBModelLayerSet(workP->sprId[3], 6);
        explodePos2D.x = 288;
        explodePos2D.y = 240;
        explodePos2D.z = 500;
        MB2Dto3D(&explodePos2D, &explodePos3D);
        MTXLookAt(explodeMtx, &cameraP->pos, &cameraP->up, &explodePos3D);
        MTXInverse(explodeMtx, explodeMtx);
        explodeMtx[0][3] = explodeMtx[1][3] = explodeMtx[2][3] = 0;
        MBModelMtxSet(workP->sprId[3], &explodeMtx);
        MBModelPosSetV(workP->sprId[3], &explodePos3D);
        espKill(workP->sprId[2]);
        workP->sprId[2] = -1;
        MBAudFXPlay(MSM_SE_BOARD_109);
        for(i=0; i<30; i++) {
            weight = i/30.0f;
            scale = (1+(5.0f*weight))+(0.5*HuSin(16*(weight*360)));
            scale *= 0.4f;
            MBModelScaleSet(workP->sprId[3], scale, scale, scale);
            if(i >= 25) {
                MBModelAlphaSet(workP->sprId[3], 255*HuCos(((i-25)/5.0f)*90.0f));
            }
            if(_CheckFlag(FLAG_MG_CIRCUIT)) {
                espPosSet(workP->sprId[0], 288, 88-(300*HuSin(weight*90)));
            }
            HuPrcVSleep();
        }
        MBModelKill(workP->sprId[3]);
        workP->sprId[3] = MB_MODEL_NONE;
        while(!MBStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        no = MgListExec(workP->type);
        _SetFlag(FLAG_BOARD_MG);
    }
    GwSystem.subGameNo = -1;
    MgCallNext(workP->type, no);
    for(i=0; i<7; i++) {
        if(workP->sprId[i] >= 0) {
            espKill(workP->sprId[i]);
        }
    }
    if(workP->guideMdlId >= 0) {
        MBGuideEnd(workP->guideMdlId);
    }
    if(battleF) {
        for(i=0; i<3; i++) {
            if(workP->battleMdlId[i] >= 0) {
                MBModelKill(workP->battleMdlId[i]);
            }
        }
    }
    HuPrcEnd();
}

static void MgCallKill(void)
{
    MgCallProc = NULL;
}

static int MgTypeNextGet(int *color);

static int MgPlayerGroupUpdate(int *colorIn)
{
    int i, j;
    int statusNo;
    int type;
    int colorP1;
    int playerNum;
    BOOL colorChange;
    
    int origColor[GW_PLAYER_MAX];
    int colorTbl[GW_PLAYER_MAX];
    
    for(i=0; i<GW_PLAYER_MAX; i++) {
        GwPlayer[i].grpBackup = MBPlayerGrpGet(i);
    }
    if(!GWTeamFGet()) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            origColor[i] = MBStatusColorGet(i);
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX/2; i++) {
            origColor[i] = (i == 0) ? STATUS_COLOR_RED : STATUS_COLOR_BLUE;
        }
    }
    if(colorIn) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            MBStatusColorSet(i, colorIn[i]);
            colorTbl[i] = colorIn[i];
        }
    }
    type = MgTypeNextGet(colorTbl);
    if(!GWTeamFGet()) {
        playerNum = 4;
    } else {
        if(type == MG_TYPE_1VS3) {
            type = (frandf() < 0.5f) ? MG_TYPE_2VS2 : MG_TYPE_4P;
        }
        
        if(type == MG_TYPE_4P) {
            colorTbl[0] = STATUS_COLOR_RED;
            colorTbl[1] = STATUS_COLOR_RED;
        } else {
            colorTbl[0] = STATUS_COLOR_RED;
            colorTbl[1] = STATUS_COLOR_BLUE;
        }
        playerNum = 2;
    }
    colorChange = FALSE;
    for(i=0; i<playerNum; i++) {
        if(origColor[i] != colorTbl[i]) {
            colorChange = TRUE;
        }
    }
    if(colorChange) {
        MBAudFXPlay(MSM_SE_BOARD_127);
        for(i=0; i<60u; i++) {
            for(j=0; j<playerNum; j++) {
                if(origColor[j] != colorTbl[j]) {
                    MBStatusRainbowSet(j, i/60.0f, colorTbl[j]);
                }
            }
            HuPrcVSleep();
        }
    }
    
    HuPrcSleep(30);
    if(GWTeamFGet()) {
        switch(type) {
            case MG_TYPE_2VS2:
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    GwPlayerConf[i].grp = MBPlayerGrpGet(i);
                }
                break;
            
            case MG_TYPE_4P:
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    GwPlayerConf[i].grp = 0;
                }
                break;
        }
        return type;
    } else {
        switch(type) {
            case MG_TYPE_1VS3:
                j=0;
                colorP1 = colorTbl[0];
                for(i=1; i<GW_PLAYER_MAX; i++) {
                    if(colorP1 == colorTbl[i]) {
                        j |= (1 << i);
                    }
                }
                if(j == 0) {
                    i = 0;
                } else {
                    for(i=1; i<GW_PLAYER_MAX; i++) {
                        if((j & (1 << i)) == 0) {
                            break;
                        }
                    }
                }
                colorP1 = colorTbl[i];
                statusNo = 0;
                MBStatusMoveSet(i, NULL, (HuVecF *)&MgStatusPos1Vs3[statusNo++], STATUS_MOVE_SIN, 15);
                GwPlayerConf[i].grp = 0;
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    if(colorP1 != colorTbl[i]) {
                        MBStatusMoveSet(i, NULL, (HuVecF *)&MgStatusPos1Vs3[statusNo++], STATUS_MOVE_SIN, 15);
                        GwPlayerConf[i].grp = 1;
                    }
                }
                break;
            
            case MG_TYPE_2VS2:
                statusNo = 0;
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    if(colorTbl[i] == STATUS_COLOR_RED) {
                        MBStatusMoveSet(i, NULL, (HuVecF *)&MgStatusPos2Vs2[statusNo++], STATUS_MOVE_SIN, 15);
                        GwPlayerConf[i].grp = 0;
                    }
                }
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    if(colorTbl[i] == STATUS_COLOR_BLUE) {
                        MBStatusMoveSet(i, NULL, (HuVecF *)&MgStatusPos2Vs2[statusNo++], STATUS_MOVE_SIN, 15);
                        GwPlayerConf[i].grp = 1;
                    }
                }
                break;
        }
        while(!MBStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        return type;
    }
}

static int MgTypeNextGet(int *color)
{
    int i;
    int no;
    int blueNum;
    int redNum;
    int randomNum;
    BOOL preferRedF;
    int playerNum;
    int chanceTotal;
    int type;
    int hatenaNum;
    BOOL sameGrpF;
    int index1, index2;
    
    int randomTypeTbl[MG_TYPE_MAX];
    
    int totalChanceTbl[MG_TYPE_MAX];
    int redPlayerTbl[GW_PLAYER_MAX];
    int bluePlayerTbl[GW_PLAYER_MAX];
    int hatenaPlayerTbl[GW_PLAYER_MAX];
    
    int chance;
    
    playerNum = 0;
    hatenaNum = 0;
    blueNum = redNum = 0;
    sameGrpF = TRUE;

    for(i=0; i<GW_PLAYER_MAX; i++) {
        switch(MBStatusColorGet(i)) {
            case STATUS_COLOR_BLUE:
                bluePlayerTbl[blueNum++] = i;
                if(redNum) {
                    sameGrpF = FALSE;
                }
                playerNum++;
                break;
                
            case STATUS_COLOR_RED:
                redPlayerTbl[redNum++] = i;
                if(blueNum) {
                    sameGrpF = FALSE;
                }
                playerNum++;
                break;
            
            default:
                hatenaPlayerTbl[hatenaNum++] = i;
                break;
        }
    }
    if(playerNum >= GW_PLAYER_MAX) {
        if(sameGrpF) {
            type = MG_TYPE_4P;
        } else if(blueNum == redNum) {
            type = MG_TYPE_2VS2;
        } else {
            type = MG_TYPE_1VS3;
        }
    } else {
        randomNum = 0;
        if(sameGrpF) {
            randomTypeTbl[randomNum++] = MG_TYPE_4P;
            if(playerNum < GW_PLAYER_MAX/2) {
                randomTypeTbl[randomNum++] = MG_TYPE_2VS2;
            }
        } else {
            randomTypeTbl[randomNum++] = MG_TYPE_2VS2;
        }
        randomTypeTbl[randomNum++] = MG_TYPE_1VS3;
        chanceTotal = 0;
        if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
            static int chanceTbl[3] = { 60, 10, 30 };
            for(i=0; i<randomNum; i++) {
                chanceTotal += chanceTbl[randomTypeTbl[i]];
                totalChanceTbl[i] = chanceTotal;
            }
        } else {
            static int chanceTbl[3] = { 30, 40, 30 };
            for(i=0; i<randomNum; i++) {
                chanceTotal += chanceTbl[randomTypeTbl[i]];
                totalChanceTbl[i] = chanceTotal;
            }
        }
        chance = frandmod(chanceTotal);
        for(i=0; i<randomNum; i++) {
            if(chance < totalChanceTbl[i]) {
                break;
            }
        }
        type = randomTypeTbl[i];
        for(i=0; i<100; i++) {
            index1 = frandmod(hatenaNum);
            index2 = frandmod(hatenaNum);
            no = hatenaPlayerTbl[index1];
            hatenaPlayerTbl[index1] = hatenaPlayerTbl[index2];
            hatenaPlayerTbl[index2] = no;
        }
        switch(type) {
            case MG_TYPE_4P:
                if(playerNum == 0) {
                    preferRedF = (frandf() < 0.5f) ? FALSE : TRUE;
                } else {
                    if(blueNum != 0) {
                        preferRedF = FALSE;
                    } else {
                        preferRedF = TRUE;
                    }
                }
                no = 0;
                if(!preferRedF) {
                    for(i=blueNum; i<GW_PLAYER_MAX; i++) {
                        bluePlayerTbl[blueNum++] = hatenaPlayerTbl[no++];
                    }
                } else {
                    for(i=redNum; i<GW_PLAYER_MAX; i++) {
                        redPlayerTbl[redNum++] = hatenaPlayerTbl[no++];
                    }
                }
                break;
            
            case MG_TYPE_1VS3:
                if(playerNum == 0) {
                    preferRedF = (frandf() < 0.5f) ? FALSE : TRUE;
                } else {
                    if(blueNum >= 2) {
                        preferRedF = FALSE;
                    } else if(redNum >= 2) {
                        preferRedF = TRUE;
                    } else {
                        preferRedF = (frandf() < 0.5f) ? FALSE : TRUE;
                    }
                }
                no = 0;
                if(!preferRedF) {
                    for(i=blueNum; i<GW_PLAYER_MAX-1; i++) {
                        bluePlayerTbl[blueNum++] = hatenaPlayerTbl[no++];
                    }
                    for(i=redNum; i<1; i++) {
                        redPlayerTbl[redNum++] = hatenaPlayerTbl[no++];
                    }
                } else {
                    for(i=blueNum; i<1; i++) {
                        bluePlayerTbl[blueNum++] = hatenaPlayerTbl[no++];
                    }
                    for(i=redNum; i<GW_PLAYER_MAX-1; i++) {
                        redPlayerTbl[redNum++] = hatenaPlayerTbl[no++];
                    }
                }
                break;
            
            case MG_TYPE_2VS2:
                no = 0;
                for(i=blueNum; i<GW_PLAYER_MAX/2; i++) {
                    bluePlayerTbl[blueNum++] = hatenaPlayerTbl[no++];
                }
                for(i=redNum; i<GW_PLAYER_MAX/2; i++) {
                    redPlayerTbl[redNum++] = hatenaPlayerTbl[no++];
                }
                break;
        }
        
    }
    for(i=0; i<blueNum; i++) {
        color[bluePlayerTbl[i]] = STATUS_COLOR_BLUE;
    }
    for(i=0; i<redNum; i++) {
        color[redPlayerTbl[i]] = STATUS_COLOR_RED;
    }
    return type;
}

typedef struct MgListWork_s {
    unsigned killF : 1;
    unsigned slideF : 1;
    unsigned mode : 3;
    unsigned no : 3;
    unsigned dispF : 1;
    s16 winNo;
    s16 sprId;
    s16 time;
    s16 maxTime;
    int hiddenMes;
} MGLISTWORK;

static int MgListGet(int type, int height, s16 *noTbl);
static void MgListObjExec(OMOBJ *obj);
static void MgListCenterStart(OMOBJ *obj);
static BOOL MgListSlideCheck(OMOBJ *obj);
static void MgListFocusSet(OMOBJ *obj);
static void MgListObjKill(OMOBJ *obj);
static void MgListTopSet(OMOBJ *obj);

static s16 MgNameColorGet(u32 nameMes);

static int MgListExec(int type)
{
    static int heightTbl[MG_TYPE_MAX] = {
        4,
        3,
        3,
        3,
        3,
        0,
        3,
        0,
        3
    };
    int i;
    MGLISTWORK *work;
    int height;
    int delay;
    MGDATA *data;
    int nextMaxTime;
    int nextTime;
    int speed;
    int nextDelay;
    int focusNo;
    int focusOfs;
    int nameColor;
    
    s16 noTbl[128];
    
    height = heightTbl[type];
    height = MgListGet(type, height, noTbl);
    for(i=0; i<4; i++) {
        MgListObj[i] = NULL;
    }
    for(i=0; i<height; i++) {
        data = &mgDataTbl[noTbl[i]];
        MgListObj[i] = MBAddObj(257, 0, 0, MgListObjExec);
        work = omObjGetWork(MgListObj[i], MGLISTWORK);
        work->dispF = TRUE;
        work->no = i;
        MgListObj[i]->trans.y = 296.0f-(((height-1)*72)/2)+(i*72);
        if(GWMgUnlockGet(noTbl[i]+GW_MGNO_BASE)) {
            work->winNo = MBWinCreateHelp(data->nameMes);
            work->hiddenMes = -1;
        } else {
            work->winNo = MBWinCreateHelp(MESSNUM_PTR("\xC3\xC3\xC3\xC3\xC3\xC3\xC3"));
            work->hiddenMes = data->nameMes;
        }
        MBWinPriSet(work->winNo, 90);
        nameColor = MgNameColorGet(data->nameMes);
        if(nameColor != 6) {
            work->sprId = espEntry(BOARD_ANM_mgCallList, 100, 0);
            espDrawNoSet(work->sprId, 32);
        } else {
            work->sprId = espEntry(BOARD_ANM_mgCallListCoin, 100, 0);
            espDrawNoSet(work->sprId, 32);
        }
        MgListCenterStart(MgListObj[i]);
    }
    MgCallFocusNo = -1;
    while(!MgListSlideCheck(MgListObj[0])) {
        HuPrcVSleep();
    }
    MgCallFocusNo = 0;
    focusNo = frandmod(height);
    delay = frandmod(30)+90;
    focusOfs = (-7+sqrtf((delay*8.0f)+49))/2;
    speed = (focusNo-focusOfs)%height;
    if(speed < 0) {
        speed += height;
    }
    nextDelay = (height*12)+(speed*4);
    nextTime = nextMaxTime = 4;
    for(i=0; i<nextDelay+delay; i++) {
        if(--nextTime == 0){ 
            MgCallFocusNo = (MgCallFocusNo+1)%height;
            if(i > nextDelay) {
                nextMaxTime++;
                nextTime = nextMaxTime;
            } else {
                nextTime = nextMaxTime;
            }
            MBAudFXPlay(MSM_SE_BOARD_05);
        }
        HuPrcVSleep();
    }
    MgCallFocusNo = focusNo;
    MgListFocusSet(MgListObj[MgCallFocusNo]);
    MBAudFXPlay(MSM_SE_BOARD_130);
    HuPrcSleep(120);
    if(_CheckFlag(FLAG_MG_CIRCUIT)) {
        if(type != MG_TYPE_KUPA && type != MG_TYPE_KETTOU) {
            for(i=0; i<height; i++) {
                if(i != MgCallFocusNo) {
                    MgListObjKill(MgListObj[i]);
                }
            }
            MgListTopSet(MgListObj[MgCallFocusNo]);
            HuPrcSleep(30);
        }
        
    }
    return noTbl[MgCallFocusNo];
}

static s16 MgNameColorGet(u32 nameMes)
{
    char *mesPtr;
    s16 i;
    if(nameMes > 0x80000000) {
        mesPtr = (char *)nameMes;
    } else {
        mesPtr = HuWinMesPtrGet(nameMes);
    }
    for(i=0; *mesPtr; mesPtr++) {
        if(*mesPtr == 0x1E) {
            return mesPtr[1]-1;
        }
    }
    return 7;
}

typedef struct MgListPack_s {
    int no;
    u8 flag;
} MGLISTPACK;

static BOOL MgCallPlayCheck(int type, s16 no);
static BOOL MgCallCheckValid(int no);

static int MgListGet(int type, int height, s16 *noTbl)
{
    int i;
    int num;
    int pack;
    MGDATA *data;
    int no;
    int callNo;
    int j;
    int order1;
    int order2;
    int temp;
    
    s16 validNo[128];
    
    MGLISTPACK packData[4] = {
        { GW_MGPACK_EASY, MG_FLAG_EASY },
        { GW_MGPACK_ACTION, MG_FLAG_ACTION },
        { GW_MGPACK_SKILL, MG_FLAG_SKILL },
        { GW_MGPACK_GOOFY, MG_FLAG_GOOFY },
    };
    for(pack=0; pack<4; pack++) {
        if(packData[pack].no == GWMgPackGet()) {
            break;
        }
    }
    if(pack >= 4) {
        pack = -1;
    }
    for(no=0, num=0, data = &mgDataTbl[0]; data->ovl != (u16)DLL_NONE; data++, no++) {
        if(data->type == type) {
            if(pack < 0 || (data->flag & packData[pack].flag)) {
                if((!_CheckFlag(FLAG_MG_CIRCUIT) && GWPartyFGet() != FALSE) || (data->flag & MG_FLAG_NOSTORY) == 0) {
                    if(!MgCallPlayCheck(type, no+GW_MGNO_BASE) && MgCallCheckValid(no)) {
                        validNo[num++] = no;
                    }
                }
            }
        }
    }
    if(num < height) {
        no = MgPlayListOfsTbl[type];
        for(i=0; i<MgCallPlayNumTbl[type];) {
            callNo = MgCallPlayTbl[type][no];
            if(callNo) {
                callNo -= GW_MGNO_BASE;
                for(j=0; j<num; j++) {
                    if(callNo == validNo[j]) {
                        break;
                    }
                }
                if(j >= num) {
                    if(pack < 0 || (pack < 4 && (mgDataTbl[callNo].flag & packData[pack].flag))) {
                        validNo[num++] = callNo;
                        if(num >= height) {
                            break;
                        }
                    }
                }
            }
            no = (no+1)%MgCallPlayNumTbl[type];
            i++;
        }
    }
    for(i=0; i<100; i++) {
        order1 = frandmod(num);
        order2 = frandmod(num);
        temp = validNo[order1];
        validNo[order1] = validNo[order2];
        validNo[order2] = temp;
    }
    if(num > height) {
        num = height;
    }
    for(i=0; i<num; i++) {
        noTbl[i] = validNo[i];
    }
    for(i; i<height; i++) {
        noTbl[i] = -1;
    }
    return num;
}

static void MgListObjExec(OMOBJ *obj)
{
    MGLISTWORK *work = omObjGetWork(obj, MGLISTWORK);
    float weight;
    float time;
    HuVec2F winSize;
    
    if(work->killF || MBKillCheck()) {
        MBWinKill(work->winNo);
        espKill(work->sprId);
        MBDelObj(obj);
        return;
    }
    if(work->dispF) {
        espDispOn(work->sprId);
        MBWinDispSet(work->winNo, TRUE);
    } else {
        espDispOff(work->sprId);
        MBWinDispSet(work->winNo, FALSE);
    }
    switch(work->mode) {
        case 0:
            if(work->time > work->maxTime) {
                work->slideF = FALSE;
                work->mode = 1;
            } else {
                weight = (float)(work->time++)/work->maxTime;
                time = HuSin(weight*90);
                if((work->no & 0x1) == 0) {
                    obj->trans.x = -160+(448*time);
                } else {
                    obj->trans.x = 736+(-448*time);
                }
            }
            break;
        
        case 1:
            if(work->no == MgCallFocusNo) {
                work->mode = 2;
                obj->scale.x = obj->scale.y = 1.2f;
            }
            break;
        
        case 2:
            if(work->no != MgCallFocusNo) {
                work->mode = 3;
                work->time = 0;
                work->maxTime = 8;
            }
            break;
        
        case 3:
            if(work->time > work->maxTime) {
                work->mode = 1;
            } else {
                weight = (float)(work->time++)/work->maxTime;
                obj->scale.x = obj->scale.y = (0.2f*HuSin((1-weight)*90))+1;
            }
            break;
        
        case 4:
            if(work->time <= work->maxTime) {
                weight = (float)(work->time++)/work->maxTime;
                
                obj->scale.x = obj->scale.y = (0.2f*HuSin((1-weight)*720))+1;
            }
            break;
        
        case 5:
            if(work->time <= work->maxTime) {
                weight = (float)(work->time++)/work->maxTime;
                obj->trans.y = obj->rot.y+(weight*(96-obj->rot.y));
                obj->scale.x = obj->scale.y = (0.5f*HuCos(weight*90))+0.5f;
            }
            break;
    }
    MBWinScaleSet(work->winNo, obj->scale.x, obj->scale.y);
    espScaleSet(work->sprId, obj->scale.x, obj->scale.y);
    espPosSet(work->sprId, obj->trans.x, obj->trans.y);
    MBWinMesMaxSizeGet(work->winNo, &winSize);
    MBWinPosSet(work->winNo, obj->trans.x-((winSize.x/2)*obj->scale.x), obj->trans.y-((winSize.y/2)*obj->scale.y));
}

static void MgListCenterStart(OMOBJ *obj)
{
    MGLISTWORK *work = omObjGetWork(obj, MGLISTWORK);
    work->slideF = TRUE;
    work->mode = 0;
    work->time = 0;
    work->maxTime = 30;
}

static void MgListFocusSet(OMOBJ *obj)
{
    MGLISTWORK *work = omObjGetWork(obj, MGLISTWORK);
    work->mode = 4;
    work->time = 0;
    work->maxTime = 90;
    if(work->hiddenMes >= 0) {
        MBWinKill(work->winNo);
        work->winNo = MBWinCreateHelp(work->hiddenMes);
    }
}

static void MgListTopSet(OMOBJ *obj)
{
    MGLISTWORK *work = omObjGetWork(obj, MGLISTWORK);
    work->mode = 5;
    work->time = 0;
    work->maxTime = 30;
    obj->rot.y = obj->trans.y;
}

static void MgListObjKill(OMOBJ *obj)
{
    MGLISTWORK *work = omObjGetWork(obj, MGLISTWORK);
    work->killF = TRUE;
}

static BOOL MgListSlideCheck(OMOBJ *obj)
{
    MGLISTWORK *work = omObjGetWork(obj, MGLISTWORK);
    if(work->slideF) {
        return FALSE;
    } else {
        return TRUE;
    }
}

void MBMgCallDispSet(BOOL dispF)
{
    MGLISTWORK *work = omObjGetWork(MgListObj[MgCallFocusNo], MGLISTWORK);
    work->dispF = dispF;
}

static int MgCallBattleCoinGet(void);
static int MgCallBattleSelect(MGCALL_WORK *workP, int listNum, s16 *list);

static int MgCallBattleExec(MGCALL_WORK *workP)
{
    int i;
    int coinType;
    int coinTotal;
    int delay;
    int coinNum;
    int speed;
    int nextMaxTime;
    int nextTime;
    BOOL coinHighF;
    int result;
    int battleCoin;
    BOOL fxPlayF;
    int nextDelay;
    int listNum;
    
    HuVecF pos;
    
    s16 mgTbl[128];
    char str[32];
    BOOL fastFocusF;
    int battleCoinOfs;
    
    s16 playerMaxSteal;
    s16 teamMaxSteal;
    
    float weight;
    float scale;
    float posY;
    float zRot;
    float velY;
    
    static const HuVecF guideRot = {};
    
    
    static int coinNumTbl[5] = {
        5,
        10,
        20,
        30,
        50
    };
    
    fxPlayF = FALSE;
    MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
    while(MBMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    MBMusPlay(MB_MUS_CHAN_BG, MSM_STREAM_BOARD_BATTLE, 127, 0);
    espKill(workP->sprId[2]);
    workP->sprId[2] = -1;
    workP->type = MG_TYPE_BATTLE;
    
    workP->sprId[1] = espEntry(BBATTLE_ANM_logo, 100, 0);
    MBAudFXPlay(MSM_SE_BOARD_111);
    posY = 88;
    velY = 6;
    zRot = 0;
    i = 0;
    do {
        weight = i/30.0f;
        if(weight > 1) {
            weight = 1;
        }
        espPosSet(workP->sprId[1], 288, 88-(120*HuSin((1-weight)*90)));
        if(weight >= 0.6f) {
            if(!fxPlayF) {
                MBAudFXPlay(MSM_SE_BOARD_112);
                fxPlayF = TRUE;
            }
            if(zRot > -40) {
                zRot--;
            }
            posY += velY;
            velY += 0.2f;
            espPosSet(workP->sprId[0], 288, posY);
            espZRotSet(workP->sprId[0], zRot);
        }
        HuPrcVSleep();
        i++;
    } while(posY < 576);
    MBGuideMdlPosGet(workP->guideMdlId, &pos);
    MBPointDepthScale(&pos, 1.5f, &pos);
    MBGuideMdlPosSetV(workP->guideMdlId, &pos);
    pos.x = 288;
    pos.y = 240;
    pos.z = 800;
    MBGuideCreate(MB_GUIDE_CHORL, &pos, (HuVecF *)&guideRot, MB_GUIDE_ATTR_MODELOFF|MB_GUIDE_ATTR_ALTMTX|MB_GUIDE_ATTR_SCREEN);
    if(!GWTeamFGet()) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            MBStatusColorSet(i, STATUS_COLOR_BLUE);
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            MBStatusPosOnGet(i, &pos);
            MBStatusMoveSet(i, NULL, &pos, STATUS_MOVE_SIN, 12);
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX/2; i++) {
            MBStatusPosOnGet(i, &pos);
            MBStatusNoMoveSet(i, NULL, &pos, STATUS_MOVE_SIN, 12);
        }
    }
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    HuPrcSleep(12);
    MBAudFXPlay(MSM_SE_GUIDE_50);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_BATTLE_START, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBTopWinKill();
    workP->sprId[4] = espEntry(BBATTLE_ANM_coinNumBox, 100, 0);
    workP->sprId[5] = espEntry(BBATTLE_ANM_coinNum, 99, 0);
    for(i=0; i<30; i++) {
        weight = i/30.0f;
        pos.x = 288;
        pos.y = 516-(224*weight);
        espPosSet(workP->sprId[4], pos.x, pos.y);
        espPosSet(workP->sprId[5], pos.x+32, pos.y);
        espPosSet(workP->sprId[1], 288, 88-(120*HuSin(weight*90)));
        HuPrcVSleep();
    }
    fastFocusF = frand() < 0.1f;
    coinType = 0;
    battleCoin = MgCallBattleCoinGet();
    delay = frandmod(30)+60;
    battleCoinOfs = (int)((-7+sqrtf((delay*8.0f)+49))/2);
    speed = (battleCoin-battleCoinOfs)%5;
    if(fastFocusF) {
        speed--;
    }
    if(speed < 0) {
        speed += 5;
    }
    nextDelay = (speed*4)+60;
    nextTime = nextMaxTime = 4;
    for(i=0; i<nextDelay+delay; i++) {
        if(--nextTime == 0) {
            coinType = (coinType+1)%5;
            if(i > nextDelay) {
                nextMaxTime++;
                nextTime = nextMaxTime;
            } else {
                nextTime = nextMaxTime;
            }
            MBAudFXPlay(MSM_SE_BOARD_05);
        }
        espBankSet(workP->sprId[5], coinType);
        HuPrcVSleep();
    }
    coinType = battleCoin;
    espBankSet(workP->sprId[5], coinType);
    MBAudFXPlay(MSM_SE_BOARD_130);
    for(i=0; i<90; i++) {
        weight = i/90.0f;
        scale = (0.2f*HuSin((1-weight)*1080))+1;
        espPosSet(workP->sprId[5], pos.x+(32*scale), pos.y);
        espScaleSet(workP->sprId[4], scale, scale);
        espScaleSet(workP->sprId[5], scale, scale);
        HuPrcVSleep();
    }
    coinNum = coinNumTbl[coinType];
    coinTotal = 0;
    coinHighF = TRUE;
    if(!GWTeamFGet()) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerCoinGet(i) >= coinNum) {
                coinTotal += coinNum;
                GwPlayer[i].coinBattle = coinNum;
            } else {
                coinTotal += MBPlayerCoinGet(i);
                coinHighF = FALSE;
                playerMaxSteal = MBPlayerCoinGet(i);
                GwPlayer[i].coinBattle = playerMaxSteal;
            }
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX/2; i++) {
            if(MBPlayerGrpCoinGet(i) >= coinNum*2) {
                coinTotal += coinNum*2;
                GwPlayer[i].coinBattle = coinNum*2;
            } else {
                coinTotal += MBPlayerGrpCoinGet(i);
                coinHighF = FALSE;
                teamMaxSteal = MBPlayerGrpCoinGet(i);
                GwPlayer[i].coinBattle = teamMaxSteal;
            }
        }
    }
    MBCoinAddAllExec(-coinNumTbl[coinType],  -coinNumTbl[coinType], -coinNumTbl[coinType], -coinNumTbl[coinType]);
    if(coinHighF) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_BATTLE_COINNUM, MBGuideSpeakerNoGet());
    } else {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_BATTLE_COINNUM_LOW, MBGuideSpeakerNoGet());
    }
    sprintf(str, "%d", coinTotal);
    MBTopWinInsertMesSet(MESSNUM_PTR(str), 0);
    MBTopWinWait();
    MBTopWinKill();
    listNum = MgListGet(MG_TYPE_BATTLE, 3, mgTbl);
    espDispOff(workP->sprId[4]);
    espDispOff(workP->sprId[5]);
    result = MgCallBattleSelect(workP, listNum, mgTbl);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_BATTLE_MGSTART, MBGuideSpeakerNoGet());
    MBTopWinWait();
    MBTopWinKill();
    _SetFlag(FLAG_BOARD_MG);
    MBModelPosGet(workP->battleMdlId[result], &pos);
    MBCameraPosViewSet(&pos, NULL, NULL, 80, -1, 60);
    return mgTbl[result];
}

typedef struct MgCallBattlePlayer_s {
    u8 killF : 1;
    s16 cursorSprId;
    s16 charSprId;
    s16 cursorPos;
    s16 cursorPosPrev;
    s16 time;
    s16 maxTime;
    s16 comSelectPos;
    s16 comBtnTime;
} MGCALL_BATTLEPLAYER;

static int MgCallBattleSelect(MGCALL_WORK *workP, int listNum, s16 *list)
{
    static const unsigned int charFileTbl[CHARNO_MAX] = {
        BBATTLE_ANM_mario,
        BBATTLE_ANM_luigi,
        BBATTLE_ANM_peach,
        BBATTLE_ANM_yoshi,
        BBATTLE_ANM_wario,
        BBATTLE_ANM_daisy,
        BBATTLE_ANM_waluigi,
        BBATTLE_ANM_kinopio,
        BBATTLE_ANM_teresa,
        BBATTLE_ANM_minikoopa,
        BBATTLE_ANM_minikoopaR,
        BBATTLE_ANM_minikoopaG,
        BBATTLE_ANM_minikoopaB
    };
    
    static const unsigned int mgData[6][2] = {
        MESS_MGNAME_M535, BBATTLE_ANM_m535Pic,
        MESS_MGNAME_M536, BBATTLE_ANM_m536Pic,
        MESS_MGNAME_M537, BBATTLE_ANM_m537Pic,
        MESS_MGNAME_M538, BBATTLE_ANM_m538Pic,
        MESS_MGNAME_M539, BBATTLE_ANM_m539Pic,
        MESS_MGNAME_M540, BBATTLE_ANM_m540Pic,
    };
    
    static int tbl2[5] = {
          6,
          6,
          3,
          3,
          1
    };
    
    static HuVec2F posTbl[3] = {
        { 128, 240 },
        { 288, 240 },
        { 448, 240 }
    };
    
    static HuVec2F playerOfsTbl[GW_PLAYER_MAX] = {
        { -56, -16 },
        { -38, -48 },
        { 6, -48 },
        { 24, -16 },
    };
    
    
    MGCALL_BATTLEPLAYER *battlePlayer;
    int i;
    u16 btn;
    int j;
    int cursorDupeNum;
    int ignoreNum;
    int selectNum;
    s16 playerDoneNum;
    u16 padNo;
    MBCAMERA *cameraP;
    int toueiMdlId;
    
    MGCALL_BATTLEPLAYER playerAll[GW_PLAYER_MAX];
    Mtx matrix;
    int ignoreTbl[3];
    int selectTbl[3];
    
    HuVecF pos2D;
    HuVecF pos3D;
    
    s16 cursorNumTbl[3];
    
    float posX;
    float posY;
    float weight;
    float time;
    
    cameraP = MBCameraGet();
    MBWinCreateHelp(MESS_BOARD_BATTLE_SELECT_HELP);
    for(i=0; i<listNum; i++) {
        workP->battleMdlId[i] = MBModelCreate(BBATTLE_HSF_touei, NULL, 0);
        for(j=0; j<6; j++) {
            if(mgData[j][0] == mgDataTbl[list[i]].nameMes) {
                break;
            }
        }
        if(j < 6) {
            Hu3DAnimCreate(HuDataSelHeapReadNum(mgData[j][1], HU_MEMNUM_OVL, HEAP_MODEL), MBModelIdGet(workP->battleMdlId[i]), "S3TC_touei");
        }
        pos2D.x = posTbl[i].x;
        pos2D.y = posTbl[i].y;
        pos2D.z = 500;
        Hu3D2Dto3D(&pos2D, HU3D_CAM0, &pos3D);
        MBModelPosSetV(workP->battleMdlId[i], &pos3D);
        MBModelScaleSet(workP->battleMdlId[i], 0.3f, 0.3f, 0.3f);
        MTXLookAt(matrix, &cameraP->pos, &cameraP->up, &pos3D);
        MTXInverse(matrix, matrix);
        matrix[0][3] = matrix[1][3] = matrix[2][3] = 0;
        MBModelMtxSet(workP->battleMdlId[i], &matrix);
    }
    for(battlePlayer=&playerAll[0], i=0; i<GW_PLAYER_MAX; i++, battlePlayer++) {
        battlePlayer->cursorSprId = espEntry(BBATTLE_ANM_cursor, (i*2)+81, 0);
        battlePlayer->charSprId = espEntry(charFileTbl[GwPlayer[i].charNo], (i*2)+80, 0);
        battlePlayer->killF = FALSE;
        battlePlayer->cursorPos = battlePlayer->cursorPosPrev = listNum/2;
        battlePlayer->time = 0;
        battlePlayer->maxTime = 18;
        battlePlayer->comSelectPos = frandmod(listNum);
        battlePlayer->comBtnTime = 120+(frandf()*60);
        espPosSet(battlePlayer->cursorSprId, posTbl[battlePlayer->cursorPos].x+playerOfsTbl[i].x, posTbl[battlePlayer->cursorPos].y+playerOfsTbl[i].y);
        espPosSet(battlePlayer->charSprId, posTbl[battlePlayer->cursorPos].x+playerOfsTbl[i].x, posTbl[battlePlayer->cursorPos].y+playerOfsTbl[i].y-6);
    }
    for(i=0; i<listNum; i++) {
        cursorNumTbl[i] = 0;
    }
    HuPrcVSleep(10);
    for(playerDoneNum=0; playerDoneNum<4; ) {
        for(battlePlayer=&playerAll[0], i=0; i<GW_PLAYER_MAX; i++, battlePlayer++) {
            if(battlePlayer->killF) {
                continue;
            }
            if(battlePlayer->cursorPos != battlePlayer->cursorPosPrev) {
                weight = (float)(++battlePlayer->time)/battlePlayer->maxTime;
                time = HuSin(weight*90);
                posX = playerOfsTbl[i].x+(posTbl[battlePlayer->cursorPosPrev].x+(time*(posTbl[battlePlayer->cursorPos].x-posTbl[battlePlayer->cursorPosPrev].x)));
                posY = posTbl[battlePlayer->cursorPosPrev].y+playerOfsTbl[i].y;
                espPosSet(battlePlayer->cursorSprId, posX, posY);
                espPosSet(battlePlayer->charSprId, posX, posY-6);
                if(battlePlayer->time >= battlePlayer->maxTime) {
                    battlePlayer->cursorPosPrev = battlePlayer->cursorPos;
                    battlePlayer->time = 0;
                }
            } else {
                if(GwPlayerConf[i].type == GW_TYPE_MAN) {
                    padNo = GwPlayer[i].padNo;
                    btn = HuPadBtnDown[padNo];
                    if(MBPadStkXGet(padNo) < -20) {
                        btn |= PAD_BUTTON_LEFT;
                    } else if(MBPadStkXGet(padNo) > 20) {
                        btn |= PAD_BUTTON_RIGHT;
                    }
                } else {
                    btn = 0;
                    if(battlePlayer->comBtnTime == 0) {
                        btn = PAD_BUTTON_A;
                    } else {
                        if(frand()%64 == 0) {
                            battlePlayer->comSelectPos = frandmod(listNum);
                        }
                        if(battlePlayer->cursorPos != battlePlayer->comSelectPos) {
                            if(battlePlayer->cursorPos > battlePlayer->comSelectPos) {
                                btn |= PAD_BUTTON_LEFT;
                            } else {
                                btn |= PAD_BUTTON_RIGHT;
                            }
                        }
                    }
                }
                if(btn & PAD_BUTTON_LEFT) {
                    if(battlePlayer->cursorPos > 0) {
                        MBAudFXPlay(MSM_SE_CMN_01);
                        battlePlayer->cursorPos--;
                    }
                }
                if(btn & PAD_BUTTON_RIGHT) {
                    if(battlePlayer->cursorPos < listNum-1) {
                        MBAudFXPlay(MSM_SE_CMN_01);
                        battlePlayer->cursorPos++;
                    }
                }
                if(btn & PAD_BUTTON_A) {
                    battlePlayer->killF = TRUE;
                    cursorNumTbl[battlePlayer->cursorPos]++;
                    playerDoneNum++;
                    espDispOff(battlePlayer->cursorSprId);
                    espDispOff(battlePlayer->charSprId);
                    MBAudFXPlay(MSM_SE_CMN_02);
                }
            }
            if(battlePlayer->comBtnTime) {
                battlePlayer->comBtnTime--;
            }
        }
        HuPrcVSleep();
    }
    MBTopWinKill();
    for(battlePlayer=&playerAll[0], i=0; i<GW_PLAYER_MAX; i++, battlePlayer++) {
        espKill(battlePlayer->cursorSprId);
        espKill(battlePlayer->charSprId);
    }
    ignoreNum = 0;
    selectNum = 0;
    for(i=0; i<listNum; i++) {
        cursorDupeNum = 0;
        for(j=0; j<listNum; j++) {
            if(i != j && cursorNumTbl[i] < cursorNumTbl[j]) {
                cursorDupeNum++;
            }
        }
        if(cursorDupeNum == 0) {
            ignoreTbl[ignoreNum++] = i;
        } else {
            selectTbl[selectNum++] = i;
        }
    }
    if(ignoreNum == 1) {
        return ignoreTbl[0];
    } else {
        return selectTbl[frandmod(selectNum)];
    }
}

static BOOL MgCallPlayCheck(int type, s16 no)
{
    s16 i;
    if(MgCallPlayTbl[type] == NULL) {
        return FALSE;
    }
    for(i=0; i<MgCallPlayNumTbl[type]; i++) {
        if(no == MgCallPlayTbl[type][i]) {
            return TRUE;
        }
    }
    return FALSE;
}

static int MgCallBattleCoinGet(void)
{
    static int maxTurnTbl[9] = {
        10,
        15,
        20,
        25,
        30,
        35,
        40,
        45,
        50
    };
    
    static int turnTbl[9][2] = {
        3, 6,
        5, 10,
        5, 15,
        8, 16,
        10, 20,
        10, 20,
        13, 26,
        15, 30,
        15, 35
    };
    
    static int maxCoinTbl[3] = {
        20,
        30,
        50
    };
    
    static int chanceTbl[4][3][5] = {
        10, 85, 5, 0, 0,
        10, 75, 15, 0, 0,
        10, 65, 25, 0, 0, 

        5, 70, 20, 5, 0,
        5, 60, 25, 10, 0,
        5, 50, 30, 15, 0,

        5, 60, 20, 10, 5,
        5, 45, 30, 15, 5,
        5, 35, 30, 20, 10,

        5, 45, 30, 15, 5,
        5, 30, 35, 20, 10,
        5, 20, 35, 25, 15
    };
    
    int no;
    int i;
    int totalChance;
    int part;
    int maxCoin;
    int chanceGrp;
    
    int chance;
    int gamePart;
    int gameLen;
    
    int typeChanceTbl[5];
    
    for(i=0; i<8u; i++) {
        if(GwSystem.turnMax <= maxTurnTbl[i]) {
            break;
        }
    }
    gameLen = i;
    for(i=0; i<2; i++) {
        if(GwSystem.turnNo < turnTbl[gameLen][i]) {
            break;
        }
    }
    gamePart = i;
    part = gamePart;
    maxCoin = MBPlayerMaxCoinGet();
    for(no=0; no<3; no++) {
        if(maxCoin < maxCoinTbl[no]) {
            break;
        }
    }
    chanceGrp = no;
    totalChance = 0;
    for(no=0; no<5; no++) {
        totalChance += chanceTbl[chanceGrp][part][no];
        typeChanceTbl[no] = totalChance;
    }
    chance = frandmod(totalChance);
    for(no=0; no<4; no++) {
        if(chance < typeChanceTbl[no]) {
            break;
        }
    }
    return no;
}

void MBMgCallKettouExec(void)
{
    float weight;
    int i;
    int sprId;
    int result;
    _SetFlag(FLAG_BOARD_WALKDONE);
    sprId = espEntry(MgCallTypeFileTbl[3], 100, 0);
    espPosSet(sprId, 288, -32);
    for(i=0; i<30u; i++) {
        weight = i/30.0f;
        espPosSet(sprId, 288, 88-(120*HuSin((1-weight)*90.0f)));
        HuPrcVSleep();
    }
    _SetFlag(FLAG_BOARD_MGKETTOU);
    result = MgListExec(MG_TYPE_KETTOU);
    MgCallNext(MG_TYPE_KETTOU, result);
    espKill(sprId);
    HuPrcSleep(-1);
}

void MBMgCallDonkeyExec(void)
{
    int result;
    _SetFlag(FLAG_BOARD_MGDONKEY);
    result = MgListExec(MG_TYPE_DONKEY);
    MgCallNext(MG_TYPE_DONKEY, result);
    HuPrcSleep(-1);
}

void MBMgCallKupaExec(void)
{
    int result;
    _SetFlag(FLAG_BOARD_MGKUPA);
    result = MgListExec(MG_TYPE_KUPA);
    MgCallNext(MG_TYPE_KUPA, result);
    HuPrcSleep(-1);
}

void MBMgCallVsExec(void)
{
    
    static int vsTypeTbl[4] = {
        MG_TYPE_KETTOU,
        MG_TYPE_2VS2,
        MG_TYPE_1VS3,
        MG_TYPE_4P
    };
    
    int playerNum;
    int i;
    int dummyPlayerNo;
    int result;
    
    int alivePlayer[GW_PLAYER_MAX];
    int deadCom[GW_PLAYER_MAX];
    int deadComNum;
    
    playerNum = MBPlayerAlivePlayerGet(alivePlayer);
    deadComNum = MBPlayerDeadComGet(deadCom);
    GwPlayerConf[MBPlayerStoryPlayerGet()].grp = 0;
    for(i=0; i<playerNum; i++) {
        GwPlayerConf[alivePlayer[i]].grp = 1;
    }
    if(playerNum == 2) {
        MBDataClose();
        CharMotionInit(CHARNO_KINOPIO);
        dummyPlayerNo = deadCom[0];
        GwPlayerConf[dummyPlayerNo].grp = 0;
        GwPlayerConf[dummyPlayerNo].charNo = CHARNO_KINOPIO;
        GwPlayerConf[dummyPlayerNo].dif = GW_DIF_VERYHARD;
    } else if(playerNum == 1) {
        GwPlayerConf[deadCom[0]].grp = 2;
        GwPlayerConf[deadCom[1]].grp = 2;
    }
    _SetFlag(FLAG_BOARD_MGVS);
    result = MgListExec(vsTypeTbl[playerNum-1]);
    MgCallNext(vsTypeTbl[playerNum-1], result);
    HuPrcSleep(-1);
}

static void MgCallNext(int type, int no)
{
    int id = no+GW_MGNO_BASE;
    int statId;
    unsigned int instDir;
    GWMgUnlockSet(id);
    GwSystem.mgNo = id-GW_MGNO_BASE;
    if(MgCallPlayTbl[type] != NULL) {
        MgCallPlayTbl[type][MgPlayListOfsTbl[type]] = id;
        if(++MgPlayListOfsTbl[type] >= MgCallPlayNumTbl[type]) {
            MgPlayListOfsTbl[type] = 0;
        }
    }
    instDir = DATA_inst;
    MgCallDir = mgDataTbl[no].dataDir;
    if(!_CheckFlag(FLAG_MG_CIRCUIT) || _CheckFlag(FLAG_BOARD_MGKETTOU) || _CheckFlag(FLAG_BOARD_MGKUPA)) {
        MBDataClose();
        statId = MBDataDirReadAsync(instDir);
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
        MBDataAsyncWait(statId);
        WipeWait();
        MBOvlGoto(DLL_instdll);
    }
}


void MBMgCallDataClose(void)
{
    if(MgCallDir != -1) {
        HuDataDirClose(MgCallDir);
    }
}

void MBMgCallTutorialExec(void)
{
    Mtx explodeMtx;
    MGCALL_WORK work;
    HuVecF pos;
    HuVecF posNew;
    HuVecF explodePos2D;
    HuVecF explodePos3D;
    int colorTbl[4];
    
    int i;
    MGCALL_WORK *workP;
    s16 winNo;
    MBCAMERA *cameraP;
    
    float weight;
    float scale;
    
    workP = &work;
    cameraP = MBCameraGet();
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    }
    WipeWait();
    MBAudFXPlay(MSM_SE_GUIDE_50);
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_MG_BEGIN, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_MG_ENDTURN, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBGuideScreenPosGet(&pos);
    posNew = pos;
    for(i=0; i<=12u; i++) {
        weight = i/12.0f;
        posNew.y = pos.y+(weight*(119-pos.y));
        MBGuideScreenPosSet(&posNew);
        HuPrcVSleep();
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MgStatusPos4P[i].y += 64;
        MgStatusPos2Vs2[i].y += 64;
        MgStatusPos1Vs3[i].y += 64;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBStatusPosOnGet(i, &pos);
        MBStatusMoveSet(i, &pos, (HuVecF *)&MgStatusPos4P[i], STATUS_MOVE_SIN, 15);
    }
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    MBWinCreate(MBWIN_TYPE_GUIDETOP, MESS_BOARD_TUTORIAL_MG_STATUS, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        colorTbl[i] = STATUS_COLOR_BLUE;
    }
    MgPlayerGroupUpdate(colorTbl);
    workP->sprId[2] = espEntry(BOARD_ANM_mgCallVs, 100, 0);
    espPosSet(workP->sprId[2], 288, 304);
    for(i=0; i<30u; i++) {
        weight = i/30.0f;
        scale = 1.5+HuSin(weight*360);
        if(weight < 0.2f) {
            scale *= (weight/0.2f);
        }
        espScaleSet(workP->sprId[2], scale, scale);
        HuPrcVSleep();
    }
    MBWinCreate(MBWIN_TYPE_GUIDETOP, MESS_BOARD_TUTORIAL_MG_4P, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    colorTbl[1] = STATUS_COLOR_RED;
    colorTbl[3] = STATUS_COLOR_RED;
    MgPlayerGroupUpdate(colorTbl);
    MBWinCreate(MBWIN_TYPE_GUIDETOP, MESS_BOARD_TUTORIAL_MG_GROUP, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_GUIDETOP, MESS_BOARD_TUTORIAL_MG_2VS2, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    colorTbl[0] = STATUS_COLOR_RED;
    MgPlayerGroupUpdate(colorTbl);
    MBWinCreate(MBWIN_TYPE_GUIDETOP, MESS_BOARD_TUTORIAL_MG_1VS3, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    colorTbl[0] = STATUS_COLOR_BLUE;
    colorTbl[1] = STATUS_COLOR_BLUE;
    MgPlayerGroupUpdate(colorTbl);
    MBWinCreate(MBWIN_TYPE_GUIDETOP, MESS_BOARD_TUTORIAL_MG_3VS1, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_GUIDETOP, MESS_BOARD_TUTORIAL_MG_TYPE, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_GUIDETOP, MESS_BOARD_TUTORIAL_MGCALL, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    workP->sprId[0] = espEntry(MgCallTypeFileTbl[1], 1500, 0);
    espPosSet(workP->sprId[0], 288, -32);
    espDrawNoSet(workP->sprId[0], 32);
    for(i=0; i<30u; i++) {
        weight = i/30.0f;
        espPosSet(workP->sprId[0], 288, 88-(120*HuSin((1-weight)*90.0f)));
        HuPrcVSleep();
    }
    for(i=0; i<60u; i++) {
        weight = i/60.0f;
        scale = 1.5f+(0.8f*HuSin(weight*(720+(720*weight))));
        espScaleSet(workP->sprId[2], scale, scale);
        HuPrcVSleep();
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBStatusPosGet(i, &pos);
        if(pos.x  >= 288) {
            pos.x = 704;
        } else {
            pos.x = -128;
        }
        MBStatusMoveSet(i, NULL, &pos, STATUS_MOVE_SIN, 15);
    }
    workP->sprId[3] = MBModelCreate(BOARD_HSF_effExplode, NULL, FALSE);
    MBModelLayerSet(workP->sprId[3], 6);
    explodePos2D.x = 288;
    explodePos2D.y = 304;
    explodePos2D.z = 500;
    MB2Dto3D(&explodePos2D, &explodePos3D);
    MBModelPosSetV(workP->sprId[3], &explodePos3D);
    MTXLookAt(explodeMtx, &cameraP->pos, &cameraP->up, &explodePos3D);
    MTXInverse(explodeMtx, explodeMtx);
    explodeMtx[0][3] = explodeMtx[1][3] = explodeMtx[2][3] = 0;
    MBModelMtxSet(workP->sprId[3], &explodeMtx);
    
    espKill(workP->sprId[2]);
    workP->sprId[2] = -1;
    MBAudFXPlay(MSM_SE_BOARD_109);
    for(i=0; i<30u; i++) {
        weight = i/30.0f;
        scale = ((1.0f+(5.0f*weight))+(0.5*HuSin(16*(weight*360))))*0.4f;
        MBModelScaleSet(workP->sprId[3], scale, scale, scale);
        if(i >= 25) {
            MBModelAlphaSet(workP->sprId[3], 255*HuCos(((i-25)/5.0f)*90.0f));
        }
        HuPrcVSleep();
    }
    MBModelKill(workP->sprId[3]);
    workP->sprId[3] = MB_MODEL_NONE;
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    MgListExec(MG_TYPE_1VS3);
    winNo = MBWinCreate(MBWIN_TYPE_GUIDETOP, MESS_BOARD_TUTORIAL_MGBATTLE, HUWIN_SPEAKER_NONE);
    MBWinWait(winNo);
    winNo = MBWinCreate(MBWIN_TYPE_GUIDETOP, MESS_BOARD_TUTORIAL_MG_COIN, HUWIN_SPEAKER_NONE);
    MBWinWait(winNo);
    WipeColorSet(255, 255, 255);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    }
    WipeWait();
    for(i=0; i<4; i++) {
        if(MgListObj[i]) {
            MgListObjKill(MgListObj[i]);
        }
    }
    espKill(workP->sprId[0]);
}

static BOOL MgCallCheckValid(int no)
{
    static const int noTbl[] = {
        501,
        502,
        503,
        504,
        563,
        564,
        565,
        576,
        505,
        506,
        507,
        508,
        509,
        510,
        579,
        511,
        512,
        513,
        514,
        515,
        524,
        516,
        566,
        517,
        518,
        519,
        520,
        522,
        523,
        525,
        567,
        568,
        569,
        570,
        577,
        526,
        527,
        528,
        521,
        529,
        530,
        531,
        574,
        532,
        533,
        534,
        575,
        535,
        536,
        537,
        538,
        539,
        540,
        541,
        542,
        543,
        544,
        545,
        546,
        547,
        548,
        549,
        550,
        551,
        552,
        553,
        554,
        555,
        559,
        560,
        561,
        562,
        571,
        572,
        573,
        -1
    };
    int i;
    for(i=0; noTbl[i] >= 0 && no != noTbl[i]-GW_MGNO_BASE; i++);

    return noTbl[i] >= 0;
}