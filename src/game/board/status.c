#include "game/board/status.h"
#include "game/board/player.h"
#include "game/board/main.h"
#include "game/board/pause.h"
#include "game/board/capsule.h"

#include "game/disp.h"

#define STATUS_SPRNO_BACK 0
#define STATUS_SPRNO_BORDER 1
#define STATUS_SPRNO_RANK 2
#define STATUS_SPRNO_COIN_NUM 3
#define STATUS_SPRNO_COIN 6
#define STATUS_SPRNO_STAR_NUM 7
#define STATUS_SPRNO_STAR 10
#define STATUS_SPRNO_KAO 11
#define STATUS_SPRNO_COM 13
#define STATUS_SPRNO_SAIMODE 15
#define STATUS_SPRNO_CAPSULE 17
#define STATUS_SPR_MAX 22

typedef struct statusWork_s {
    unsigned offF : 1;
    unsigned dispF : 1;
    unsigned moveF : 1;
    unsigned capsuleDispF : 1;
    unsigned layoutNo : 2;
    unsigned layoutBackup : 2;
    s8 capsuleFocus;
    s16 capsuleTime;
    s8 no;
    s8 playerNo;
    HUSPRGRPID gid;
    HuVec2F posBegin;
    HuVec2F posEnd;
    int moveType;
    int time;
    int timeMax;
} STATUSWORK;

typedef struct statusDisp_s {
    unsigned dispF : 1;
    int no;
} STATUSDISP;

static STATUSWORK statusWork[GW_PLAYER_MAX];
static STATUSDISP statusDispBackup[GW_PLAYER_MAX];

static HUPROCESS *statusProc;
static u8 statusForceFlag;

static HuVec2F statusPosOn[GW_PLAYER_MAX] = {
    { 114, 84 },
    { HU_DISP_WIDTH-114, 84 },
    { 114, HU_DISP_HEIGHT-84 },
    { HU_DISP_WIDTH-114, HU_DISP_HEIGHT-84 },
};

static HuVec2F statusPosOff[GW_PLAYER_MAX] = {
    { -98, 84 },
    { HU_DISP_WIDTH+98, 84 },
    { -98, HU_DISP_HEIGHT-84 },
    { HU_DISP_WIDTH+98, HU_DISP_HEIGHT-84 },
};

static HuVec2F statusPosOnTeam[GW_PLAYER_MAX/2] = {
    { 140, 84 },
    { HU_DISP_WIDTH-140, 84 },
};

static HuVec2F statusPosOffTeam[GW_PLAYER_MAX/2] = {
    { -124, 84 },
    { HU_DISP_WIDTH+124, 84 },
};

static GXColor statusColTbl[STATUS_COLOR_MAX] = {
    { 128, 128, 128, 0 },
    { 26, 132, 255, 0 },
    { 255, 26, 45, 0 },
    { 10, 180, 60, 0 },
};

static HuVec2F statusSprPosTbl[STATUS_SPR_MAX] = {
    { 0, 0 },
    { 0, 0 },
    { -68, 0 },
    { 80, 14 },
    { 64, 14 },
    { 48, 14 },
    { 26, 14 },
    { 80, -12 },
    { 64, -12 },
    { 48, -12 },
    { 26, -12 },
    { -16, 0 }
};

static HuVec2F statusCapsuleOfsTbl[2] = {
    { 24, 34 },
    { 24, -34 }
};

static HuVec2F statusComOfsTbl[2] = {
    { -16, -32 },
    { -16, 32 }
};

static HuVec2F statusTeamSprPosTbl[STATUS_SPR_MAX] = {
    { 0, 0 },
    { 0, 0 },
    { -100, 0 },
    { 112, 14 },
    { 96, 14 },
    { 80, 14 },
    { 58, 14 },
    { 112, -12 },
    { 96, -12 },
    { 80, -12 },
    { 58, -12 },
    { -48, 0 },
    { 16, 0 }
};

static HuVec2F statusTeamCapsuleOfsTbl[2] = {
    { -24, 34 },
    { -24, 34 }
};

static HuVec2F statusTeamComOfsTbl[2] = {
    { -48, -32 },
    { -48, -32 }
};

static HuVec2F statusCircuitCapsuleOfsTbl[3] = {
    { 38, 12 },
    { 54, -12 },
    { 70, 12 }
};

static void StatusProcExec(void);
static void StatusProcKill(void);
static BOOL StatusUpdate(void);
static BOOL StatusTeamUpdate(void);
static void StatusPosUpdate(STATUSWORK *status);

static void StatusPauseHook(BOOL dispF);

static void StatusSprCreate(int statusNo, STATUSWORK *status);
static void StatusTeamSprCreate(int statusNo, STATUSWORK *status);

void MBStatusInit(void)
{
    int i;
    memset(&statusWork[0], 0, sizeof(statusWork));
    if(!GWTeamFGet()) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            StatusSprCreate(i, &statusWork[i]);
            statusWork[i].offF = TRUE;
            statusWork[i].moveF = FALSE;
            statusWork[i].dispF = FALSE;
            statusWork[i].capsuleDispF = TRUE;
            statusWork[i].no = i;
            statusWork[i].capsuleFocus = -1;
            statusWork[i].capsuleTime = 0;
            statusWork[i].playerNo = i;
            if(i < GW_PLAYER_MAX/2) {
                statusWork[i].layoutNo = 0;
            } else {
                statusWork[i].layoutNo = 1;
            }
            statusWork[i].layoutBackup = statusWork[i].layoutNo;
            MBStatusColorSet(i, GwPlayer[i].color);
            HuSprGrpDrawNoSet(statusWork[i].gid, 32);
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX/2; i++) {
            GXColor *colorP;
            StatusTeamSprCreate(i, &statusWork[i]);
            statusWork[i].offF = TRUE;
            statusWork[i].moveF = FALSE;
            statusWork[i].dispF = FALSE;
            statusWork[i].capsuleDispF = TRUE;
            statusWork[i].capsuleFocus = -1;
            statusWork[i].capsuleTime = 0;
            statusWork[i].no = i;
            statusWork[i].playerNo = i;
            statusWork[i].layoutNo = 0;
            statusWork[i].layoutBackup = statusWork[i].layoutNo;
            colorP = (i == 0) ? &statusColTbl[STATUS_COLOR_RED] : &statusColTbl[STATUS_COLOR_BLUE];
            HuSprColorSet(statusWork[i].gid, STATUS_SPRNO_BACK, colorP->r, colorP->g, colorP->b);
            HuSprGrpDrawNoSet(statusWork[i].gid, 32);
        }
    }
    statusProc = MBPrcCreate(StatusProcExec, 8193, 16384);
    MBPrcDestructorSet(statusProc, StatusProcKill);
    MBStatusDispForceSetAll(FALSE);
}

void MBStatusClose(void)
{
    STATUSWORK *status;
    int i;
    for(status=&statusWork[0], i=0; i<GW_PLAYER_MAX; i++, status++) {
        if(status->offF && status->gid != HUSPR_GRP_NONE) {
            HuSprGrpKill(status->gid);
            status->gid = HUSPR_GRP_NONE;
        }
    }
}

void MBStatusReinit(void)
{
    MBStatusClose();
    HuPrcKill(statusProc);
    MBStatusInit();
}

static void StatusProcExec(void)
{
    STATUSWORK *status = &statusWork[0];
    MBPauseHookPush(StatusPauseHook);
    while(1) {
        if(!GWTeamFGet()) {
            if(StatusUpdate()) {
                break;
            }
        } else {
            if(StatusTeamUpdate()) {
                break;
            }
        }
        HuPrcVSleep();
    }
    HuPrcEnd();
}

static void StatusProcKill(void)
{
    statusProc = NULL;
}

static BOOL StatusUpdate(void)
{
    int i, j;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        STATUSWORK *status = &statusWork[i];
        int capsuleNo;
        if(!MBPlayerAliveCheck(i)) {
            for(j=0; j<STATUS_SPR_MAX; j++) {
                HuSprDispOff(status->gid, j);
            }
            statusWork[i].moveF = FALSE;
            continue;
        }
        GwPlayer[i].rank = MBPlayerRankGet(i);
        HuSprBankSet(status->gid, STATUS_SPRNO_RANK, GwPlayer[i].rank);
        if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
            MBSpriteNumSet(status->gid, STATUS_SPRNO_COIN_NUM, MBPlayerCoinGet(i));
            MBSpriteNumSet(status->gid, STATUS_SPRNO_STAR_NUM, MBPlayerStarGet(i));
            for(j=0; j<3; j++) {
                capsuleNo = MBPlayerCapsuleGet(i, j);
                if(capsuleNo != CAPSULE_NULL && status->capsuleDispF) {
                    HuSprBankSet(status->gid, STATUS_SPRNO_CAPSULE+j, MBCapsuleColorNoGet(capsuleNo));
                    HuSprPosSet(status->gid, STATUS_SPRNO_CAPSULE+j, statusCapsuleOfsTbl[status->layoutNo].x+(j*24), statusCapsuleOfsTbl[status->layoutNo].y);
                    HuSprDispOn(status->gid, STATUS_SPRNO_CAPSULE+j);
                    if(status->capsuleFocus == j) {
                        float scale = 1+fabs(HuCos(status->capsuleTime++ * 8.0f));
                        HuSprScaleSet(status->gid, STATUS_SPRNO_CAPSULE+j, scale, scale);
                    } else {
                        status->capsuleTime = 0;
                        HuSprScaleSet(status->gid, STATUS_SPRNO_CAPSULE+j, 1, 1);
                    }
                } else {
                    HuSprDispOff(status->gid, STATUS_SPRNO_CAPSULE+j);
                }
            }
        } else {
            for(j=0; j<3; j++) {
                capsuleNo = MBPlayerCapsuleGet(i, j);
                if(capsuleNo != CAPSULE_NULL && status->capsuleDispF) {
                    HuSprBankSet(status->gid, STATUS_SPRNO_CAPSULE+j, MBCapsuleColorNoGet(capsuleNo));
                    HuSprPosSet(status->gid, STATUS_SPRNO_CAPSULE+j, statusCircuitCapsuleOfsTbl[j].x, statusCircuitCapsuleOfsTbl[j].y);
                    HuSprDispOn(status->gid, STATUS_SPRNO_CAPSULE+j);
                } else {
                    HuSprDispOff(status->gid, STATUS_SPRNO_CAPSULE+j);
                }
            }
        }
        if(GwPlayer[i].comF) {
            HuSprDispOn(status->gid, STATUS_SPRNO_COM);
            HuSprPosSet(status->gid, STATUS_SPRNO_COM, statusComOfsTbl[status->layoutNo].x, statusComOfsTbl[status->layoutNo].y);
        } else {
            HuSprDispOff(status->gid, STATUS_SPRNO_COM);
        }
        if(GwPlayer[i].saiMode != MB_SAIMODE_NORMAL) {
            HuSprDispOn(status->gid, STATUS_SPRNO_SAIMODE);
            HuSprPosSet(status->gid, STATUS_SPRNO_SAIMODE, -32, -24);
            HuSprBankSet(status->gid, STATUS_SPRNO_SAIMODE, GwPlayer[i].saiMode-MB_SAIMODE_KINOKO);
        } else {
            HuSprDispOff(status->gid, STATUS_SPRNO_SAIMODE);
        }
        StatusPosUpdate(status);
    }
    return FALSE;
}

static BOOL StatusTeamUpdate(void)
{
    int i, j;
    for(i=0; i<GW_PLAYER_MAX/2; i++) {
        STATUSWORK *status = &statusWork[i];
        int grpNum;
        int capsuleNo;
        HuSprBankSet(status->gid, STATUS_SPRNO_RANK, MBPlayerGrpRankGet(i));
        grpNum = MBPlayerGrpCoinGet(i);
        MBSpriteNumSet(status->gid, STATUS_SPRNO_COIN_NUM, grpNum);
        grpNum = MBPlayerGrpStarGet(i);
        MBSpriteNumSet(status->gid, STATUS_SPRNO_STAR_NUM, grpNum);
        for(j=0; j<5; j++) {
            capsuleNo = MBPlayerGrpCapsuleGet(i, j);
            if(capsuleNo != CAPSULE_NULL && status->capsuleDispF) {
                HuSprBankSet(status->gid, STATUS_SPRNO_CAPSULE+j, MBCapsuleColorNoGet(capsuleNo));
                HuSprPosSet(status->gid, STATUS_SPRNO_CAPSULE+j, statusTeamCapsuleOfsTbl[status->layoutNo].x+(j*24), statusTeamCapsuleOfsTbl[status->layoutNo].y);
                HuSprDispOn(status->gid, STATUS_SPRNO_CAPSULE+j);
            } else {
                HuSprDispOff(status->gid, STATUS_SPRNO_CAPSULE+j);
            }
        }
        for(j=0; j<2; j++) {
            if(GwPlayer[MBPlayerGrpFindPlayer(i, j)].comF) {
                HuSprDispOn(status->gid, STATUS_SPRNO_COM+j);
                HuSprPosSet(status->gid, STATUS_SPRNO_COM+j, statusTeamComOfsTbl[status->layoutNo].x+(j*64), statusTeamComOfsTbl[status->layoutNo].y);
            } else {
                HuSprDispOff(status->gid, STATUS_SPRNO_COM+j);
            }
            if(GwPlayer[MBPlayerGrpFindPlayer(i, j)].saiMode != MB_SAIMODE_NORMAL) {
                HuSprDispOn(status->gid, STATUS_SPRNO_SAIMODE+j);
                HuSprPosSet(status->gid, STATUS_SPRNO_SAIMODE+j, -32+(j*64), -24);
                HuSprBankSet(status->gid, STATUS_SPRNO_SAIMODE+j, GwPlayer[MBPlayerGrpFindPlayer(i, j)].saiMode-MB_SAIMODE_KINOKO);
            } else {
                HuSprDispOff(status->gid, STATUS_SPRNO_SAIMODE+j);
            }
        }
        StatusPosUpdate(status);
    }
    return FALSE;
}

static void StatusPosUpdate(STATUSWORK *status)
{
    float weight;
    float time;
    HuVec2F pos;
    if(status->time >= status->timeMax) {
        status->moveF = FALSE;
        return;
    }
    time = (float)(++status->time)/status->timeMax;
    switch(status->moveType) {
        case STATUS_MOVE_LINEAR:
            weight = time;
            break;
       
        case STATUS_MOVE_SIN:
            weight = HuSin(time*90.0f);
            break;
        
        case STATUS_MOVE_REVCOS:
            weight = 1-HuCos(time*90.0f);
            break;
    }
    pos.x = status->posBegin.x+(weight*(status->posEnd.x-status->posBegin.x));
    pos.y = status->posBegin.y+(weight*(status->posEnd.y-status->posBegin.y));
    HuSprGrpPosSet(status->gid, pos.x, pos.y);
}

static STATUSWORK *StatusPlayerGet(int playerNo);

void MBStatusMoveTo(int playerNo, HuVecF *posBegin, HuVecF *posEnd)
{
    MBStatusMoveSet(playerNo, posBegin, posEnd, STATUS_MOVE_REVCOS, 16);
}

void MBStatusMoveFrom(int playerNo, HuVecF *posBegin, HuVecF *posEnd)
{
    MBStatusMoveSet(playerNo, posBegin, posEnd, STATUS_MOVE_SIN, 16);
}

static void StatusMoveSet(STATUSWORK *status, HuVecF *posBegin, HuVecF *posEnd, int type, int time)
{
    status->posBegin.x = posBegin->x;
    status->posBegin.y = posBegin->y;
    status->posEnd.x = posEnd->x;
    status->posEnd.y = posEnd->y;
    status->moveType = type;
    status->time = 0;
    status->timeMax = time;
    status->moveF = TRUE;
}

void MBStatusMoveSet(int playerNo, HuVecF *posBegin, HuVecF *posEnd, int type, int time)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    if(posBegin == NULL) {
        HuVecF pos;
        MBStatusPosGet(playerNo, &pos);
        StatusMoveSet(status, &pos, posEnd, type, time);
    } else {
        StatusMoveSet(status, posBegin, posEnd, type, time);
    }
}

void MBStatusNoMoveSet(int statusNo, HuVecF *posBegin, HuVecF *posEnd, int type, int time)
{
    STATUSWORK *status = &statusWork[statusNo];
    if(posBegin == NULL) {
        HuVecF pos;
        MBStatusNoPosGet(statusNo, &pos);
        StatusMoveSet(status, &pos, posEnd, type, time);
    } else {
        StatusMoveSet(status, posBegin, posEnd, type, time);
    }
}

void MBStatusPosOnGet(int statusNo, HuVecF *pos)
{
    if(!GWTeamFGet()) {
        pos->x = statusPosOn[statusNo].x;
        pos->y = statusPosOn[statusNo].y;
    } else {
        pos->x = statusPosOnTeam[statusNo].x;
        pos->y = statusPosOnTeam[statusNo].y;
    }
}

void MBStatusPosOffGet(int statusNo, HuVecF *pos)
{
    if(!GWTeamFGet()) {
        pos->x = statusPosOff[statusNo].x;
        pos->y = statusPosOff[statusNo].y;
    } else {
        pos->x = statusPosOffTeam[statusNo].x;
        pos->y = statusPosOffTeam[statusNo].y;
    }
}

BOOL MBStatusMoveCheck(int playerNo)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    return status->moveF == FALSE;
}

BOOL MBStatusOffCheckAll(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(statusWork[i].offF && statusWork[i].moveF) {
            return FALSE;
        }
    }
    return TRUE;
}

static void StatusSetDisp(int playerNo, int statusNo, BOOL dispF)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    HuVec2F posBegin;
    HuVec2F posEnd;
    
    if(!dispF) {
        if(!status->dispF) {
            return;
        }
        posBegin = status->posEnd;
        posEnd = status->posBegin;
        MBStatusMoveTo(playerNo, (HuVecF *)&posBegin, (HuVecF *)&posEnd);
        status->dispF = FALSE;
        status->moveF = TRUE;
    } else {
        if(status->dispF) {
            return;
        }
        MBStatusPosOffGet(statusNo, (HuVecF *)&posBegin);
        MBStatusPosOnGet(statusNo, (HuVecF *)&posEnd);
        MBStatusMoveFrom(playerNo, (HuVecF *)&posBegin, (HuVecF *)&posEnd);
        status->dispF = TRUE;
        status->moveF = TRUE;
    }
    if(dispF) {
        status->no = statusNo;
        if(status->no < 2) {
            MBStatusLayoutSet(playerNo, STATUS_LAYOUT_TOP);
        } else {
            MBStatusLayoutSet(playerNo, STATUS_LAYOUT_BOTTOM);
        }
    }
}

void MBStatusDispSet(int playerNo, BOOL dispF)
{
    if(!GWTeamFGet()) {
        StatusSetDisp(playerNo, playerNo, dispF);
    } else {
        StatusSetDisp(playerNo, MBPlayerGrpGet(playerNo), dispF);
    }
}

void MBStatusDispFocusSet(int playerNo, BOOL dispF)
{
    StatusSetDisp(playerNo, 0, dispF);
}

void MBStatusDispBackup(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        statusDispBackup[i].dispF = MBStatusDispGet(i);
        statusDispBackup[i].no = MBStatusNoGet(i);
    }
}

void MBStatusDispRestore(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        StatusSetDisp(i, statusDispBackup[i].no, statusDispBackup[i].dispF);
    }
}

BOOL MBStatusDispGet(int playerNo)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    return status->dispF;
}

int MBStatusNoGet(int playerNo)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    return status->no;
}

void MBStatusDispSetAll(BOOL dispF)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBStatusDispSet(i, dispF);
    }
}

void MBStatusDispForceSet(int playerNo, BOOL dispF)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    status->dispF = dispF;
    if(dispF) {
        MBStatusPosOffGet(status->no, (HuVecF *)&status->posBegin);
        MBStatusPosOnGet(status->no, (HuVecF *)&status->posEnd);
    } else {
        MBStatusPosOnGet(status->no, (HuVecF *)&status->posBegin);
        MBStatusPosOffGet(status->no, (HuVecF *)&status->posEnd);
    }
    HuSprGrpPosSet(status->gid, status->posEnd.x, status->posEnd.y);
    status->time = status->timeMax = 0;
    if(!GWTeamFGet()) {
        status->no = playerNo;
    } else {
        status->no = MBPlayerGrpGet(playerNo);
    }
    if(status->no < 2) {
        MBStatusLayoutSet(playerNo, STATUS_LAYOUT_TOP);
    } else {
        MBStatusLayoutSet(playerNo, STATUS_LAYOUT_BOTTOM);
    }
    status->moveF = FALSE;
}

void MBStatusDispForceSetAll(BOOL dispF)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBStatusDispForceSet(i, dispF);
    }
}

void MBStatusCapsuleDispSet(int statusNo, BOOL dispF)
{
    STATUSWORK *status = &statusWork[statusNo];
    if(dispF) {
        status->capsuleDispF = TRUE;
    } else {
        status->capsuleDispF = FALSE;
    }
}

void MBStatusLayoutSet(int statusNo, int layoutNo)
{
    STATUSWORK *status = &statusWork[statusNo];
    status->layoutNo = layoutNo;
}

void MBStatusCapsuleFocusSet(int statusNo, int index)
{
    STATUSWORK *status = &statusWork[statusNo];
    status->capsuleFocus = index;
}

static STATUSWORK *StatusPlayerGet(int playerNo)
{
    if(!GWTeamFGet()) {
        return &statusWork[playerNo];
    } else {
        return &statusWork[MBPlayerGrpGet(playerNo)];
    }
}

void MBStatusPosGet(int playerNo, HuVecF *pos)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    HUSPRGRP *gp = &HuSprGrpData[status->gid];
    pos->x = gp->pos.x;
    pos->y = gp->pos.y;
    pos->z = 0;
}

void MBStatusNoPosGet(int statusNo, HuVecF *pos)
{
    STATUSWORK *status = &statusWork[statusNo];
    HUSPRGRP *gp = &HuSprGrpData[status->gid];
    pos->x = gp->pos.x;
    pos->y = gp->pos.y;
    pos->z = 0;
}

void MBStatusKaoNoSet(int statusNo, int kaoNo)
{
    STATUSWORK *status = &statusWork[statusNo];
    HuSprBankSet(status->gid, STATUS_SPRNO_KAO, kaoNo);
}


#define STATUS_SPR_INITNUM 11

static int statusKaoFileTbl[CHARNO_MAX] = {
    BOARD_ANM_statusKaoMario,
    BOARD_ANM_statusKaoLuigi,
    BOARD_ANM_statusKaoPeach,
    BOARD_ANM_statusKaoYoshi,
    BOARD_ANM_statusKaoWario,
    BOARD_ANM_statusKaoDaisy,
    BOARD_ANM_statusKaoWaluigi,
    BOARD_ANM_statusKaoKinopio,
    BOARD_ANM_statusKaoTeresa,
    BOARD_ANM_statusKaoMinikoopa,
    BOARD_ANM_statusKaoMinikoopaR,
    BOARD_ANM_statusKaoMinikoopaG,
    BOARD_ANM_statusKaoMinikoopaB
};


static void StatusSprCreate(int statusNo, STATUSWORK *status)
{
    static int fileTbl[STATUS_SPR_INITNUM] = {
        BOARD_ANM_statusBack,
        BOARD_ANM_statusBorder,
        BOARD_ANM_statusRank,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusCoin,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusStar,
    };

    static s16 prioTbl[STATUS_SPR_INITNUM] = {
        1520,
        1510,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500
    };
    int charNo = GwPlayerConf[statusNo].charNo;
    int i;
    HUSPRID sprId;
    float posX, posY;
    status->gid = HuSprGrpCreate(STATUS_SPR_MAX);
    for(i=0; i<STATUS_SPR_INITNUM; i++) {
        int dataNum = fileTbl[i];
        s16 prio = prioTbl[i];
        MBSpriteCreate(MBDATANUM(dataNum), prio, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, i, sprId);
        HuSprAttrSet(status->gid, i, HUSPR_ATTR_NOANIM);
        HuSprAttrSet(status->gid, i, HUSPR_ATTR_LINEAR);
        HuSprPosSet(status->gid, i, statusSprPosTbl[i].x, statusSprPosTbl[i].y);
    }
    MBSpriteCreate(MBDATANUM(statusKaoFileTbl[charNo]), 1500, NULL, &sprId);
    HuSprGrpMemberSet(status->gid, STATUS_SPRNO_KAO, sprId);
    HuSprAttrSet(status->gid, STATUS_SPRNO_KAO, HUSPR_ATTR_LINEAR);
    posX = statusSprPosTbl[STATUS_SPRNO_KAO].x;
    posY = statusSprPosTbl[STATUS_SPRNO_KAO].y;
    HuSprPosSet(status->gid, STATUS_SPRNO_KAO, posX, posY);
    MBSpriteCreate(MBDATANUM(BOARD_ANM_statusCom), 1490, NULL, &sprId);
    HuSprGrpMemberSet(status->gid, STATUS_SPRNO_COM, sprId);
    HuSprAttrSet(status->gid, STATUS_SPRNO_COM, HUSPR_ATTR_LINEAR);
    MBSpriteCreate(MBDATANUM(BOARD_ANM_statusSaiMode), 1490, NULL, &sprId);
    HuSprGrpMemberSet(status->gid, STATUS_SPRNO_SAIMODE, sprId);
    HuSprAttrSet(status->gid, STATUS_SPRNO_SAIMODE, HUSPR_ATTR_LINEAR|HUSPR_ATTR_NOANIM|HUSPR_ATTR_DISPOFF);
    for(i=0; i<3; i++) {
        MBSpriteCreate(MBDATANUM(BOARD_ANM_statusCapsule), 1500, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, STATUS_SPRNO_CAPSULE+i, sprId);
        HuSprAttrSet(status->gid, STATUS_SPRNO_CAPSULE+i, HUSPR_ATTR_NOANIM);
        HuSprAttrSet(status->gid, STATUS_SPRNO_CAPSULE+i, HUSPR_ATTR_LINEAR);
    }
    HuSprTPLvlSet(status->gid, STATUS_SPRNO_BACK, 0.7f);
    if(GWPartyFGet() == FALSE) {
        for(i=STATUS_SPRNO_STAR_NUM; i<=STATUS_SPRNO_STAR; i++) {
            HuSprDispOff(status->gid, i);
        }
        for(i=STATUS_SPRNO_COIN_NUM; i<=STATUS_SPRNO_COIN; i++) {
            HuSprPosSet(status->gid, i, statusSprPosTbl[i].x, statusSprPosTbl[i].y-14);
        }
    }
    if(_CheckFlag(FLAG_MG_CIRCUIT)) {
        for(i=STATUS_SPRNO_COIN_NUM; i<=STATUS_SPRNO_STAR; i++) {
            HuSprDispOff(status->gid, i);
        }
    }
}

static void StatusTeamSprCreate(int statusNo, STATUSWORK *status)
{
    static int fileTbl[STATUS_SPR_INITNUM] = {
        BOARD_ANM_statusTeamBack,
        BOARD_ANM_statusTeamBorder,
        BOARD_ANM_statusRankTeamRed,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusCoin,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusNum,
        BOARD_ANM_statusStar,
    };

    static s16 prioTbl[STATUS_SPR_INITNUM] = {
        1520,
        1510,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500
    };
    int i;
    HUSPRID sprId;
    int kaoNo;
    float posX, posY;
    status->gid = HuSprGrpCreate(STATUS_SPR_MAX);
    for(i=0; i<STATUS_SPR_INITNUM; i++) {
        int dataNum = fileTbl[i];
        s16 prio = prioTbl[i];
        if(i == STATUS_SPRNO_RANK && statusNo != 0) {
            dataNum = BOARD_ANM_statusRankTeamBlue;
        }
        MBSpriteCreate(MBDATANUM(dataNum), prio, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, i, sprId);
        HuSprAttrSet(status->gid, i, HUSPR_ATTR_NOANIM);
        HuSprAttrSet(status->gid, i, HUSPR_ATTR_LINEAR);
        HuSprPosSet(status->gid, i, statusTeamSprPosTbl[i].x, statusTeamSprPosTbl[i].y);
    }
    kaoNo = 0;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(statusNo == MBPlayerGrpGet(i)) {
            MBSpriteCreate(MBDATANUM(statusKaoFileTbl[GwPlayerConf[i].charNo]), 1500, NULL, &sprId);
            HuSprGrpMemberSet(status->gid, STATUS_SPRNO_KAO+kaoNo, sprId);
            HuSprAttrSet(status->gid, STATUS_SPRNO_KAO, HUSPR_ATTR_LINEAR);
            posX = statusTeamSprPosTbl[STATUS_SPRNO_KAO+kaoNo].x;
            posY = statusTeamSprPosTbl[STATUS_SPRNO_KAO+kaoNo].y;
            HuSprPosSet(status->gid, STATUS_SPRNO_KAO+kaoNo, posX, posY);
            kaoNo++;
        }
    }
    for(i=0; i<2; i++) {
        MBSpriteCreate(MBDATANUM(BOARD_ANM_statusCom), 1490, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, STATUS_SPRNO_COM+i, sprId);
        HuSprAttrSet(status->gid, STATUS_SPRNO_COM+i, HUSPR_ATTR_LINEAR);
        MBSpriteCreate(MBDATANUM(BOARD_ANM_statusSaiMode), 1490, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, STATUS_SPRNO_SAIMODE+i, sprId);
        HuSprAttrSet(status->gid, STATUS_SPRNO_SAIMODE+i, HUSPR_ATTR_LINEAR|HUSPR_ATTR_NOANIM|HUSPR_ATTR_DISPOFF);
    }
    for(i=0; i<5; i++) {
        MBSpriteCreate(MBDATANUM(BOARD_ANM_statusCapsule), 1500, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, STATUS_SPRNO_CAPSULE+i, sprId);
        HuSprAttrSet(status->gid, STATUS_SPRNO_CAPSULE+i, HUSPR_ATTR_NOANIM);
        HuSprAttrSet(status->gid, STATUS_SPRNO_CAPSULE+i, HUSPR_ATTR_LINEAR);
    }
    HuSprTPLvlSet(status->gid, STATUS_SPRNO_BACK, 0.7f);
}

#undef STATUS_SPR_INITNUM

void MBSpriteCreate(int dataNum, int prio, ANIMDATA **animP, HUSPRID *sprIdP)
{
    HUSPRID sprId;
    void *data;
    ANIMDATA *anim;
    data = HuDataSelHeapReadNum(dataNum, HU_MEMNUM_OVL, HUHEAPTYPE_MODEL);
    anim = HuSprAnimRead(data);
    sprId = HuSprCreate(anim, prio, 0);
    if(animP) {
        *animP = anim;
    }
    if(sprIdP) {
        *sprIdP = sprId;
    }
}

void MBSpriteNumSet(HUSPRGRPID gid, s16 memberNo, int num)
{
    int digit100 = num/100;
    int digit10 = (num-(digit100*100))/10;
    int digit1 = (num-((digit100*100)+(digit10*10)))%10;
    if(digit100 == 0) {
        HuSprBankSet(gid, memberNo+2, 10);
    } else {
        HuSprBankSet(gid, memberNo+2, digit100);
    }
    if(digit10 == 0 && digit100 == 0) {
        HuSprBankSet(gid, memberNo+1, digit1);
        HuSprDispOff(gid, memberNo);
    } else {
        HuSprBankSet(gid, memberNo+1, digit10);
        HuSprBankSet(gid, memberNo, digit1);
        HuSprDispOn(gid, memberNo);
    }
}

static void StatusPauseHook(BOOL dispF)
{
    int i;
    STATUSWORK *status;
    if(!dispF) {
        statusForceFlag = 0;
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBStatusDispGet(i)) {
                status = StatusPlayerGet(i);
                HuSprGrpPosSet(status->gid, status->posBegin.x, status->posBegin.y);
                statusForceFlag |= (1 << i);
            }
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(statusForceFlag & (1 << i)) {
                status = StatusPlayerGet(i);
                HuSprGrpPosSet(status->gid, status->posEnd.x, status->posEnd.y);
            }
        }
    }
}

void MBStatusRainbowSet(int statusNo, float time, int colorEnd)
{
    STATUSWORK *status = &statusWork[statusNo];
    int colorTbl[] = {
        STATUS_COLOR_GREEN,
        STATUS_COLOR_BLUE,
        STATUS_COLOR_GREEN,
        STATUS_COLOR_RED,
        STATUS_COLOR_GREEN,
        STATUS_COLOR_GRAY
    };
    int colorMax = 6;
    int colorTime;
    float weight;
    GXColor *colorP;
    GXColor *colorNext;
    int colorR, colorG, colorB;
    colorTbl[colorMax-1] = colorEnd;
    colorTime = time*(colorMax-1);
    colorP = &statusColTbl[colorTbl[colorTime]];
    colorNext = &statusColTbl[colorTbl[colorTime+1]];
    weight = (float)(colorMax-1)*fmod(time, 1.0f/(colorMax-1));
    colorR = (int)colorP->r+(weight*(colorNext->r-colorP->r));
    colorG = (int)colorP->g+(weight*(colorNext->g-colorP->g));
    colorB = (int)colorP->b+(weight*(colorNext->b-colorP->b));
    HuSprColorSet(status->gid, STATUS_SPRNO_BACK, colorR, colorG, colorB);
    (void)colorTime;
}

void MBStatusColorSet(int playerNo, int color)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    if(!GWTeamFGet()) {
        HuSprColorSet(status->gid, STATUS_SPRNO_BACK, statusColTbl[color].r, statusColTbl[color].g, statusColTbl[color].b);
    }
    GwPlayer[playerNo].color = color;
}

int MBStatusColorGet(int playerNo)
{
    return GwPlayer[playerNo].color;
}

void MBStatusColorAllSet(int color)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBStatusColorSet(i, color);
    }
}