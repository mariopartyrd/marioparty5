#include "REL/instDll.h"

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

#define GW_SDPOINT_INC(type) do { \
    if(type <= MG_TYPE_BATTLE) { \
        GwCommon.sdPoint[type]++; \
        if(GwCommon.sdPoint[type] > 999) { \
            GwCommon.sdPoint[type] = 999; \
        } \
    } \
} while(0)

static OMOBJ *outViewObj;
OMOBJMAN *objman;
static u16 MgNo;
static u16 BoardNo;
static s16 lbl_1_bss_70;
static float lbl_1_bss_6C;
static HUSPRGRPID MgNameGrpId;
static u8 LanguageNo;
HU3DMODELID CharMdlId[INST_CHARMDL_MAX];
HU3DMOTID CharMotId[INST_CHARMDL_MAX][5];
s16 CharNo[GW_PLAYER_MAX];
static HUSPRGRPID InstHelpGrpId;
static HUSPRGRPID MgNameBarGrpId;
static s16 InstBtn;
static s16 InstBtnDown;
static MGDATA *MgDataP;
static u32 *InstMesTbl;
static s16 SubGameNo;
static s16 LoadDirStat;
static BOOL LoadDoneF;
s16 lbl_1_bss_8;
BOOL CameraDebugF;
s16 InstMode;

GXColor ToueiFogColor = { 20, 0 ,0 , 0 };
static OMCAMERAVIEW CameraBgView = {
    { 0, 250, 0 },
    { 0, 0, 0 },
    7000
};

static OMCAMERAVIEW CameraFgView = {
    { 0, 0, 0 },
    { 0, 0, 0 },
    1600
};

static OMCAMERAVIEW CameraMgMovieView = {
    { 0, 150, 0 },
    { 0, 0, 0 },
    140
};

void InstMain(void);
void CameraMove(OMOBJ *obj);

void ObjectSetup(void)
{
    int i;
    int GLightId;
    
    OSReport("******* INST ObjectSetup *********\n");
    objman = omInitObjMan(50, 8192);
    LanguageNo = GWLanguageGet();
    MgNo = GwSystem.mgNo;
    GWMgUnlockSet(MgNo+GW_MGNO_BASE);
    MgDataP = &mgDataTbl[MgNo];
    if(_CheckFlag(FLAG_INST_DECA)) {
        InstMesTbl = &MgDataP->instMes[1][0];
    } else {
        InstMesTbl = &MgDataP->instMes[0][0];
    }
    lbl_1_bss_70 = 0;
    lbl_1_bss_8 = 0;
    _ClearFlag(FLAG_MG_PAUSE_OFF);
    for(i=0; sndGrpTable[i].ovl != DLL_NONE; i++) {
        if(sndGrpTable[i].ovl == mgDataTbl[MgNo].ovl) {
            break;
        }
    }
    if(sndGrpTable[i].ovl != DLL_NONE) {
        HuAudSndGrpSetSet(sndGrpTable[i].grpSet);
    }
    if(!_CheckFlag(FLAG_INST_MGMODE) && !_CheckFlag(FLAG_MG_PRACTICE)) {
        if(mgDataTbl[MgNo].ovl == DLL_m519dll) {
            if(GWSubGameNoGet() == -1) {
                GWSubGameNoSet(frandmod(3));
            }
        } else {
            GWSubGameNoSet(-1);
        }
        mgSubMode = 0;
    }
    SubGameNo = GWSubGameNoGet();
    if(SubGameNo < 0 || !MgDataP->instPic[SubGameNo]) {
        SubGameNo = 0;
    }
    OSReport("SubGameNo = %d\n", SubGameNo);
    if((omprevovl >= DLL_m501dll && omprevovl <= DLL_m580dll && !_CheckFlag(FLAG_MG_PRACTICE) && _CheckFlag(FLAG_INST_MGMODE)) || omovlevtno == 2) {
        if(MgDataP->flag & MG_FLAG_GRPORDER) {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grp = GwPlayerConf[i].grp/2;
            }
        }
        HuDataDirClose(mgDataTbl[MgNo].dataDir);
        omOvlReturn(1);
        return;
    }
    _ClearFlag(FLAG_MG_PRACTICE);
    if(!_CheckFlag(FLAG_INST_MGMODE) && !_CheckFlag(FLAG_INST_NOHISCHG)) {
        OMOVLHIS *his = omOvlHisGet(0);
        if(MgDataP->type != MG_TYPE_4P && MgDataP->type != MG_TYPE_1VS3 && MgDataP->type != MG_TYPE_2VS2 && MgDataP->type != MG_TYPE_BATTLE) {
            omOvlHisChg(0, DLL_instdll, 2, his->stat);
        } else {
            omOvlHisChg(0, DLL_resultdll, his->evtno, his->stat);
        }
    }
    if(MgDataP->type == MG_TYPE_4P) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            GwPlayerConf[i].grp = i;
        }
    }
    if(omovlevtno == 0) {
        int grpMap[GW_PLAYER_MAX][GW_PLAYER_MAX];
        int grpNum[2];
        if(MgDataP->flag & MG_FLAG_GRPORDER) {
            grpNum[0] = grpNum[1] = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(GwPlayerConf[i].grp >= 2) {
                    break;
                }
            }
            if(i == GW_PLAYER_MAX) {
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    grpMap[GwPlayerConf[i].grp][grpNum[GwPlayerConf[i].grp]++] = i;
                }
                if(frand() & 0x1) {
                    GwPlayerConf[grpMap[0][0]].grp = 0;
                    GwPlayerConf[grpMap[0][1]].grp = 1;
                } else {
                    GwPlayerConf[grpMap[0][0]].grp = 1;
                    GwPlayerConf[grpMap[0][1]].grp = 0;
                }
                if(frand() & 0x1) {
                    GwPlayerConf[grpMap[1][0]].grp = 2;
                    GwPlayerConf[grpMap[1][1]].grp = 3;
                } else {
                    GwPlayerConf[grpMap[1][0]].grp = 3;
                    GwPlayerConf[grpMap[1][1]].grp = 2;
                }
            }
        } else if(mgDataTbl[MgNo].type == MG_TYPE_1VS3) {
            grpNum[0] = grpNum[1] = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(GwPlayerConf[i].grp > 1) {
                    break;
                }
                if(GwPlayerConf[i].grp < 0) {
                    break;
                }
                grpNum[GwPlayerConf[i].grp]++;
            }
            if(i != 4 || grpNum[0] != 1 || grpNum[1] != 3) {
                OSReport("Error: Illegal Group pattern.Adjusted.\n");
                GwPlayerConf[0].grp = 0;
                GwPlayerConf[1].grp = GwPlayerConf[2].grp = GwPlayerConf[3].grp = 1;
            }
        } else if(mgDataTbl[MgNo].type == MG_TYPE_2VS2) {
            grpNum[0] = grpNum[1] = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(GwPlayerConf[i].grp > 1) {
                    break;
                }
                if(GwPlayerConf[i].grp < 0) {
                    break;
                }
                grpNum[GwPlayerConf[i].grp]++;
            }
            if(i != 4 || grpNum[0] != 2 || grpNum[1] != 2) {
                OSReport("Error: Illegal Group pattern.Adjusted.\n");
                GwPlayerConf[0].grp = GwPlayerConf[1].grp = 0;
                GwPlayerConf[2].grp = GwPlayerConf[3].grp = 1;
            }
        }
    }
    if(mgDataTbl[MgNo].type == MG_TYPE_LAST || !_CheckFlag(FLAG_MGINST_ON) || !GWMgInstFGet()) {
        GW_SDPOINT_INC(MgDataP->type);
        HuDataDirClose(DATA_inst);
        omSysPauseEnable(TRUE);
        omOvlCall(mgDataTbl[MgNo].ovl, 0, 0);
        return;
    }
    HuDataDirClose(mgDataTbl[MgNo].dataDir);
    lbl_1_bss_6C = 20;
    BoardNo = GwSystem.boardNo;
    if(mgBoard2Force) {
        BoardNo = 1;
    }
    omSysPauseEnable(FALSE);
    Hu3DCameraCreate(HU3D_CAM0|HU3D_CAM1);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 5, 20, 15000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0, 0, 640, 480, 0, 1);
    Hu3DCameraPerspectiveSet(HU3D_CAM1, 30, 20, 15000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM1, 0, 0, 640, 480, 0, 1);
    omCameraViewSetMulti(HU3D_CAM0, &CameraBgView);
    omCameraViewSetMulti(HU3D_CAM1, &CameraFgView);
    GLightId = Hu3DGLightCreate(0, 100, 1000, 0, -0.5f, -1, 255, 255, 255);
    Hu3DGLightInfinitytSet(GLightId);
    outViewObj = omAddObj(objman, 32730, 0, 0, omOutViewMulti);
    outViewObj->work[0] = 2;
    omAddObj(objman, 0, 32, 32, CameraMove);
    HuAudSStreamStop(0);
    HuAudSStreamPlay(MSM_STREAM_MGINST);
    Hu3DBGColorSet(0, 0, 0);
    HuWinInit(1);
    HuPrcChildCreate(InstMain, 1000, 12288, 0, objman);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        CharNo[i] = GwPlayerConf[i].charNo;
    }
    if(omprevovl >= DLL_m501dll && omprevovl <= DLL_m576dll) {
        InstMode = 2;
    } else {
        InstMode = 0;
    }
}

void InstCreate(void);
void InstMgNameStart(void);
void InstMgNameEnd(void);
void InstHelpMain(void);
void InstToueiMain(void);
void InstWinMain(void);


static void LoadProc(void);

void InstMain(void)
{
    s16 i; //r31
    BOOL exitF; //r30
    BOOL comInputF; //r29
    
    exitF = FALSE;
    
    InstCreate();
    HuPrcChildCreate(InstWinMain, 100, 12288, 0, HuPrcCurrentGet());
    HuPrcChildCreate(LoadProc, 2000, 12288, 0, HuPrcCurrentGet());
    HuPrcChildCreate(InstHelpMain, 100, 12288, 0, HuPrcCurrentGet());
    HuPrcChildCreate(InstToueiMain, 100, 12288, 0, HuPrcCurrentGet());
    if(InstMode == 0) {
        InstMode = 1;
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
    } else {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
    }
    WipeWait();
    InstMgNameStart();
    InstMode = 2;
    comInputF = FALSE;
    if(MgDataP->type != MG_TYPE_KETTOU) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(GwPlayerConf[i].type == GW_TYPE_MAN) {
                break;
            }
        }
        if(i == GW_PLAYER_MAX) {
            comInputF = TRUE;
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if((GwPlayerConf[i].grp == 0 || GwPlayerConf[i].grp == 1) && (GwPlayerConf[i].type == GW_TYPE_MAN)) {
                break;
            }
        }
        if(i == GW_PLAYER_MAX) {
            comInputF = TRUE;
        }
    }
    while(1) {
        for(InstBtn=InstBtnDown=i=0; i<GW_PLAYER_MAX; i++) {
            if(MgDataP->type != MG_TYPE_KETTOU) {
                if(GwPlayerConf[i].type == GW_TYPE_MAN) {
                    InstBtn |= HuPadBtn[GwPlayerConf[i].padNo];
                    InstBtnDown |= HuPadBtnDown[GwPlayerConf[i].padNo];
                }
            } else {
                if(GwPlayerConf[i].type == GW_TYPE_MAN && GwPlayerConf[i].grp < 2) {
                    InstBtn |= HuPadBtn[GwPlayerConf[i].padNo];
                    InstBtnDown |= HuPadBtnDown[GwPlayerConf[i].padNo];
                }
            }
        }
        if(comInputF) {
            HuPrcSleep(60);
            InstBtnDown = PAD_BUTTON_START;
        }
        if(InstBtnDown & PAD_BUTTON_START) {
            break;
        }
        if(InstBtnDown & PAD_TRIGGER_Z) {
            OMOVLHIS *his = omOvlHisGet(0);
            omOvlHisChg(0, DLL_instdll, 1, his->stat);
            _SetFlag(FLAG_MG_PRACTICE);
            break;
        }
        if(omSysExitReq || (mgInstExitF && (InstBtnDown & PAD_BUTTON_B))) {
            if(MgDataP->flag & MG_FLAG_GRPORDER) {
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    GwPlayerConf[i].grp = GwPlayerConf[i].grp/2;
                }
            }
            exitF = TRUE;
            break;
        }
        HuPrcVSleep();
    }
    if(!exitF) {
        InstMode = 3;
        InstMgNameEnd();
    }
    if(!exitF && !_CheckFlag(FLAG_MG_PRACTICE)) {
        GW_SDPOINT_INC(MgDataP->type);
    }
    if(!exitF) {
        HuAudFXPlay(MSM_SE_INST_01);
    } else {
        HuAudFXPlay(MSM_SE_CMN_04);
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 40);
    HuAudSStreamAllFadeOut(1000);
    WipeWait();
    HuTHPClose();
    while(HuTHPProcCheck()) {
        HuPrcVSleep();
    }
    if(!exitF) {
        while(!LoadDoneF) {
            HuPrcVSleep();
        }
        omOvlCall(MgDataP->ovl, 0, 0);
    } else {
        if(!HuDataGetAsyncStat(LoadDirStat)) {
            HuDataDirCancel(LoadDirStat);
        } else {
            HuDataDirClose(mgDataTbl[MgNo].dataDir);
        }
        omOvlReturn(1);
    }
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

void InstCreate(void)
{
    s16 i; //r31
    s16 num; //r30
    HUSPRGRPID grpId; //r29
    HUSPRID sprId; //r28
    s16 otherNum; //r27
    s16 guideIdx; //r26
    HU3DMODEL *modelP; //r25
    ANIMDATA *animP; //r24
    
    HuVecF pos3D; //sp+0x1C
    HuVecF pos2D; //sp+0x10
    s16 playerNo[GW_PLAYER_MAX]; //sp+0x8
    
    static unsigned int soraMdlTbl[MG_TYPE_MAX] = {
        INST_HSF_sora1,
        INST_HSF_sora4,
        INST_HSF_sora2,
        INST_HSF_sora1,
        INST_HSF_sora_koopa,
        INST_HSF_sora_koopa,
        INST_HSF_sora3,
        INST_HSF_sora1,
        INST_HSF_sora_donkey
    };
    static unsigned int guideMdlTbl[3] = {
        INST_HSF_guide_choru,
        INST_HSF_guide_neru,
        INST_HSF_guide_niru
    };
    StageMdlTbl[STAGE_MODEL_SORA].dataNum = soraMdlTbl[MgDataP->type];
    if(_CheckFlag(FLAG_INST_MGMODE)) {
        guideIdx = 2;
    } else if(GWPartyFGet() == TRUE) {
        guideIdx = 0;
    } else {
        guideIdx = 1;
    }
    StageMdlTbl[STAGE_MODEL_GUIDE].dataNum = guideMdlTbl[guideIdx];
    if(MgDataP->type == MG_TYPE_4P || MgDataP->type == MG_TYPE_BATTLE) {
        StageMdlTbl[STAGE_MODEL_TOUEI1].pos.x = -420;
        StageMdlTbl[STAGE_MODEL_TOUEI1].pos.y = 220;
        StageMdlTbl[STAGE_MODEL_TOUEI2].pos.x = -300;
        StageMdlTbl[STAGE_MODEL_TOUEI2].pos.y = 94;
        StageMdlTbl[STAGE_MODEL_TOUEI3].pos.x = 300;
        StageMdlTbl[STAGE_MODEL_TOUEI3].pos.y = 94;
        StageMdlTbl[STAGE_MODEL_TOUEI4].pos.x = 420;
        StageMdlTbl[STAGE_MODEL_TOUEI4].pos.y = 220;
    } else if(MgDataP->type == MG_TYPE_1VS3) {
        for(i=0, num=1; i<GW_PLAYER_MAX; i++) {
            if(GwPlayerConf[i].grp == 0) {
                playerNo[0] = i;
            } else {
                playerNo[num++] = i;
            }
        }
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[0]].pos.x = -360;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[0]].pos.y = 157;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[1]].pos.x = 360;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[1]].pos.y = 220;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[2]].pos.x = 297;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[2]].pos.y = 94;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[3]].pos.x = 423;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[3]].pos.y = 94;
    } else if(MgDataP->type == MG_TYPE_2VS2) {
        for(i=0, num=0; i<GW_PLAYER_MAX; i++) {
            if(GwPlayerConf[i].grp > num) {
                num = GwPlayerConf[i].grp;
            }
        }
        if(num >= 2) {
            for(i=num=0, otherNum=2; i<GW_PLAYER_MAX; i++) {
                playerNo[GwPlayerConf[i].grp] = i;
            }
        } else {
            for(i=num=0, otherNum=2; i<GW_PLAYER_MAX; i++) {
                if(GwPlayerConf[i].grp == 0) {
                    playerNo[num++] = i;
                } else {
                    playerNo[otherNum++] = i;
                }
            }
        }
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[0]].pos.x = -423;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[0]].pos.y = 157;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[1]].pos.x = -297;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[1]].pos.y = 157;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[2]].pos.x = 297;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[2]].pos.y = 157;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[3]].pos.x = 423;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[3]].pos.y = 157;
    } else if(MgDataP->type == MG_TYPE_LAST) {
        for(i=0, num=1; i<GW_PLAYER_MAX; i++) {
            if(GwPlayerConf[i].grp == 0) {
                playerNo[0] = i;
            } else {
                playerNo[num++] = i;
            }
        }
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[0]].pos.x = -360;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[0]].pos.y = 157;
        StageMdlTbl[STAGE_MODEL_TOUEI5].pos.x = 360;
        StageMdlTbl[STAGE_MODEL_TOUEI5].pos.y = 157;
    } else if(MgDataP->type == MG_TYPE_KETTOU) {
        for(i=0, num=2; i<GW_PLAYER_MAX; i++) {
            if(GwPlayerConf[i].grp < 2) {
                playerNo[GwPlayerConf[i].grp] = i;
            } else {
                playerNo[num++] = i;
            }
        }
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[0]].pos.x = -360;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[0]].pos.y = 157;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[1]].pos.x = 360;
        StageMdlTbl[STAGE_MODEL_TOUEI1+playerNo[1]].pos.y = 157;
    } else {
        StageMdlTbl[STAGE_MODEL_TOUEI1].pos.x = -423;
        StageMdlTbl[STAGE_MODEL_TOUEI1].pos.y = 220;
        StageMdlTbl[STAGE_MODEL_TOUEI2].pos.x = -297;
        StageMdlTbl[STAGE_MODEL_TOUEI2].pos.y = 220;
        StageMdlTbl[STAGE_MODEL_TOUEI3].pos.x = -423;
        StageMdlTbl[STAGE_MODEL_TOUEI3].pos.y = 94;
        StageMdlTbl[STAGE_MODEL_TOUEI4].pos.x = -297;
        StageMdlTbl[STAGE_MODEL_TOUEI4].pos.y = 94;
        StageMdlTbl[STAGE_MODEL_TOUEI5].pos.x = 360;
        StageMdlTbl[STAGE_MODEL_TOUEI5].pos.y = 157;
    }
    StageModelCreate(StageMdlTbl);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        CharMdlId[i] = CharModelCreate(CharNo[i], CHAR_MODEL1);
        CharMotId[i][0] = CharMotionCreate(CharNo[i], CHARMOT_HSF_c000m1_300);
        CharMotId[i][1] = CharMotionCreate(CharNo[i], CHARMOT_HSF_c000m1_303);
        CharMotionSet(CharNo[i], CharMotId[i][0]);
        Hu3DModelCameraSet(CharMdlId[i], HU3D_CAM0);
        Hu3DModelPosSet(CharMdlId[i], ((i%2)*200)-300, (i/2)*270, 0);
        if(CharNo[i] == CHARNO_TERESA) {
            Hu3DModelScaleSet(CharMdlId[i], 0.8f, 0.8f, 0.8f);
        } else {
            Hu3DModelScaleSet(CharMdlId[i], 1, 1, 1);
        }
        Hu3DModelAttrSet(CharMdlId[i], HU3D_MOTATTR_LOOP);
        CharModelDataClose(CharNo[i]);
    }
    CharMdlId[INST_CHARMDL_NPC] = HU3D_MODELID_NONE;
    if(MgDataP->type == MG_TYPE_LAST) {
        Hu3DModelDispOff(StageMdlId[STAGE_MODEL_TOUEI1+playerNo[1]]);
        Hu3DModelDispOff(StageMdlId[STAGE_MODEL_TOUEI1+playerNo[2]]);
        Hu3DModelDispOff(StageMdlId[STAGE_MODEL_TOUEI1+playerNo[3]]);
    } else if(MgDataP->type == MG_TYPE_KETTOU) {
        Hu3DModelDispOff(StageMdlId[STAGE_MODEL_TOUEI1+playerNo[2]]);
        Hu3DModelDispOff(StageMdlId[STAGE_MODEL_TOUEI1+playerNo[3]]);
    }
    if(MgDataP->type == MG_TYPE_KUPA || MgDataP->type == MG_TYPE_LAST) {
        CharMdlId[INST_CHARMDL_NPC] = Hu3DModelCreateData(INST_HSF_koopa);
        CharMotId[INST_CHARMDL_NPC][0] = Hu3DJointMotionData(CharMdlId[INST_CHARMDL_NPC], INST_HSF_koopa_idle);
        CharMotId[INST_CHARMDL_NPC][1] = Hu3DJointMotionData(CharMdlId[INST_CHARMDL_NPC], INST_HSF_koopa_jump);
        Hu3DMotionSet(CharMdlId[INST_CHARMDL_NPC], CharMotId[INST_CHARMDL_NPC][0]);
        Hu3DModelCameraSet(CharMdlId[INST_CHARMDL_NPC], HU3D_CAM0);
        Hu3DModelPosSet(CharMdlId[INST_CHARMDL_NPC], 150, 0, 0);
        Hu3DModelScaleSet(CharMdlId[INST_CHARMDL_NPC], 0.7f, 0.7f, 0.7f);
        Hu3DModelAttrSet(CharMdlId[INST_CHARMDL_NPC], HU3D_MOTATTR_LOOP);
    } else if(MgDataP->type == MG_TYPE_DONKEY) {
        CharMdlId[INST_CHARMDL_NPC] = Hu3DModelCreateData(INST_HSF_donkey);
        CharMotId[INST_CHARMDL_NPC][0] = Hu3DJointMotionData(CharMdlId[INST_CHARMDL_NPC], INST_HSF_donkey_idle);
        CharMotId[INST_CHARMDL_NPC][1] = Hu3DJointMotionData(CharMdlId[INST_CHARMDL_NPC], INST_HSF_donkey_jump);
        Hu3DMotionSet(CharMdlId[INST_CHARMDL_NPC], CharMotId[INST_CHARMDL_NPC][0]);
        Hu3DModelCameraSet(CharMdlId[INST_CHARMDL_NPC], HU3D_CAM0);
        Hu3DModelPosSet(CharMdlId[INST_CHARMDL_NPC], 150, 30, 0);
        Hu3DModelScaleSet(CharMdlId[INST_CHARMDL_NPC], 1, 1, 1);
        Hu3DModelAttrSet(CharMdlId[INST_CHARMDL_NPC], HU3D_MOTATTR_LOOP);
    }
    Hu3DModelAttrSet(StageMdlId[STAGE_MODEL_STAGE1], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(StageMdlId[STAGE_MODEL_STAGE2], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(StageMdlId[STAGE_MODEL_SORA], HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(StageMdlId[STAGE_MODEL_SORA], 0);
    Hu3DMotionSpeedSet(StageMdlId[STAGE_MODEL_SORA], 0.5f);
    ToueiDispInit(MgDataP->movie[SubGameNo]);
    MgNameGrpId = MgNameCreate(MgDataP->nameMes);
    HuSprGrpPosSet(MgNameGrpId, 288, 64);
    InstHelpGrpId = grpId = HuSprGrpCreate(1);
    if(!mgInstExitF) {
        sprId = HuSprCreate(HuSprAnimDataRead(INST_ANM_help), 100, 0);
    } else {
        sprId = HuSprCreate(HuSprAnimDataRead(INST_ANM_help_exit), 100, 0);
    }
    HuSprGrpMemberSet(grpId, 0, sprId);
    HuSprAttrSet(grpId, 0, HUSPR_ATTR_LINEAR);
    modelP = &Hu3DData[StageMdlId[STAGE_MODEL_GUIDE]];
    pos3D = modelP->pos;
    pos3D.y += 70;
    Hu3D3Dto2D(&pos3D, HU3D_CAM1, &pos2D);
    HuSprGrpPosSet(grpId, pos2D.x, pos2D.y);
    MgNameBarGrpId = grpId = HuSprGrpCreate(1);
    sprId = HuSprCreate(HuSprAnimDataRead(INST_ANM_mgname_bar), 100, 0);
    HuSprGrpMemberSet(grpId, 0, sprId);
    HuSprGrpPosSet(grpId, 288, 64);
    HuDataDirClose(DATA_inst);
    animP = HuSprAnimRead(HuDataReadNumHeapShortForce(MgDataP->instPic[SubGameNo], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DAnimCreate(animP, StageMdlId[STAGE_MODEL_MGMOVIE], "S3TC_mg_movie");
    if(InstMode != 2) {
        HuSprGrpPosSet(MgNameGrpId, 1000, 0);
        HuSprGrpScaleSet(MgNameBarGrpId, 0, 0);
        for(i=0; i<INST_CHARMDL_MAX; i++) {
            if(CharMdlId[i] != HU3D_MODELID_NONE) {
                Hu3DModelPosSet(StageMdlId[STAGE_MODEL_TOUEI1+i], 1000, 1000, 0);
            }
        }
        Hu3DModelScaleSet(StageMdlId[STAGE_MODEL_MGMOVIE], 0, 0, 0);
        HuSprGrpScaleSet(InstHelpGrpId, 0, 0);
        Hu3DModelPosSet(StageMdlId[STAGE_MODEL_GUIDE], 0, -1000, 0);
    }
}

void InstMgNameStart(void)
{
    s16 i;
    float t;
    
    if(InstMode == 2) {
        return;
    }
    HuSprGrpPosSet(MgNameGrpId, 1000, 0);
    for(i=1; i<=30; i++) {
        if(i < 20) {
            t = i/20.0;
            HuSprGrpScaleSet(MgNameBarGrpId, 1, 1.1*HuSin(t*90));
        }
        if(i == 20) {
            HuSprGrpScaleSet(MgNameBarGrpId, 1, 1);
        }
        if(i >= 20) {
            t = (i-20)/10.0;
            HuSprGrpPosSet(MgNameGrpId, (HuCos(t*90)*500)+288, 64);
        }
        HuPrcVSleep();
    }
}

void InstMgNameEnd(void)
{
    s16 i;
    float t;
    for(i=1; i<=10; i++) {
        t = i/10.0;
        HuSprGrpPosSet(MgNameGrpId, 288, 64-((1-HuCos(t*90))*100));
        HuSprGrpPosSet(MgNameBarGrpId, 288, 64-((1-HuCos(t*90))*100));
        HuPrcVSleep();
    }
    HuPrcSleep(20);
    omCameraViewMoveMulti(HU3D_CAM1, &CameraMgMovieView, 40, OM_CAMERAMOVE_COS);
}

void InstHelpMain(void)
{
    s16 i; //r31
    HU3DMODEL *modelP; //r30
    
    float angle; //f31
    float t; //f30
    float s; //f29
    HuVecF pos3D; //sp+0x14
    HuVecF pos2D; //sp+0x8
    
    angle = 0;
    if(InstMode == 0 || InstMode == 1) {
        while(InstMode != 1) {
            HuPrcVSleep();
        }
        for(i=0; i<=20; i++) {
            s = 1-HuSin((i/20.0)*90.0);
            pos3D = StageMdlTbl[STAGE_MODEL_GUIDE].pos;
            Hu3DModelPosSet(StageMdlId[STAGE_MODEL_GUIDE], pos3D.x, pos3D.y-(s*300), pos3D.z);
            HuPrcVSleep();
        }
        for(i=0; i<=20; i++) {
            s = HuSin((i/20.0)*90.0);
            HuSprGrpScaleSet(InstHelpGrpId, s,  s);
            HuPrcVSleep();
        }
    }
    t = 0;
    while(1) {
        pos3D = StageMdlTbl[STAGE_MODEL_GUIDE].pos;
        Hu3DModelPosSet(StageMdlId[STAGE_MODEL_GUIDE], (t*(5*HuSin(angle/2)))+pos3D.x, (t*(10*HuSin(angle)))+pos3D.y, pos3D.z);
        modelP = &Hu3DData[StageMdlId[STAGE_MODEL_GUIDE]];
        pos3D = modelP->pos;
        pos3D.y += 70;
        Hu3D3Dto2D(&pos3D, HU3D_CAM1, &pos2D);
        HuSprGrpPosSet(InstHelpGrpId, pos2D.x, pos2D.y);
        HuSprGrpScaleSet(InstHelpGrpId, (t*HuSin(angle*2)*0.05)+1, (t*HuSin(angle*2)*0.05)+1);
        angle += 2;
        if(angle > 720) {
            angle -= 720;
        }
        t += 0.002;
        if(t > 1.0) {
            t = 1;
        }
        if(InstMode == 3) {
            break;
        }
        HuPrcVSleep();
    }
    for(i=0; i<=10; i++) {
        s = HuCos((i/10.0)*90.0);
        HuSprGrpScaleSet(InstHelpGrpId, s,  s);
        HuPrcVSleep();
    }
    while(1) {
        HuPrcVSleep();
    }
}

#include "REL/instDll/effect.c"
#include "datanum/effect.h"

void InstToueiMain(void)
{
    s16 i; //r31
    s16 j; //r30
    ANIMDATA *animDot; //r24
    //int dir; //r22
    ANIMDATA *animGlow; //r21
    
    float t; //f31
    float s; //f30
    float angle; //f29
    
    INST_EFFECT *effDot[INST_CHARMDL_MAX]; //sp+0x4C
    INST_EFFECT *effGlow[INST_CHARMDL_MAX]; //sp+0x38
    HuVecF pos; //sp+0x2C
    HuVecF movieScale; //sp+0x20
    HuVecF moviePos; //sp+0x14
    HuVecF dir; //sp+0x8
    
    angle = 0;
    animDot = HuSprAnimDataRead(INST_ANM_eff_dot);
    animGlow = HuSprAnimDataRead(EFFECT_ANM_glow);
    HuDataDirClose(EFFECT_ANM_glow);
    for(i=0; i<INST_CHARMDL_MAX; i++) {
        if(CharMdlId[i] != HU3D_MODELID_NONE) {
            effDot[i] = ParticleDotCreate(animDot, 2, HU3D_CAM1);
            effGlow[i] = ParticleGlowCreate(animGlow, 2, HU3D_CAM1);
        }
    }
    if(InstMode == 0 || InstMode == 1) {
        while(InstMode != 1) {
            HuPrcVSleep();
        }
        for(i=0; i<=30; i++) {
            if(i > 10) {
                t = (i-10)/20.0;
                moviePos.x = moviePos.y = 0;
                moviePos.z = 1600;
                for(j=0; j<INST_CHARMDL_MAX; j++) {
                    if(CharMdlId[j] != HU3D_MODELID_NONE) {
                        pos = StageMdlTbl[j+STAGE_MODEL_TOUEI1].pos;
                        pos.x += (((pos.x < 0) ? -1.0 : 1.0)*(1-HuSin(t*90)))*250;
                        Hu3DModelPosSet(StageMdlId[j+STAGE_MODEL_TOUEI1], pos.x, pos.y, pos.z);
                        VECSubtract(&moviePos, &pos, &dir);
                        Hu3DModelRotSet(StageMdlId[j+STAGE_MODEL_TOUEI1], 0, HuAtan(dir.x, dir.z), 0);
                    }
                }
            }
            movieScale = StageMdlTbl[STAGE_MODEL_MGMOVIE].scale;
            t = HuSin((i/30.0)*90.0);
            Hu3DModelScaleSet(StageMdlId[STAGE_MODEL_MGMOVIE], movieScale.x*t, movieScale.y*t, movieScale.z*t);
            HuPrcVSleep();
        }
    } else {
        moviePos.x = moviePos.y = 0;
        moviePos.z = 1600;
        for(j=0; j<INST_CHARMDL_MAX; j++) {
            if(CharMdlId[j] != HU3D_MODELID_NONE) {
                pos = StageMdlTbl[j+STAGE_MODEL_TOUEI1].pos;
                VECSubtract(&moviePos, &pos, &dir);
                Hu3DModelRotSet(StageMdlId[j+STAGE_MODEL_TOUEI1], 0, HuAtan(dir.x, dir.z), 0);
            }
        }
    }
    s = 0;
    while(1) {
        pos = StageMdlTbl[STAGE_MODEL_MGMOVIE].pos;
        Hu3DModelPosSet(StageMdlId[STAGE_MODEL_MGMOVIE], pos.x, pos.y+(s*(3*HuSin(angle))), pos.z);
        for(i=0; i<INST_CHARMDL_MAX; i++) {
            if(CharMdlId[i] != HU3D_MODELID_NONE) {
                pos = StageMdlTbl[i+STAGE_MODEL_TOUEI1].pos;
                Hu3DModelPosSet(StageMdlId[STAGE_MODEL_TOUEI1+i], pos.x, pos.y+(s*(3*HuSin(angle+(i*120)))), pos.z);
            }
        }
        angle += 3;
        if(angle > 720) {
            angle -= 720;
        }
        s += 0.05;
        if(s > 1.0) {
            s = 1;
        }
        if(InstMode == 3) {
            break;
        }
        HuPrcVSleep();
    }
    for(i=0; i<INST_CHARMDL_MAX; i++) {
        if(CharMdlId[i] != HU3D_MODELID_NONE && !(Hu3DData[StageMdlId[i+STAGE_MODEL_TOUEI1]].attr & HU3D_ATTR_DISPOFF)) {
            Hu3DModelDispOff(StageMdlId[i+STAGE_MODEL_TOUEI1]);
            Hu3DModelCameraSet(CharMdlId[i], HU3D_CAM1);
            if(i < GW_PLAYER_MAX) {
                CharMotionSet(CharNo[i], CharMotId[i][1]);
            } else {
                Hu3DMotionSet(CharMdlId[i], CharMotId[i][1]);
            }
            pos = StageMdlTbl[i+STAGE_MODEL_TOUEI1].pos;
            pos.y -= 70;
            Hu3DModelPosSetV(CharMdlId[i], &pos);
            Hu3DModelScaleSet(CharMdlId[i], 1, 1, 1);
            Hu3DModelAttrReset(CharMdlId[i], HU3D_MOTATTR_LOOP);
            Hu3DMotionTimeSet(CharMdlId[i], Hu3DMotionMaxTimeGet(CharMdlId[i]));
            pos = StageMdlTbl[i+STAGE_MODEL_TOUEI1].pos;
            ParticleDotPosSet(effDot[i], &pos);
            
        }
    }
    if(MgDataP->type == MG_TYPE_KETTOU) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(GwPlayerConf[i].grp < 2) {
                CharFXPlay(GwPlayerConf[i].charNo, CHARVOICEID(2));
            }
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            CharFXPlay(GwPlayerConf[i].charNo, CHARVOICEID(2));
        }
    }
    if(MgDataP->type == MG_TYPE_DONKEY) {
        HuAudFXPlay(MSM_SE_GUIDE_31);
    } else if(MgDataP->type == MG_TYPE_KUPA) {
        HuAudFXPlay(MSM_SE_GUIDE_14);
    }
    for(s=1, j=0; j<=50; j++) {
        t = j/50.0;
        for(i=0; i<INST_CHARMDL_MAX; i++) {
            if(CharMdlId[i] != HU3D_MODELID_NONE) {
                moviePos = StageMdlTbl[STAGE_MODEL_MGMOVIE].pos;
                moviePos.x += 0.3*StageMdlTbl[i+STAGE_MODEL_TOUEI1].pos.x;
                moviePos.y += 0.3*StageMdlTbl[i+STAGE_MODEL_TOUEI1].pos.y;
                moviePos.z -= 300;
                pos = StageMdlTbl[i+STAGE_MODEL_TOUEI1].pos;
                pos.y -= 70;
                VECSubtract(&moviePos, &pos, &dir);
                Hu3DModelRotSet(CharMdlId[i], t*70, ((dir.x < 0) ? -1 : 1)*(t*180), 0);
                VECScale(&dir, &dir, t);
                VECAdd(&pos, &dir, &dir);
                dir.z += 300*HuSin(t*180);
                dir.y += 100*HuSin(t*180);
                Hu3DModelPosSetV(CharMdlId[i], &dir);
                if(t > 0.5) {
                    Hu3DModelScaleSet(CharMdlId[i], s, s, s);
                }
                ParticleGlowPosSet(effGlow[i], &dir);
            }
        }
        if(t > 0.5) {
            s -= 0.02;
        }
        HuPrcVSleep();
    }
    
    while(1) {
        HuPrcVSleep();
    }
}

void InstWinMain(void)
{
    s16 i; //r31
    s16 pageNo; //r30
    u32 ctrlMes; //r29
    u32 instMes; //r28
    s16 j; //r27
    s16 insertMesNo; //r26
    
    float t; //f31
    
    s16 charMes[GW_PLAYER_MAX][GW_PLAYER_MAX]; //sp+0x38
    HuVec2F instSize; //sp+0x30
    HuVec2F ctrlSize; //sp+0x28
    
    HuVec2F instPos; //sp+0x20
    HuVec2F ctrlPos; //sp+0x18
    s16 grpNum[GW_PLAYER_MAX]; //sp+0x10
    
    HUWINID instWinId[2]; //sp+0xC
    HUWINID ctrlWinId[2]; //sp+0x8
    
    static u32 ctrlMesTbl[5] = {
        MESS_MGINSTSYS_RULES_INST,
        MESS_MGINSTSYS_CTRL1_INST,
        MESS_MGINSTSYS_CTRL2_INST,
        MESS_MGINSTSYS_ADVICE1_INST,
        MESS_MGINSTSYS_ADVICE2_INST
    };
    for(i=0; i<2; i++) {
        instSize.x = 400;
        instSize.y = 128;
        instPos.x = 560-instSize.x;
        instPos.y = 440-instSize.y;
        instWinId[i] = HuWinCreate(instPos.x, instPos.y, instSize.x, instSize.y, 0);
        HuWinMesSpeedSet(instWinId[i], 0);
        HuWinMesSet(instWinId[i], InstMesTbl[0]);
        ctrlSize.x = 320;
        ctrlSize.y = 48;
        ctrlPos.x = 560-ctrlSize.x;
        ctrlPos.y = 440-instSize.y-ctrlSize.y;
        ctrlWinId[i] = HuWinCreate(ctrlPos.x, ctrlPos.y, ctrlSize.x, ctrlSize.y, 0);
        HuWinAttrSet(ctrlWinId[i], HUWIN_ATTR_ALIGN_CENTER);
        HuWinMesSpeedSet(ctrlWinId[i], 0);
        HuWinMesSet(ctrlWinId[i], ctrlMesTbl[0]);
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        grpNum[i] = 0;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        charMes[GwPlayerConf[i].grp][grpNum[GwPlayerConf[i].grp]] = GwPlayerConf[i].charNo;
        grpNum[GwPlayerConf[i].grp]++;
    }
    for(i=insertMesNo=0; i<GW_PLAYER_MAX; i++) {
        for(j=0; j<grpNum[i]; j++) {
            HuWinInsertMesSet(instWinId[0], charMes[i][j], (s32)insertMesNo);
            HuWinInsertMesSet(instWinId[1], charMes[i][j], (s32)insertMesNo);
            insertMesNo++;
        }
    }
    HuWinDispOff(instWinId[1]);
    HuWinDispOff(ctrlWinId[1]);
    if(InstMode == 0 || InstMode == 1) {
        HuWinPosSet(ctrlWinId[0], 1000, 0);
        HuWinPosSet(instWinId[0], 1000, 0);
        while(InstMode != 1) {
            HuPrcVSleep();
        }
        HuPrcSleep(20);
        for(i=0; i<=20; i++) {
            HuWinPosSet(ctrlWinId[0], ((1-HuSin((i/20.0)*90))*400)+ctrlPos.x, ctrlPos.y);
            HuWinPosSet(instWinId[0], instPos.x, instPos.y+((1-HuSin((i/20.0)*90))*200));
            HuPrcVSleep();
        }
    }
    pageNo = 0;
    ctrlMes = ctrlMesTbl[0];
    instMes = InstMesTbl[0];
    while(1) {
        if(InstBtnDown & (PAD_TRIGGER_R|PAD_BUTTON_TRIGGER_R)) {
            HuWinPriSet(ctrlWinId[0], 100);
            HuWinPriSet(instWinId[0], 100);
            HuWinPriSet(ctrlWinId[1], 50);
            HuWinPriSet(instWinId[1], 50);
            HuWinMesSet(ctrlWinId[1], ctrlMes);
            HuWinMesSet(instWinId[1], instMes);
            HuWinDispOn(instWinId[1]);
            HuWinDispOn(ctrlWinId[1]);
            HuWinPosSet(ctrlWinId[1], 1000, 0);
            HuWinPosSet(instWinId[1], 1000, 0);
            HuPrcVSleep();
            pageNo++;
            if(pageNo > 4) {
                pageNo = 0;
            }
            if(pageNo == 2 && InstMesTbl[2] == 0) {
                pageNo++;
            }
            if(pageNo == 4 && InstMesTbl[4] == 0) {
                pageNo = 0;
            }
            ctrlMes = ctrlMesTbl[pageNo];
            if(pageNo == 1 && InstMesTbl[2] == 0) {
                ctrlMes = MESS_MGINSTSYS_CTRLALL_INST;
            }
            if(pageNo == 3 && InstMesTbl[4] == 0) {
                ctrlMes = MESS_MGINSTSYS_ADVICEALL_INST;
            }
            HuWinMesSet(ctrlWinId[0], ctrlMes);
            instMes = InstMesTbl[pageNo];
            HuWinMesSet(instWinId[0], instMes);
            HuAudFXPlay(MSM_SE_INST_02);
            for(i=0; i<15; i++) {
                t = i/15.0;
                HuWinPosSet(ctrlWinId[1], ctrlPos.x+(30*HuSin(t*90)), ctrlPos.y+(200*(1-HuCos(t*90))));
                HuWinPosSet(instWinId[1], instPos.x+(30*HuSin(t*90)), instPos.y+(200*(1-HuCos(t*90))));
                HuPrcVSleep();
            }
            HuWinDispOff(instWinId[1]);
            HuWinDispOff(ctrlWinId[1]);
        } else if(InstBtnDown & (PAD_TRIGGER_L|PAD_BUTTON_TRIGGER_L)) {
            HuWinPriSet(ctrlWinId[0], 50);
            HuWinPriSet(instWinId[0], 50);
            HuWinPriSet(ctrlWinId[1], 100);
            HuWinPriSet(instWinId[1], 100);
            HuWinMesSet(ctrlWinId[1], ctrlMes);
            HuWinMesSet(instWinId[1], instMes);
            HuWinDispOn(instWinId[1]);
            HuWinDispOn(ctrlWinId[1]);
            HuWinPosSet(ctrlWinId[1], ctrlPos.x, ctrlPos.y);
            HuWinPosSet(instWinId[1], instPos.x, instPos.y);
            HuPrcVSleep();
            pageNo--;
            if(pageNo < 0) {
                pageNo = 4;
            }
            if(pageNo == 2 && InstMesTbl[2] == 0) {
                pageNo--;
            }
            if(pageNo == 4 && InstMesTbl[4] == 0) {
                pageNo--;
            }
            ctrlMes = ctrlMesTbl[pageNo];
            if(pageNo == 1 && InstMesTbl[2] == 0) {
                ctrlMes = MESS_MGINSTSYS_CTRLALL_INST;
            }
            if(pageNo == 3 && InstMesTbl[4] == 0) {
                ctrlMes = MESS_MGINSTSYS_ADVICEALL_INST;
            }
            HuWinMesSet(ctrlWinId[0], ctrlMes);
            instMes = InstMesTbl[pageNo];
            HuWinMesSet(instWinId[0], instMes);
            HuAudFXPlay(MSM_SE_INST_03);
            for(i=1; i<=15; i++) {
                t = i/15.0;
                HuWinPosSet(ctrlWinId[0], ctrlPos.x+(30*HuCos(t*90)), ctrlPos.y+(200*(1-HuSin(t*90))));
                HuWinPosSet(instWinId[0], instPos.x+(30*HuCos(t*90)), instPos.y+(200*(1-HuSin(t*90))));
                HuPrcVSleep();
            }
            HuWinDispOff(instWinId[1]);
            HuWinDispOff(ctrlWinId[1]);
        }
        if(InstMode == 3) {
            break;
        }
        HuPrcVSleep();
    }
    for(i=0; i<=10; i++) {
        HuWinPosSet(ctrlWinId[0], ((1-HuCos((i/10.0)*90))*400)+ctrlPos.x, ctrlPos.y);
        HuWinPosSet(instWinId[0], instPos.x, instPos.y+((1-HuCos((i/10.0)*90))*200));
        HuPrcVSleep();
    }
    while(1) {
        HuPrcVSleep();
    }
}

void CameraMove(OMOBJ *obj)
{
    HuVecF pos;
    HuVecF offset;
    HuVecF dir;
    HuVecF yOfs;

    float rotZ;
    s8 stickPos;
    if(CameraDebugF) {
        CRotM[1].y += 0.1f * HuPadStkX[0];
        CRotM[1].x += 0.1f * HuPadStkY[0];
        CZoomM[1] += HuPadTrigL[0] / 2;
        CZoomM[1] -= HuPadTrigR[0] / 2;
        if (CZoomM[1] < 100.0f) {
            CZoomM[1] = 100.0f;
        }
        pos.x = CenterM[1].x + (CZoomM[1] * (HuSin(CRotM[1].y) * HuCos(CRotM[1].x)));
        pos.y = (CenterM[1].y + (CZoomM[1] * -HuSin(CRotM[1].x)));
        pos.z = (CenterM[1].z + (CZoomM[1] * (HuCos(CRotM[1].y) * HuCos(CRotM[1].x))));
        offset.x = CenterM[1].x - pos.x;
        offset.y = CenterM[1].y - pos.y;
        offset.z = CenterM[1].z - pos.z;
        dir.x = (HuSin(CRotM[1].y) * HuSin(CRotM[1].x));
        dir.y = HuCos(CRotM[1].x);
        dir.z = (HuCos(CRotM[1].y) * HuSin(CRotM[1].x));
        rotZ = CRotM[1].z;
        yOfs.x = dir.x * (offset.x * offset.x + (1.0f - offset.x * offset.x) * HuCos(rotZ))
            + dir.y * (offset.x * offset.y * (1.0f - HuCos(rotZ)) - offset.z * HuSin(rotZ))
            + dir.z * (offset.x * offset.z * (1.0f - HuCos(rotZ)) + offset.y * HuSin(rotZ));

        yOfs.y = dir.y * (offset.y * offset.y + (1.0f - offset.y * offset.y) * HuCos(rotZ))
            + dir.x * (offset.x * offset.y * (1.0f - HuCos(rotZ)) + offset.z * HuSin(rotZ))
            + dir.z * (offset.y * offset.z * (1.0f - HuCos(rotZ)) - offset.x * HuSin(rotZ));

        yOfs.z = dir.z * (offset.z * offset.z + (1.0f - offset.z * offset.z) * HuCos(rotZ))
            + (dir.x * (offset.x * offset.z * (1.0 - HuCos(rotZ)) - offset.y * HuSin(rotZ))
                + dir.y * (offset.y * offset.z * (1.0 - HuCos(rotZ)) + offset.x * HuSin(rotZ)));

        VECCrossProduct(&dir, &offset, &offset);
        VECNormalize(&offset, &offset);
        stickPos = (HuPadSubStkX[0] & 0xF8);
        if (stickPos != 0) {
            CenterM[1].x += 0.05f * (offset.x * stickPos);
            CenterM[1].y += 0.05f * (offset.y * stickPos);
            CenterM[1].z += 0.05f * (offset.z * stickPos);
        }
        VECNormalize(&yOfs, &offset);
        stickPos = -(HuPadSubStkY[0] & 0xF8);
        if (stickPos != 0) {
            CenterM[1].x += 0.05f * (offset.x * stickPos);
            CenterM[1].y += 0.05f * (offset.y * stickPos);
            CenterM[1].z += 0.05f * (offset.z * stickPos);
        }
    }
}

static void LoadProc(void)
{
    
    OSTick tick;
    LoadDoneF = FALSE;
    tick = OSGetTick();
    LoadDirStat = HuDataDirReadAsync(mgDataTbl[MgNo].dataDir);
    while(!HuDataGetAsyncStat(LoadDirStat)) {
        HuPrcVSleep();
    }
    LoadDoneF = TRUE;
    OSReport("Load Time:%d\n", OSTicksToMilliseconds(OSGetTick()-tick));
    OSReport("Finished!\n");
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}