#include "REL/decathlonDll.h"

static unsigned int playerMotList[] = {
    CHARMOT_HSF_c000m1_300,
    CHARMOT_HSF_c000m1_301,
    CHARMOT_HSF_c000m1_302,
    CHARMOT_HSF_c000m1_361,
    CHARMOT_HSF_c000m1_360,
    0
};

static HuVecF playerPosTbl[GW_PLAYER_MAX*2] = {
    { -300, -10, -1920 },
    { -100, -10, -1920 },
    { 100, -10, -1920 },
    { 300, -10, -1920 },
    { -250, -10, -2020 }, 
    { -50, -10, -2020 },
    { 150, -10, -2020 },
    { 350, -10, -2020 }
};

static HuVecF guidePosTbl[4] = {
    { 0, 100, 0 },
    { -350, 100, -1920 },
    { 0, 60, -4200 },
    { 0, 100, -1800 },
};

static HuVecF openingAimPos = { 0, 0, -4000 };

DECAMG decaMgTbl[10] = {
    {
        DLL_m502dll,
        502,
        0,
        DECATHLON_ANM_icon_m502,
        DECA_SCORE_TIME,
        FALSE
    },
    {
        DLL_m504dll,
        504,
        0,
        DECATHLON_ANM_icon_m504,
        DECA_SCORE_TIME,
        FALSE
    },
    {
        DLL_m563dll,
        563,
        0,
        DECATHLON_ANM_icon_m563,
        DECA_SCORE_NUMBER,
        TRUE
    },
    {
        DLL_m506dll,
        506,
        0,
        DECATHLON_ANM_icon_m506,
        DECA_SCORE_DIST,
        TRUE
    },
    {
        DLL_m507dll,
        507,
        0,
        DECATHLON_ANM_icon_m507,
        DECA_SCORE_DIST_FRAC,
        FALSE
    },
    {
        DLL_m510dll,
        510,
        0,
        DECATHLON_ANM_icon_m510,
        DECA_SCORE_NUMBER_CAP,
        TRUE
    },
    {
        DLL_m511dll,
        511,
        0,
        DECATHLON_ANM_icon_m511,
        DECA_SCORE_TIME,
        FALSE
    },
    {
        DLL_m513dll,
        513,
        0,
        DECATHLON_ANM_icon_m513,
        DECA_SCORE_TIME,
        FALSE
    },
    {
        DLL_m512dll,
        512,
        0,
        DECATHLON_ANM_icon_m512,
        DECA_SCORE_TIME,
        FALSE
    },
    {
        DLL_m514dll,
        514,
        0,
        DECATHLON_ANM_icon_m514,
        DECA_SCORE_DIST_FOOT,
        TRUE
    },
};

HuVecF lbl_1_data_17C = { 0, -100, -2000 };

static DECASTAGEMDL stageMdlTbl[DECA_MDL_STAGE_NUM] = {
    {
        DECATHLON_HSF_hata,
        HU3D_CAM4,
        2,
        HU3D_ATTR_NONE,
        HU3D_MOTATTR_LOOP
    },
    {
        DECATHLON_HSF_stage,
        HU3D_CAM4,
        2,
        HU3D_ATTR_NONE,
        HU3D_MOTATTR_LOOP
    },
    {
        DECATHLON_HSF_crowd,
        HU3D_CAM4,
        2,
        HU3D_ATTR_NONE,
        HU3D_MOTATTR_LOOP
    },
    {
        DECATHLON_HSF_stage_night,
        HU3D_CAM4,
        0,
        HU3D_ATTR_NONE,
        HU3D_MOTATTR_PAUSE
    }, 
    {
        DECATHLON_HSF_stage_fb,
        HU3D_CAM1,
        0,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_LOOP
    },
    {
        DECATHLON_HSF_kikyuu,
        HU3D_CAM4,
        3,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_LOOP
    },
    {
        DECATHLON_HSF_fuusen,
        HU3D_CAM4,
        3,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_LOOP
    },
    {
        DECATHLON_HSF_hikari,
        HU3D_CAM4,
        3,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_PAUSE
    },
    {
        DECATHLON_HSF_moon,
        HU3D_CAM4,
        3,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE
    },
    {
        DECATHLON_HSF_hanabi,
        HU3D_CAM4,
        4,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE
    },
    {
        DECATHLON_HSF_cover,
        HU3D_CAM4,
        2,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_PAUSE
    },
    {
        DECATHLON_HSF_light,
        HU3D_CAM4,
        4,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_LOOP
    },
    {
        DECATHLON_HSF_light,
        HU3D_CAM4,
        4,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_LOOP
    },
    {
        DECATHLON_HSF_light,
        HU3D_CAM4,
        4,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_LOOP
    },
    {
        DECATHLON_HSF_light,
        HU3D_CAM4,
        4,
        HU3D_ATTR_DISPOFF,
        HU3D_MOTATTR_LOOP
    }
};

#define FXID(charBase, no) (charBase+(no))

static int charSoundTbl[10][2] = {
    FXID(MSM_SE_VOICE_MARIO, 5),
    FXID(MSM_SE_VOICE_MARIO, 4),
    
    FXID(MSM_SE_VOICE_LUIGI, 5),
    FXID(MSM_SE_VOICE_LUIGI, 4),
    
    FXID(MSM_SE_VOICE_PEACH, 5),
    FXID(MSM_SE_VOICE_PEACH, 4),
    
    FXID(MSM_SE_VOICE_YOSHI, 5),
    FXID(MSM_SE_VOICE_YOSHI, 4),
    
    FXID(MSM_SE_VOICE_WARIO, 5),
    FXID(MSM_SE_VOICE_WARIO, 4),
    
    FXID(MSM_SE_VOICE_DAISY, 5),
    FXID(MSM_SE_VOICE_DAISY, 4),
    
    FXID(MSM_SE_VOICE_WALUIGI, 5),
    FXID(MSM_SE_VOICE_WALUIGI, 4),
    
    FXID(MSM_SE_VOICE_KINOPIO, 5),
    FXID(MSM_SE_VOICE_KINOPIO, 4),
    
    FXID(MSM_SE_VOICE_TERESA, 5),
    FXID(MSM_SE_VOICE_TERESA, 4),
    
    FXID(MSM_SE_VOICE_MINIKOOPAR, 5),
    FXID(MSM_SE_VOICE_MINIKOOPAR, 4),
};

#undef FXID

static s16 winSoundTbl[15][2] = {
    60, 32,
    90, 76,
    120, 96,
    120, 76,
    150, 52,
    160, 52,
    160, 76,
    180, 32,
    180, 52,
    180, 76,
    190, 32,
    190, 76,
    210, 52,
    240, 52,
    240, 96
};

DECAWORK decaWork;
DECAMODEWORK *decaModeWorkP;
DECAWORK *decaWorkP;

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


void DecaSortChar(s8 *a, s8 *b, s8 num)
{
    int i, j;
    for(i=0; i<num-1; i++) {
        for(j=i+1; j<num; j++) {
            if(a[i] > a[j]) {
                s8 temp = a[i];
                a[i] = a[j];
                a[j] = temp;
                if(b) {
                    s8 temp = b[i];
                    b[i] = b[j];
                    b[j] = temp;
                }
            }
        }
    }
}

void DecaSortInt(int *a, int *b, s8 num)
{
    int i, j;
    for(i=0; i<num-1; i++) {
        for(j=i+1; j<num; j++) {
            if(a[i] >= a[j]) {
                int temp = a[i];
                a[i] = a[j];
                a[j] = temp;
                if(b) {
                    int temp = b[i];
                    b[i] = b[j];
                    b[j] = temp;
                }
            }
        }
    }
}

void DecaArrayShuffle(s8 *arr, int num)
{
    s8 usednum[256];
    int i, j;
    int no;
    for(i=num; i--;) {
        usednum[i] = arr[i];
    }
    for(i=num; i--;) {
        if(i) {
            no = frandmod(i+1);
        } else {
            no = 0;
        }
        arr[i] = usednum[no];
        for(j=0; j<=i; j++) {
            if(j != no && j > no) {
                usednum[j-1] = usednum[j];
            }
        }
    }
}

extern inline float fabsf2(float x)
{
   return fabs(x);
}

BOOL DecaAngleSub(float *angle, float end, float speed)
{
    float diff = *angle-end;
    float change = fmodf(diff, 180);
    float wrap = fmodf(diff, 360);
    if(fabsf2(wrap) > 180.0f) {
        if(change > 0) {
            change = -(180.0f-fabsf2(change));
        } else {
            change = 180.0f-fabsf2(change);
        }
    }
    if(fabsf2(change) < speed) {
        *angle = end;
        return TRUE;
    }
    if(change > 0) {
        change -= speed;
    } else {
        change += speed;
    }
    *angle = end+change;
    return FALSE;
}

void DecaSleep(int time)
{
    while(time--) {
        HuPrcVSleep();
    }
}

void DecaWipeIn(int type, int time)
{
    WipeCreate(WIPE_MODE_IN, type, time);
    WipeWait();
}

void DecaWipeOut(int type, int time)
{
    WipeCreate(WIPE_MODE_OUT, type, time);
    WipeWait();
}

HUWINID DecaWinIdGet(void)
{
    return decaWorkP->winId;
}

void DecaWinOpen(void)
{
    if(!DecaFlagGet(DECAFLAG_WINBUSY)) {
        DecaFlagSet(DECAFLAG_WINBUSY);
        HuWinExOpen(decaWorkP->winId);
    }
}

void DecaWinClose(void)
{
    if(DecaFlagGet(DECAFLAG_WINBUSY)) {
        DecaFlagReset(DECAFLAG_WINBUSY);
        HuWinExClose(decaWorkP->winId);
        HuWinHomeClear(decaWorkP->winId);
    }
}

void DecaFlagSet(unsigned int flag)
{
    decaWorkP->flag |= flag;
}

void DecaFlagReset(unsigned int flag)
{
    decaWorkP->flag &= ~flag;
}

unsigned int DecaFlagGet(unsigned int flag)
{
    return decaWorkP->flag & flag;
}

void DecaUnkFlagSet(unsigned int flag)
{
    decaModeWorkP->unkFlag |= flag;
}

void DecaUnkFlagReset(unsigned int flag)
{
    decaModeWorkP->unkFlag &= ~flag;
}

unsigned int DecaUnkFlagGet(unsigned int flag)
{
    return decaModeWorkP->unkFlag & flag;
}

void DecaMesFlagSet(unsigned int flag)
{
    decaWorkP->mesFlag |= flag;
}

void DecaMesFlagReset(unsigned int flag)
{
    decaWorkP->mesFlag &= ~flag;
}

unsigned int DecaMesFlagGet(unsigned int flag)
{
    return decaWorkP->mesFlag & flag;
}

u32 DecaMgNameMesGet(void)
{
    return mgDataTbl[decaMgTbl[decaModeWorkP->mgNo].no].nameMes;
}

int DecaMgNoGet(void)
{
    return decaModeWorkP->mgNo;
}

int DecaScoreMaxGet(void)
{
    int score[GW_PLAYER_MAX];
    int order[GW_PLAYER_MAX];
    int i;
    for(i=GW_PLAYER_MAX; i--;) {
        score[i] = GwMgDecaScore[i].finalScore;
        order[i] = i;
    }
    DecaSortInt(score, order, GW_PLAYER_MAX);
    return score[GW_PLAYER_MAX-1];
}

int DecaPlayerOrderGet(int *result)
{
    int score[GW_PLAYER_MAX];
    int order[GW_PLAYER_MAX];
    int i;
    int num;
    for(i=GW_PLAYER_MAX; i--;) {
        score[i] = GwMgDecaScore[i].finalScore;
        order[i] = i;
    }
    DecaSortInt(score, order, GW_PLAYER_MAX);
    num = 0;
    result[0] = order[GW_PLAYER_MAX-1];
    num++;
    for(i=GW_PLAYER_MAX-1; i--;) {
        if(score[i] != score[i+1]) {
            break;
        }
        result[num] = order[i];
        num++;
    }
    return num;
}

int DecaPlayerRankGet(int no, int *tieNum, int *outScore)
{
    int score[GW_PLAYER_MAX];
    int order[GW_PLAYER_MAX];
    int i;
    int player;
    int playerScore;
    for(i=GW_PLAYER_MAX; i--;) {
        score[i] = GwMgDecaScore[i].finalScore;
        order[i] = i;
    }
    DecaSortInt(score, order, GW_PLAYER_MAX);
    playerScore = score[(GW_PLAYER_MAX-1)-no];
    if(tieNum) {
        player = no;
        *tieNum = no;
        while(player--) {
            if(playerScore != score[(GW_PLAYER_MAX-1)-player]) {
                break;
            }
            *tieNum = player;
        }
    }
    if(outScore) {
        *outScore = playerScore;
    }
    return order[(GW_PLAYER_MAX-1)-no];
}

int DecaGameEmotionGet(void)
{
    int i;
    int score[GW_PLAYER_MAX];
    int rank[GW_PLAYER_MAX];
    s8 emotionNum[8];
    int scoreDist;
    int score23Dist;
    
    for(i=GW_PLAYER_MAX; i--;) {
        rank[i] = DecaPlayerRankGet(i, NULL, &score[i]);
    }
    for(i=8; i--;) {
        emotionNum[i] = 0;
    }
    switch(decaModeWorkP->mgNo) {
        case 9:
            emotionNum[4]--;
        
        case 8:
            emotionNum[3]--;
        
        case 7:
            emotionNum[6]--;
            emotionNum[2]--;
        
        case 6:
            emotionNum[1]--;
        
        case 5:
            emotionNum[3]++;
        
        case 4:
        case 3:
        case 2:
            emotionNum[5]++;
        
        case 1:
            emotionNum[4]++;
            emotionNum[0]++;
            emotionNum[2]++;
        
        case 0:
            emotionNum[6]++;
            emotionNum[1]++;
            break;
    }
    scoreDist = (score[0]-score[1])+(score[1]-score[2])+(score[2]-score[3]);
    score23Dist = score[1]-score[2];
    if(decaModeWorkP->gameEmotion != 6 && emotionNum[6] && score[0] < ((decaModeWorkP->mgNo+1)*500)) {
        decaModeWorkP->gameEmotion = 6;
    } else if(decaModeWorkP->gameEmotion != 0 && emotionNum[0] && (score[0]-score[1]) > 800) {
        decaModeWorkP->gameEmotion = 0;
    } else if(decaModeWorkP->gameEmotion != 2 && emotionNum[2] && (score[2]-score[3]) > ((decaModeWorkP->mgNo+1)*200)) {
        decaModeWorkP->gameEmotion = 2;
    } else if(scoreDist && decaModeWorkP->gameEmotion != 1 && emotionNum[1] && scoreDist < ((decaModeWorkP->mgNo+1)*100)) {
        decaModeWorkP->gameEmotion = 1;
    } else if(scoreDist && decaModeWorkP->gameEmotion != 3 && emotionNum[3] && scoreDist < ((decaModeWorkP->mgNo+1)*100)) {
        decaModeWorkP->gameEmotion = 3;
    } else if(score23Dist && decaModeWorkP->gameEmotion != 5 && emotionNum[5] && (score[0]-score[1]) < 100) {
        decaModeWorkP->gameEmotion = 5;
    } else if(decaModeWorkP->gameEmotion != 4 && emotionNum[4] && scoreDist > ((decaModeWorkP->mgNo+1)*200)) {
        decaModeWorkP->gameEmotion = 4;
    } else {
        decaModeWorkP->gameEmotion = 7;
    }
    return decaModeWorkP->gameEmotion;
}

int DecaMgHighScoreGet(void)
{
    return GwCommon.decaHighScore[decaModeWorkP->mgNo];
}

int DecaMgScoreGet(int playerNo)
{
    return GwPlayer[playerNo].mgScore;
}

int DecaScoreGet(int playerNo)
{
    return MgDecaScoreCalc(decaModeWorkP->mgNo, DecaMgScoreGet(playerNo));
}

void DecaMotionStartEndSet(int mdlId, float start, float end)
{
    Hu3DMotionStartEndSet(mdlId, start, end);
    Hu3DMotionTimeSet(mdlId, start);
    Hu3DMotionShapeTimeSet(mdlId, start);
}

void DecaOvlEvtnoSet(int evtno)
{
    OMOVLHIS *his = omOvlHisGet(0);
    omOvlHisChg(0, his->ovl, evtno, his->stat);
}

void DecaCharSoundPlay(DECAWORK_PLAYER *player, int seNo)
{
    HuAudFXPlayPan(charSoundTbl[player->charNo][seNo], (int)(32*(player->pos.x/400))+64);
}

void DecaProcessCreate(void (*func)(void))
{
    HuPrcChildCreate(func, 100, 4096, 0, HuPrcCurrentGet());
    HuPrcVSleep();
}

static void DecaCameraPath(void)
{
    CamMotionExPath(HU3D_CAM4, decaWorkP->camMot, decaWorkP->camMotNum);
    DecaFlagSet(DECAFLAG_CAMPATH_DONE);
    HuPrcEnd();
}

void DecaCameraPathStart(void)
{
    DecaFlagReset(DECAFLAG_CAMPATH_DONE);
    DecaProcessCreate(DecaCameraPath);
}

static void DecaMgExitMain(void)
{
    u32 tick;
    int statId;
    decaWorkP->dirRead = FALSE;
    tick = OSGetTick();
    statId = HuDataDirReadAsync(mgDataTbl[decaWorkP->mgNoDir].dataDir);
    while(!HuDataGetAsyncStat(statId)) {
        HuPrcVSleep();
    }
    decaWorkP->dirRead = TRUE;
    OSReport("Load Time:%d\n", OSTicksToMilliseconds(OSGetTick()-tick));
    OSReport("Finished!\n");
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

void DecaMgExit(int mgNo)
{
    decaWorkP->mgNoDir = mgNo;
    HuDataDirClose(DATA_decathlon);
    CharModelDataClose(CHARNO_NONE);
    if(!DecaFlagGet(DECAFLAG_INSTEXIT)) {
        HuPrcChildCreate(DecaMgExitMain, 200, 12288, 0, HuPrcCurrentGet());
    } else {
        decaWorkP->dirRead = TRUE;
    }
}

static void DecaCameraMotion(void)
{
    if(decaWorkP->cameraMotPrev >= 0) {
        Hu3DCameraMotionOff(decaWorkP->stageMdlId[decaWorkP->cameraMotPrev]);
        Hu3DModelAttrSet(decaWorkP->stageMdlId[decaWorkP->cameraMotPrev], HU3D_MOTATTR_PAUSE);
    }
    Hu3DModelAttrReset(decaWorkP->stageMdlId[decaWorkP->cameraMot], HU3D_MOTATTR_PAUSE);
    Hu3DCameraMotionOn(decaWorkP->stageMdlId[decaWorkP->cameraMot], HU3D_CAM4);
    Hu3DCameraMotionStart(decaWorkP->stageMdlId[decaWorkP->cameraMot], HU3D_CAM4);
    decaWorkP->cameraMotPrev = decaWorkP->cameraMot;
    DecaSleep(Hu3DMotionMaxTimeGet(decaWorkP->stageMdlId[decaWorkP->cameraMot]));
    DecaFlagSet(DECAFLAG_CAMPATH_DONE);
    HuPrcEnd();
}

void DecaCameraMotionStart(int cameraMot)
{
    DecaFlagReset(DECAFLAG_CAMPATH_DONE);
    if(decaWorkP->cameraMotPrev == cameraMot) {
        DecaFlagSet(DECAFLAG_CAMPATH_DONE);
    } else {
        decaWorkP->cameraMot = cameraMot;
        if(cameraMot < 0) {
            if(decaWorkP->cameraMotPrev >= 0) {
                Hu3DCameraMotionOff(decaWorkP->stageMdlId[decaWorkP->cameraMotPrev]);
                Hu3DModelAttrSet(decaWorkP->stageMdlId[decaWorkP->cameraMotPrev], HU3D_MOTATTR_PAUSE);
            }
            decaWorkP->cameraMotPrev = decaWorkP->cameraMot;
            DecaFlagSet(DECAFLAG_CAMPATH_DONE);
        } else {
            DecaFlagReset(DECAFLAG_CAMPATH_DONE);
            DecaProcessCreate(DecaCameraMotion);
        }
    }
}

void DecaWinMain(void)
{
    int seNo[8];
    int no;
    int seVol;
    int motionTime;
    
    for(no=0; no<8; no++) {
        seNo[no] = MSM_SENO_NONE;
    }
    motionTime = 0;
    seVol = 0;
    no = 0;
    HuPrcVSleep();
    while(1) {
        HuPrcVSleep();
        while(seVol != 15 && winSoundTbl[seVol][0] <= motionTime) {
            if(seNo[no] >= 0) {
                HuAudFXStop(seNo[no]);
            }
            seNo[no] = HuAudFXPlayPan(1251, winSoundTbl[seVol][1]);
            no++;
            no %= 8;
            seVol++;
        }
        motionTime = Hu3DMotionTimeGet(decaWorkP->stageMdlId[DECA_MDL_HANABI]);
        if(motionTime == 0) {
            seVol = 0;
        }
        OSReport("MotionTime %3d\t\tSE Vol %2d\n", motionTime, seVol);
    }
}

static void DecaWinMusicPlay(void)
{
    HuAudJinglePlay(MSM_STREAM_JNGL_MGCIRCUIT_WIN);
    DecaSleep(300);
    HuAudSStreamPauseFadeOut(decaWorkP->streamNo, FALSE, 1000);
    HuPrcEnd();
}

void DecaWinMusicStart(void)
{
    HuPrcChildCreate(DecaWinMusicPlay, 100, 4096, 0, HuPrcCurrentGet());
}

void ObjectSetup(vod)
{
    _SetFlag(FLAG_MGINST_ON);
    HuPrcChildCreate(DecaMain, 200, 12288, 0, HuPrcCurrentGet());
}

void DecaMain(void)
{
    BOOL mgF;
    BOOL runF;
    OMOVLHIS *his;
    his = omOvlHisGet(0);
    DecaInit();
    runF = TRUE;
    while(runF) {
        mgF = TRUE;
        switch(his->evtno) {
            case 0:
                DecaEvtOpening();
                runF = TRUE;
                break;
            
            case 1:
                if(!DecaEvtMgScore()) {
                    mgF = FALSE;
                }
                break;
                
            case 2:
                DecaEvtEnding();
                mgF = FALSE;
                break;
            
            case 3:
                if(DecaFlagGet(DECAFLAG_EXIT)) {
                    DecaEvtLast();
                }
                mgF = FALSE;
                runF = FALSE;
                break;
        }
        if(mgF) {
            DecaEvtMgNext();
        }
    }
    DecaExit();
    omOvlReturn(1);
    HuPrcEnd();
}

static void DecaStageMdlCreate(void)
{
    DECASTAGEMDL *mdl;
    int mdlId;
    int i;
    for(i=DECA_MDL_STAGE_NUM; i--;) {
        mdl = &stageMdlTbl[i];
        decaWorkP->stageMdlId[i] = mdlId = Hu3DModelCreateData(mdl->fileNum);
        Hu3DModelCameraSet(mdlId, mdl->cameraBit);
        Hu3DModelLayerSet(mdlId, mdl->layer);
        if(mdl->attr) {
            Hu3DModelAttrSet(mdlId, mdl->attr);
        }
        if(mdl->motAttr) {
            Hu3DModelAttrSet(mdlId, mdl->motAttr);
        }
    }
    DecaMotionStartEndSet(decaWorkP->stageMdlId[DECA_MDL_CROWD], 0, 360);
    DecaMotionStartEndSet(decaWorkP->stageMdlId[DECA_MDL_STAGE], 0, 999);
    {
        int motFile[DECA_MDL_CAMERAMOT_NUM] = {
            DECATHLON_HSF_camera_focus,
            DECATHLON_HSF_camera_init,
            DECATHLON_HSF_camera_end_start,
            DECATHLON_HSF_camera_end_exit,
            DECATHLON_HSF_camera_alt,
            DECATHLON_HSF_camera_mg1,
            DECATHLON_HSF_camera_mg2,
            DECATHLON_HSF_camera_mg3,
            DECATHLON_HSF_camera_mg4,
            DECATHLON_HSF_camera_mg5,
            DECATHLON_HSF_camera_mg6,
            DECATHLON_HSF_camera_mg7,
            DECATHLON_HSF_camera_mg8,
            DECATHLON_HSF_camera_mg9
        };
        for(i=DECA_MDL_CAMERAMOT_NUM; i--;) {
            decaWorkP->stageMdlId[i+DECA_MDL_CAMERAMOT_START] = mdlId = Hu3DModelCameraCreate(Hu3DMotionCreateData(motFile[i]), HU3D_CAM4);
            Hu3DModelAttrSet(mdlId, HU3D_MOTATTR_PAUSE);
            Hu3DCameraMotionOff(mdlId);
        }
        mdlId = Hu3DModelCreateData(DECATHLON_HSF_main_light);
        decaWorkP->stageMdlId[DECA_MDL_LIGHT_MAIN] = mdlId;
        Hu3DModelAttrSet(mdlId, HU3D_MOTATTR_PAUSE);
        Hu3DModelPosSet(mdlId, 0, -100, 0);
        Hu3DModelRotSet(mdlId, 90, 0, 0);
    }
}

static void DecaMgScoreDisp(BOOL emotionF)
{
    HuVecF pos, up, target;
    float fov, dist;
    CAMMOTIONWORK *camMot;
    DECAWORK_PLAYER *player;
    int i;
    int lightMdlId;
    
    DecaCameraMotionStart(DECA_MDL_CAMERAMOT_NULL);
    Hu3DCameraPosGet(HU3D_CAM4, &pos, &up, &target);
    Hu3DCameraPerspectiveGet(HU3D_CAM4, &fov, &dist, &dist);
    camMot = &decaWorkP->camMot[0];
    decaWorkP->camMotNum = 0;
    camMot->pos = pos;
    camMot->up = up;
    camMot->target = target;
    camMot->fov = fov;
    camMot->time = 0;
    decaWorkP->camMotNum++;
    camMot++;
    camMot->pos = decaModeWorkP->endCamPos;
    camMot->up = decaModeWorkP->endCamUp;
    camMot->target = decaModeWorkP->endCamTarget;
    camMot->fov = decaModeWorkP->endCamFov;
    camMot->time = 60;
    decaWorkP->camMotNum++;
    camMot++;
    DecaCameraPathStart();
    Hu3DModelDispOn(decaWorkP->stageMdlId[DECA_MDL_COVER]);
    Hu3DModelAttrReset(decaWorkP->stageMdlId[DECA_MDL_COVER], HU3D_MOTATTR_PAUSE);
    DecaMotionStartEndSet(decaWorkP->stageMdlId[DECA_MDL_COVER], 0, 29);
    DecaWinClose();
    while(!DecaFlagGet(DECAFLAG_CAMPATH_DONE)) {
        HuPrcVSleep();
    }
    DecaScoreModeSet(DECA_SCOREMODE_MGSCORE);
    DecaSleep(120);
    DecaScoreModeSet(DECA_SCOREMODE_SCORE);
    DecaSleep(120);
    DecaScoreModeSet(DECA_SCOREMODE_ADD_BEGIN);
    while(!DecaScoreIdleCheck()) {
        HuPrcVSleep();
    }
    DecaSleep(120);
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        GwMgDecaScore[i].mgScore[decaModeWorkP->mgNo] = DecaMgScoreGet(player->no);
        GwMgDecaScore[i].finalScore += DecaScoreGet(player->no);
    }
    DecaScoreModeSet(DECA_SCOREMODE_NEWRANK_BEGIN);
    while(!DecaScoreIdleCheck()) {
        HuPrcVSleep();
    }
    if(emotionF) {
        DecaProcessCreate(DecaMessGameEmotion);
        while(!DecaMesFlagGet(DECA_MESSFLAG_EMOTION)) {
            HuPrcVSleep();
        }
        HuAudSStreamPauseFadeOut(decaWorkP->streamNo, TRUE, 1000);
    } else {
        DecaProcessCreate(DecaMessBtnWait);
        while(!DecaMesFlagGet(DECA_MESSFLAG_BTNWAIT)) {
            HuPrcVSleep();
        }
        DecaSleep(120);
        for(i=4; i--;) {
            lightMdlId = decaWorkP->stageMdlId[DECA_MDL_LIGHT1+i];
            Hu3DModelDispOff(lightMdlId);
            Hu3DModelAttrSet(lightMdlId, HU3D_MOTATTR_PAUSE);
        }
        for(player=&decaWorkP->player[0], i=GW_PLAYER_MAX; i--; player++) {
            if(player->mot != 0) {
                CharMotionShiftSet(player->charNo, player->motId[0], 0, 5, HU3D_MOTATTR_LOOP);
                player->mot = 0;
            }
        }
    }
    DecaScoreModeSet(DECA_SCOREMODE_HIDE);
    camMot = &decaWorkP->camMot[0];
    decaWorkP->camMotNum = 0;
    camMot->pos = decaModeWorkP->endCamPos;
    camMot->up = decaModeWorkP->endCamUp;
    camMot->target = decaModeWorkP->endCamTarget;
    camMot->fov = decaModeWorkP->endCamFov;
    camMot->time = 0;
    decaWorkP->camMotNum++;
    camMot++;
    camMot->pos = pos;
    camMot->up = up;
    camMot->target = target;
    camMot->fov = fov;
    camMot->time = 60;
    decaWorkP->camMotNum++;
    camMot++;
    DecaCameraPathStart();
    Hu3DModelDispOn(decaWorkP->stageMdlId[DECA_MDL_COVER]);
    Hu3DModelAttrReset(decaWorkP->stageMdlId[DECA_MDL_COVER], HU3D_MOTATTR_PAUSE);
    DecaMotionStartEndSet(decaWorkP->stageMdlId[DECA_MDL_COVER], 30, 60);
    DecaWinClose();
    while(!DecaFlagGet(DECAFLAG_CAMPATH_DONE)) {
        HuPrcVSleep();
    }
}

void DecaRecordSave(void)
{
    GWDECASCORE scores[10+GW_PLAYER_MAX];
    int recordOrder[10+GW_PLAYER_MAX];
    int finalScore[10+GW_PLAYER_MAX];
    int i;
    int j;
    int playerNum;
    for(i=10; i--;) {
        scores[i] = GwCommon.decaScore[i];
        finalScore[i] = GwCommon.decaScore[i].finalScore;
        recordOrder[i] = i;
    }
    for(playerNum=0, i=0; i<GW_PLAYER_MAX; i++) {
        if(GwPlayerConf[i].type == GW_TYPE_MAN) {
            scores[playerNum+10] = GwMgDecaScore[i];
            finalScore[playerNum+10] = GwMgDecaScore[i].finalScore;
            recordOrder[playerNum+10] = playerNum+10;
            playerNum++;
        }
    }
    for(i=10; i--;) {
        if(decaMgTbl[i].saveLessF) {
            for(j=GW_PLAYER_MAX; j--;) {
                if(GwPlayerConf[j].type == GW_TYPE_MAN && GwCommon.decaHighScore[i] < GwMgDecaScore[j].mgScore[i]) {
                    GwCommon.decaHighScore[i] = GwMgDecaScore[j].mgScore[i];
                }
            }
        } else {
            for(j=GW_PLAYER_MAX; j--;) {
                if(GwMgDecaScore[j].mgScore[i] != 0 && GwPlayerConf[j].type == GW_TYPE_MAN && GwCommon.decaHighScore[i] > GwMgDecaScore[j].mgScore[i]) {
                    GwCommon.decaHighScore[i] = GwMgDecaScore[j].mgScore[i];
                }
            }
        }
    }
    DecaSortInt(finalScore, recordOrder, playerNum+10);
    for(i=0; i<10; i++) {
        GwCommon.decaScore[i] = scores[recordOrder[(playerNum+10-1)-i]];
    }
}

void DecaStageReset(void)
{
    int i;
    int lightMdlId;
    decaModeWorkP->unkFlag = 0;
    decaModeWorkP->gameEmotion = -1;
    decaWorkP->winId = 0;
    decaWorkP->mesFlag = 0;
    decaWorkP->flag = 0;
    decaWorkP->playerRotMode = 0;
    decaWorkP->playerRotSpeed = 2;
    Hu3DModelDispOff(decaWorkP->stageMdlId[DECA_MDL_MOON]);
    Hu3DModelAttrSet(decaWorkP->stageMdlId[DECA_MDL_MOON], HU3D_MOTATTR_PAUSE);
    Hu3DModelDispOff(decaWorkP->stageMdlId[DECA_MDL_HANABI]);
    Hu3DModelAttrSet(decaWorkP->stageMdlId[DECA_MDL_HANABI], HU3D_MOTATTR_PAUSE);
    for(i=4; i--;) {
        lightMdlId = decaWorkP->stageMdlId[DECA_MDL_LIGHT1+i];
        Hu3DModelDispOff(lightMdlId);
        Hu3DModelAttrSet(lightMdlId, HU3D_MOTATTR_PAUSE);
    }
    DecaCameraMotionStart(DECA_MDL_CAMERAMOT_NULL);
}

static void DecaPlayerRotate(void)
{
    HuVecF dir;
    HuVecF aimPos;
    DECAWORK_PLAYER *player;
    int i;
    int doneNum;
    float rotEnd;
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        switch(decaWorkP->playerRotMode) {
            case 0:
                player->rotEnd = 0;
                break;
            
            case 2:
                aimPos = decaWorkP->guidePos;
                aimPos.z += 350;
                VECSubtract(&aimPos, &player->pos, &aimPos);
                aimPos.y = 0;
                VECNormalize(&aimPos, &dir);
                if(dir.z > 1) {
                    dir.z = 1;
                } else if(dir.z < -1) {
                    dir.z = -1;
                }
                rotEnd = acosf(dir.z);
                if(dir.x < 0) {
                    rotEnd = -rotEnd;
                }
                player->rotEnd = (180*rotEnd)/M_PI;
                break;
            
            case 1:
                aimPos = openingAimPos;
                VECSubtract(&aimPos, &player->pos, &aimPos);
                aimPos.y = 0;
                VECNormalize(&aimPos, &dir);
                if(dir.z > 1) {
                    dir.z = 1;
                } else if(dir.z < -1) {
                    dir.z = -1;
                }
                rotEnd = acosf(dir.z);
                if(dir.x < 0) {
                    rotEnd = -rotEnd;
                }
                player->rotEnd = (180*rotEnd)/M_PI;
                break;
        }
        if(!DecaAngleSub(&player->rot, player->rotEnd, 0.01f)) {
            if(player->mot != 1) {
                CharMotionShiftSet(player->charNo, player->motId[1], 0, 5, HU3D_MOTATTR_LOOP);
                player->mot = 1;
            }
        }
    }
    while(1) {
        doneNum = 0;
        for(player=&decaWorkP->player[0], i=GW_PLAYER_MAX; i--; player++) {
            if(DecaAngleSub(&player->rot, player->rotEnd, decaWorkP->playerRotSpeed)) {
                if(player->mot != 0) {
                    CharMotionShiftSet(player->charNo, player->motId[0], 0, 5, HU3D_MOTATTR_LOOP);
                    player->mot = 0;
                }
                doneNum++;
            }
            Hu3DModelRotSet(player->mdlId, 0, player->rot, 0);
        }
        if(doneNum == 4) {
            break;
        }
        HuPrcVSleep();
    }
    DecaFlagSet(DECAFLAG_PLAYERROT_DONE);
    HuPrcEnd();
}

void DecaPlayerRotateStart(int mode, float speed)
{
    DecaFlagReset(DECAFLAG_PLAYERROT_DONE);
    decaWorkP->playerRotMode = mode;
    decaWorkP->playerRotSpeed = speed;
    DecaProcessCreate(DecaPlayerRotate);
}

static void FbCopyHook(s16 layerNo)
{
    if(Hu3DCameraBit == HU3D_CAM1) {
        Hu3DFbCopyExec(decaWorkP->fbCopyX, decaWorkP->fbCopyY, decaWorkP->fbCopyW, decaWorkP->fbCopyH,
            GX_TF_RGB5A3, FALSE, decaWorkP->fbCopyAnim->bmp->data);
    }
}

void DecaFbCopyCreate(void)
{
    Hu3DCameraCreate(HU3D_CAM1);
    Hu3DCameraPerspectiveSet(HU3D_CAM1, 20, 20, 20000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM1, 0, 0, 640, 480, 0, 1);
    Hu3DCameraScissorSet(HU3D_CAM1, 0, 0, 640, 480);
    decaWorkP->fbCopyX = 192;
    decaWorkP->fbCopyY = 100;
    decaWorkP->fbCopyW = 256;
    decaWorkP->fbCopyH = 256;
    Hu3DZClearLayerSet(7+HU3D_LAYER_HOOK_POST);
    if(!decaWorkP->fbCopyAnim) {
        decaWorkP->fbCopyAnim = HuSprAnimMake(256, 256, ANIM_BMP_RGB5A3);
        decaWorkP->fbCopyAnim->bmp->data = HuMemDirectMallocNum(HEAP_MODEL, decaWorkP->fbCopyAnim->bmp->dataSize, HU_MEMNUM_OVL);
        memset(decaWorkP->fbCopyAnim->bmp->data, 0, decaWorkP->fbCopyAnim->bmp->dataSize);
    }
    Hu3DLayerHookSet(6, FbCopyHook);
    Hu3DModelDispOn(decaWorkP->stageMdlId[DECA_MDL_STAGE_FB]);
    decaWorkP->guidePos = guidePosTbl[2];
    Hu3DModelDispOn(decaWorkP->guideMdlId);
    Hu3DModelPosSetV(decaWorkP->guideMdlId, &decaWorkP->guidePos);
    Hu3DModelRotSet(decaWorkP->guideMdlId, 0, 0, 0);
    Hu3DModelCameraSet(decaWorkP->guideMdlId, HU3D_CAM1);
    Hu3DCameraPosSet(HU3D_CAM1, decaWorkP->guidePos.x, 180+decaWorkP->guidePos.y, decaWorkP->guidePos.z+2000, 0, 1, 0, decaWorkP->guidePos.x, decaWorkP->guidePos.y, decaWorkP->guidePos.z);
    HuPrcVSleep();
    Hu3DAnimCreate(decaWorkP->fbCopyAnim, decaWorkP->stageMdlId[DECA_MDL_STAGE], "eizou06");
    DecaMotionStartEndSet(decaWorkP->stageMdlId[DECA_MDL_STAGE], 1376, 1735);
}

static void DecaEnterSoundFade(void)
{
    int time;
    int maxTime = 390;
    DecaSleep(30);
    for(time=0; time<=maxTime; time++) {
        float vol = (float)time/(float)maxTime;
        vol *= 127;
        HuAudFXVolSet(decaWorkP->seNo, (s16)vol);
        HuPrcVSleep();
    }
    HuPrcEnd();
}

static void DecaEnterEffect(void)
{
    Hu3DModelAttrReset(decaWorkP->stageMdlId[DECA_MDL_CROWD], HU3D_MOTATTR_LOOP);
    DecaMotionStartEndSet(decaWorkP->stageMdlId[DECA_MDL_CROWD], 361, 480);
    if(decaWorkP->seNo >= 0) {
        if(DecaFlagGet(DECAFLAG_FADE_ENTER)) {
            HuAudFXVolSet(decaWorkP->seNo, 0);
        }
        HuAudFXStop(decaWorkP->seNo);
    }
    decaWorkP->seNo = HuAudFXPlay(MSM_SE_INST_26);
    if(DecaFlagGet(DECAFLAG_FADE_ENTER)) {
        HuAudFXVolSet(decaWorkP->seNo, 0);
        HuPrcChildCreate(DecaEnterSoundFade, 200, 4096, 0, HuPrcCurrentGet());
        DecaFlagReset(DECAFLAG_FADE_ENTER);
    }
    while(1) {
        do {
            HuPrcVSleep();
        } while(!Hu3DMotionEndCheck(decaWorkP->stageMdlId[DECA_MDL_CROWD]));
        decaWorkP->enterTimer--;
        if(decaWorkP->enterTimer == 0) {
            break;
        }
        Hu3DModelAttrReset(decaWorkP->stageMdlId[DECA_MDL_CROWD], HU3D_MOTATTR_PAUSE);
        DecaMotionStartEndSet(decaWorkP->stageMdlId[DECA_MDL_CROWD], 361, 480);
        HuPrcVSleep();
    }
    HuPrcVSleep();
    HuAudFXStop(decaWorkP->seNo);
    decaWorkP->seNo = HuAudFXPlay(MSM_SE_INST_25);
    Hu3DModelAttrSet(decaWorkP->stageMdlId[DECA_MDL_CROWD], HU3D_MOTATTR_LOOP);
    DecaMotionStartEndSet(decaWorkP->stageMdlId[DECA_MDL_CROWD], 0, 360);
    decaWorkP->enterProc = NULL;
    HuPrcEnd();
}

void DecaEnterEffectStart(int enterTimer)
{
    decaWorkP->enterTimer = enterTimer;
    decaWorkP->enterProc = HuPrcChildCreate(DecaEnterEffect, 100, 4096, 0, HuPrcCurrentGet());
}

void DecaInit(void)
{
    DECAWORK_PLAYER *player;
    int i;
    HU3DMODELID guideMdlId;
    int mgNo;
    HU3DMOTID guideMotId;
    
    OMOVLHIS *his;
    his = omOvlHisGet(0);
    decaModeWorkP = (void *)mgModeWork;
    decaWorkP = &decaWork;
    {
        HuVecF shadowPos = { 0, 5000, -1920 };
        HuVecF shadowUp = { 0, 0, 1 };
        HuVecF shadowTarget = { 0, 0, -1920 };
        HuVecF altDown = { 0, -1, 0 };
        HuVecF tempPos = { 0, 50, 4000 };
        HuVecF altBack = { 0, 0, -1 };
        decaWorkP->objman = omInitObjMan(200, 1000);
        Hu3DCameraCreate(HU3D_CAM4);
        Hu3DCameraPerspectiveSet(HU3D_CAM4, 60, 20, 60000, 1.2f);
        Hu3DCameraViewportSet(HU3D_CAM4, 0, 0, 640, 480, 0, 1);
        Hu3DCameraScissorSet(HU3D_CAM4, 0, 0, 640, 480);
        Hu3DShadowCreate(15, 20, 15000);
        Hu3DShadowTPLvlSet(0.8f);
        Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
        Hu3DShadowSizeSet(64);
    }
    for(i=0; i<10; i++) {
        for(mgNo=0; mgDataTbl[mgNo].ovl != decaMgTbl[i].ovl; mgNo++) {
            if(mgDataTbl[mgNo].ovl == (u16)DLL_NONE) {
                OSReport("mgtournament.c : ＤＬＬが見つかりません！！\n");
                abort();
            }
        }
        decaMgTbl[i].no = mgNo;
    }
    Hu3DParManInit();
    decaWorkP->cameraMotPrev = DECA_MDL_CAMERAMOT_NULL;
    decaWorkP->winId = 0;
    decaWorkP->mesFlag = 0;
    decaWorkP->flag = 0;
    decaWorkP->playerRotMode = 0;
    decaWorkP->playerRotSpeed = 2;
    decaWorkP->fbCopyAnim = NULL;
    decaWorkP->seNo = MSM_SENO_NONE;
    decaWorkP->streamNo = MSM_STREAMNO_NONE;
    {
        HuVec2F winSize;
        
        HuWinInit(1);
        HuWinMesMaxSizeBetGet(&winSize, MESS_MG_DECATHLON_WELCOME, MESS_MG_DECATHLON_INST);
        decaWorkP->winId = HuWinExCreateFrame(-10000, 360, winSize.x, winSize.y, HUWIN_SPEAKER_NONE, 1);
        HuWinMesPalSet(decaWorkP->winId, 7, 0, 0, 0);
    }
    DecaStageMdlCreate();
    DecaScoreInit();
    if(his->evtno == 0) {
        _SetFlag(FLAG_INST_NOHISCHG);
        _SetFlag(FLAG_INST_MGMODE);
        _SetFlag(FLAG_INST_DECA);
        decaModeWorkP->gameEmotion = -1;
    }
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        player->no = i;
        player->charNo = GwPlayerConf[player->no].charNo;
        player->padNo = GwPlayerConf[player->no].padNo;
        player->mdlId = CharModelMotListCreate(player->charNo, CHAR_MODEL1, playerMotList, player->motId);
        player->mot = -1;
        player->pos = playerPosTbl[i];
        player->rot = 0;
        player->rotEnd = 0;
        CharMotionVoiceOnSet(player->charNo, playerMotList[3], FALSE);
        if(player->mot != 0) {
            CharMotionShiftSet(player->charNo, player->motId[0], 0, 5, HU3D_MOTATTR_LOOP);
            player->mot = 0;
        }
        Hu3DModelDispOff(player->mdlId);
        Hu3DModelPosSet(player->mdlId, player->pos.x, player->pos.y, player->pos.z);
        Hu3DModelLayerSet(player->mdlId, 4);
        Hu3DModelCameraSet(player->mdlId, HU3D_CAM4);
        decaWorkP->stageMdlId[i+DECA_MDL_PLAYER_START] = player->mdlId;
    }
    {
        HuVecF guidePos = guidePosTbl[0];
        guideMdlId = Hu3DModelCreateData(DECATHLON_HSF_nerl);
        decaWorkP->guideMdlId = guideMdlId;
        decaWorkP->guidePos = guidePos;
        guideMotId = Hu3DJointMotionData(guideMdlId, DECATHLON_HSF_nerl_idle);
        Hu3DMotionSet(guideMdlId, guideMotId);
        Hu3DModelAttrSet(guideMdlId, HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(guideMdlId, 4);
        Hu3DModelCameraSet(guideMdlId, HU3D_CAM4|HU3D_CAM1);
        Hu3DModelPosSet(guideMdlId, guidePos.x, guidePos.y, guidePos.z);
    }
    for(i=DECA_MDL_MAX; i--;) {
        Hu3DCameraMotionOff(decaWorkP->stageMdlId[i]);
    }
    Hu3DModelShadowMapObjSet(decaWorkP->stageMdlId[DECA_MDL_STAGE_NIGHT], "bmerge31");
    for(i=0; i<GW_PLAYER_MAX; i++) {
        Hu3DModelShadowSet(decaWorkP->player[i].mdlId);
    }
    Hu3DModelShadowSet(decaWorkP->guideMdlId);
}

void DecaExit(void)
{
    HuDataDirClose(DATA_decathlon);
    CharModelDataClose(CHARNO_NONE);
    GWMgInstFSet(TRUE);
    if(decaWorkP->fbCopyAnim) {
        HuSprAnimKill(decaWorkP->fbCopyAnim);
        decaWorkP->fbCopyAnim = NULL;
    }
    DecaOvlEvtnoSet(0);
    _ClearFlag(FLAG_INST_NOHISCHG);
    _ClearFlag(FLAG_INST_MGMODE);
    _ClearFlag(FLAG_INST_DECA);
    memset(decaModeWorkP, 0, sizeof(DECAMODEWORK));
}

void DecaEvtOpening(void)
{
    DECAWORK_PLAYER *player;
    int i;
    int mdlId;
    
    CAMMOTIONWORK *camMot;
    int camMotNum;
    
    DecaOvlEvtnoSet(1);
    DecaFlagReset(0);
    decaWorkP->mesFlag = 0;
    DecaFlagSet(DECAFLAG_MUS_FADEOUT|DECAFLAG_FADE_ENTER);
    DecaScoreStart();
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        player->pos = playerPosTbl[i];
        if(player->mot != 0) {
            CharMotionShiftSet(player->charNo, player->motId[0], 0, 5, HU3D_MOTATTR_LOOP);
            player->mot = 0;
        }
        Hu3DModelDispOff(player->mdlId);
        Hu3DModelPosSet(player->mdlId, player->pos.x, player->pos.y, player->pos.z);
        Hu3DModelLayerSet(player->mdlId, 4);
        Hu3DModelCameraSet(player->mdlId, HU3D_CAM4);
        GwMgDecaScore[i].finalScore = 0;
    }
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        GwMgDecaScore[i].charNo = player->charNo;
    }
    decaModeWorkP->mgNo = 0;
    decaWorkP->guidePos = guidePosTbl[2];
    Hu3DModelDispOn(decaWorkP->guideMdlId);
    Hu3DModelPosSetV(decaWorkP->guideMdlId, &decaWorkP->guidePos);
    Hu3DModelRotSet(decaWorkP->guideMdlId, 0, 0, 0);
    Hu3DModelCameraSet(decaWorkP->guideMdlId, HU3D_CAM1);
    Hu3DMotionTimeSet(decaWorkP->stageMdlId[DECA_MDL_STAGE_NIGHT], decaModeWorkP->mgNo);
    Hu3DMotionTimeSet(decaWorkP->stageMdlId[DECA_MDL_LIGHT_MAIN], decaModeWorkP->mgNo);
    Hu3DModelLightInfoSet(decaWorkP->stageMdlId[DECA_MDL_LIGHT_MAIN], TRUE);
    Hu3DModelDispOn(decaWorkP->stageMdlId[DECA_MDL_HIKARI]);
    Hu3DModelAttrReset(decaWorkP->stageMdlId[DECA_MDL_HIKARI], HU3D_MOTATTR_PAUSE);
    mdlId = decaWorkP->stageMdlId[DECA_MDL_KIKYUU];
    Hu3DModelDispOn(mdlId);
    mdlId = decaWorkP->stageMdlId[DECA_MDL_FUUSEN];
    Hu3DModelDispOn(mdlId);
    decaWorkP->seNo = HuAudFXPlay(MSM_SE_INST_25);
    DecaEnterEffectStart(5);
    OSReport("%d\n", MgDecaScoreCalc(decaModeWorkP->mgNo, 1410));
    camMot = decaWorkP->camMot;
    camMotNum = 29;
    camMot = decaWorkP->camMot;
    decaWorkP->camMotNum = CamMotionExPathGet(decaWorkP->stageMdlId[DECA_MDL_CAMERAMOT_INIT], 1, 60, 0, &camMot, &camMotNum);
    camMot = &decaWorkP->camMot[decaWorkP->camMotNum-1];
    decaModeWorkP->endCamPos = camMot->pos;
    decaModeWorkP->endCamUp = camMot->up;
    decaModeWorkP->endCamTarget = camMot->target;
    decaModeWorkP->endCamFov = camMot->fov;
    camMot = &decaWorkP->camMot[0];
    decaWorkP->camMotNum = CamMotionExPathGet(decaWorkP->stageMdlId[DECA_MDL_CAMERAMOT_FOCUS], 18, 480, 0, &camMot, &camMotNum);
    camMot = &decaWorkP->camMot[decaWorkP->camMotNum-1];
    decaModeWorkP->startCamPos = camMot->pos;
    decaModeWorkP->startCamUp = camMot->up;
    decaModeWorkP->startCamTarget = camMot->target;
    decaModeWorkP->startCamFov = camMot->fov;
    HuAudJinglePlay(MSM_STREAM_JNGL_MGDECA_START);
    DecaCameraMotionStart(DECA_MDL_CAMERAMOT_NULL);
    DecaCameraMotionStart(DECA_MDL_CAMERAMOT_FOCUS);
    DecaWipeIn(WIPE_TYPE_PREVTYPE, 60);
    DecaSleep(120);
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        Hu3DModelDispOn(player->mdlId);
    }
    while(!DecaFlagGet(DECAFLAG_CAMPATH_DONE)) {
        HuPrcVSleep();
    }
    DecaFbCopyCreate();
    DecaPlayerRotateStart(1, 2);
    while(!DecaFlagGet(DECAFLAG_PLAYERROT_DONE)) {
        HuPrcVSleep();
    }
    decaWorkP->streamNo = HuAudBGMPlay(MSM_STREAM_MGDECA);
    DecaProcessCreate(DecaMessInst);
    while(!DecaMesFlagGet(DECA_MESSFLAG_INST)) {
        HuPrcVSleep();
    }
    if(DecaFlagGet(0x4)) {
        
    }
    DecaProcessCreate(DecaMessBegin);
    while(!DecaMesFlagGet(DECA_MESSFLAG_BEGIN)) {
        HuPrcVSleep();
    }
    Hu3DCameraKill(HU3D_CAM1);
    DecaMotionStartEndSet(decaWorkP->stageMdlId[DECA_MDL_STAGE], 0, 999);
    if(decaWorkP->fbCopyAnim) {
        Hu3DLayerHookSet(6, NULL);
        HuSprAnimKill(decaWorkP->fbCopyAnim);
        decaWorkP->fbCopyAnim = NULL;
    }
}

BOOL DecaEvtMgScore(void)
{
    DECAWORK_PLAYER *player;
    int i;
    int tempNo;
    int no;
    int mdlId;
    no = decaModeWorkP->mgNo+1;
    if(no == 10) {
        DecaOvlEvtnoSet(2);
        return FALSE;
    }
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        player->pos = playerPosTbl[i+4];
        if(player->mot != 0) {
            CharMotionShiftSet(player->charNo, player->motId[0], 0, 5, HU3D_MOTATTR_LOOP);
            player->mot = 0;
        }
        Hu3DModelDispOn(player->mdlId);
        Hu3DModelPosSet(player->mdlId, player->pos.x, player->pos.y, player->pos.z);
        Hu3DModelLayerSet(player->mdlId, 4);
        Hu3DModelCameraSet(player->mdlId, HU3D_CAM4);
    }
    decaWorkP->guidePos = guidePosTbl[1];
    Hu3DModelPosSetV(decaWorkP->guideMdlId, &decaWorkP->guidePos);
    Hu3DModelRotSet(decaWorkP->guideMdlId, 0, 40, 0);
    Hu3DModelDispOn(decaWorkP->guideMdlId);
    Hu3DModelCameraSet(decaWorkP->guideMdlId, HU3D_CAM4);
    Hu3DMotionTimeSet(decaWorkP->stageMdlId[DECA_MDL_STAGE_NIGHT], no);
    Hu3DMotionTimeSet(decaWorkP->stageMdlId[DECA_MDL_LIGHT_MAIN], no);
    Hu3DModelLightInfoSet(decaWorkP->stageMdlId[DECA_MDL_LIGHT_MAIN], TRUE);
    mdlId = decaWorkP->stageMdlId[DECA_MDL_KIKYUU];
    Hu3DModelDispOn(mdlId);
    mdlId = decaWorkP->stageMdlId[DECA_MDL_FUUSEN];
    Hu3DModelDispOn(mdlId);
    DecaEnterEffectStart(2);
    DecaScoreStart();
    
    decaWorkP->seNo = HuAudFXPlay(MSM_SE_INST_25);
    decaWorkP->streamNo = HuAudBGMPlay(MSM_STREAM_MGDECA);
    OSReport("ただいまの時間 : %d\n", no);
    DecaCameraMotionStart(DECA_MDL_CAMERAMOT_NULL);
    DecaCameraMotionStart(DECA_MDL_CAMERAMOT_MG1+decaModeWorkP->mgNo);
    DecaWipeIn(WIPE_TYPE_PREVTYPE, 60);
    while(!DecaFlagGet(DECAFLAG_CAMPATH_DONE)) {
        HuPrcVSleep();
    }
    DecaProcessCreate(DecaMessResultStart);
    while(!DecaMesFlagGet(DECA_MESSFLAG_RESULTSTART)) {
        HuPrcVSleep();
    }
    DecaMgScoreDisp(TRUE);
    HuAudJinglePlay(MSM_STREAM_JNGL_MGWARS_MG_WIN);
    for(tempNo=0; tempNo<GW_PLAYER_MAX; tempNo++) {
        int tieNum;
        player = &decaWorkP->player[DecaPlayerRankGet(tempNo, &tieNum, NULL)];
        if(tieNum) {
            break;
        }
        DecaCharSoundPlay(player, 0);
        if(player->mot != 3) {
            CharMotionShiftSet(player->charNo, player->motId[3], 0, 5, HU3D_MOTATTR_NONE);
            player->mot = 3;
        }
    }
    DecaSleep(180);
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        if(player->mot != 0) {
            CharMotionShiftSet(player->charNo, player->motId[0], 0, 5, HU3D_MOTATTR_LOOP);
            player->mot = 0;
        }
    }
    decaModeWorkP->mgNo++;
    return TRUE;
}

void DecaEvtMgNext(void)
{
    
    CAMMOTIONWORK *camMot;
    DECAMG *mg = &decaMgTbl[decaModeWorkP->mgNo];
    int mgNo;
    if(!DecaFlagGet(DECAFLAG_MUS_FADEOUT)) {
        HuAudSStreamPauseFadeOut(decaWorkP->streamNo, FALSE, 1000);
    }
    DecaProcessCreate(DecaMessMgNo);
    while(!DecaMesFlagGet(DECA_MESSFLAG_MGNO)) {
        HuPrcVSleep();
    }
    DecaEnterEffectStart(1);
    camMot = &decaWorkP->camMot[0];
    DecaCameraMotionStart(DECA_MDL_CAMERAMOT_NULL);
    decaWorkP->camMotNum = 0;
    camMot->pos = decaModeWorkP->startCamPos;
    camMot->up = decaModeWorkP->startCamUp;
    camMot->target = decaModeWorkP->startCamTarget;
    camMot->fov = decaModeWorkP->startCamFov;
    camMot->time = 0;
    decaWorkP->camMotNum++;
    camMot++;
    camMot->pos = decaModeWorkP->endCamPos;
    camMot->up = decaModeWorkP->endCamUp;
    camMot->target = decaModeWorkP->endCamTarget;
    camMot->fov = decaModeWorkP->endCamFov;
    camMot->time = 60;
    decaWorkP->camMotNum++;
    camMot++;
    DecaCameraPathStart();
    Hu3DModelDispOn(decaWorkP->stageMdlId[DECA_MDL_COVER]);
    Hu3DModelAttrReset(decaWorkP->stageMdlId[DECA_MDL_COVER], HU3D_MOTATTR_PAUSE);
    DecaMotionStartEndSet(decaWorkP->stageMdlId[DECA_MDL_COVER], 0, 29);
    DecaWinClose();
    while(!DecaFlagGet(DECAFLAG_CAMPATH_DONE)) {
        HuPrcVSleep();
    }
    HuAudFXPlay(MSM_SE_DEFAULT_75);
    DecaScoreModeSet(DECA_SCOREMODE_RECORD);
    DecaProcessCreate(DecaMessMgStart);
    while(!DecaMesFlagGet(DECA_MESSFLAG_MGSTART)) {
        HuPrcVSleep();
    }
    if(DecaFlagGet(DECAFLAG_EXIT)) {
        DecaWinClose();
        if(decaWorkP->seNo >= 0) {
            HuAudFXStop(decaWorkP->seNo);
        }
        if(decaWorkP->enterProc) {
            HuPrcKill(decaWorkP->enterProc);
            decaWorkP->enterProc = NULL;
        }
        decaWorkP->seNo = MSM_SENO_NONE;
        if(decaWorkP->streamNo >= 0) {
            HuAudSStreamFadeOut(decaWorkP->streamNo, 1000);
            decaWorkP->streamNo = MSM_STREAMNO_NONE;
        }
        DecaWipeOut(WIPE_TYPE_WHITEFADE, 60);
        DecaOvlEvtnoSet(3);
        return;
    }
    DecaMgExit(mg->no);
    if(decaWorkP->seNo >= 0) {
        HuAudFXStop(decaWorkP->seNo);
    }
    if(decaWorkP->enterProc) {
        HuPrcKill(decaWorkP->enterProc);
        decaWorkP->enterProc = NULL;
    }
    decaWorkP->seNo = MSM_SENO_NONE;
    if(decaWorkP->streamNo >= 0) {
        HuAudSStreamFadeOut(decaWorkP->streamNo, 1000);
        decaWorkP->streamNo = MSM_STREAMNO_NONE;
    }
    DecaWipeOut(WIPE_TYPE_WHITEFADE, 60);
    while(!decaWorkP->dirRead) {
        HuPrcVSleep();
    }
    GwSystem.mgNo = mg->mgNo-GW_MGNO_BASE;
    mgNo = mg->mgNo;
    GWMgUnlockSet(mgNo);
    GwSystem.mgNo = mgNo-GW_MGNO_BASE;
    if(DecaFlagGet(DECAFLAG_INSTEXIT)) {
        GWMgInstFSet(TRUE);
    } else {
        GWMgInstFSet(FALSE);
    }
    omOvlCall(DLL_instdll, 0, 0);
    while(1) {
        HuPrcVSleep();
    }
}

void DecaEvtEnding(void)
{
    DECAWORK_PLAYER *player;
    int i;
    int tempNo;
    int mdlId;
    int playerTbl[GW_PLAYER_MAX];
    int tieTbl[GW_PLAYER_MAX];
    int no;
    int tieNum;
    HUPROCESS *winProc;
    CAMMOTIONWORK *camMot;
    float camDist;
    int camMotNum;
    no = decaModeWorkP->mgNo+1;
    DecaFlagSet(DECAFLAG_FADE_ENDING);
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        player->pos = playerPosTbl[i];
        if(player->mot != 0) {
            CharMotionShiftSet(player->charNo, player->motId[0], 0, 5, HU3D_MOTATTR_LOOP);
            player->mot = 0;
        }
        Hu3DModelDispOn(player->mdlId);
        Hu3DModelPosSet(player->mdlId, player->pos.x, player->pos.y, player->pos.z);
        Hu3DModelLayerSet(player->mdlId, 4);
        Hu3DModelCameraSet(player->mdlId, HU3D_CAM4);
    }
    decaWorkP->guidePos = guidePosTbl[3];
    Hu3DModelPosSetV(decaWorkP->guideMdlId, &decaWorkP->guidePos);
    Hu3DModelRotSet(decaWorkP->guideMdlId, 0, 0, 0);
    Hu3DModelDispOn(decaWorkP->guideMdlId);
    Hu3DModelCameraSet(decaWorkP->guideMdlId, HU3D_CAM4);
    Hu3DMotionTimeSet(decaWorkP->stageMdlId[DECA_MDL_STAGE_NIGHT], no);
    Hu3DMotionTimeSet(decaWorkP->stageMdlId[DECA_MDL_LIGHT_MAIN], no);
    Hu3DModelLightInfoSet(decaWorkP->stageMdlId[DECA_MDL_LIGHT_MAIN], TRUE);
    Hu3DModelDispOn(decaWorkP->stageMdlId[DECA_MDL_MOON]);
    Hu3DModelAttrReset(decaWorkP->stageMdlId[DECA_MDL_MOON], HU3D_MOTATTR_PAUSE);
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        GwMgDecaScore[i].mgScore[decaModeWorkP->mgNo] = DecaMgScoreGet(player->no);
        GwMgDecaScore[i].finalScore += DecaScoreGet(player->no);
    }
    DecaScoreStart();
    decaWorkP->seNo = HuAudFXPlay(MSM_SE_INST_25);
    decaWorkP->streamNo = HuAudBGMPlay(MSM_STREAM_PARTYRESULT);
    DecaCameraMotionStart(DECA_MDL_CAMERAMOT_NULL);
    DecaCameraMotionStart(DECA_MDL_CAMERAMOT_END_START);
    DecaWipeIn(WIPE_TYPE_PREVTYPE, 60);
    while(!DecaFlagGet(DECAFLAG_CAMPATH_DONE)) {
        HuPrcVSleep();
    }
    DecaSleep(60);
    HuAudFXPlayPan(MSM_SE_DEFAULT_83, 64);
    mdlId = decaWorkP->stageMdlId[DECA_MDL_LIGHT1];
    Hu3DModelDispOn(mdlId);
    Hu3DModelAttrReset(mdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelPosSet(mdlId, decaWorkP->guidePos.x, 0, decaWorkP->guidePos.z);
    DecaSleep(30);
    DecaProcessCreate(DecaMessEnd);
    while(!DecaMesFlagGet(DECA_MESSFLAG_END)) {
        HuPrcVSleep();
    }
    DecaWinClose();
    Hu3DModelDispOff(decaWorkP->stageMdlId[DECA_MDL_LIGHT1]);
    DecaSleep(120);
    DecaProcessCreate(DecaMessMaxScore);
    while(!DecaMesFlagGet(DECA_MESSFLAG_MAXSCORE)) {
        HuPrcVSleep();
    }
    DecaSleep(120);
    HuAudSStreamPauseFadeOut(decaWorkP->streamNo, TRUE, 1000);
    DecaProcessCreate(DecaMessWinner);
    while(!DecaMesFlagGet(DECA_MESSFLAG_WINNER)) {
        HuPrcVSleep();
    }
    DecaWinClose();
    for(tempNo=0; tempNo<GW_PLAYER_MAX; tempNo++) {
        playerTbl[tempNo] = DecaPlayerRankGet(tempNo, &tieNum, NULL);
        if(tieNum) {
            for(i=tempNo; i<GW_PLAYER_MAX; i++) {
                tieTbl[i-tempNo] = DecaPlayerRankGet(i, &tieNum, NULL);
            }
            break;
        }
    }
    for(i=tempNo; i--;) {
        HuAudFXPlayPan(MSM_SE_DEFAULT_83, (playerTbl[i]*16)+32);
        mdlId = decaWorkP->stageMdlId[DECA_MDL_LIGHT1+i];
        Hu3DModelDispOn(mdlId);
        Hu3DModelAttrReset(mdlId, HU3D_MOTATTR_PAUSE);
        Hu3DModelPosSetV(mdlId, &playerPosTbl[playerTbl[i]]);
    }
    DecaEnterEffectStart(1);
    if(tempNo == 1) {
        DecaSleep(30);
    } else {
        DecaSleep(30);
    }
    DecaWinMusicStart();
    for(i=0; i<GW_PLAYER_MAX-tempNo; i++) {
        player = &decaWorkP->player[tieTbl[i]];
        if(player->mot != 4) {
            CharMotionShiftSet(player->charNo, player->motId[4], 0, 5, HU3D_MOTATTR_NONE);
            player->mot = 4;
        }
    }
    for(i=tempNo; i--;) {
        player = &decaWorkP->player[playerTbl[i]];
        DecaCharSoundPlay(player, 1);
        if(player->mot != 3) {
            CharMotionShiftSet(player->charNo, player->motId[3], 0, 5, HU3D_MOTATTR_NONE);
            player->mot = 3;
        }
    }
    DecaSleep(180);
    for(player=&decaWorkP->player[0], i=0; i<GW_PLAYER_MAX; i++, player++) {
        GwMgDecaScore[i].finalScore -= DecaScoreGet(player->no);
    }
    DecaMgScoreDisp(FALSE);
    decaModeWorkP->mgNo++;
    DecaProcessCreate(DecaMessReset);
    while(!DecaMesFlagGet(DECA_MESSFLAG_RESET)) {
        HuPrcVSleep();
    }
    DecaWinClose();
    DecaRecordSave();
    if(DecaFlagGet(DECAFLAG_RESET)) {
        DecaStageReset();
        if(decaWorkP->seNo >= 0) {
            HuAudFXStop(decaWorkP->seNo);
        }
        if(decaWorkP->enterProc) {
            HuPrcKill(decaWorkP->enterProc);
            decaWorkP->enterProc = NULL;
        }
        decaWorkP->seNo = MSM_SENO_NONE;
        if(decaWorkP->streamNo >= 0) {
            HuAudSStreamFadeOut(decaWorkP->streamNo, 1000);
            decaWorkP->streamNo = MSM_STREAMNO_NONE;
        }
        DecaWipeOut(WIPE_TYPE_WHITEFADE, 60);
        DecaScoreModeSet(DECA_SCOREMODE_HIDE);
        DecaOvlEvtnoSet(0);
    } else {
        Hu3DModelDispOn(decaWorkP->stageMdlId[DECA_MDL_HANABI]);
        Hu3DModelAttrReset(decaWorkP->stageMdlId[DECA_MDL_HANABI], HU3D_MOTATTR_PAUSE);
        for(i=15; i--;) {
            winSoundTbl[i][0] += 40;
        }
        winProc = HuPrcChildCreate(DecaWinMain, 200, 4096, 0, HuPrcCurrentGet());
        camMotNum = 29;
        camMot = &decaWorkP->camMot[1];
        decaWorkP->camMotNum = CamMotionExPathGet(decaWorkP->stageMdlId[DECA_MDL_CAMERAMOT_END_EXIT], 1, 
            Hu3DMotionMaxTimeGet(decaWorkP->stageMdlId[DECA_MDL_CAMERAMOT_END_EXIT]), 0, &camMot, &camMotNum);
        for(i=decaWorkP->camMotNum; i--;) {
            camMot->time += 360;
            camMot++;
        }
        decaWorkP->camMotNum++;
        camMot = &decaWorkP->camMot[0];
        Hu3DCameraPosGet(HU3D_CAM4, &camMot->pos, &camMot->up, &camMot->target);
        Hu3DCameraPerspectiveGet(HU3D_CAM4, &camMot->fov, &camDist, &camDist);
        camMot->time = 0;
        DecaCameraMotionStart(DECA_MDL_CAMERAMOT_NULL);
        DecaCameraPathStart();
        DecaSleep(300);
        if(decaWorkP->streamNo >= 0) {
            HuAudSStreamFadeOut(decaWorkP->streamNo, 1000);
            decaWorkP->streamNo = MSM_STREAMNO_NONE;
        }
        DecaWipeOut(WIPE_TYPE_WHITEFADE, 60);
        while(!DecaFlagGet(DECAFLAG_CAMPATH_DONE)) {
            HuPrcVSleep();
        }
        if(decaWorkP->seNo >= 0) {
            HuAudFXStop(decaWorkP->seNo);
        }
        if(decaWorkP->enterProc) {
            HuPrcKill(decaWorkP->enterProc);
            decaWorkP->enterProc = NULL;
        }
        decaWorkP->seNo = MSM_SENO_NONE;
        HuPrcKill(winProc);
        DecaOvlEvtnoSet(3);
    }
}

void DecaEvtLast(void)
{
    
}