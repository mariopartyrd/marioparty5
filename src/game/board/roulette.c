#include "game/board/roulette.h"
#include "game/board/audio.h"
#include "game/board/player.h"
#include "game/board/status.h"

#include "game/sprite.h"
#include "datanum/capsule.h"


static s8 rouletteAngleNo = -1;
static HUSPRGRPID rouletteSprGrpId = HUSPR_GRP_NONE;
static int rouletteSeNo = MSM_SENO_NONE;
static float rouletteDecel = 0.92f;

static s8 rouletteResultTbl[GW_PLAYER_MAX];
static s16 rouletteComDelay;
static s16 rouletteCounter;
static float rouletteMaxSpeed;
static s16 rouletteValueForce;
static float rouletteOffset;
static s16 rouletteValueForceF;
static u8 rouletteCapsuleF;
static OMOBJ *rouletteObj;

typedef struct RouletteWork_s {
    unsigned killF : 1;
    unsigned stopF : 1;
    unsigned mode : 4;
    u8 maxPlayer;
    s8 playerNo;
    s8 time;
    u8 hideTime;
    u8 angleNo;
    s16 maxCounter;
    s16 angle;
    s16 speed;
} ROULETTE_WORK;

static void RouletteObjMain(OMOBJ *obj);

static BOOL RouletteCreate(int playerNo, int maxPlayer, BOOL capsuleF)
{
    ROULETTE_WORK *work;
    int i;
    int num;
    if(capsuleF) {
        maxPlayer = 3;
        rouletteCapsuleF = TRUE;
    } else {
        rouletteCapsuleF = FALSE;
    }
    if(rouletteCapsuleF) {
        rouletteDecel = ((frand() & 0x7F)*(1.0f/127.0f)*0.05f)+0.92f;
    } else {
        rouletteDecel = 0.92f;
    }
    maxPlayer = 3;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        rouletteResultTbl[i] = -1;
    }
    if(!rouletteCapsuleF) {
        for(i=0, num=0; i<GW_PLAYER_MAX; i++) {
            if(maxPlayer != 3 || i != playerNo) {
                if(MBPlayerAliveCheck(i)) {
                    rouletteResultTbl[num++] = i;
                }
            }
        }
    } else {
        for(i=0, num=0; i<3; i++) {
            rouletteResultTbl[num++] = i;
        }
    }
    maxPlayer = num;
    if(maxPlayer <= 1) {
        if(num == 0) {
            rouletteResultTbl[0] = playerNo;
        }
        rouletteAngleNo = 0;
        return FALSE;
    }
    rouletteObj = MBAddObj(259, 0, 0, RouletteObjMain);
    work = omObjGetWork(rouletteObj, ROULETTE_WORK);
    work->killF = FALSE;
    work->stopF = FALSE;
    work->maxPlayer = maxPlayer;
    work->playerNo = playerNo;
    work->time = 0;
    work->hideTime = 0;
    work->angleNo = 0;
    work->angle = 0;
    rouletteMaxSpeed = 18;
    if(GwPlayer[work->playerNo].comF) {
        work->maxCounter = 180;
    } else {
        work->maxCounter = 3600;
    }
    return TRUE;
}

BOOL MBRouletteCreate(int playerNo, int maxPlayer)
{
    return RouletteCreate(playerNo, maxPlayer, FALSE);
}

BOOL MBRouletteKaneCreate(int playerNo)
{
    return RouletteCreate(playerNo, 3, TRUE);
}

BOOL MBRouletteCheck(void)
{
    return (rouletteObj != NULL) ? FALSE : TRUE;
}

void MBRouletteWait(void)
{
    while(!MBRouletteCheck()) {
        HuPrcVSleep();
    }
}

int MBRouletteResultGet(void)
{
    return rouletteResultTbl[rouletteAngleNo];
}

void MBRouletteMaxSpeedSet(float speed)
{
    if(speed <= 0) {
        speed = 18;
    }
    rouletteMaxSpeed = speed;
}

static void RouletteObjKill(void);

static void RouletteObjInit(ROULETTE_WORK *work);
static void RouletteObjDispOn(ROULETTE_WORK *work);
static void RouletteObjWait(ROULETTE_WORK *work);
static void RouletteObjResultDisp(ROULETTE_WORK *work);
static void RouletteObjDispOff(ROULETTE_WORK *work);

static void RouletteObjMain(OMOBJ *obj)
{
    ROULETTE_WORK *work = omObjGetWork(obj, ROULETTE_WORK);
    if(work->killF || MBKillCheck()) {
        RouletteObjKill();
        rouletteObj = NULL;
        MBDelObj(obj);
        return;
    }
    if(rouletteCounter < work->maxCounter) {
        rouletteCounter++;
    }
    switch(work->mode) {
        case 0:
            RouletteObjInit(work);
            break;
        
        case 1:
            RouletteObjDispOn(work);
            break;
        
        case 2:
            RouletteObjWait(work);
            break;
        
        case 3:
            RouletteObjResultDisp(work);
            break;
        
        case 4:
            RouletteObjDispOff(work);
            break;
    }
}

static void RouletteResultTblGet(int maxPlayer, int playerNo, s8 *result);
static s16 RouletteComDelayGet(int playerNo, int maxPlayer);

static int rouletteFileTbl[8] = {
    BOARD_ANM_rouletteBorder,
    BOARD_ANM_rouletteSeg3,
    BOARD_ANM_rouletteSeg3,
    BOARD_ANM_rouletteSeg3,
    BOARD_ANM_rouletteSeg2,
    BOARD_ANM_rouletteSeg2,
    BOARD_ANM_rouletteCursor,
    BOARD_ANM_rouletteCursorShadow
};

static s8 roulettePrioTbl[8+GW_PLAYER_MAX] = {
    60,
    80,
    80,
    80,
    80,
    80,
    20,
    30,
    70,
    70,
    70,
    70
};

static int charFileTbl[CHARNO_MAX] = {
    BOARD_ANM_rouletteMario,
    BOARD_ANM_rouletteLuigi,
    BOARD_ANM_roulettePeach,
    BOARD_ANM_rouletteYoshi,
    BOARD_ANM_rouletteWario,
    BOARD_ANM_rouletteDaisy,
    BOARD_ANM_rouletteWaluigi,
    BOARD_ANM_rouletteKinopio,
    BOARD_ANM_rouletteTeresa,
    BOARD_ANM_rouletteMinikoopa,
    BOARD_ANM_rouletteMinikoopaR,
    BOARD_ANM_rouletteMinikoopaG,
    BOARD_ANM_rouletteMinikoopaB
};

static int rouletteCapsuleFileTbl[4] = {
    CAPSULE_ANM_rouletteCoin,
    CAPSULE_ANM_rouletteStar,
    CAPSULE_ANM_rouletteCapsuleMulti,
    CAPSULE_ANM_rouletteCapsule
};

static void RouletteObjInit(ROULETTE_WORK *work)
{
    GXColor colorTbl[13] = {
        { 227, 67, 67, 255 },
        { 68, 67, 227, 255 },
        { 241, 158, 220, 255 },
        { 67, 228, 68, 255 },
        { 138, 60, 180, 255 },
        { 227, 228, 68, 255 },
        { 40, 40, 40, 255 },
        { 227, 227, 227, 255 },
        { 40, 227, 227, 255 },
        { 227, 139, 40, 255 },
        { 180, 40, 40, 255 },
        { 40, 180, 40, 255 },
        { 40, 40, 180, 255 },
    };
    
    GXColor color;
    s16 i;
    s16 num;
    HUSPRID sprId;
    
    s16 comDelay;
    int sprOfs;
    
    float zRotScale;
    float zRotOfs;
    float rate;
    
    rouletteSeNo = MSM_SENO_NONE;
    rouletteCounter = 0;
    switch(work->maxPlayer) {
        case 3:
            num = 3;
            sprOfs = 1;
            rate = 60;
            break;
        
        case 2:
            num = 2;
            sprOfs = 4;
            rate = 60;
            break;
            
    }
    rouletteSprGrpId = HuSprGrpCreate(num+12);
    RouletteResultTblGet(work->maxPlayer, work->playerNo, rouletteResultTbl);
    if(GwPlayer[work->playerNo].comF) {
        rouletteComDelay = RouletteComDelayGet(work->playerNo, work->maxPlayer);;
        switch(GwPlayer[work->playerNo].dif) {
            case GW_DIF_EASY:
                comDelay = frandmod(80);
                break;
            
            case GW_DIF_NORMAL:
                comDelay = frandmod(60);
                break;
            
            case GW_DIF_HARD:
                comDelay = frandmod(30);
                break;
            
            case GW_DIF_VERYHARD:
                comDelay = frandmod(10);
                break;
        }
        comDelay -= frandmod(20);
        rouletteComDelay += comDelay;
        if(rouletteComDelay > 360.0f) {
            rouletteComDelay -= 360.0f;
        }
        if(rouletteComDelay < 0) {
            rouletteComDelay += 360.0f;
        }
    } else {
        rouletteComDelay = 0;
    }
    for(i=0; i<=7; i++) {
        MBSpriteCreate(rouletteFileTbl[i], roulettePrioTbl[i], NULL, &sprId);
        HuSprGrpMemberSet(rouletteSprGrpId, i, sprId);
        HuSprAttrSet(rouletteSprGrpId, i, HUSPR_ATTR_LINEAR);
    }
    for(i=1; i<6; i++) {
        HuSprDispOff(rouletteSprGrpId, i);
    }
    switch(work->maxPlayer) {
        case 3:
            for(i=0; i<3; i++) {
                HuSprDispOn(rouletteSprGrpId, i+1);
            }
            break;
        
        case 2:
            for(i=0; i<4; i++) {
                HuSprDispOn(rouletteSprGrpId, i+4);
            }
            break;
    }
    for(i=0; i<num; i++) {
        if(!rouletteCapsuleF) {
            s16 charNo = GwPlayer[rouletteResultTbl[i]].charNo;
            MBSpriteCreate(charFileTbl[charNo], roulettePrioTbl[i+8], NULL, &sprId);
        } else {
            MBSpriteCreate(rouletteCapsuleFileTbl[rouletteResultTbl[i]], roulettePrioTbl[i+8], NULL, &sprId);
        }
        HuSprGrpMemberSet(rouletteSprGrpId, i+8, sprId);
        HuSprAttrSet(rouletteSprGrpId, i+8, HUSPR_ATTR_LINEAR);
    }
    HuSprPosSet(rouletteSprGrpId, 7, 3, 3);
    HuSprColorSet(rouletteSprGrpId, 7, 64, 64, 64);
    HuSprGrpPosSet(rouletteSprGrpId, 288, 240);
    HuSprGrpScaleSet(rouletteSprGrpId, 0.01f, 0.01f);
    OSs16tof32(&num, &zRotScale);
    zRotScale = 360/zRotScale;
    for(i=0; i<num; i++) {
        switch(work->maxPlayer) {
            case 3:
                switch(i) {
                    case 0:
                        HuSprPosSet(rouletteSprGrpId, i+8, 50, -35);
                        break;
                    
                    case 1:
                        HuSprPosSet(rouletteSprGrpId, i+8, 0, 57);
                        break;
                    
                    case 2:
                        HuSprPosSet(rouletteSprGrpId, i+8, -50, -35);
                        break;
                }
                break;
            
            case 2:
                switch(i) {
                    case 0:
                        HuSprPosSet(rouletteSprGrpId, i+8, 50, 0);
                        break;
                    
                    case 1:
                        HuSprPosSet(rouletteSprGrpId, i+8, -50, 0);
                        break;
                        
                }
                break;
        }
        OSs16tof32(&i, &zRotOfs);
        HuSprZRotSet(rouletteSprGrpId, sprOfs+i, zRotScale*zRotOfs);
        color = colorTbl[GwPlayer[rouletteResultTbl[i]].charNo];
        HuSprColorSet(rouletteSprGrpId, sprOfs+i, color.r, color.g, color.b);
    }
    for(i=0; i<3; i++) {
        if(rouletteResultTbl[i] == rouletteValueForce) {
            break;
        }
    }
    rouletteOffset = i*120.0f;
    if(rouletteOffset > 180) {
        rouletteOffset -= 360;
    }
    work->mode = 1;
}

static int RoulettePadGet(ROULETTE_WORK *work);
static int RouletteAngleNoGet(ROULETTE_WORK *work);

static void RouletteObjWait(ROULETTE_WORK *work)
{
    float speed;
    float angle;
    
    float pitch;
    
    s16 angle2;
    s16 angleDist;
    u32 btn;
    s16 pitchVal;
    
    OSs16tof32(&work->angle, &angle);
    OSs16tof32(&work->speed, &speed);
    angle = 0.0625f*angle;
    speed = 0.0625f*speed;
    if(!work->stopF) {
        if(speed < 18) {
            speed += 0.7f;
            if(speed > rouletteMaxSpeed) {
                speed = rouletteMaxSpeed;
            }
        } else {
            btn = RoulettePadGet(work);
            if(btn & PAD_BUTTON_A) {
                work->stopF = TRUE;
            }
        }
    } else {
        OSf32tos16(&angle, &angle2);
        switch(work->maxPlayer) {
            case 3:
                angleDist = 120;
                break;
            
            case 2:
                angleDist = 180;
                break;
        }
        angle2 %= angleDist;
        if(speed < 0.5f && (angle2 < 2 || angle2 >= angleDist-2)) {
            speed += speed/2;
        }
        speed *= rouletteDecel;
    }
    if(work->angleNo != RouletteAngleNoGet(work)) {
        work->angleNo = RouletteAngleNoGet(work);
    }
    pitch = 8191-(8191*(speed/rouletteMaxSpeed));
    OSf32tos16(&pitch, &pitchVal);
    HuAudFXPitchSet(rouletteSeNo, -pitchVal);
    if(speed > -0.0000001f && speed < 0.0000001f) {
        work->mode = 3;
        if(rouletteSeNo != MSM_SENO_NONE) {
            MBAudFXStop(rouletteSeNo);
            rouletteSeNo = MSM_SENO_NONE;
        }
        MBAudFXPlay(MSM_SE_BOARD_11);
    }
    angle += speed;
    if(angle > 360) {
        angle -= 360;
    }
    HuSprZRotSet(rouletteSprGrpId, 6, angle);
    HuSprZRotSet(rouletteSprGrpId, 7, angle);
    angle *= 16;
    speed *= 16;
    OSf32tos16(&angle, &work->angle);
    OSf32tos16(&speed, &work->speed);
}

static void RouletteObjResultDisp(ROULETTE_WORK *work)
{
    u8 hideTime;
    if(work->hideTime == 0) {
        work->angle >>= 4;
        rouletteAngleNo = RouletteAngleNoGet(work);
    }
    hideTime = work->hideTime%6;
    if(hideTime < 3) {
        HuSprDispOff(rouletteSprGrpId, rouletteAngleNo+8);
    } else {
        HuSprDispOn(rouletteSprGrpId, rouletteAngleNo+8);
    }
    if(work->hideTime < 90) {
        work->hideTime++;
    } else {
        HuSprDispOn(rouletteSprGrpId, rouletteAngleNo+8);
        work->mode = 4;
    }
}

static void RouletteObjDispOn(ROULETTE_WORK *work)
{
    float angle;
    if(work->time < 90) {
        work->time += 3;
    } else {
        _CheckFlag(FLAG_BOARD_TUTORIAL);
        work->time = 90;
        work->mode = 2;
        rouletteSeNo = MBAudFXPlay(MSM_SE_BOARD_09);
    }
    OSs8tof32(&work->time, &angle);
    HuSprGrpScaleSet(rouletteSprGrpId, HuSin(angle), HuSin(angle));
}

static void RouletteObjDispOff(ROULETTE_WORK *work)
{
    float angle;
    if(work->time > 0) {
        work->time -= 3;
    } else {
        work->time = 0;
        work->killF = TRUE;
        work->mode = 255;
        rouletteValueForceF = FALSE;
    }
    OSs8tof32(&work->time, &angle);
    HuSprGrpScaleSet(rouletteSprGrpId, HuSin(angle), HuSin(angle));
}

static void RouletteObjKill(void)
{
    if(rouletteSprGrpId != HUSPR_GRP_NONE) {
        HuSprGrpKill(rouletteSprGrpId);
        rouletteSprGrpId = HUSPR_GRP_NONE;
    }
}

static void RouletteResultTblGet(int maxPlayer, int playerNo, s8 *result)
{
    int i;
    for(i=0; i<255; i++) {
        int index1 = (frand() & 0x7FFF)%maxPlayer;
        int index2 = (frand() & 0x7FFF)%maxPlayer;
        if(index1 != index2) {
            s8 temp = result[index1];
            result[index1] = result[index2];
            result[index2] = temp;
        }
    }
    for(i=0; i<maxPlayer; i++) {
        
    }
}

static int RouletteComPadGet(ROULETTE_WORK *work);

static int RoulettePadGet(ROULETTE_WORK *work)
{
    int btn;
    if(rouletteCounter >= work->maxCounter) {
        return PAD_BUTTON_A;
    }
    if(!GwPlayer[work->playerNo].comF) {
        btn = HuPadBtnDown[GwPlayer[work->playerNo].padNo];
        return btn;
    } else {
        btn = RouletteComPadGet(work);
        if(rouletteValueForceF) {
            ROULETTE_WORK *workP = omObjGetWork(rouletteObj, ROULETTE_WORK);
            float angleStart;
            float angleEnd;
            float angle;
            float speed;
            OSs16tof32(&workP->angle, &angle);
            OSs16tof32(&workP->speed, &speed);
            angle = 0.0625f*angle;
            speed = 0.0625f*speed;
            angleStart = angle+(speed*46);
            angleStart = fmod(angleStart, 360);
            if(angleStart > 180) {
                angleStart -= 360;
            }
            angleEnd = rouletteOffset-angleStart;
            if(angleEnd >= 360) {
                angleEnd += 360;
            }
            if(angleEnd >= 0 && angleEnd < (120-(2*speed))) {
                btn = PAD_BUTTON_A;
            } else {
                btn = 0;
            }
        }
        return btn;
    }
}

void MBRouletteComValueSet(s16 value)
{
    rouletteValueForceF = TRUE;
    rouletteValueForce = value;
}

static s16 RouletteComDelayGet(int playerNo, int maxPlayer)
{
    int playerBest;
    int num;
    int i;
    s16 delay;
    if(GWPartyFGet() == TRUE) {
        playerBest = MBPlayerBestPathGet();
    } else {
        if(MBPlayerStoryComCheck(playerNo)) {
            playerBest = MBPlayerStoryPlayerGet();
        } else {
            playerBest = -1;
        }
    }
    if(playerBest == -1 || playerBest == playerNo) {
        do {
            playerBest = frandmod(GW_PLAYER_MAX);
        } while(playerBest == playerNo);
        
    }
    num = maxPlayer;
    for(delay=i=0; i<num; i++) {
        if(playerBest == rouletteResultTbl[i]) {
            float angle = i*(360/num);
            angle += 45;
            angle -= 180;
            if(angle < 0) {
                angle += 360;
            }
            if(angle >= 360) {
                angle -= 360;
            }
            OSf32tos16(&angle, &delay);
            break;
        }
    }
    return delay;
}

static int RouletteComPadGet(ROULETTE_WORK *work)
{
    int angle;
    s16 origAngle;
    int delayMax;
    int delayMin;
    origAngle = work->angle >> 4;
    switch(work->maxPlayer) {
        case 3:
            angle = 120;
            break;
        
        case 2:
            angle = 180;
            break;
    }
    delayMin = rouletteComDelay;
    delayMax = rouletteComDelay+angle;
    if(origAngle >= delayMin && origAngle < delayMax) {
        return PAD_BUTTON_A;
    } else {
        return 0;
    }
}

static int RouletteAngleNoGet(ROULETTE_WORK *work)
{
    int result;
    switch(work->maxPlayer) {
        case 3:
            result = work->angle/120;
            break;
            
        case 2:
            result = work->angle/180;
            break;
    }
    return result;
}