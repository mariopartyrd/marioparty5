#include "game/board/main.h"
#include "game/board/effect.h"
#include "game/board/audio.h"
#include "game/board/model.h"
#include "game/board/masu.h"
#include "game/board/scroll.h"
#include "game/board/opening.h"
#include "game/board/capsule.h"
#include "game/board/branch.h"
#include "game/board/camera.h"
#include "game/board/player.h"
#include "game/board/guide.h"
#include "game/board/window.h"
#include "game/board/coin.h"
#include "game/board/roulette.h"
#include "game/board/star.h"
#include "game/board/capsule.h"

#include "game/sprite.h"
#include "game/wipe.h"
#include "game/gamemes.h"
#include "game/pad.h"

#include "datanum/w06.h"
#include "messnum/board_w06.h"

#define MB6EV_CAKEMOVE_1 MASU_FLAG_BITFIELD(0, 2, 1)
#define MB6EV_CAKEMOVE_2 MASU_FLAG_BITFIELD(0, 2, 2)
#define MB6EV_CAKEMOVE_3 MASU_FLAG_BITFIELD(0, 2, 3)
#define MB6EV_CAKEMOVE_FLAG MASU_FLAG_BIT_MULTI(0, 2)
#define MB6EV_CAKETHROW_FLAG MASU_FLAG_BIT(2)
#define MB6EV_KANE_FLAG MASU_FLAG_BIT(3)
#define MB6EV_EVENT_LINK_FLAG MASU_FLAG_BIT(4)
#define MB6EV_CAKEMOVE_LINK_FLAG MASU_FLAG_BIT(5)
#define MB6EV_COOKIE_FLAG MASU_FLAG_BIT_MULTI(6, 4)

#define MB6EV_CAKEMOVE_MAX 3
#define MB6EV_COOKIE_MAX 4
#define MB6EV_CAKEBALL_MAX 12

#define MB6_OBJ_LINE 0
#define MB6_OBJ_BACK 1
#define MB6_OBJ_BACK_EFF 2
#define MB6_OBJ_KANE 3
#define MB6_OBJ_CAKETHROW 4
#define MB6_OBJ_CAKETHROW_EFF 5

#define MB6_OBJ_MAX 6


typedef struct MB6Ev_Player_s {
    int ballNo;
    float ballDist;
    BOOL inEventF;
    float unkC;
    int mode;
    int masuId;
    BOOL shockEndF;
    int timer;
    float speed;
    float gravity;
    float angleStart;
    float angleSpeed;
    s16 motNo[10];
    HuVecF pos;
    HuVecF rot;
    HuVecF vel;
    HuVecF masuDir;
} MB6EV_PLAYER;

typedef struct MB6Ev_Cookie_s {
    int flag;
    int mode;
    int time;
    HuVecF pos;
    HuVecF rot;
    int modelId;
    int fallMode[4];
    MBMODELID fallMdlId[4];
} MB6EV_COOKIE;

typedef struct MB6Ev_CakeMove_s {
    int masuId;
    char *cakeHook;
    char *knifeHook;
    MBMODELID cakeMdlId;
    MBMODELID knifeMdlId;
} MB6EV_CAKEMOVE;

typedef struct MB6Ev_CakeBall_s {
    BOOL enableF;
    int mode;
    int unk8;
    int collStat;
    BOOL cancelHookF;
    float rotY;
    HuVecF hookPos;
    HuVecF targetPos;
    float scale;
    float time;
    float gravity;
} MB6EV_CAKEBALL;

typedef struct MB6_Work_s {
    int objMdlId[MB6_OBJ_MAX];
    int dummyMdlId[2];
    int cakeBallMdlId[MB6EV_CAKEBALL_MAX];
    int cakeThrowTimer;
    int cakeThrowBallNo;
    float cakeThrowRot;
    float cakeThrowRotSpeed;
    s8 cakeThrowBallF[5];
    ANIMDATA *effAnimParty;
    ANIMDATA *effAnimPuff;
    ANIMDATA *effAnimCookie;
    HU3DMODELID effMdlId[4]; 
    HU3DMODELID cookieEffMdlId;
    int unk80;
    int lightId;
    float lightPosX;
    float lightPosY;
    float lightPosZ;
    float lightDirX;
    float lightDirY;
    float lightDirZ;
    GXColor lightColor;
    int masuIdCookie[GW_PLAYER_MAX];
    MB6EV_PLAYER evPlayer[GW_PLAYER_MAX];
    MB6EV_COOKIE evCookie[MB6EV_COOKIE_MAX];
    MB6EV_CAKEMOVE evCakeMove[MB6EV_CAKEMOVE_MAX];
    MB6EV_CAKEBALL evCakeBall[MB6EV_CAKEBALL_MAX];
    int kaneMdlId[3];
    HSFMATERIAL *kaneMatBuf[3];
    HSFBUFFER haikeiBuf[2];
    HSFBUFFER haikeiBuf2[2];
} MB6_WORK;

typedef struct MB6SaveWork_s {
    u8 cookieF[2];
} MB6_SAVEWORK;

static MBMODELID mb6_MainMdlId;
static MB6_SAVEWORK *mb6_SaveWorkP;
static MB6_WORK mb6_Work;
static OMOBJ *mb6_MainObj;

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern void ObjectSetup(void);

int _prolog(void) {
    const VoidFunc* ctors = _ctors;
    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

void _epilog(void) {
    const VoidFunc* dtors = _dtors;
    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

void MB6_Create(void);
void MB6_Kill(void);
void MB6_LightSet(void);
void MB6_LightReset(void);
int MB6Ev_MasuWalkPre(int playerNo);
int MB6Ev_MasuWalkPost(int playerNo);
int MB6Ev_MasuHatena(int playerNo);
int MB6_MasuPathCheck(s16 id, u32 flag, s16 *linkTbl, BOOL endF);

void MB6Ev_CookieCreate(void);
void MB6Ev_CookieObjUpdate(void);
void MB6Ev_CookieKill(void);
void MB6Ev_Cookie(int playerNo, int masuId);

void MB6Ev_CookieEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix);

void MB6Ev_CakeMoveCreate(void);
void MB6Ev_CakeMoveKill(void);
void MB6Ev_CakeMove(int playerNo, int masuId);

void MB6Ev_CakeThrow(int playerNo);

void MB6Ev_KaneBackCreate(void);
void MB6Ev_KaneBackKill(void);
void MB6Ev_KaneBackTPLvlSet(float tpLvl);
void MB6Ev_Kane(int playerNo);

float MB6_AngleDiff(float start, float end);
unsigned int MB6Ev_BitGet(int outMask, unsigned int inMask);
unsigned int MB6Ev_MasuFlagGet(int outMask, unsigned int inMask);

void ObjectSetup(void)
{
    GWPartyFSet(TRUE);
    MBObjectSetup(MBNO_WORLD_6, MB6_Create, MB6_Kill);
}

static void MB6_ObjectCreate(void);
static void MB6_MainObjUpdate(OMOBJ *obj);

void MB6_Create(void)
{
    int boardNo = MBBoardNoGet();
    HuAudSndGrpSetSet(MSM_GRP_BD6);
    memset(&mb6_Work, 0, sizeof(MB6_WORK));
    mb6_SaveWorkP = MBSaveWorkPGet();
    MBMasuCreate(W06_BIN_masuData);
    mb6_MainMdlId = MBModelCreate(W06_HSF_map, NULL, FALSE);
    MBModelPosSet(mb6_MainMdlId, 0, 0, 0);
    MBModelAttrSet(mb6_MainMdlId, HU3D_MOTATTR_LOOP);
    MBScrollInit(W06_HSF_scrollCol, 10690, -631);
    {
        HuVecF cameraPos = { 0, 300, -1000 };
        HuVecF cameraRot = { -33, 0, 0 };
        HuVecF mapCameraPos = { 0, 0, -1000 };
        HuVecF mapCameraRot = { -60, 0, 0 };
        MBOpeningViewSet(&cameraRot, &cameraPos, 16000);
        MBMapViewCameraSet(&mapCameraRot, &mapCameraPos, 17000);
    }
    MBLightHookSet(MB6_LightSet, MB6_LightReset);
    MBMasuPathCheckHookSet(MB6_MasuPathCheck);
    MBCapsuleColCreate(W06_HSF_capsuleCol);
    MBBranchFlagSet(MASU_FLAG_BIT(4));
    
    MB6_ObjectCreate();
    MB6Ev_CookieCreate();
    MB6Ev_CakeMoveCreate();
    HuDataDirClose(DATA_w06);
    MBMasuWalkPreHookSet(MB6Ev_MasuWalkPre);
    MBMasuWalkPostHookSet(MB6Ev_MasuWalkPost);
    MBMasuHatenaHookSet(MB6Ev_MasuHatena);
    mb6_MainObj = MBAddObj(32768, 0, 0, MB6_MainObjUpdate);
    
}

static void MB6_ObjectCreate(void)
{
    int i;
    int modelId;
    static unsigned int objFileTbl[MB6_OBJ_MAX] = {
        W06_HSF_line,
        W06_HSF_back,
        W06_HSF_backEff,
        W06_HSF_kane,
        W06_HSF_cakeThrow,
        W06_HSF_cakeThrowEff
    };
    static unsigned int ballFileTbl[4] = {
        W06_HSF_cakeThrowBall1,
        W06_HSF_cakeThrowBall2,
        W06_HSF_cakeThrowBall3,
        W06_HSF_cakeThrowBall4
    };
    
    for(i=0; i<MB6_OBJ_MAX; i++) {
        mb6_Work.objMdlId[i] = MB_MODEL_NONE;
    }
    for(i=0; i<2; i++) {
        mb6_Work.dummyMdlId[i] = MB_MODEL_NONE;
    }
    for(i=0; i<4; i++) {
        mb6_Work.effMdlId[i] = HU3D_MODELID_NONE;
    }
    for(i=0; i<MB6_OBJ_MAX; i++) {
        if(objFileTbl[i]) {
            modelId = MBModelCreate(objFileTbl[i], NULL, FALSE);
            mb6_Work.objMdlId[i] = modelId;
            MBModelPosSet(modelId, 0, 0, 0);
            MBMotionSpeedSet(modelId, 0);
            MBMotionShapeSpeedSet(modelId, 0);
            MBModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
        }
    }
    modelId = mb6_Work.objMdlId[MB6_OBJ_BACK];
    if(modelId >= 0) {
        MBMotionSpeedSet(modelId, 1);
    }
    modelId = mb6_Work.objMdlId[MB6_OBJ_BACK_EFF];
    if(modelId >= 0) {
        MBMotionSpeedSet(modelId, 0.333333f);
    }
    modelId = mb6_Work.objMdlId[MB6_OBJ_CAKETHROW];
    if(modelId >= 0) {
        MBModelPosSet(modelId, 2400, 0, 400);
        mb6_Work.cakeThrowRotSpeed = 0.25f;
    }
    modelId = mb6_Work.objMdlId[MB6_OBJ_CAKETHROW_EFF];
    if(modelId >= 0) {
        MBModelDispSet(modelId, FALSE);
        MBModelAttrReset(modelId, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<=12; i++) {
        modelId = MBModelCreate(ballFileTbl[i & 0x3], NULL, TRUE);
        mb6_Work.cakeBallMdlId[i] = modelId;
        if(modelId >= 0) {
            MBModelDispSet(modelId, FALSE);
        }
    }
    modelId = mb6_Work.objMdlId[MB6_OBJ_KANE];
    if(modelId >= 0) {
        MBMotionSpeedSet(modelId, 0);
        MBMotionTimeSet(modelId, 15);
    }
    mb6_Work.effAnimParty = HuSprAnimRead(HuDataReadNum(W06_ANM_partyEff, HU_MEMNUM_OVL));
    mb6_Work.effAnimPuff = HuSprAnimRead(HuDataReadNum(W06_ANM_puffEff, HU_MEMNUM_OVL));
    mb6_Work.effAnimCookie = HuSprAnimRead(HuDataReadNum(W06_ANM_cookieEff, HU_MEMNUM_OVL));
    mb6_Work.effMdlId[0] = MBEffCreate(mb6_Work.effAnimParty, 1);
    mb6_Work.effMdlId[1] = MBEffCreate(mb6_Work.effAnimPuff, 1);
    Hu3DModelDispOff(mb6_Work.effMdlId[0]);
    Hu3DModelDispOff(mb6_Work.effMdlId[1]);
    mb6_Work.kaneMdlId[0] = mb6_MainMdlId;
    mb6_Work.kaneMdlId[1] = mb6_Work.objMdlId[MB6_OBJ_BACK];
    mb6_Work.kaneMdlId[2] = mb6_Work.objMdlId[MB6_OBJ_KANE];
    MB6Ev_KaneBackCreate();
}

void MB6_Kill(void)
{
    int i;
    MB6Ev_KaneBackKill();
    if(mb6_MainMdlId >= 0) {
        MBModelKill(mb6_MainMdlId);
        mb6_MainMdlId = MB_MODEL_NONE;
    }
    MBCameraModelViewSet(MB_MODEL_NONE, NULL, NULL, -1, 45, 63);
    for(i=0; i<MB6_OBJ_MAX; i++) {
        if(mb6_Work.objMdlId[i] >= 0) {
            MBModelKill(mb6_Work.objMdlId[i]);
        }
        mb6_Work.objMdlId[i] = MB_MODEL_NONE;
    }
    for(i=0; i<2; i++) {
        if(mb6_Work.dummyMdlId[i] >= 0) {
            MBModelKill(mb6_Work.dummyMdlId[i]);
        }
        mb6_Work.dummyMdlId[i] = MB_MODEL_NONE;
    }
    for(i=0; i<4; i++) {
        if(mb6_Work.effMdlId[i] >= 0) {
            MBEffKill(mb6_Work.effMdlId[i]);
        }
        mb6_Work.effMdlId[i] = HU3D_MODELID_NONE;
    }
    MB6Ev_CookieKill();
    MB6Ev_CakeMoveKill();
}

static void MB6_MainObjUpdate(OMOBJ *obj)
{
    MBCAMERA *cameraP;
    if(MBKillCheck()) {
        if(mb6_MainObj != NULL) {
            omDelObj(mbObjMan, mb6_MainObj);
        }
        return;
    }
    cameraP = MBCameraGet();
    MB6Ev_CookieObjUpdate();
    mb6_Work.cakeThrowRot += mb6_Work.cakeThrowRotSpeed;
    if(mb6_Work.cakeThrowRot >= 360) {
        mb6_Work.cakeThrowRot -= 360;
    }
    if(mb6_Work.cakeThrowRot < 0) {
        mb6_Work.cakeThrowRot += 360;
    }
    MBModelRotSet(mb6_Work.objMdlId[MB6_OBJ_CAKETHROW], 0, mb6_Work.cakeThrowRot, 0);
}

void MB6_LightSet(void)
{
    Hu3DShadowCreate(45, 1000, 250000);
    Hu3DShadowTPLvlSet(0.3f);
    Hu3DBGColorSet(0, 0, 0);
    mb6_Work.lightPosX = -3700;
    mb6_Work.lightPosY = 12000;
    mb6_Work.lightPosZ = 22000;
    mb6_Work.lightDirX = 3700;
    mb6_Work.lightDirY = -12000;
    mb6_Work.lightDirZ = -15000;
    mb6_Work.lightColor.r = 255;
    mb6_Work.lightColor.g = 255;
    mb6_Work.lightColor.b = 255;
    mb6_Work.lightId = Hu3DGLightCreate(mb6_Work.lightPosX, mb6_Work.lightPosY, mb6_Work.lightPosZ,
        mb6_Work.lightDirX, mb6_Work.lightDirY, mb6_Work.lightDirZ,
        mb6_Work.lightColor.r, mb6_Work.lightColor.g, mb6_Work.lightColor.b);
    Hu3DGLightStaticSet(mb6_Work.lightId, TRUE);
    Hu3DGLightInfinitytSet(mb6_Work.lightId);
}

void MB6_LightReset(void)
{
    
}

int MB6Ev_MasuHatena(int playerNo)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, GwPlayer[playerNo].masuId);
    u32 flag = masuP->flag & 0xFFFF;
    if(flag & MB6EV_CAKEMOVE_FLAG) {
        MB6Ev_CakeMove(playerNo, GwPlayer[playerNo].masuId);
    } else if(flag & MB6EV_CAKETHROW_FLAG) {
        MB6Ev_CakeThrow(playerNo);
    } else if(flag & MB6EV_KANE_FLAG) {
        MB6Ev_Kane(playerNo);
    }
}

int MB6_MasuStub()
{
    return 0;
}

int MB6Ev_MasuWalkPre(int playerNo)
{
    return 0;
}

void MB6Ev_CookiePathFlagSet(int masuId);

int MB6Ev_MasuWalkPost(int playerNo)
{
    s16 masuId = GwPlayer[playerNo].masuId;
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    u32 flag = masuP->flag  & 0xFFFF;
    if(flag & MB6EV_COOKIE_FLAG) {
        mb6_Work.masuIdCookie[playerNo] = masuId;
        MB6Ev_CookiePathFlagSet(masuId);
        MB6Ev_Cookie(playerNo, mb6_Work.masuIdCookie[playerNo]);
        mb6_Work.masuIdCookie[playerNo] = -1;
    }
    return 0;
}

static HuVecF mb6ev_CookiePosTbl[MB6EV_COOKIE_MAX] = {
    { 1450, 850, -2375 },
    { 1725, 850, -2100 },
    { -1750, 450, -550 },
    { -1475, 450, -275 }
};

static float mb6ev_CookieRotTbl[MB6EV_COOKIE_MAX] = {
    180,
    90,
    270,
    0
};

static int mb6ev_CookieNoTbl[MB6EV_COOKIE_MAX] = {
    1,
    0,
    1,
    0
};

static u32 mb6ev_CookiePathFlagTbl[MB6EV_COOKIE_MAX] = {
    MASU_FLAG_BIT(6),
    MASU_FLAG_BIT(7),
    MASU_FLAG_BIT(8),
    MASU_FLAG_BIT(9)
};

void MB6Ev_CookieCreate(void)
{
    MB6EV_COOKIE *cookieP;
    int i;
    int modelId;
    int j;
    BOOL enableF;
    
    HuVecF ofs;
    
    memset(&mb6_Work.evCookie[0], 0, MB6EV_COOKIE_MAX*sizeof(MB6EV_COOKIE));
    for(i=0; i<GW_PLAYER_MAX; i++) {
        mb6_Work.masuIdCookie[i] = -1;
    }
    for(cookieP=&mb6_Work.evCookie[0], i=0; i<MB6EV_COOKIE_MAX; i++, cookieP++) {
        cookieP->flag = -1;
        cookieP->pos = mb6ev_CookiePosTbl[i];
        cookieP->rot.y = mb6ev_CookieRotTbl[i];
        ofs.x = 150*HuSin(cookieP->rot.y);
        ofs.y = 0;
        ofs.z = 150*HuCos(cookieP->rot.y);
        enableF = (mb6_SaveWorkP->cookieF[i >> 1] & 0x1) == mb6ev_CookieNoTbl[i];
        if(enableF) {
            cookieP->mode = 2;
        } else {
            MBBranchFlagSet(mb6ev_CookiePathFlagTbl[i]);
        }
        modelId = MBModelCreate(W06_HSF_cookie, NULL, TRUE);
        cookieP->modelId = modelId;
        MBModelPosSet(modelId, cookieP->pos.x, cookieP->pos.y, cookieP->pos.z);
        MBModelRotSet(modelId, 0, cookieP->rot.y+180, 0);
        MBModelDispSet(modelId, enableF);
        for(j=0; j<4; j++) {
            cookieP->fallMode[j] = cookieP->mode;
            modelId = MBModelCreate(W06_HSF_cookieFall, NULL, TRUE);
            cookieP->fallMdlId[j] = modelId;
            MBModelPosSet(modelId, cookieP->pos.x+(ofs.x*j), cookieP->pos.y, cookieP->pos.z+(ofs.z*j));
            MBModelRotSet(modelId, 0, cookieP->rot.y+180, 0);
            MBModelDispSet(modelId, FALSE);
            MBMotionSpeedSet(modelId, 0);
            if(enableF) {
                MBMotionTimeSet(modelId, 29);
            }
        }
    }
    mb6_Work.cookieEffMdlId = MBEffCreate(mb6_Work.effAnimCookie, 64);
    if(mb6_Work.cookieEffMdlId > 0) {
        MBEFFECT *effP = Hu3DData[mb6_Work.cookieEffMdlId].hookData;
        effP->time = mb6_Work.cookieEffMdlId;
        MBEffHookSet(mb6_Work.cookieEffMdlId, MB6Ev_CookieEffHook);
        Hu3DModelLayerSet(mb6_Work.cookieEffMdlId, 2);
    }
}

void MB6Ev_CookieKill(void)
{
    MB6EV_COOKIE *cookieP;
    int j;
    int i;
    for(cookieP=&mb6_Work.evCookie[0], i=0; i<MB6EV_COOKIE_MAX; i++, cookieP++) {
        MBModelKill(cookieP->modelId);
        for(j=0; j<4; j++) {
            if(cookieP->fallMdlId[j] >= 0) {
                MBModelKill(cookieP->fallMdlId[j]);
                cookieP->fallMdlId[j] = MB_MODEL_NONE;
            }
        }
    }
    MBEffKill(mb6_Work.cookieEffMdlId);
}

void MB6Ev_CookiePathFlagSet(int masuId)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    u32 flag = masuP->flag & MB6EV_COOKIE_FLAG;
    int i;
    for(i=0; i<MB6EV_COOKIE_MAX; i++) {
        if(flag == mb6ev_CookiePathFlagTbl[i]) {
            int other = i^1;
            MBBranchFlagSet(mb6ev_CookiePathFlagTbl[i]);
            MBBranchFlagReset(mb6ev_CookiePathFlagTbl[other]);
        }
    }
}

void MB6Ev_CookieEffCreate(int modelId, HuVecF *pos, int no);

void MB6Ev_CookieObjUpdate(void)
{
    MB6EV_COOKIE *cookieP;
    int j;
    int modelId;
    int i;
    BOOL resetF;
    BOOL dispF;
    float time;
    
    HuVecF pos;
    
    for(cookieP=&mb6_Work.evCookie[0], i=0; i<MB6EV_COOKIE_MAX; i++, cookieP++) {
        if(cookieP->flag >= 0) {
            if(cookieP->flag == 0) {
                cookieP->mode = 2;
                time = 29;
                dispF = TRUE;
            } else {
                cookieP->mode = 0;
                time = 0;
                dispF = FALSE;
            }
            MBModelDispSet(cookieP->modelId, FALSE);
            for(j=0; j<4; j++) {
                modelId = cookieP->fallMdlId[j];
                MBModelDispSet(modelId, dispF);
                MBMotionSpeedSet(modelId, 0);
                MBMotionTimeSet(modelId, time);
                cookieP->fallMode[j] = cookieP->mode;
            }
            cookieP->time = 0;
            cookieP->mode++;
            cookieP->flag = -1;
        }
        switch(cookieP->mode) {
            case 0:
                break;
                
            case 1:
                j = cookieP->time/12;
                if(cookieP->time == j*12 && j < 4) {
                    cookieP->fallMode[j] = cookieP->mode;
                    modelId = cookieP->fallMdlId[j];
                    MBMotionSpeedSet(modelId, 1);
                    MBModelDispSet(modelId, TRUE);
                    if(j == 0) {
                        MBAudFXPlay(MSM_SE_BD6_09);
                    }
                }
                resetF = TRUE;
                for(j=0; j<4; j++) {
                    if(cookieP->fallMode[j] == cookieP->mode) {
                        modelId = cookieP->fallMdlId[j];
                        if(MBMotionTimeGet(modelId) >= 29) {
                            MBMotionTimeSet(modelId, 29);
                            MBMotionSpeedSet(modelId, 0);
                            cookieP->fallMode[j] = 2;
                            MBAudFXPlay(MSM_SE_BD6_02);
                        }
                    }
                    if(cookieP->fallMode[j] < 2) {
                        resetF = FALSE;
                    }
                }
                if(resetF) {
                    MBModelDispSet(cookieP->modelId, TRUE);
                    for(j=0; j<4; j++) {
                        MBModelDispSet(cookieP->fallMdlId[j], FALSE);
                    }
                    cookieP->mode = 2;
                }
                break;
            
            case 2:
                break;
                
            case 3:
                j = cookieP->time/12;
                if(cookieP->time == j*12 && j < 4) {
                    cookieP->fallMode[j] = cookieP->mode;
                    modelId = cookieP->fallMdlId[j];
                    MBMotionSpeedSet(modelId, 1);
                    MBModelPosGet(modelId, &pos);
                    MB6Ev_CookieEffCreate(mb6_Work.cookieEffMdlId, &pos, i);
                    MBAudFXPlay(MSM_SE_BD6_01);
                    if(j == 0) {
                        MBAudFXPlay(MSM_SE_BD6_08);
                    }
                }
                resetF = TRUE;
                for(j=0; j<4; j++) {
                    if(cookieP->fallMode[j] == cookieP->mode) {
                        modelId = cookieP->fallMdlId[j];
                        if(MBMotionEndCheck(modelId)) {
                            MBMotionSpeedSet(modelId, 0);
                            MBModelDispSet(modelId, FALSE);
                            cookieP->fallMode[j] = 0;
                        }
                    }
                    if(cookieP->fallMode[j] >= 2) {
                        resetF = FALSE;
                    }
                }
                if(resetF) {
                    cookieP->mode =  0;
                    j = i^1;
                    mb6_Work.evCookie[j].flag = TRUE;
                }
                break;
        }
        cookieP->time++;
    }
}

void MB6Ev_Cookie(int playerNo, int masuId)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    u32 flag = masuP->flag & MB6EV_COOKIE_FLAG;
    int linkMasuId = masuP->linkTbl[0];
    MASU *linkMasuP = MBMasuGet(MASU_LAYER_DEFAULT, linkMasuId);
    int i;
    HuVecF pos;
    HuVecF speed;
    float time;
    float zoom;
    BOOL endF;
    BOOL posFixF;
    
    MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_RUN, HU3D_MOTATTR_LOOP);
    for(i=0; i<MB6EV_COOKIE_MAX; i++) {
        if(flag == mb6ev_CookiePathFlagTbl[i]) {
            break;
        }
    }
    if(i >= MB6EV_COOKIE_MAX) {
        i = 0;
    }
    i &= 0xFFFE;
    VECAdd(&mb6ev_CookiePosTbl[i], &mb6ev_CookiePosTbl[i+1], &pos);
    VECScale(&pos, &pos, 0.5f);
    pos.x += ((HuSin(mb6ev_CookieRotTbl[i])+HuSin(mb6ev_CookieRotTbl[i+1]))*150)*2;
    pos.z += ((HuCos(mb6ev_CookieRotTbl[i])+HuCos(mb6ev_CookieRotTbl[i+1]))*150)*2;
    pos.z += 75;
    MBCameraFocusPlayerSet(-1);
    zoom = MBCameraZoomGet();
    MBWalkNumDispSet(playerNo, FALSE);
    MBCameraPosViewSet(&pos, NULL, NULL, 4000, -1, 45);
    for(i=0; i<MB6EV_COOKIE_MAX; i++) {
        if(flag == mb6ev_CookiePathFlagTbl[i]) {
            mb6_Work.evCookie[i].flag = FALSE;
            mb6_SaveWorkP->cookieF[i >> 1] = 1-mb6ev_CookieNoTbl[i];
        }
    }
    time = 48;
    MBPlayerPosGet(playerNo, &pos);
    VECSubtract(&linkMasuP->pos, &pos, &speed);
    VECScale(&speed, &speed, 1/time);
    posFixF = FALSE;
    endF = FALSE;
    while(!endF) {
        endF = TRUE;
        for(i=0; i<MB6EV_COOKIE_MAX; i++) {
            if(mb6_Work.evCookie[i].mode & 0x1) {
                endF = FALSE;
            }
            if(mb6_Work.evCookie[i].flag >= 0) {
                endF = FALSE;
            }
        }
        if(time > 0) {
            MBPlayerPosGet(playerNo, &pos);
            VECAdd(&pos, &speed, &pos);
            MBPlayerPosSetV(playerNo, &pos);
            if(time < 24 && !posFixF) {
                MBPlayerPosFixWarp(playerNo, linkMasuId, FALSE);
                posFixF = TRUE;
            }
            time--;
            if(time <= 0) {
                MBPlayerMotIdleSet(playerNo);
            }
        }
        HuPrcVSleep();
    }
    GwPlayer[playerNo].masuId = linkMasuId;
    MBCameraPlayerViewSet(playerNo, NULL, NULL, zoom, -1, 45);
    for(i=0; i<45u; i++) {
        HuPrcVSleep();
    }
    MBCameraFocusPlayerSet(playerNo);
    MBWalkNumDispSet(playerNo, TRUE);
    
}

int MB6_MasuPathCheck(s16 id, u32 flag, s16 *linkTbl, BOOL endF)
{
    int linkNum = 0;
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, id);
    int i;
    for(i=0; i<masuP->linkNum; i++) {
        MASU *linkMasuP;
        int linkMasuId;
        linkMasuId = masuP->linkTbl[i];
        linkMasuP = MBMasuGet(MASU_LAYER_DEFAULT, linkMasuId);
        if(linkMasuP->flag & MB6EV_COOKIE_FLAG) {
            linkTbl[linkNum++] = linkMasuId;
        }
    }
    if(linkNum <= 0) {
        linkNum = -1;
    }
    return linkNum;
}

void MB6Ev_CookieEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix)
{
    MBEFFECTDATA *effDataP;
    int i;
    int dispNum;
    if(effP->count == 0) {
        for(effDataP = effP->data, i=0; i<effP->num; i++, effDataP++) {
            effDataP->scale = 0;
            effDataP->color.a = 0;
        }
        effP->count = 1;
    }
    dispNum = 0;
    for(effDataP = effP->data, i=0; i<effP->num; i++, effDataP++) {
        if(effDataP->time) {
            effDataP->vel.y += -0.8166668f;
            VECAdd(&effDataP->pos, &effDataP->vel, &effDataP->pos);
            if(effDataP->time < 9.0f) {
                effDataP->color.a = 0.6f*effDataP->color.a;
            }
            if(--effDataP->time == 0) {
                effDataP->time = 0;
                effDataP->scale = 0;
                effDataP->color.a = 0;
            }
            dispNum++;
        }
    }
    if(dispNum <= 0) {
        Hu3DModelDispOff(effP->time);
    }
}

void MB6Ev_CookieEffCreate(int modelId, HuVecF *pos, int no)
{
    MBEFFECT *effP = Hu3DData[modelId].hookData;
    HuVecF ofs[4] = {
        { 200, 0, 75 },
        { 75, 0, 200 },
        { 75, 0, 200 },
        { 200, 0, 75 },
    };
    HuVecF dir;
    MBEFFECTDATA *effDataP;
    int i;
    unsigned int max;
    
    Hu3DModelDispOn(modelId);
    for(effDataP=effP->data, max=20, i=0; i<effP->num; i++, effDataP++) {
        if(effDataP->time) {
            continue;
        }
        effDataP->time = ((0.3f*MBEffRandF())+0.4f)*60;
        effDataP->animBank = (int)(MBEffRandF()*4) & 0x3;
        effDataP->pos = *pos;
        dir.x = MBEffSRandF();
        dir.y = MBEffRandF();
        dir.z = MBEffSRandF();
        effDataP->pos.x += dir.x*ofs[no].x;
        effDataP->pos.y += (dir.y*-0.2f)*100;
        effDataP->pos.z += dir.z*ofs[no].z;
        effDataP->vel.x = (2.5f*(dir.x+(MBEffSRandF()*0.3f)))*1.6666666f;
        effDataP->vel.y = (1.6666666f*(MBEffRandF()*-1.3f))-0.8333333f;
        effDataP->vel.z = (2.5f*(dir.z+(MBEffSRandF()*0.3f)))*1.6666666f;
        effDataP->rot.z = MBEffRandF()*360;
        effDataP->scale = (MBEffRandF()*45)+5;
        effDataP->color.a = (MBEffRandF()*50)+200;
        effDataP->color.r = effDataP->color.g = effDataP->color.b = 255;
        max--;
        if(max < 1) {
            break;
        }
    }
}

void MB6Ev_CakeMoveCreate(void)
{
    MB6EV_CAKEMOVE *cakeMove;
    int i;
    int modelId;
    
    static unsigned int cakeMoveData[MB6EV_CAKEMOVE_MAX][3] = {
        MB6EV_CAKEMOVE_1,
        W06_HSF_cakeMoveA,
        W06_HSF_knifeA,
        MB6EV_CAKEMOVE_2,
        W06_HSF_cakeMoveB,
        W06_HSF_knifeB,
        MB6EV_CAKEMOVE_3,
        W06_HSF_cakeMoveC,
        W06_HSF_knifeC,
    };
    static char *cakeHookTbl[MB6EV_CAKEMOVE_MAX] = {
        "sweet_d-m_hook1",
        "sweet_d-m_hook2",
        "sweet_d-m_hook3"
    };
    static char *knifeHookTbl[MB6EV_CAKEMOVE_MAX] = {
        "sweet_d-knife",
        "sweet_d-knife",
        "sweet_d-knife"
    };
    
    memset(&mb6_Work.evCakeMove[0], 0, sizeof(mb6_Work.evCakeMove));
    for(cakeMove=&mb6_Work.evCakeMove[0], i=0; i<MB6EV_CAKEMOVE_MAX; i++, cakeMove++) {
        cakeMove->masuId = MBMasuFlagFind(MASU_LAYER_DEFAULT, MB6Ev_MasuFlagGet(cakeMoveData[i][0], MB6EV_CAKEMOVE_FLAG), MB6EV_CAKEMOVE_FLAG);
        modelId = MBModelCreate(cakeMoveData[i][1], NULL, 0);
        cakeMove->cakeMdlId = modelId;
        MBMotionSpeedSet(modelId, 0);
        MBModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
        modelId = MBModelCreate(cakeMoveData[i][2], NULL, 0);
        cakeMove->knifeMdlId = modelId;
        MBMotionSpeedSet(modelId, 0);
        MBModelDispSet(modelId, FALSE);
        MBModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
        cakeMove->cakeHook = cakeHookTbl[i];
        cakeMove->knifeHook = knifeHookTbl[i];
    }
}

void MB6Ev_CakeMoveKill(void)
{
    MB6EV_CAKEMOVE *cakeMove;
    int i;
    for(cakeMove=&mb6_Work.evCakeMove[0], i=0; i<MB6EV_CAKEMOVE_MAX; i++, cakeMove++) {
        cakeMove->masuId = 0;
        if(cakeMove->cakeMdlId >= 0) {
            MBModelKill(cakeMove->cakeMdlId);
        }
        cakeMove->cakeMdlId = MB_MODEL_NONE;
        if(cakeMove->knifeMdlId >= 0) {
            MBModelKill(cakeMove->knifeMdlId);
        }
        cakeMove->knifeMdlId = MB_MODEL_NONE;
    }
}


static void MB6Ev_CakeMovePlayerUpdate(MB6EV_CAKEMOVE *cakeMove);
static void MB6Ev_KnifeEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix);
static void MB6Ev_KnifeEffCreate(int modelId);

#define CAKEMOVE_PLAYER_KILL -1
#define CAKEMOVE_PLAYER_INIT 0
#define CAKEMOVE_PLAYER_IDLE 1
#define CAKEMOVE_PLAYER_WARN 2
#define CAKEMOVE_PLAYER_WARNBEGIN 3
#define CAKEMOVE_PLAYER_SHOCKBEGIN 4
#define CAKEMOVE_PLAYER_SHOCK 5
#define CAKEMOVE_PLAYER_LIFT 6
#define CAKEMOVE_PLAYER_JUMP 7
#define CAKEMOVE_PLAYER_RUN 8
#define CAKEMOVE_PLAYER_ROTATE 9
#define CAKEMOVE_PLAYER_NULL 10


void MB6Ev_CakeMove(int playerNo, int masuId)
{
    MB6EV_CAKEMOVE *cakeMove;
    int i;
    MB6EV_PLAYER *evPlayer;
    int playerNum;
    BOOL doneF;
    int step;
    int effMdlId;
    unsigned int time;
    MASU *masuLink;
    int masuLinkId;
    float cameraZoom;
    
    Mtx matrix;
    HuVecF pos;
    
    for(cakeMove=&mb6_Work.evCakeMove[0], i=0; i<MB6EV_CAKEMOVE_MAX; i++, cakeMove++) {
        if(cakeMove->masuId == masuId) {
            break;
        }
    }
    if(i >= MB6EV_CAKEMOVE_MAX) {
        return;
    }
    masuLinkId = MBMasuFlagFind(MASU_LAYER_DEFAULT, MB6EV_CAKEMOVE_LINK_FLAG, MB6EV_CAKEMOVE_LINK_FLAG);
    MBMotionTimeSet(cakeMove->knifeMdlId, 0);
    MBMotionSpeedSet(cakeMove->knifeMdlId, 0);
    MBModelAttrReset(cakeMove->knifeMdlId, HU3D_MOTATTR_PAUSE);
    memset(&mb6_Work.evPlayer[0], 0, sizeof(mb6_Work.evPlayer));
    for(evPlayer=&mb6_Work.evPlayer[0], i=0; i<GW_PLAYER_MAX; i++, evPlayer++) {
        if(masuId == GwPlayer[i].masuId) {
            evPlayer->inEventF = TRUE;
        }
        if(i == playerNo) {
            evPlayer->shockEndF = TRUE;
        }
    }
    MB6Ev_CakeMovePlayerUpdate(cakeMove);
    effMdlId = MBEffCreate(mb6_Work.effAnimPuff, 32);
    MBEffHookSet(effMdlId, MB6Ev_KnifeEffHook);
    Hu3DModelLayerSet(effMdlId, 2);
    mb6_Work.evPlayer[playerNo].mode = CAKEMOVE_PLAYER_WARNBEGIN;
    for(i=0; i<90u; i++) {
        MB6Ev_CakeMovePlayerUpdate(cakeMove);
        HuPrcVSleep();
    }
    MBModelDispSet(cakeMove->knifeMdlId, TRUE);
    HuPrcVSleep();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        mb6_Work.evPlayer[i].mode = CAKEMOVE_PLAYER_SHOCKBEGIN;
    }
    Hu3DModelObjPosGet(MBModelIdGet(cakeMove->knifeMdlId), cakeMove->knifeHook, &pos);
    Hu3DModelPosSetV(effMdlId, &pos);
    MB6Ev_KnifeEffCreate(effMdlId);
    MBAudFXPlay(MSM_SE_BD6_07);
    MBCameraFocusPlayerSet(-1);
    cameraZoom = MBCameraZoomGet();
    MBCameraPosViewSet(NULL, NULL, NULL, 3500, -1, 45);
    MBMotionSpeedSet(cakeMove->knifeMdlId, 0.5f);
    for(i=0; i<45u; i++) {
        if(i == 20) {
            MBMotionSpeedSet(cakeMove->knifeMdlId, 0.0f);
        }
        MB6Ev_CakeMovePlayerUpdate(cakeMove);
        HuPrcVSleep();
    }
    MBMotionSpeedSet(cakeMove->knifeMdlId, 0.5f);
    for(i=0; !MBMotionEndCheck(cakeMove->knifeMdlId); i++) {
        if(i == 40 || i == 130) {
            MBAudFXPlay(MSM_SE_BD6_03);
        }
        MB6Ev_CakeMovePlayerUpdate(cakeMove);
        HuPrcVSleep();
    }
    MBEffKill(effMdlId);
    MBMotionTimeSet(cakeMove->cakeMdlId, 0.0f);
    MBMotionSpeedSet(cakeMove->cakeMdlId, 1.0f);
    MBModelAttrReset(cakeMove->cakeMdlId, HU3D_MOTATTR_PAUSE);
    MBPlayerPosFixFlagSet(TRUE);
    for(doneF = FALSE, step=0; doneF == FALSE;) {
        switch(step) {
            case 0:
                if(MBMotionTimeGet(cakeMove->cakeMdlId) < 55) {
                    break;
                }
                MBMotionSpeedSet(cakeMove->cakeMdlId, 0);
                masuLink = MBMasuGet(MASU_LAYER_DEFAULT, masuLinkId);
                pos = masuLink->pos;
                pos.z += 500;
                MBCameraPosViewSet(&pos, NULL, NULL, -1, -1, 120);
                time = 0;
                step++;
                break;
            
            case 1:
                time++;
                if(time < 120) {
                    break;
                }
                MBMotionTimeSet(cakeMove->cakeMdlId, 100);
                MBMotionSpeedSet(cakeMove->cakeMdlId, 1);
                step++;
                break;
           
            case 2:
                if(!MBMotionEndCheck(cakeMove->cakeMdlId)) {
                    break;
                }
                MBAudFXPlay(MSM_SE_BD6_04);
                doneF = TRUE;
                break;
        }
        Hu3DMotionCalc(MBModelIdGet(cakeMove->cakeMdlId));
        Hu3DModelObjMtxGet(MBModelIdGet(cakeMove->cakeMdlId), cakeMove->cakeHook, matrix);
        MBMasuMtxSet(cakeMove->masuId, matrix);
        MB6Ev_CakeMovePlayerUpdate(cakeMove);
        HuPrcVSleep();
    }
    Hu3DModelObjMtxGet(MBModelIdGet(cakeMove->cakeMdlId), cakeMove->cakeHook, matrix);
    MBMasuMtxSet(cakeMove->masuId, matrix);
    for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
        evPlayer = &mb6_Work.evPlayer[i];
        if(evPlayer->inEventF && (i != playerNo)) {
            evPlayer->mode = CAKEMOVE_PLAYER_LIFT;
            evPlayer->speed = playerNum*60u;
            if(playerNum != 0) {
                MBPlayerMotIdleSet(i);
            }
            playerNum++;
        }
    }
    evPlayer = &mb6_Work.evPlayer[playerNo];
    evPlayer->mode = CAKEMOVE_PLAYER_LIFT;
    evPlayer->speed = playerNum*60u;
    if(playerNum != 0) {
        MBPlayerMotIdleSet(playerNo);
    }
    MBPlayerPosFixFlagSet(FALSE);
    for(doneF=FALSE; !doneF;) {
        MB6Ev_CakeMovePlayerUpdate(cakeMove);
        doneF = TRUE;
        for(evPlayer=&mb6_Work.evPlayer[0], i=0; i<GW_PLAYER_MAX; i++, evPlayer++) {
            if(evPlayer->inEventF) {
                if(evPlayer->mode > CAKEMOVE_PLAYER_IDLE || !MBPlayerPosFixCheck()) {
                    doneF = FALSE;
                }
            }
        }
        HuPrcVSleep();
    }
    WipeColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    MBMotionTimeSet(cakeMove->cakeMdlId, 0);
    MBMotionSpeedSet(cakeMove->cakeMdlId, 0);
    MBModelAttrSet(cakeMove->cakeMdlId, HU3D_MOTATTR_PAUSE);
    MBMotionTimeSet(cakeMove->knifeMdlId, 0);
    MBMotionSpeedSet(cakeMove->knifeMdlId, 0);
    MBModelAttrSet(cakeMove->knifeMdlId, HU3D_MOTATTR_PAUSE);
    MBModelDispSet(cakeMove->knifeMdlId, FALSE);
    MBCameraFocusPlayerSet(playerNo);
    MBCameraZoomSet(cameraZoom);
    MBCameraSkipSet(FALSE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    while(WipeCheckEnd()) {
        Hu3DModelObjMtxGet(MBModelIdGet(cakeMove->cakeMdlId), cakeMove->cakeHook, matrix);
        MBMasuMtxSet(cakeMove->masuId, matrix);
        HuPrcVSleep();
    }
    for(i=0; i<60; i++) {
        HuPrcVSleep();
    }
    for(evPlayer=&mb6_Work.evPlayer[0], i=0, playerNum=0; i<GW_PLAYER_MAX; i++, evPlayer++) {
        playerNum++;
        if(!evPlayer->inEventF) {
            playerNum = 0;
        }
        evPlayer->mode = CAKEMOVE_PLAYER_KILL;
    }
    MB6Ev_CakeMovePlayerUpdate(cakeMove);
}


static void MB6Ev_CakeMovePlayerUpdate(MB6EV_CAKEMOVE *cakeMove)
{
    MB6EV_PLAYER *evPlayer;
    int i;
    MASU *masuP;
    int masuLinkId;
    
    HuVecF pos;
    HuVecF moveDir;
    
    for(evPlayer=&mb6_Work.evPlayer[0], i=0; i<GW_PLAYER_MAX; i++, evPlayer++) {
        if(!evPlayer->inEventF) {
            continue;
        }
        switch(evPlayer->mode) {
            case CAKEMOVE_PLAYER_INIT:
                evPlayer->motNo[0] = MBPlayerMotionCreate(i, CHARMOT_HSF_c000m1_325);
                MBPlayerPosGet(i, &evPlayer->pos);
                MBPlayerRotGet(i, &evPlayer->rot);
                evPlayer->masuId = GwPlayer[i].masuId;
                evPlayer->angleStart = evPlayer->rot.y;
                evPlayer->angleSpeed = 0.1f;
                evPlayer->mode++;
                break;
           
            case CAKEMOVE_PLAYER_IDLE:
                if(!MBPlayerPosFixCheck()) {
                    MBPlayerPosGet(i, &evPlayer->pos);
                    MBPlayerRotGet(i, &evPlayer->rot);
                    evPlayer->angleStart = evPlayer->rot.y;
                }
                evPlayer->timer--;
                if(evPlayer->timer == 0) {
                    evPlayer->angleSpeed = 0.075f;
                }
                break;
           
            case CAKEMOVE_PLAYER_WARN:
                if(MBPlayerMotionEndCheck(i)) {
                    MBPlayerMotIdleSet(i);
                    evPlayer->mode = CAKEMOVE_PLAYER_IDLE;
                }
                break;
           
            case CAKEMOVE_PLAYER_WARNBEGIN:
                MBPlayerMotionNoShiftSet(i, evPlayer->motNo[0], 0, 8, HU3D_MOTATTR_LOOP);
                evPlayer->mode = CAKEMOVE_PLAYER_IDLE;
                break;
            
            case CAKEMOVE_PLAYER_SHOCKBEGIN:
                Hu3DModelObjPosGet(MBModelIdGet(cakeMove->knifeMdlId), cakeMove->knifeHook, &pos);
                MBPlayerMotionNoSet(i, MB_PLAYER_MOT_SHOCK, HU3D_MOTATTR_NONE);
                evPlayer->angleStart = HuAtan(pos.x-evPlayer->pos.x, pos.z-evPlayer->pos.z);
                evPlayer->angleSpeed = 0.2f;
                evPlayer->mode = CAKEMOVE_PLAYER_SHOCK;
                break;
           
            case CAKEMOVE_PLAYER_SHOCK:
                if(!MBPlayerMotionEndCheck(i)) {
                    break;
                    
                }
                if(evPlayer->shockEndF) {
                    evPlayer->angleStart = 0;
                } else {
                    masuP = MBMasuGet(MASU_LAYER_DEFAULT, GwPlayer[i].masuId);
                    evPlayer->angleStart = HuAtan(evPlayer->pos.x-masuP->pos.x, evPlayer->pos.z-masuP->pos.z);
                }
                evPlayer->angleSpeed = 0;
                evPlayer->timer = 40;
                evPlayer->mode = CAKEMOVE_PLAYER_WARNBEGIN;
                break;
          
            case CAKEMOVE_PLAYER_LIFT:
                evPlayer->speed--;
                if(evPlayer->speed >= 0) {
                    break;
                }
                MBPlayerPosGet(i, &evPlayer->pos);
                masuP = MBMasuGet(MASU_LAYER_DEFAULT, evPlayer->masuId);
                pos.x = masuP->matrix[0][3];
                pos.y = masuP->matrix[1][3];
                pos.z = masuP->matrix[2][3];
                VECSubtract(&evPlayer->pos, &pos, &evPlayer->masuDir);
                masuLinkId = MBMasuFlagFind(MASU_LAYER_DEFAULT, MB6EV_CAKEMOVE_LINK_FLAG, MB6EV_CAKEMOVE_LINK_FLAG);
                evPlayer->masuId = masuLinkId;
                masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuLinkId);
                VECSubtract(&masuP->pos, &evPlayer->pos, &moveDir);
                evPlayer->angleStart = HuAtan(moveDir.x, moveDir.z);
                {
                    float speed = 0.8f;
                    float height = -2450;
                    moveDir.x /= speed;
                    moveDir.z /= speed;
                    moveDir.y = (moveDir.y/speed)-(0.5f*speed*height);
                    VECScale(&moveDir, &evPlayer->vel, 0.016666668f);
                    evPlayer->speed = 60*speed;
                    evPlayer->gravity = 0.016666668f*(0.016666668f*height);
                    MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_JUMP, 0, 8, HU3D_MOTATTR_NONE);
                }
                evPlayer->timer = 0;
                evPlayer->mode++;
                break;
           
            case CAKEMOVE_PLAYER_JUMP:
                evPlayer->angleSpeed = 0.2f;
                if(evPlayer->timer < 3) {
                    evPlayer->timer++;
                    break;
                } else {
                    evPlayer->vel.y += evPlayer->gravity;
                }
                VECAdd(&evPlayer->pos, &evPlayer->vel, &evPlayer->pos);
                if(evPlayer->timer < 10 && evPlayer->speed <= 5.0f) {
                    MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_JUMPEND, 0, 0, HU3D_MOTATTR_NONE);
                    evPlayer->timer = 10;
                }
                MBPlayerPosSetV(i, &evPlayer->pos);
                evPlayer->speed--;
                if(evPlayer->speed > 0) {
                    break;
                }
                evPlayer->gravity = 0;
                evPlayer->vel.x = evPlayer->vel.y = evPlayer->vel.z = 0;
                if(MBPlayerMotionEndCheck(i)) {
                    masuP = MBMasuGet(MASU_LAYER_DEFAULT, evPlayer->masuId);
                    evPlayer->masuId = masuP->linkTbl[0];
                    masuP = MBMasuGet(MASU_LAYER_DEFAULT, evPlayer->masuId);
                    pos = masuP->pos;
                    evPlayer->speed = 30;
                    VECSubtract(&pos, &evPlayer->pos, &moveDir);
                    VECScale(&moveDir, &evPlayer->vel, 1/evPlayer->speed);
                    evPlayer->angleStart = HuAtan(moveDir.x, moveDir.z);
                    MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_RUN, 0, 0, HU3D_MOTATTR_LOOP);
                    evPlayer->timer = 0;
                    evPlayer->mode++;
                }
                break;
           
            case CAKEMOVE_PLAYER_RUN:
                evPlayer->angleSpeed = 0.2f;
                VECAdd(&evPlayer->pos, &evPlayer->vel, &evPlayer->pos);
                MBPlayerPosSetV(i, &evPlayer->pos);
                if(evPlayer->timer == 0 && evPlayer->speed < 18.0f) {
                    MBPlayerPosFixWarp(i, evPlayer->masuId, FALSE);
                    evPlayer->timer++;
                }
                evPlayer->speed--;
                if(evPlayer->speed > 0) {
                    break;
                }
                GwPlayer[i].masuId = evPlayer->masuId;
                MBPlayerMotIdleSet(i);
                evPlayer->angleStart = 0;
                evPlayer->speed = 30;
                evPlayer->mode++;
                break;
            
            case CAKEMOVE_PLAYER_ROTATE:
                evPlayer->angleSpeed = 0.2f;
                evPlayer->speed--;
                if(evPlayer->speed > 0) {
                    break;
                }
                evPlayer->angleStart = evPlayer->rot.y = 0;
                evPlayer->mode = CAKEMOVE_PLAYER_IDLE;
                break;
           
            case CAKEMOVE_PLAYER_KILL:
                MBPlayerMotIdleSet(i);
                if(evPlayer->motNo[0] >= 0) {
                    MBPlayerMotionKill(i, evPlayer->motNo[0]);
                }
                evPlayer->motNo[0] = -1;
                evPlayer->inEventF = FALSE;
                break;
           
            case CAKEMOVE_PLAYER_NULL:
                break;
        }
        MBAngleMoveTo(&evPlayer->rot.y, evPlayer->angleStart, evPlayer->angleSpeed);
        MBPlayerRotSetV(i, &evPlayer->rot);
    }
}

static void MB6Ev_KnifeEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix)
{
    MBEFFECTDATA *effDataP;
    int i;
    for(effDataP=effP->data, i=0; i<effP->num; i++, effDataP++) {
        if(effDataP->time) {
            VECAdd(&effDataP->pos, &effDataP->vel, &effDataP->pos);
            effDataP->vel.x *= 0.95f;
            effDataP->vel.y *= 0.95f;
            effDataP->vel.z *= 0.95f;
            effDataP->scale += 6;
            effDataP->color.a *= 0.98f;
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
}

static void MB6Ev_KnifeEffCreate(int modelId)
{
    MBEFFECT *effP = Hu3DData[modelId].hookData;
    MBEFFECTDATA *effDataP;
    int i;
    HuVecF dir;
    effP->blendMode = MB_EFFECT_BLEND_ADDCOL;
    for(effDataP=effP->data, i=0; i<effP->num; i++, effDataP++) {
        float angle1, angle2;
        
        if(effDataP->time) {
            continue;
        }
        effDataP->time = 60.0f*(0.7f+(0.5f*MBEffRandF()));
        angle1 = 360*MBEffRandF();
        angle2 = 360*MBEffRandF();
        dir.x = (HuSin(angle1) * HuCos(angle2));
        dir.y = HuSin(angle2);
        dir.z = (HuCos(angle1) * HuCos(angle2));
        VECScale(&dir, &effDataP->pos, 50);
        VECScale(&dir, &effDataP->vel, (1.6666666f*(6.0f*MBEffRandF()))+3.3333333f);
        effDataP->rot.z = 360*MBEffRandF();
        effDataP->scale = (60*MBEffRandF())+60;
        effDataP->color.a = (100*MBEffRandF())+80;
        effDataP->color.r = (100*MBEffRandF())+150;
        effDataP->color.g = (100*MBEffRandF())+150;
        effDataP->color.b = (100*MBEffRandF())+150;
    }
}

#undef CAKEMOVE_PLAYER_KILL
#undef CAKEMOVE_PLAYER_INIT
#undef CAKEMOVE_PLAYER_IDLE
#undef CAKEMOVE_PLAYER_WARN
#undef CAKEMOVE_PLAYER_WARNBEGIN
#undef CAKEMOVE_PLAYER_SHOCKBEGIN
#undef CAKEMOVE_PLAYER_SHOCK
#undef CAKEMOVE_PLAYER_LIFT
#undef CAKEMOVE_PLAYER_JUMP
#undef CAKEMOVE_PLAYER_RUN
#undef CAKEMOVE_PLAYER_ROTATE
#undef CAKEMOVE_PLAYER_NULL

static void MB6Ev_CakeThrowBallInit(void);
static void MB6Ev_CakeThrowPlayerUpdate(int playerNo);
static void MB6Ev_CakeThrowBallUpdate(int playerNo);

#define CAKETHROW_PLAYER_KILL -1
#define CAKETHROW_PLAYER_INIT 0
#define CAKETHROW_PLAYER_BALLINIT 2
#define CAKETHROW_PLAYER_IDLE 3
#define CAKETHROW_PLAYER_JUMP_BALL 4
#define CAKETHROW_PLAYER_JUMP 5
#define CAKETHROW_PLAYER_END 6

static char *mb6ev_CakeThrowCupHookTbl[5] = {
    "aweet_d-c_hook1",
    "aweet_d-c_hook2",
    "aweet_d-c_hook3",
    "aweet_d-c_hook4",
    "aweet_d-c_hook5"
};

void MB6Ev_CakeThrow(int playerNo)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, GwPlayer[playerNo].masuId);
    int linkMasuId;
    int i;
    int score;
    int timerGMesId;
    float zoom;
    HuVecF masuPos;

    for(linkMasuId=-1, i=0; i<masuP->linkNum; i++){ 
        linkMasuId = masuP->linkTbl[i];
        if(MBMasuGet(MASU_LAYER_DEFAULT, linkMasuId)->flag & MB6EV_EVENT_LINK_FLAG) {
            break;
        }
        linkMasuId = -1;
    }
    if(linkMasuId < 0) {
        return;
    }
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, linkMasuId);
    MBPlayerMasuWalkSpeed(playerNo, linkMasuId, 30, TRUE);
    MBPlayerMotIdleSet(playerNo);
    MBCameraFocusPlayerSet(-1);
    zoom = MBCameraZoomGet();
    MBCameraPosViewSet(NULL, NULL, NULL, 2800, -1, 30);
    masuPos = masuP->pos;
    masuPos.x -= 200;
    masuPos.z -= 300;
    masuPos.y += 100;
    MBGuideCreateFlag(MB_GUIDE_TERL, &masuPos, FALSE, TRUE, TRUE, FALSE);
    for(i=0; i<30u; i++) {
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_GUIDE_56);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_CAKETHROW_ENTER, HUWIN_SPEAKER_TERL);
    MBTopWinWait();
    MBTopWinKill();
    MB6Ev_CakeThrowBallInit();
    memset(&mb6_Work.evPlayer[0], 0, sizeof(mb6_Work.evPlayer));
    mb6_Work.evPlayer[playerNo].inEventF = TRUE;
    mb6_Work.evPlayer[playerNo].masuId = linkMasuId;
    MB6Ev_CakeThrowPlayerUpdate(playerNo);
    masuPos.x = 0.5f*(2400+masuP->pos.x);
    masuPos.y = masuP->pos.y;
    masuPos.z = 0.5f*(400+masuP->pos.z);
    MBCameraPosViewSet(&masuPos, NULL, NULL, 2800, -1, 30);
    for(i=0; i<30u; i++) {
        HuPrcVSleep();
    }
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_CAKETHROW_INST, HUWIN_SPEAKER_TERL);
    MBTopWinWait();
    MBTopWinKill();
    mb6_Work.cakeThrowTimer = 900;
    mb6_Work.cakeThrowRotSpeed = 0.25f;
    mb6_Work.evPlayer[playerNo].mode = CAKETHROW_PLAYER_BALLINIT;
    timerGMesId = GMesTimerCreate((mb6_Work.cakeThrowTimer+59)*0.016666668f);
    GMesTimerValueSet(timerGMesId, (mb6_Work.cakeThrowTimer+59)*0.016666668f);
    for(i=0; i<30; i++) {
        mb6_Work.cakeThrowRotSpeed += 0.016666668f;
        HuPrcVSleep();
    }
    while(mb6_Work.evPlayer[playerNo].mode != CAKETHROW_PLAYER_END) {
        MB6Ev_CakeThrowPlayerUpdate(playerNo);
        MB6Ev_CakeThrowBallUpdate(playerNo);
        if(mb6_Work.cakeThrowTimer > 0) {
            mb6_Work.cakeThrowTimer--;
        }
        GMesTimerValueSet(timerGMesId, (mb6_Work.cakeThrowTimer+59)*0.016666668f);
        HuPrcVSleep();
    }
    MB6Ev_CakeThrowPlayerUpdate(playerNo);
    GMesTimerEnd(timerGMesId);
    for(i=0; i<120; i++) {
        if(i >= 60) {
            mb6_Work.cakeThrowRotSpeed -= 0.008333334f;
        }
        MB6Ev_CakeThrowPlayerUpdate(playerNo);
        MB6Ev_CakeThrowBallUpdate(playerNo);
        HuPrcVSleep();
    }
    MBCameraPlayerViewSet(playerNo, NULL, NULL, 2800, -1, 30);
    for(i=0; i<30u; i++) {
        HuPrcVSleep();
    }
    MBCameraFocusPlayerSet(playerNo);
    MBCameraSkipSet(TRUE);
    for(i=0, score=0; i<5; i++) {
        if(mb6_Work.cakeThrowBallF[i]) {
            score++;
        }
    }
    if(score) {
        char insertMes2[12];
        char insertMes1[8];
        int winId;
        sprintf(insertMes1, "%1d", score);
        sprintf(insertMes2, "%2d", score*5);
        MBAudFXPlay(MSM_SE_GUIDE_55);
        winId = MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_CAKETHROW_RESULT, HUWIN_SPEAKER_TERL);
        MBWinInsertMesSet(winId, MESSNUM_PTR(insertMes1), 0);
        MBWinInsertMesSet(winId, MESSNUM_PTR(insertMes2), 1);
        MBTopWinWait();
        MBTopWinKill();
        MBCoinAddDispExec(playerNo, score*5, TRUE);
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_WIN, 0, 8, HU3D_MOTATTR_NONE);
        MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_WIN, CHARVOICEID(5));
    } else {
        MBAudFXPlay(MSM_SE_GUIDE_57);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_CAKETHROW_FAIL, HUWIN_SPEAKER_TERL);
        MBTopWinWait();
        MBTopWinKill();
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_LOSE, 0, 8, HU3D_MOTATTR_NONE);
        MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_LOSE, CHARVOICEID(10));
    }
    MBGuideKill();
    MBCameraPlayerViewSet(playerNo, NULL, NULL, zoom, -1, 30);
    while(!MBPlayerMotionEndCheck(playerNo)) {
        HuPrcVSleep();
    }
    for(i=0; i<60u; i++) {
        HuPrcVSleep();
    }
    mb6_Work.evPlayer[playerNo].mode = CAKETHROW_PLAYER_KILL;
    MB6Ev_CakeThrowPlayerUpdate(playerNo);
    MBPlayerMasuWalkSpeed(playerNo, GwPlayer[playerNo].masuId, 30, TRUE);
}

static void MB6Ev_CakeThrowBallInit(void)
{
    int i;
    int modelId;
    memset(&mb6_Work.evCakeBall[0], 0, MB6EV_CAKEBALL_MAX*sizeof(MB6EV_CAKEBALL));
    mb6_Work.cakeThrowBallNo = 0;
    for(i=0; i<12; i++) {
        modelId = mb6_Work.cakeBallMdlId[i];
        if(modelId >= 0) {
            MBModelDispSet(modelId, FALSE);
            MBModelScaleSet(modelId, 1, 1, 1);
        }
    }
    for(i=0; i<5; i++) {
        mb6_Work.cakeThrowBallF[i] = FALSE;
    }
}

#define CAKETHROW_BALL_NONE 0
#define CAKETHROW_BALL_MAIN 1
#define CAKETHROW_BALL_THROW 2
#define CAKETHROW_BALL_SHRINK 3

static int MB6Ev_CakeThrowBallSet(int playerNo)
{
    int nextBallNo = -1;
    int ballNo = mb6_Work.cakeThrowBallNo;
    MB6EV_CAKEBALL *ballP;
    int modelId;
    Mtx modelMtx;
    do {
        ballP = &mb6_Work.evCakeBall[ballNo];
        if(!ballP->enableF) {
            nextBallNo = ballNo;
        }
        ballNo++;
        if(ballNo >= MB6EV_CAKEBALL_MAX) {
            ballNo = 0;
        }
        if(ballNo == mb6_Work.cakeThrowBallNo) {
            break;
        }
    } while(nextBallNo < 0);
    if(nextBallNo < 0) {
        return -1;
    }
    mb6_Work.cakeThrowBallNo = ballNo;
    memset(ballP, 0, sizeof(MB6EV_CAKEBALL));
    modelId = mb6_Work.cakeBallMdlId[nextBallNo];
    MTXIdentity(modelMtx);
    MBModelDispSet(modelId, TRUE);
    MBModelMtxSet(modelId, &modelMtx);
    MBModelPosSet(modelId, 0, 0, 0);
    MBModelRotSet(modelId, 0, 0, 0);
    MBModelScaleSet(modelId, 1, 1, 1);
    MBModelHookSet(MBPlayerModelGet(playerNo), CharModelItemHookGet(GwPlayer[playerNo].charNo, CHAR_MODEL1, 0), modelId);
    ballP->scale = 1;
    ballP->enableF = TRUE;
    ballP->mode = 1;
    return nextBallNo;
}

static int MB6Ev_CakeThrowBallTargetSet(float rotY, HuVecF *pos);
static int MB6Ev_CakeThrowCupCheck(MB6EV_CAKEBALL *ballP);
static int MB6Ev_CakeThrowBallCheck(MB6EV_CAKEBALL *ballP);

static void MB6Ev_CakeThrowBallUpdate(int playerNo)
{
    Mtx rot;
    Mtx trans;
    HuVecF pos;
    MB6EV_CAKEBALL *ballP;
    int i;
    int modelId;
    float speed;
    float ofsY;
    
    MBModelPosGet(mb6_Work.objMdlId[MB6_OBJ_CAKETHROW], &pos);
    MTXTrans(trans, pos.x, pos.y, pos.z);
    MTXRotDeg(rot, 'Y', mb6_Work.cakeThrowRot);
    MTXConcat(trans, rot, rot);
    for(ballP=&mb6_Work.evCakeBall[0], i=0; i<MB6EV_CAKEBALL_MAX; i++, ballP++) {
        if(!ballP->enableF) {
            continue;
        }
        switch(ballP->mode) {
            case CAKETHROW_BALL_NONE:
                break;
                
            case CAKETHROW_BALL_MAIN:
                if(!ballP->cancelHookF) {
                    break;
                }
                ballP->cancelHookF = FALSE;
                Hu3DModelObjPosGet(MBModelIdGet(MBPlayerModelGet(playerNo)), CharModelItemHookGet(GwPlayer[playerNo].charNo, CHAR_MODEL1, 0), &ballP->hookPos);
                MBModelHookReset(MBPlayerModelGet(playerNo));
                ballP->collStat = MB6Ev_CakeThrowBallTargetSet(ballP->rotY, &ballP->targetPos);
                OSReport("Coll Stat %d\n", ballP->collStat);
                speed = 0.8f;
                ofsY = 60*speed;
                VECSubtract(&ballP->targetPos, &ballP->hookPos, &ballP->targetPos);
                VECScale(&ballP->targetPos, &ballP->targetPos, 1/speed);
                ofsY = -2450;
                ballP->targetPos.y += -0.5f*speed*ofsY;
                VECScale(&ballP->targetPos, &ballP->targetPos, 0.016666668f);
                ballP->time = 60*speed;
                ballP->gravity = 0.016666668f*(0.016666668f*ofsY);
                ballP->mode++;
                
            case CAKETHROW_BALL_THROW:
                ballP->targetPos.y += ballP->gravity;
                VECAdd(&ballP->hookPos, &ballP->targetPos, &ballP->hookPos);
                MB6Ev_CakeThrowCupCheck(ballP);
                ballP->time--;
                if(ballP->time > 0) {
                    break;
                }
                if(ballP->collStat == 2) {
                    if(MB6Ev_CakeThrowBallCheck(ballP) < 0 && ballP->hookPos.y > 240.00002f) {
                        break;
                    }
                    MBAudFXPlay(MSM_SE_BD6_06);
                } else {
                    if(ballP->hookPos.y > 240.00002f) {
                        break;
                    }
                }
                MTXInverse(rot, trans);
                MTXMultVec(trans, &ballP->hookPos, &ballP->targetPos);
                ballP->time = 60;
                ballP->mode++;
                
            case CAKETHROW_BALL_SHRINK:
                MTXScale(trans, ballP->scale, ballP->scale, ballP->scale);
                MTXConcat(rot, trans, trans);
                MBModelMtxSet(mb6_Work.cakeBallMdlId[i], &trans);
                MTXMultVecSR(rot, &ballP->targetPos, &ballP->hookPos);
                if(ballP->collStat == 2 && mb6_Work.cakeThrowTimer != 0) {
                    break;
                }
                ballP->time--;
                if(ballP->time >= 0) {
                    break;
                }
                ballP->scale *= 0.9f;
                if(ballP->scale < 0.1f) {
                    ballP->enableF = FALSE;
                }
                break;
        }
        modelId = mb6_Work.cakeBallMdlId[i];
        if(ballP->enableF) {
            MBModelPosSetV(modelId, &ballP->hookPos);
        } else {
            MBModelDispSet(modelId, FALSE);
        }
    }
}

static int MB6Ev_CakeThrowBallTargetSet(float rotY, HuVecF *pos)
{
    int modelId = mb6_Work.objMdlId[MB6_OBJ_CAKETHROW];
    int collStat = 0;
    int i;
    HuVecF hookPos;
    HuVecF cupPos;
    HuVecF modelPos;
    MBModelRotSet(modelId, 0, mb6_Work.cakeThrowRot+(60*(0.8f*mb6_Work.cakeThrowRotSpeed)), 0);
    Hu3DMotionCalc(MBModelIdGet(modelId));
    MBModelPosGet(modelId, &modelPos);
    Hu3DModelObjPosGet(MBModelIdGet(modelId), mb6ev_CakeThrowCupHookTbl[0], &hookPos);
    modelPos.y = hookPos.y;
    modelPos.x += 350*(1-rotY);
    *pos = modelPos;
    for(i=0; i<5; i++) {
        Hu3DModelObjPosGet(MBModelIdGet(modelId), mb6ev_CakeThrowCupHookTbl[i], &cupPos);
        VECSubtract(&cupPos, &modelPos, &hookPos);
        if(VECMag(&hookPos) < 83 && collStat < 2) {
            collStat = 2;
            if(mb6_Work.cakeThrowBallF[i]) {
                collStat = 1;
            }
            *pos = cupPos;
        }
    }
    MBModelRotSet(modelId, 0, mb6_Work.cakeThrowRot, 0);
    return collStat;
}

static int MB6Ev_CakeThrowBallCheck(MB6EV_CAKEBALL *ballP)
{
    int modelId = mb6_Work.objMdlId[MB6_OBJ_CAKETHROW];
    int ballNo = -1;
    int i;
    int effMdlId;
    HuVecF hookPos;
    HuVecF cupPos;
    Hu3DMotionCalc(MBModelIdGet(modelId));
    for(i=0; i<5; i++) {
        Hu3DModelObjPosGet(MBModelIdGet(modelId), mb6ev_CakeThrowCupHookTbl[i], &cupPos);
        VECSubtract(&cupPos, &ballP->hookPos, &hookPos);
        if(ballP->collStat == 2 && VECMag(&hookPos) < 83) {
            ballNo = i;
            mb6_Work.cakeThrowBallF[i] = TRUE;
            effMdlId = mb6_Work.objMdlId[MB6_OBJ_CAKETHROW_EFF];
            MBModelPosSetV(effMdlId, &cupPos);
            MBModelDispSet(effMdlId, TRUE);
            MBMotionTimeSet(effMdlId, 0);
            MBMotionSpeedSet(effMdlId, 1);
            break;
        }
    }
    MBModelRotSet(modelId, 0, mb6_Work.cakeThrowRot, 0);
    return ballNo;
}

static int MB6Ev_CakeThrowCupCheck(MB6EV_CAKEBALL *ballP)
{
    int modelId = mb6_Work.objMdlId[MB6_OBJ_CAKETHROW];
    int ballNo = -1;
    int i;
    HuVecF hookPos;
    HuVecF dir;
    HuVecF cupPos;
    
    float dot;
    float mag;
    float magFlat;
    
    Hu3DMotionCalc(MBModelIdGet(modelId));
    for(i=0; i<5; i++) {
        Hu3DModelObjPosGet(MBModelIdGet(modelId), mb6ev_CakeThrowCupHookTbl[i], &cupPos);
        VECSubtract(&cupPos, &ballP->hookPos, &hookPos);
        mag = VECMag(&hookPos);
        dir = hookPos;
        dir.y = 0.00001f;
        magFlat = VECMag(&dir);
        VECNormalize(&dir, &dir);
        dot = (ballP->targetPos.x*dir.x)+(ballP->targetPos.z*dir.z);
        switch(ballP->collStat) {
            case 0:
            case 1:
                if(ballP->time >= 0) {
                    if(mag <= 83 && dot > 0) {
                        ballP->targetPos.x += 2*dir.x*ballP->targetPos.x;
                        ballP->targetPos.z += 2*dir.z*ballP->targetPos.z;
                    }
                } else {
                    if(magFlat <= 40 && dot > 0) {
                        ballP->targetPos.x += 2*dir.x*ballP->targetPos.x;
                        ballP->targetPos.z += 2*dir.z*ballP->targetPos.z;
                    }
                }
                break;
            
            case 2:
                break;
        }
    }
    MBModelRotSet(modelId, 0, mb6_Work.cakeThrowRot, 0);
    return ballNo;
}

#undef CAKETHROW_BALL_NONE
#undef CAKETHROW_BALL_MAIN
#undef CAKETHROW_BALL_THROW
#undef CAKETHROW_BALL_SHRINK

static void MB6Ev_CakeThrowPlayerUpdate(int playerNo)
{
    MB6EV_PLAYER *evPlayer = &mb6_Work.evPlayer[playerNo];
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, evPlayer->masuId);
    static unsigned int motFileTbl[4] = {
        CHARMOT_HSF_c000m1_507,
        CHARMOT_HSF_c000m1_508,
        CHARMOT_HSF_c000m1_509,
        CHARMOT_HSF_c000m1_510
    };
    int i;
    float speed;
    float height;
    switch(evPlayer->mode) {
        case CAKETHROW_PLAYER_INIT:
            MBPlayerPosGet(playerNo, &evPlayer->pos);
            MBPlayerRotGet(playerNo, &evPlayer->rot);
            evPlayer->angleStart = evPlayer->rot.y;
            evPlayer->angleSpeed = 0.15f;
            evPlayer->ballNo = -1;
            evPlayer->ballDist = 0;
            for(i=0; i<4; i++) {
                evPlayer->motNo[i] = MBPlayerMotionCreate(playerNo, motFileTbl[i]);
            }
            MBPlayerMotIdleSet(playerNo);
            evPlayer->mode++;
            evPlayer->timer = 0;
            break;
        
        case CAKETHROW_PLAYER_BALLINIT:
            if(evPlayer->timer == 0) {
                MBPlayerMotionNoShiftSet(playerNo, evPlayer->motNo[0], 0, 8, HU3D_MOTATTR_LOOP);
                evPlayer->angleStart = 180;
                evPlayer->angleSpeed = 0.15f;
                evPlayer->ballNo = -1;
                evPlayer->timer++;
            } else if(evPlayer->timer == 1) {
                if(fabsf(MB6_AngleDiff(evPlayer->rot.y, evPlayer->angleStart)) < 10.0f) {
                    evPlayer->ballNo = MB6Ev_CakeThrowBallSet(playerNo);
                    if(evPlayer->ballNo >= 0) {
                        evPlayer->ballDist = 0;
                        evPlayer->angleStart = 270;
                        evPlayer->timer++;
                    }
                }
                
            } else if(evPlayer->timer == 2) {
                evPlayer->mode = CAKETHROW_PLAYER_IDLE;
                evPlayer->timer = 0;
            }
            break;
        
        case CAKETHROW_PLAYER_IDLE:
            if(evPlayer->timer == 0) {
                MBPlayerMotionNoShiftSet(playerNo, evPlayer->motNo[0], 0, 8, HU3D_MOTATTR_LOOP);
                evPlayer->timer++;
            } else if(mb6_Work.cakeThrowTimer) {
                BOOL jumpF = FALSE;
                if(GwPlayer[playerNo].comF) {
                    if(MBEffRandF() < 0.1f) {
                        jumpF = TRUE;
                    }
                } else {
                    if(HuPadBtnDown[GwPlayer[playerNo].padNo] & PAD_BUTTON_A) {
                        jumpF = TRUE;
                    }
                }
                if(jumpF) {
                    evPlayer->mode = CAKETHROW_PLAYER_JUMP_BALL;
                    evPlayer->timer = 0;
                }
            } else {
                evPlayer->mode = CAKETHROW_PLAYER_END;
                evPlayer->timer = 0;
            }
            break;
        
        case CAKETHROW_PLAYER_JUMP_BALL:
            if(evPlayer->timer == 0) {
                MBPlayerMotionNoShiftSet(playerNo, evPlayer->motNo[2], 0, 0, HU3D_MOTATTR_NONE);
                evPlayer->angleSpeed = 0.2f;
                evPlayer->speed = 0;
                evPlayer->timer++;
            } else if(evPlayer->timer == 1) {
                evPlayer->speed++;
                if(evPlayer->speed >= 4) {
                    speed = 0.8f;
                    height = -2450;
                    evPlayer->vel.x = evPlayer->vel.y = evPlayer->vel.z = 0;
                    evPlayer->vel.y = (evPlayer->vel.y/speed)-(0.5f*speed*height);
                    VECScale(&evPlayer->vel, &evPlayer->vel, 0.016666668f);
                    evPlayer->speed = 60*speed;
                    evPlayer->gravity = 0.016666668f*(0.016666668f*height);
                    evPlayer->ballDist = 0;
                    evPlayer->timer++;
                }
            } else if(evPlayer->timer == 2) {
                evPlayer->vel.y += evPlayer->gravity;
                VECAdd(&evPlayer->pos, &evPlayer->vel, &evPlayer->pos);
                speed = 0.041666668f;
                if(evPlayer->vel.y < 0) {
                    speed = -speed;
                }
                evPlayer->ballDist += speed;
                evPlayer->speed--;
                if(evPlayer->speed >= 4 && evPlayer->speed <= 45) {
                    BOOL cancelF = FALSE;
                    if(GwPlayer[playerNo].comF) {
                        if(MBEffRandF() < 0.1f) {
                            cancelF = TRUE;
                        }
                    } else {
                        if(HuPadBtnDown[GwPlayer[playerNo].padNo] & PAD_BUTTON_A) {
                            cancelF = TRUE;
                        }
                    }
                    if(mb6_Work.cakeThrowTimer && cancelF) {
                        MBPlayerMotionNoShiftSet(playerNo, evPlayer->motNo[3], 0, 0, HU3D_MOTATTR_NONE);
                        mb6_Work.evCakeBall[evPlayer->ballNo].cancelHookF = TRUE;
                        mb6_Work.evCakeBall[evPlayer->ballNo].rotY = evPlayer->ballDist;
                        evPlayer->ballNo = -1;
                        evPlayer->mode = CAKETHROW_PLAYER_JUMP;
                        evPlayer->timer = 0;
                        MBAudFXPlay(MSM_SE_BD6_05);
                    }
                } else {
                    if(evPlayer->speed <= 0) {
                        evPlayer->pos.y = masuP->pos.y;
                        evPlayer->mode = CAKETHROW_PLAYER_IDLE;
                        evPlayer->timer = 0;
                    }
                }
            }
            MBPlayerPosSetV(playerNo, &evPlayer->pos);
            break;
        
        case CAKETHROW_PLAYER_JUMP:
            if(evPlayer->timer == 0) {
                evPlayer->vel.y += evPlayer->gravity;
                VECAdd(&evPlayer->pos, &evPlayer->vel, &evPlayer->pos);
                evPlayer->speed--;
                if(evPlayer->speed <= 0) {
                    evPlayer->pos.y = masuP->pos.y;
                    evPlayer->mode = CAKETHROW_PLAYER_BALLINIT;
                    evPlayer->timer = 0;
                }
            }
            MBPlayerPosSetV(playerNo, &evPlayer->pos);
            break;
        
        case CAKETHROW_PLAYER_END:
            if(evPlayer->timer == 0) {
                MBModelHookReset(MBPlayerModelGet(playerNo));
                MBPlayerMotIdleSet(playerNo);
                evPlayer->angleStart = 0;
                evPlayer->angleSpeed = 0.15f;
                evPlayer->timer++;
            }
            break;
        
        case CAKETHROW_PLAYER_KILL:
            for(i=0; i<4; i++) {
                if(evPlayer->motNo[i] >= 0) {
                    MBPlayerMotionKill(playerNo, evPlayer->motNo[i]);
                }
                evPlayer->motNo[i] = MB_MOT_NONE;
            }
            break;
    }
    MBAngleMoveTo(&evPlayer->rot.y, evPlayer->angleStart, evPlayer->angleSpeed);
    MBPlayerRotSetV(playerNo, &evPlayer->rot);
}

#undef CAKETHROW_PLAYER_KILL
#undef CAKETHROW_PLAYER_INIT
#undef CAKETHROW_PLAYER_BALLINIT
#undef CAKETHROW_PLAYER_IDLE
#undef CAKETHROW_PLAYER_JUMP_BALL
#undef CAKETHROW_PLAYER_JUMP
#undef CAKETHROW_PLAYER_END

static void MB6Ev_KanePartyEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix);
static void MB6Ev_KanePartyEffCreate(int mdlId);
static void MB6Ev_KaneDustEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix);
static void MB6Ev_KaneDustEffCreate(int mdlId, HuVecF *pos);

void MB6Ev_Kane(int playerNo)
{
    int i;
    int j;
    MB6EV_PLAYER *evPlayer;
    int modelId;
    int capsuleNum;
    
    MASU *linkMasuP;
    MASU *masuP;
    int coin;
    int dustEffMdlId;
    unsigned int time;
    
    int linkMasuId;
    int star;
    int masuId;
    int bobleTime;
    float scale;
    float angle;
    float capAngle;
    float zoom;
    float capSpace;
    
    int capsuleMdlId[16];
    HuVecF pos;
    HuVecF rot;
    HuVecF rotOrig;
    int coinNum[GW_PLAYER_MAX];
    int capsuleTbl[GW_PLAYER_MAX];
    
    MBMODELID bobleMdlId[4];
    int partyEffMdlId[2];
    int guideMdlId;
    int result;
    int capsuleFree;
    
    static HuVecF boblePosTbl[4] = {
        { -250, 1970, -3400 },
        { 250, 1970, -3400 },
        { -345, 1970, -3200 },
        { 350, 1970, -3200 },
    };
    
    static u32 resultMesTbl[3] = {
        MESS_BOARD_W06_KANE_RESULT_COIN,
        MESS_BOARD_W06_KANE_RESULT_STAR,
        MESS_BOARD_W06_KANE_RESULT_CAPSULE
    };
    
    static HuVecF effPosTbl[2] = {
        { -500, 1700, -3500 },
        { 500, 1700, -3500 },
    };
    
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    
    for(linkMasuId=masuId, i=0; i<masuP->linkNum; i++){ 
        linkMasuId = masuP->linkTbl[i];
        if(MBMasuGet(MASU_LAYER_DEFAULT, linkMasuId)->flag & MB6EV_EVENT_LINK_FLAG) {
            break;
        }
        linkMasuId = masuId;
    }
    linkMasuP = MBMasuGet(MASU_LAYER_DEFAULT, linkMasuId);
    zoom = MBCameraZoomGet();
    for(i=0; i<2; i++) {
        partyEffMdlId[i] = MBEffCreate(mb6_Work.effAnimParty, 256);
        MBEffHookSet(partyEffMdlId[i], MB6Ev_KanePartyEffHook);
        Hu3DModelLayerSet(partyEffMdlId[i], 2);
        Hu3DModelPosSetV(partyEffMdlId[i], &effPosTbl[i]);
    }
    dustEffMdlId = MBEffCreate(mb6_Work.effAnimPuff, 64);
    MBEffHookSet(dustEffMdlId, MB6Ev_KaneDustEffHook);
    Hu3DModelLayerSet(dustEffMdlId, 2);
    memset(&mb6_Work.evPlayer[0], 0, sizeof(mb6_Work.evPlayer));
    for(evPlayer=&mb6_Work.evPlayer[0], i=0; i<GW_PLAYER_MAX; i++, evPlayer++) {
        evPlayer->inEventF = TRUE;
        MBPlayerPosGet(i, &evPlayer->pos);
        evPlayer->motNo[0] = MBPlayerMotionCreate(i, CHARMOT_HSF_c000m1_506);
        evPlayer->motNo[1] = MBPlayerMotionCreate(i, CHARMOT_HSF_c000m1_356);
    }
    for(i=0; i<30u; i++) {
        MB6Ev_KaneBackTPLvlSet(1-(i/30.0f));
        HuPrcVSleep();
    }
    for(i=0; i<30u; i++) {
        HuPrcVSleep();
    }
    MB6Ev_KanePartyEffCreate(partyEffMdlId[0]);
    MB6Ev_KanePartyEffCreate(partyEffMdlId[1]);
    MB6Ev_KaneDustEffCreate(dustEffMdlId, &effPosTbl[0]);
    MB6Ev_KaneDustEffCreate(dustEffMdlId, &effPosTbl[1]);
    MBAudFXPlay(MSM_SE_BD6_11);
    MBAudFXPlay(MSM_SE_BD6_12);
    MBPlayerRotateStart(playerNo, 180, 18);
    MBCameraFocusPlayerSet(-1);
    MBCameraPosViewSet(NULL, NULL, NULL, 2500, -1, 45);
    pos = linkMasuP->pos;
    pos.y += 150;
    guideMdlId = MBGuideCreateFlag(MB_GUIDE_TERL, &pos, FALSE, TRUE, TRUE, FALSE);
    MBAudFXPlay(MSM_SE_GUIDE_56);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_KANE_BEGIN, HUWIN_SPEAKER_TERL);
    MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo), 0);
    MBTopWinWait();
    MBTopWinKill();
    WipeColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    pos = masuP->pos;
    pos.x -= 150;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(i != playerNo) {
            MBPlayerPosSetV(i, &pos);
            pos.x += 150;
        }
    }
    MBPlayerPosSetV(playerNo, &linkMasuP->pos);
    MBPlayerRotSet(playerNo, 0, 0, 0);
    MBPlayerMotIdleSet(playerNo);
    MBGuideMdlPosSet(guideMdlId, linkMasuP->pos.x-300, linkMasuP->pos.y+100, linkMasuP->pos.z);
    MBCameraFocusPlayerSet(playerNo);
    MBCameraZoomSet(4000);
    MBCameraSkipSet(FALSE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    WipeWait();
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_KANE_PARTY, HUWIN_SPEAKER_TERL);
    MBTopWinWait();
    MBTopWinKill();
    for(i=0; i<4; i++) {
        bobleMdlId[i] = modelId = MBModelCreate(W06_HSF_boble, NULL, TRUE);
        MBModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
        MBModelPosSetV(modelId, &boblePosTbl[i]);
        MBModelScaleSet(modelId, 0, 0, 0);
        MBModelDispSet(modelId, FALSE);
    }
    MBCameraFocusPlayerSet(-1);
    MBCameraRotGet(&rotOrig);
    MBPlayerPosGet(playerNo, &pos);
    pos.y = 1643;
    rot.x = -18;
    rot.y = rot.z = 0;
    MBCameraPosViewSet(&pos, &rot, NULL, 2250, -1, 120);
    MBMotionSpeedSet(mb6_Work.objMdlId[MB6_OBJ_KANE], 1);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerMotionNoSet(i, mb6_Work.evPlayer[i].motNo[0], HU3D_MOTATTR_LOOP);
    }
    bobleTime = 0;
    for(i=0; i<180u; i++) {
        if(i > (int)((MBMotionMaxTimeGet(modelId)*2)-10)) {
            modelId = mb6_Work.objMdlId[MB6_OBJ_KANE];
            if(MBMotionTimeGet(modelId) < 15) {
                MBMotionSpeedSet(modelId, MBMotionSpeedGet(modelId)*0.93f);
            }
            if(MBMotionTimeGet(modelId) >= 15) {
                MBMotionTimeSet(modelId, 15);
                MBMotionSpeedSet(modelId, 0);
            }
        }
        if(i < 120 && ((i-15)%30) == 0) {
            MBAudFXPlay(MSM_SE_BD6_10);
        }
        bobleTime++;
        for(j=0; j<4; j++) {
            if(bobleTime > (j+1)*30) {
                if(!MBModelDispGet(bobleMdlId[j])) {
                    MBAudFXPlay(MSM_SE_BD6_04);
                }
                MBModelDispSet(bobleMdlId[j], TRUE);
                MBModelScaleGet(bobleMdlId[j], &rot);
                rot.x += 0.1f*(1-rot.x);
                rot.y = rot.z = rot.x;
                MBModelScaleSetV(bobleMdlId[j], &rot);
            }
        }
        HuPrcVSleep();
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerMotionNoSet(i, mb6_Work.evPlayer[i].motNo[1], HU3D_MOTATTR_NONE);
        MBPlayerWinLoseVoicePlay(i, mb6_Work.evPlayer[i].motNo[1], CHARVOICEID(5));
    }
    for(i=0; !MBPlayerMotionEndCheck(playerNo);) {
        if(i++ == 20) {
            MB6Ev_KanePartyEffCreate(partyEffMdlId[0]);
            MB6Ev_KanePartyEffCreate(partyEffMdlId[1]);
            MB6Ev_KaneDustEffCreate(dustEffMdlId, &effPosTbl[0]);
            MB6Ev_KaneDustEffCreate(dustEffMdlId, &effPosTbl[1]);
            MBAudFXPlay(MSM_SE_BD6_11);
        }
        HuPrcVSleep();
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerMotIdleSet(i);
    }
    MBCameraFocusPlayerSet(-1);
    VECSubtract(&linkMasuP->pos, &masuP->pos, &pos);
    VECScale(&pos, &pos, 0.4f);
    VECAdd(&masuP->pos, &pos, &pos);
    MBCameraPosViewSet(&pos, &rotOrig, NULL, 2500, -1, 45);
    for(i=0; i<45u; i++) {
        for(j=0; j<4; j++) {
            MBModelScaleGet(bobleMdlId[j], &rot);
            VECScale(&rot, &rot, 0.95f);
            MBModelScaleSetV(bobleMdlId[j], &rot);
        }
        MB6Ev_KaneBackTPLvlSet(0.022222223f*i);
        HuPrcVSleep();
    }
    for(i=0; i<4; i++) {
        if(bobleMdlId[i] >= 0) {
            MBModelKill(bobleMdlId[i]);
        }
    }
    MB6Ev_KaneBackTPLvlSet(1);
    modelId = mb6_Work.objMdlId[MB6_OBJ_KANE];
    if(modelId >= 0) {
        MBMotionSpeedSet(modelId, 0);
        MBMotionTimeSet(modelId, 15);
    }
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_KANE_ROULETTE, HUWIN_SPEAKER_TERL);
    MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo), 0);
    MBTopWinWait();
    MBTopWinKill();
    if(MBRouletteKaneCreate(playerNo)) {
        MBRouletteWait();
    }
    result = MBRouletteResultGet();
    MBAudFXPlay(MSM_SE_GUIDE_55);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_KANE_ROULETTE_RESULT, HUWIN_SPEAKER_TERL);
    MBTopWinInsertMesSet(resultMesTbl[result], 0);
    MBTopWinWait();
    MBTopWinKill();
    switch(result) {
        case 0:
            coin = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(i != playerNo) {
                    j = MBPlayerCoinGet(i);
                    if(j >= 10) {
                        coin += 10;
                    } else {
                        coin += j;
                    }
                    coinNum[i] = -10;
                    if(j) {
                        MBCoinAddProcExec(i, -10, TRUE, TRUE);
                    }
                }
            }
            coinNum[playerNo] = coin;
            if(coin == 0) {
                MBAudFXPlay(MSM_SE_GUIDE_57);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_KANE_5COIN, HUWIN_SPEAKER_TERL);
                MBTopWinWait();
                MBTopWinKill();
                coin = 5;
            }
            MBCoinAddDispExec(playerNo, coin, TRUE);
            MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_WIN, HU3D_MOTATTR_NONE);
            MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_WIN, CHARVOICEID(5));
            break;
        
        case 1:
            star = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(i != playerNo) {
                    j = MBPlayerStarGet(i);
                    if(j >= 1) {
                        star++;
                        MBPlayerStarAdd(i, -1);
                    }
                }
            }
            if(star) {
                MBMusPauseFadeOut(MB_MUS_CHAN_BG, TRUE, 1000);
                MBPlayerPosGet(playerNo, &pos);
                pos.y += 700;
                MBStarGetProcExec(playerNo, &pos, star, FALSE);
            } else {
                MBAudFXPlay(MSM_SE_GUIDE_57);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_KANE_STAR_FAIL, HUWIN_SPEAKER_TERL);
                MBTopWinWait();
                MBTopWinKill();
                MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_LOSE, HU3D_MOTATTR_NONE);
                MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_LOSE, CHARVOICEID(10));
            }
            break;
        
        case 2:
            capsuleNum = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(i != playerNo) {
                    j = MBPlayerCapsuleNumGet(i);
                    if(j >= 1) {
                        capsuleTbl[capsuleNum] = MBPlayerCapsuleGet(i, 0);
                        MBPlayerCapsuleRemove(i, 0);
                        capsuleNum++;
                    }
                }
            }
            capsuleFree = MBPlayerCapsuleMaxGet()-MBPlayerCapsuleNumGet(playerNo);
            if(capsuleNum && capsuleFree) {
                if(capsuleNum > capsuleFree) {
                    capsuleNum = capsuleFree;
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_KANE_CAP_FULL, HUWIN_SPEAKER_TERL);
                    MBTopWinWait();
                    MBTopWinKill();
                }
                for(i=0; i<capsuleNum; i++) {
                    capsuleMdlId[i] = MBModelCreate(MBCapsuleMdlGet(capsuleTbl[i]), NULL, TRUE);
                }
                MBPlayerPosGet(playerNo, &pos);
                pos.y += 500;
                MBAudFXPlay(MSM_SE_BOARD_33);
                MBAudFXPlay(MSM_SE_BD6_14);
                scale = 1;
                angle = 0;
                capSpace = 360.0f/capsuleNum;
                time = 0;
                for(time=0; time<240; time++) {
                    capAngle = angle;
                    for(i=0; i<capsuleNum; i++) {
                        MBModelPosSet(capsuleMdlId[i], pos.x+(scale*(100*HuSin(capAngle))), pos.y, pos.z+(scale*(100*HuCos(capAngle))));
                        MBModelScaleSet(capsuleMdlId[i], scale, scale, scale);
                        capAngle += capSpace;
                    }
                    pos.y += -1.6666667f;
                    angle += 6.0000005f;
                    if(angle >= 360) {
                        angle -= 360;
                    }
                    if(time > 180) {
                        scale *= 0.95f;
                    }
                    HuPrcVSleep();
                }
                for(i=0; i<capsuleNum; i++) {
                    MBModelKill(capsuleMdlId[i]);
                }
                j = 0;
                for(i=MBPlayerCapsuleNumGet(playerNo); i<MBPlayerCapsuleMaxGet(); i++) {
                    if(j < capsuleNum) {
                        MBPlayerCapsuleAdd(playerNo, capsuleTbl[j++]);
                    }
                }
                MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_WIN, HU3D_MOTATTR_NONE);
                MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_WIN, CHARVOICEID(5));
            } else {
                MBAudFXPlay(MSM_SE_GUIDE_57);
                if(capsuleNum) {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_KANE_CAP_FULL, HUWIN_SPEAKER_TERL);
                } else {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_KANE_CAP_FAIL, HUWIN_SPEAKER_TERL);
                }
                MBTopWinWait();
                MBTopWinKill();
                MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_LOSE, HU3D_MOTATTR_NONE);
                MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_LOSE, CHARVOICEID(10));
            }
            break;
    }
    while(!MBPlayerMotionEndCheck(playerNo)) {
        HuPrcVSleep();
    }
    for(i=0; i<60u; i++) {
        HuPrcVSleep();
    }
    MBMusPauseFadeOut(MB_MUS_CHAN_BG, FALSE, 1000);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_KANE_END, HUWIN_SPEAKER_TERL);
    MBTopWinWait();
    MBTopWinKill();
    MBPlayerMotIdleSet(playerNo);
    MBGuideKill();
    WipeColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    for(evPlayer=&mb6_Work.evPlayer[0], i=0; i<GW_PLAYER_MAX; i++, evPlayer++) {
        MBPlayerPosSetV(i, &evPlayer->pos);
        MBPlayerMotionKill(i, evPlayer->motNo[0]);
        MBPlayerMotionKill(i, evPlayer->motNo[1]);
    }
    MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_NONE);
    MBEffKill(partyEffMdlId[0]);
    MBEffKill(partyEffMdlId[1]);
    MBEffKill(dustEffMdlId);
    MBCameraFocusPlayerSet(playerNo);
    MBCameraZoomSet(zoom);
    MBCameraSkipSet(FALSE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    WipeWait();
    MB3MiracleGetExec(playerNo);
}

static void MB6Ev_KanePartyEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix)
{
    MBEFFECTDATA *effDataP;
    int i;
    int dispNum;
    static HuVecF colMinTbl[8] = {
        { 128, 128, 128 },
        { 200, 0, 0 },
        { 0, 200, 0 },
        { 0, 0, 200 },
        { 200, 200, 0 },
        { 0, 200, 200 },
        { 200, 0, 200 },
        { 200, 150, 0 }
    };
    static float colMaxTbl[16][2] = {
        { 255, 63 },
        { 255, 127 },
        { 255, 191 },
        { 255, 255 },
        { 255, 191 },
        { 255, 127 },
        { 255, 63 },
        { 255, 0 },
        { 0, 31 },
        { 0, 63 },
        { 0, 95 },
        { 0, 127 },
        { 0, 95 },
        { 0, 63 },
        { 0, 31 },
        { 0, 0 }
    };
    
    float colMin;
    
    if(effP->count == 0) {
        for(effDataP = effP->data, i=0; i<effP->num; i++, effDataP++) {
            effDataP->scale = 0;
            effDataP->color.a = 0;
        }
        effP->count = 1;
    }
    for(effDataP = effP->data, i=0; i<effP->num; i++, effDataP++) {
        if(effDataP->time) {
            VECAdd(&effDataP->pos, &effDataP->vel, &effDataP->pos);
            VECAdd(&effDataP->pos, &effDataP->accel, &effDataP->pos);
            VECScale(&effDataP->accel, &effDataP->accel, 0.94f);
            effDataP->vel.y *= 0.93f;
            effDataP->vel.y += -0.27222225f;
            effDataP->speedDecay += effDataP->colorIdx;
            if(effDataP->speedDecay >= 16) {
                effDataP->speedDecay -= 16;
            } else if(effDataP->speedDecay < 0) {
                effDataP->speedDecay += 16;
            }
            effDataP->animBank = (int)effDataP->speedDecay & 0xF;
            effDataP->rot.z += effDataP->scaleBase;
            colMin = colMinTbl[effDataP->parManId].x;
            effDataP->color.r = colMin+(0.003125f*((colMaxTbl[effDataP->animBank][0]-colMin)*(colMaxTbl[effDataP->animBank][1])));
            colMin = colMinTbl[effDataP->parManId].y;
            effDataP->color.g = colMin+(0.003125f*((colMaxTbl[effDataP->animBank][0]-colMin)*(colMaxTbl[effDataP->animBank][1])));
            colMin = colMinTbl[effDataP->parManId].z;
            effDataP->color.b = colMin+(0.003125f*((colMaxTbl[effDataP->animBank][0]-colMin)*(colMaxTbl[effDataP->animBank][1])));
            effDataP->time--;
            if(effDataP->time <= 0) {
                effDataP->time = 0;
                effDataP->scale = 0;
                effDataP->color.a = 0;
            } else if(effDataP->time < 10) {
                if(effDataP->color.a >= 20) {
                    effDataP->color.a -= 20;
                }
            }
        }
    }
}

static void MB6Ev_KanePartyEffCreate(int mdlId)
{
    MBEFFECT *effP = Hu3DData[mdlId].hookData;
    MBEFFECTDATA *effDataP;
    int i;
    float angle;
    float scale;
    float speed;
    for(effDataP = effP->data, i=0; i<effP->num; i++, effDataP++) {
        if(effDataP->time == 0) {
            effDataP->time++;
            effDataP->rot.z = MBEffRandF()*360;
            angle = MBEffRandF();
            effDataP->animBank = (int)(MBEffRandF()*16) & 0xF;
            effDataP->parManId = (int)(MBEffRandF()*8) & 0x7;
            effDataP->scale = (MBEffRandF()*20)+20;
            effDataP->color.a = (MBEffRandF()*50)+180;
            effDataP->color.r = 255;
            effDataP->color.g = 255;
            effDataP->color.b = 255;
            effDataP->pos.x = (MBEffSRandF()*0.2f)*100;
            effDataP->pos.y = (MBEffSRandF()*0.2f)*100;
            effDataP->pos.z = (MBEffSRandF()*0.2f)*100;
            angle = MBEffRandF()*360;
            scale = MBEffRandF();
            speed = 0.8333333f+(1.6666666f*(1.5f*scale));
            effDataP->vel.x = speed*HuSin(angle);
            effDataP->vel.z = speed*HuCos(angle);
            scale *= 0.8f;
            scale = 1-(scale*scale);
            angle = scale*MBEffRandF();
            scale = 1-((1-angle)*(1-angle));
            angle += 0.7f*(scale-angle);
            effDataP->vel.y = 16.666666f+(1.6666666f*(30*angle));
            effDataP->accel = effDataP->vel;
            angle = 1+(0.1f*(effDataP->accel.y-16.666666f));
            effDataP->accel.x *= angle;
            effDataP->accel.z *= angle;
            effDataP->accel.y = 0;
            effDataP->vel.z += ((MBEffRandF()*1.2f)*1.6666666f)+0.8333333f;
            effDataP->time = (MBEffRandF()*60)+120;
            effDataP->speedDecay = MBEffRandF()*15;
            effDataP->colorIdx = (MBEffRandF()*0.5f)+0.5f;
            if(MBEffSRandF() < 0) {
                effDataP->colorIdx *= -1;
            }
            effDataP->scaleBase = ((MBEffRandF()*40)+10)*0.016666668f;
        }
    }
}

static void MB6Ev_KaneDustEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix)
{
    MBEFFECTDATA *effDataP;
    int i;
    for(effDataP = effP->data, i=0; i<effP->num; i++, effDataP++) {
        if(effDataP->time) {
            VECAdd(&effDataP->pos, &effDataP->vel, &effDataP->pos);
            effDataP->vel.x *= 0.98f;
            effDataP->vel.y *= 0.95f;
            effDataP->vel.z *= 0.98f;
            effDataP->scale += 6;
            effDataP->color.a *= 0.98f;
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
}

static void MB6Ev_KaneDustEffCreate(int mdlId, HuVecF *pos)
{
    MBEFFECT *effP = Hu3DData[mdlId].hookData;
    MBEFFECTDATA *effDataP;
    int i;
    unsigned int num;
    float angle;
    float speed;
    HuVecF dir;
    for(effDataP = effP->data, num=32, i=0; i<effP->num; i++, effDataP++) {
        if(effDataP->time == 0) {
            effDataP->time = ((MBEffRandF()*0.5f)+0.7f)*60;
            angle = MBEffSRandF()*360;
            dir.x = HuSin(angle);
            dir.y = 0;
            dir.z = HuCos(angle);
            VECScale(&dir, &effDataP->pos, 40);
            effDataP->pos.y += (MBEffSRandF()*0.3f)*100;
            VECAdd(&effDataP->pos, pos, &effDataP->pos);
            angle = MBEffRandF();
            speed = ((MBEffRandF()*2)*1.6666666f)+3.3333333f;
            VECScale(&dir, &effDataP->vel, speed*angle);
            effDataP->vel.y = 1.6666666f+(1.6666666f*(10*angle));
            effDataP->rot.z = MBEffRandF()*360;
            effDataP->scale = (MBEffRandF()*40)+20;
            effDataP->color.a = (MBEffRandF()*80)+80;
            angle = MBEffRandF();
            effDataP->color.r = (angle*50)+200;
            effDataP->color.g = (angle*50)+190;
            effDataP->color.b = (angle*50)+195;
            num--;
            if(num < 1) {
                break;
            }
        }
        
    }
}

void MB6Ev_KaneBackCreate(void)
{
    int i;
    HSFOBJECT *objP;
    
    HU3DMODEL *modelP;
    HSFDATA *hsfP;
    int backMdlId;
    
    static char *hookTbl[2] = {
        "mnashi-haikei",
        "mnashi-haikei1"
    };
    for(i=0; i<3; i++) {
        modelP = &Hu3DData[MBModelIdGet(mb6_Work.kaneMdlId[i])];
        hsfP = modelP->hsf;
        mb6_Work.kaneMatBuf[i] = HuMemDirectMalloc(HEAP_MODEL, hsfP->materialNum*sizeof(HSFMATERIAL));
        memcpy(mb6_Work.kaneMatBuf[i], hsfP->material, hsfP->materialNum*sizeof(HSFMATERIAL));
    }
    backMdlId = MBModelIdGet(mb6_Work.kaneMdlId[1]);
    for(i=0; i<2; i++) {
        objP = Hu3DModelObjPtrGet(backMdlId, hookTbl[i]);
        if(objP->mesh.color != NULL) {
            mb6_Work.haikeiBuf[i] = *objP->mesh.color;
            mb6_Work.haikeiBuf2[i] = *objP->mesh.color;
            mb6_Work.haikeiBuf2[i].data = HuMemDirectMalloc(HEAP_MODEL, sizeof(GXColor)*objP->mesh.color->count);
            memcpy(mb6_Work.haikeiBuf2[i].data, objP->mesh.color->data, sizeof(GXColor)*objP->mesh.color->count);
        }
    }
}

void MB6Ev_KaneBackKill(void)
{
    int i;
    HU3DMODEL *modelP;
    HSFDATA *hsfP;
    for(i=0; i<3; i++) {
        if(mb6_Work.kaneMatBuf[i] != NULL) {
            modelP = &Hu3DData[MBModelIdGet(mb6_Work.kaneMdlId[i])];
            hsfP = modelP->hsf;
            memcpy(hsfP->material, mb6_Work.kaneMatBuf[i], hsfP->materialNum*sizeof(HSFMATERIAL));
            HuMemDirectFree(mb6_Work.kaneMatBuf[i]);
            mb6_Work.kaneMatBuf[i] = NULL;
        }
    }
    for(i=0; i<2; i++) {
        if(mb6_Work.haikeiBuf2[i].data != NULL) {
            HuMemDirectFree(mb6_Work.haikeiBuf2[i].data);
            mb6_Work.haikeiBuf2[i].data = NULL;
        }
    }
}

void MB6Ev_KaneBackTPLvlSet(float tpLvl)
{
    int i;
    HSFMATERIAL *matSrc;
    HSFMATERIAL *matDst;
    GXColor *colorSrc;
    GXColor *colorDst;
    int j;
    HSFDATA *hsfP;
    HU3DMODEL *modelP;
    int colorNum;
    int baseColorR;
    int baseColorG;
    int baseColorB;
    Hu3DAmbColorSet(0.00390625f*(110+(tpLvl*146)), 0.00390625f*(96+(tpLvl*160)), 0.00390625f*(64+(tpLvl*192)));
    for(i=0; i<3; i++) {
        modelP = &Hu3DData[MBModelIdGet(mb6_Work.kaneMdlId[i])];
        hsfP = modelP->hsf;
        matDst = hsfP->material;
        matSrc = mb6_Work.kaneMatBuf[i];
        for(j=0; j<hsfP->materialNum; j++, matDst++, matSrc++) {
            matDst->color[0] = 0.00390625f*(matSrc->color[0]*(130+(126*tpLvl)));
            matDst->color[1] = 0.00390625f*(matSrc->color[1]*(120+(136*tpLvl)));
            matDst->color[2] = 0.00390625f*(matSrc->color[2]*(100+(156*tpLvl)));
        }
    }
    baseColorR = 64+(192*tpLvl);
    baseColorG = 56+(200*tpLvl);
    baseColorB = 48+(208*tpLvl);
    for(i=0; i<2; i++) {
        if(mb6_Work.haikeiBuf2[i].data != NULL) {
            colorNum = mb6_Work.haikeiBuf[i].count;
            colorSrc = mb6_Work.haikeiBuf2[i].data;
            colorDst = mb6_Work.haikeiBuf[i].data;
            for(j=0; j<colorNum; j++, colorSrc++, colorDst++) {
                colorDst->r = (colorSrc->r*baseColorR)>>8;
                colorDst->g = (colorSrc->g*baseColorG)>>8;
                colorDst->b = (colorSrc->b*baseColorB)>>8;
            }
        }
    }
}

float MB6_DistRatioGet(HuVecF *vec1, HuVecF *vec2, HuVecF *vec3)
{
    float mag = VECMag(vec3);
    if(mag > 0) {
        mag = ((vec3->x*(vec1->x-vec2->x))+(vec3->y*(vec1->y-vec2->y))+(vec3->z*(vec1->z-vec2->z)))/mag;
    }
    return mag;
}

float MB6_AngleDiff(float start, float end)
{
    float angle = fmodf(end-start, 360);
    if(angle < 0) {
        angle += 360;
    }
    if(angle > 180) {
        angle -= 360;
    }
    return angle;
}

unsigned int MB6Ev_BitGet(int outMask, unsigned int inMask)
{
    unsigned int bitIn;
    int i;
    unsigned int result;
    int bitOut;
    
    outMask &= inMask;
    result = 0;
    bitOut = bitIn = (1 << 0);
    
    for(i=0; i<32; i++) {
        if(inMask & bitIn) {
            if(outMask & bitIn) {
                result |= bitOut;
            }
            bitOut <<= 1;
        }
        bitIn <<= 1;
    }
    return result;
}

unsigned int MB6Ev_MasuFlagGet(int outMask, unsigned int inMask)
{
    unsigned int bitIn;
    int i;
    int bitOut;
    unsigned int result;
    bitIn = bitOut = (1 << 0);
    result = 0;
    for(i=0; i<32; i++) {
        if(inMask & bitIn) {
            if(outMask & bitOut) {
                result |= bitIn;
            }
            bitOut <<= 1;
        }
        bitIn <<= 1;
    }
    return result;
}