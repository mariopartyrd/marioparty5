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

#include "game/sprite.h"
#include "game/wipe.h"

#include "datanum/w06s.h"

#define MB6EV_CAKEMOVE_MAX 2
#define MB6EV_CAKEMOVE_1_FLAG MASU_FLAG_BIT(0)
#define MB6EV_CAKEMOVE_2_FLAG (MASU_FLAG_BIT(0)|MASU_FLAG_BIT(1))
#define MB6EV_CAKEMOVE_FLAG (MB6EV_CAKEMOVE_1_FLAG|MB6EV_CAKEMOVE_2_FLAG)
#define MB6EV_CAKEMOVE_LINK_FLAG MASU_FLAG_BIT(3)

#define MB6_OBJ_MAX 3

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

typedef struct MB6Ev_CakeMove_s {
    int masuId;
    char *cakeHook;
    char *knifeHook;
    MBMODELID cakeMdlId;
    MBMODELID knifeMdlId;
} MB6EV_CAKEMOVE;

typedef struct MB6_Work_s {
    int objMdlId[MB6_OBJ_MAX];
    ANIMDATA *effAnim1;
    ANIMDATA *cakeEffAnim;
    HU3DMODELID effMdlId[4]; 
    HU3DMODELID cookieEffMdlId;
    int lightId;
    float lightPosX;
    float lightPosY;
    float lightPosZ;
    float lightDirX;
    float lightDirY;
    float lightDirZ;
    GXColor lightColor;
    MB6EV_PLAYER evPlayer[GW_PLAYER_MAX];
    MB6EV_CAKEMOVE evCakeMove[MB6EV_CAKEMOVE_MAX];
} MB6_WORK;

typedef struct MB6SaveWork_s {
    u8 cookieF[2];
} MB6_SAVEWORK;

static MBMODELID mb6_MainMdlId;
static MB6_SAVEWORK *mb6_SaveWorkP;
static MB6_WORK mb6_Work;
static OMOBJ *mb6_MainObj;

#include "relprolog.inc"

void MB6_Create(void);
void MB6_Kill(void);
void MB6_LightSet(void);
void MB6_LightReset(void);
int MB6Ev_MasuWalkPre(int playerNo);
int MB6Ev_MasuWalkPost(int playerNo);
int MB6Ev_MasuHatena(int playerNo);

void MB6Ev_CakeMoveCreate(void);
void MB6Ev_CakeMoveKill(void);
void MB6Ev_CakeMove(int playerNo, int masuId);

unsigned int MB6Ev_BitGet(int outMask, unsigned int inMask);
unsigned int MB6Ev_MasuFlagGet(int outMask, unsigned int inMask);

void ObjectSetup(void)
{
    GWPartyFSet(FALSE);
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
    MBMasuCreate(W06S_BIN_masuData);
    mb6_MainMdlId = MBModelCreate(W06S_HSF_map, NULL, FALSE);
    MBModelPosSet(mb6_MainMdlId, 0, 0, 0);
    MBModelAttrSet(mb6_MainMdlId, HU3D_MOTATTR_LOOP);
    MBScrollInit(W06S_HSF_scrollCol, 10690, -631);
    {
        HuVecF cameraPos = { 1100, 670, 970 };
        HuVecF cameraRot = { -24, 0, 0 };
        HuVecF mapCameraPos = { 1250, -120, -55 };
        HuVecF mapCameraRot = { -55, 0, 0 };
        MBOpeningViewSet(&cameraRot, &cameraPos, 9740);
        MBMapViewCameraSet(&mapCameraRot, &mapCameraPos, 10450);
    }
    MBLightHookSet(MB6_LightSet, MB6_LightReset);
    MBCapsuleColCreate(W06S_HSF_capsuleCol);
    MBBranchFlagSet(MASU_FLAG_BIT(2));
    MB6_ObjectCreate();
    MB6Ev_CakeMoveCreate();
    HuDataDirClose(DATA_w06s);
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
        W06S_HSF_line,
        W06S_HSF_back,
        W06S_HSF_backEff
    };
    for(i=0; i<MB6_OBJ_MAX; i++) {
        mb6_Work.objMdlId[i] = MB_MODEL_NONE;
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
    modelId = mb6_Work.objMdlId[1];
    if(modelId >= 0) {
        MBMotionSpeedSet(modelId, 1);
    }
    modelId = mb6_Work.objMdlId[2];
    if(modelId >= 0) {
        MBMotionSpeedSet(modelId, 0.333333f);
    }
    mb6_Work.cakeEffAnim = HuSprAnimRead(HuDataReadNum(W06S_ANM_puffEff, HU_MEMNUM_OVL));
    mb6_Work.effMdlId[1] = MBEffCreate(mb6_Work.cakeEffAnim, 1);
    Hu3DModelDispOff(mb6_Work.effMdlId[1]);
}

void MB6_Kill(void)
{
    int i;
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
    for(i=0; i<4; i++) {
        if(mb6_Work.effMdlId[i] >= 0) {
            MBEffKill(mb6_Work.effMdlId[i]);
        }
        mb6_Work.effMdlId[i] = HU3D_MODELID_NONE;
    }
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
    MASU *masu = MBMasuGet(MASU_LAYER_DEFAULT, GwPlayer[playerNo].masuId);
    u16 flag = masu->flag;
    if(flag & MB6EV_CAKEMOVE_FLAG) {
        MB6Ev_CakeMove(playerNo, GwPlayer[playerNo].masuId);
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

int MB6Ev_MasuWalkPost(int playerNo)
{
    return 0;
}

void MB6Ev_CakeMoveCreate(void)
{
    MB6EV_CAKEMOVE *cakeMove;
    int i;
    int modelId;
    
    static unsigned int cakeMoveData[MB6EV_CAKEMOVE_MAX][3] = {
        MB6EV_CAKEMOVE_1_FLAG,
        W06S_HSF_cakeMove1,
        W06S_HSF_knife1,
        MB6EV_CAKEMOVE_2_FLAG,
        W06S_HSF_cakeMove2,
        W06S_HSF_knife2,
    };
    static char *cakeHookTbl[MB6EV_CAKEMOVE_MAX] = {
        "sweet_d-m_hook1",
        "sweet_d-m_hook3"
    };
    static char *knifeHookTbl[MB6EV_CAKEMOVE_MAX] = {
        "sweet_d-knife",
        "sweet_d-knife"
    };
    
    memset(&mb6_Work.evCakeMove[0], 0, sizeof(mb6_Work.evCakeMove));
    for(cakeMove=&mb6_Work.evCakeMove[0], i=0; i<MB6EV_CAKEMOVE_MAX; i++, cakeMove++) {
        cakeMove->masuId = MBMasuFlagFind(MASU_LAYER_DEFAULT, MB6Ev_MasuFlagGet(cakeMoveData[i][0], MB6EV_CAKEMOVE_FLAG), MB6EV_CAKEMOVE_FLAG);
        modelId = MBModelCreate(cakeMoveData[i][1], NULL, 0);
        cakeMove->cakeMdlId = modelId;
        MBMotionSpeedSet(modelId, 0);
        modelId = MBModelCreate(cakeMoveData[i][2], NULL, 0);
        cakeMove->knifeMdlId = modelId;
        MBMotionSpeedSet(modelId, 0);
        MBModelDispSet(modelId, FALSE);
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
    memset(&mb6_Work.evPlayer[0], 0, sizeof(mb6_Work.evPlayer));
    for(evPlayer=&mb6_Work.evPlayer[0], i=0; i<GW_PLAYER_MAX; i++, evPlayer++) {
        if(masuId == GwPlayer[i].masuId && MBPlayerAliveCheck(i)) {
            evPlayer->inEventF = TRUE;
            GwPlayer[i].storySkipEventF = TRUE;
        }
        if(i == playerNo) {
            evPlayer->shockEndF = TRUE;
        }
    }
    MB6Ev_CakeMovePlayerUpdate(cakeMove);
    effMdlId = MBEffCreate(mb6_Work.cakeEffAnim, 32);
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
    MBMotionTimeSet(cakeMove->knifeMdlId, 0);
    MBMotionSpeedSet(cakeMove->knifeMdlId, 0);
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

#undef MBEffRandF
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

float MB6_DistRatioGet(HuVecF *vec1, HuVecF *vec2, HuVecF *vec3)
{
    float mag = VECMag(vec3);
    if(mag > 0) {
        mag = ((vec3->x*(vec1->x-vec2->x))+(vec3->y*(vec1->y-vec2->y))+(vec3->z*(vec1->z-vec2->z)))/mag;
    }
    return mag;
}

float MB6_WrapAngle(float start, float end)
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