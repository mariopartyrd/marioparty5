#include "REL/fileseldll.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/wipe.h"
#include "game/saveload.h"
#include "game/pad.h"

#include "relprolog.inc"

#include "messnum/mpsystem.h"
#include "messnum/syshelp.h"


static OMCAMERAVIEW cameraView1 = {
    { 0, 250, 0 },
    { 0, 0, 0 },
    7000
};

static OMCAMERAVIEW cameraView2 = {
    { 0, 235, 0 },
    { 0, 0, 0 },
    1200
};

static OMOBJ *outview;
static OMOBJMAN *objman;
HU3DMODELID charToueiMdlId[44];
static HU3DMOTID charMotId[11][2];
static HU3DMODELID charMdlId[11];
HU3DMODELID toueiMdlId[SAVE_BOX_MAX];
static HUSPRGRPID backGrpId;
static HUSPRGRPID boxGrpId[SAVE_BOX_MAX];
static HUWINID boxNameWinId[SAVE_BOX_MAX];
static HUSPRGRPID circleInGrpId;
FILESEL_BOX fileselBox[SAVE_BOX_MAX];
HUWINID infoWinId;
HUWINID helpWinId;
static float wipeTime;
static u16 charHideF;

void FileselMain(void);
void FileselInit(void);

void ObjectSetup(void)
{
    int lightId;
    OSReport("******* FileSelect ObjectSetup *********\n");
    objman = omInitObjMan(50, 512);
    omGameSysInit(objman);
    lightId = Hu3DGLightCreate(-1000, 1500, 1000, 1, -1.5, -1, 208, 160, 160);
    Hu3DGLightInfinitytSet(lightId);
    lightId = Hu3DGLightCreate(0, 0, 1000, 0, 0, -1, 96, 96, 96);
    Hu3DGLightInfinitytSet(lightId);
    outview = omAddObj(objman, 32730, 0, 0, omOutViewMulti);
    outview->work[0] = 2;
    Hu3DCameraCreate(HU3D_CAM0|HU3D_CAM1);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 5, 20, 15000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0, 0, 640, 480, 0, 1);
    Hu3DCameraPerspectiveSet(HU3D_CAM1, 30, 20, 15000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM1, 0, 0, 640, 480, 0, 1);
    omCameraViewSetMulti(HU3D_CAM0, &cameraView1);
    omCameraViewSetMulti(HU3D_CAM1, &cameraView2);
    HuPrcChildCreate(FileselMain, 256, 12288, 0, HuPrcCurrentGet());
    Hu3DBGColorSet(0, 0, 0);
    HuWinInit(1);
    HuAudSStreamPlay(MSM_STREAM_FILESEL);
    OSReport("SAVE DATA %x\n", 15642);
}

s32 FileselSelect(void);
void FileselFileLoad(s16 boxNo);

void FileselMain(void)
{
    float scale;
    int result;
    int i;
    FileselInit();
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 1);
    WipeWait();
    scale = 1.5f;
    HuAudFXPlay(MSM_SE_MENU_01);
    for(i=1; scale > 0.02; i++) {
        scale *= 0.92;
        HuSprGrpScaleSet(circleInGrpId, scale, scale);
        HuPrcVSleep();
    }
    HuSprGrpScaleSet(circleInGrpId, 0, 0);
    HuWinExOpen(infoWinId);
    SLSaveFlagSet(TRUE);
    repeat:
    result = FileBoxInit(HUWIN_NONE);
    if(result != CARD_RESULT_WRONGDEVICE) {
        if(result == CARD_RESULT_BUSY) {
            FileCommonInit();
            result = CARD_RESULT_READY;
            SLSaveFlagSet(FALSE);
        } else {
            result = FileselSelect();
            if(result == FILESEL_RESULT_CANCEL) {
                goto end;
            }
            if(result == CARD_RESULT_NOCARD) {
                goto repeat;
            }
            if(result == CARD_RESULT_BUSY) {
                FileCommonInit();
                result = CARD_RESULT_READY;
                SLSaveFlagSet(FALSE);
                for(i=0; i<SAVE_BOX_MAX; i++) {
                    HuSprDispOff(boxGrpId[i], 0);
                }
            }
        }
        HuCardUnMount(curSlotNo);
        charHideF = TRUE;
        if(result >= 0) {
            FileselFileLoad(result);
            while(1) {
                HuPrcVSleep();
            }
        }
    }
    end:
    HuCardUnMount(curSlotNo);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 60);
    HuAudSStreamAllFadeOut(1000);
    WipeWait();
    HuDataDirClose(DATA_filesel);
    omOvlReturn(1);
    while(1) {
        HuPrcVSleep();
    }
}

void FileselBackUpdate(void);
void FileselCharUpdate(void);

void FileselInit(void)
{
    GXColor winColor = { 43, 23, 128, 255 };
    s16 i;
    HUSPRGRPID grpId;
    s16 j;
    HUSPRID sprId;
    HUWIN *winP;
    HU3DPROJID projId;
    ANIMDATA *anim;
    ANIMDATA *numAnim;
    ANIMDATA *fileBgAnim;
    ANIMDATA *fileNumAnim;
    ANIMDATA *fileCopyAnim;
    
    HuVecF pos, target, up;
    
    i = Hu3DModelCreateData(FILESEL_HSF_stage);
    Hu3DModelScaleSet(i, 1, 1.5f, 1);
    Hu3DModelAttrSet(i, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(i, HU3D_CAM0);
    projId = Hu3DProjectionCreate(HuSprAnimDataRead(FILESEL_ANM_fade), 1, 100, 10000);
    pos.x = 0;
    pos.y = 50;
    pos.z = 5000;
    target.x = target.z = 0;
    target.y = 250;
    up.x = 0;
    up.z = 0;
    up.y = 1;
    Hu3DProjectionPosSet(projId, &pos, &up, &target);
    Hu3DProjectionTPLvlSet(projId, 1);
    for(i=0; i<7; i++) {
        charToueiMdlId[i] = Hu3DModelCreateData(FILESEL_HSF_mario+(i*3));
        charMotId[i][0] = Hu3DJointMotionData(charToueiMdlId[i], FILESEL_HSF_marioIdle+(i*3));
        Hu3DMotionSet(charToueiMdlId[i], charMotId[i][0]);
        Hu3DModelCameraSet(charToueiMdlId[i], HU3D_CAM0);
        Hu3DModelAttrSet(charToueiMdlId[i], HU3D_MOTATTR_LOOP);
        Hu3DModelPosSet(charToueiMdlId[i], ((i%4)*150)-300, (i/4)*270, 0);
        Hu3DModelScaleSet(charToueiMdlId[i], 0.8f, 0.8f, 0.8f);
    }
    charToueiMdlId[i] = Hu3DModelCreateData(FILESEL_HSF_star);
    Hu3DModelCameraSet(charToueiMdlId[i], HU3D_CAM0);
    j = Hu3DJointMotionData(charToueiMdlId[i], FILESEL_HSF_starMot);
    Hu3DMotionSet(charToueiMdlId[i], j);
    Hu3DModelPosSet(charToueiMdlId[i], ((i%4)*150)-300, (i/4)*270, 0);
    Hu3DModelScaleSet(charToueiMdlId[i], 0.3f, 0.3f, 0.3f);
    Hu3DModelAttrSet(charToueiMdlId[i], HU3D_MOTATTR_LOOP);
    for(i=0; i<10; i++) {
        charMdlId[i] = Hu3DModelCreateData(FILESEL_HSF_mario+(i*3));
        charMotId[i][1] = Hu3DJointMotionData(charMdlId[i], FILESEL_HSF_marioRun+(i*3));
        Hu3DMotionSet(charMdlId[i], charMotId[i][1]);
        Hu3DModelCameraSet(charMdlId[i], HU3D_CAM1);
        Hu3DModelAttrSet(charMdlId[i], HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(charMdlId[i], 1);
        Hu3DModelProjectionSet(charMdlId[i], projId);
    }
    for(i=0; i<SAVE_BOX_MAX; i++) {
        toueiMdlId[i] = Hu3DModelCreateData(FILESEL_HSF_touei);
        Hu3DModelPosSet(toueiMdlId[i], ((i%4)*200)-400, (i/4)*270, 0);
        Hu3DModelScaleSet(toueiMdlId[i], 0.7f, 0.7f, 0.3f);
        Hu3DModelCameraSet(toueiMdlId[i], HU3D_CAM1);
        Hu3DModelLayerSet(toueiMdlId[i], 2);
        Hu3DModelDispOff(toueiMdlId[i]);
    }
    ToueiDispInit();
    HuSprExecLayerCameraSet(32, 2, 1);
    HuSprExecLayerCameraSet(96, 2, 1+HU3D_LAYER_HOOK_POST);
    grpId = HuSprGrpCreate(1);
    circleInGrpId = grpId;
    anim = HuSprAnimDataRead(FILESEL_ANM_circleIn);
    sprId = HuSprCreate(anim, 10, 0);
    HuSprGrpMemberSet(grpId, 0, sprId);
    HuSprDrawNoSet(grpId, 0, 0);
    HuSprGrpPosSet(grpId, 288, 240);
    HuSprGrpScaleSet(grpId, 2, 2);
    grpId = HuSprGrpCreate(1);
    backGrpId = grpId;
    anim = HuSprAnimDataRead(FILESEL_ANM_back);
    sprId = HuSprCreate(anim, 1000, 0);
    HuSprGrpMemberSet(grpId, 0, sprId);
    HuSprDrawNoSet(grpId, 0, 32);
    HuSprPosSet(grpId, 0, 288, 240);
    fileBgAnim = HuSprAnimDataRead(FILESEL_ANM_fileBack);
    numAnim = HuSprAnimDataRead(FILESEL_ANM_num);
    fileNumAnim = HuSprAnimDataRead(FILESEL_ANM_fileNum);
    fileCopyAnim = HuSprAnimDataRead(FILESEL_ANM_fileCopy);
    for(i=0; i<SAVE_BOX_MAX; i++) {
        grpId = HuSprGrpCreate(18);
        boxGrpId[i] = grpId;
        sprId = HuSprCreate(fileBgAnim, 100, 0);
        HuSprGrpMemberSet(grpId, 0, sprId);
        sprId = HuSprCreate(fileNumAnim, 95, i);
        HuSprGrpMemberSet(grpId, 16, sprId);
        HuSprPosSet(grpId, 16, 0, -95);
        sprId = HuSprCreate(fileCopyAnim, 99, 0);
        HuSprGrpMemberSet(grpId, 17, sprId);
        HuSprDispOff(grpId, 17);
        for(j=0; j<10; j++) {
            sprId = HuSprCreate(numAnim, 95, 0);
            HuSprGrpMemberSet(grpId, j+1, sprId);
            HuSprPosSet(grpId, j+1, (j*16)-72, 45);
            HuSprAttrSet(grpId, j+1, HUSPR_ATTR_LINEAR);
        }
        for(j=0; j<5; j++) {
            sprId = HuSprCreate(numAnim, 95, 0);
            HuSprGrpMemberSet(grpId, j+11, sprId);
            HuSprPosSet(grpId, j+11, (j*16)-32, 70);
            HuSprAttrSet(grpId, j+11, HUSPR_ATTR_LINEAR);
        }
        HuSprGrpDrawNoSet(grpId, 96);
        HuSprGrpPosSet(grpId, -1000, -1000);
        boxNameWinId[i] = HuWinCreate(0, 0, 160, 40, 0);
        HuWinAttrSet(boxNameWinId[i], HUWIN_ATTR_ALIGN_CENTER);
        HuWinMesSpeedSet(boxNameWinId[i], 0);
        HuWinMesSet(boxNameWinId[i], FILESEL_NULL_MES);
        HuWinBGTPLvlSet(boxNameWinId[i], 0);
        winP = &winData[boxNameWinId[i]];
        winP->charPadX = 0;
        fileselBox[i].toueiMdlId = toueiMdlId[i];
        fileselBox[i].grpId = grpId;
        
        fileselBox[i].winId = boxNameWinId[i];
    }
    HuPrcChildCreate(FileselBackUpdate, 256, 8192, 0, HuPrcCurrentGet());
    infoWinId = HuWinExCreateFrame(16, 376, 544, 64, HUWIN_SPEAKER_NONE, 0);
    winP = &winData[infoWinId];
    winP->padMask = HUWIN_PLAYER_1;
    HuWinBGColSet(infoWinId, &winColor);
    helpWinId = HuWinCreate(-10000, 344, 544, 40, 0);
    HuWinAttrSet(helpWinId, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(helpWinId, 0);
    HuWinBGTPLvlSet(helpWinId, 0);
    NameEnterInit();
    HuPrcChildCreate(FileselCharUpdate, 256, 8192, 0, HuPrcCurrentGet());
}

void FileselSprUpdate(s16 boxNo)
{
    HUSPRGRPID grpId = fileselBox[boxNo].grpId;
    OSCalendarTime *time = &fileselBox[boxNo].time;
    u8 i;
    char str[8];
    if(!fileselBox[boxNo].validF) {
        fileselBox[boxNo].charNo = 8;
        fileselBox[boxNo].name[0] = 0;
        HuWinMesSet(boxNameWinId[boxNo], MESS_MPSYSTEM_NAME_DEFAULT);
        for(i=0; i<10; i++) {
            HuSprDispOn(grpId, i+1);
            HuSprBankSet(grpId, i+1, 12);
        }
        for(i=0; i<5; i++) {
            HuSprDispOn(grpId, i+11);
            HuSprBankSet(grpId, i+11, 12);
        }
        HuSprBankSet(grpId, 3, 11);
        HuSprBankSet(grpId, 6, 11);
        HuSprBankSet(grpId, 13, 10);
    } else {
        for(i=0; i<10; i++) {
            HuSprDispOn(grpId, i+1);
        }
        for(i=0; i<5; i++) {
            HuSprDispOn(grpId, i+11);
        }
        HuSprBankSet(grpId, 3, 11);
        HuSprBankSet(grpId, 6, 11);
        sprintf(str, "%02d", time->mon+1);
        for(i=0; i<2; i++) {
            HuSprBankSet(grpId, i+1, str[i] & 0xF);
        }
        sprintf(str, "%02d", time->mday);
        for(i=0; i<2; i++) {
            HuSprBankSet(grpId, i+4, str[i] & 0xF);
        }
        sprintf(str, "%04d", time->year);
        for(i=0; i<4; i++) {
            HuSprBankSet(grpId, i+7, str[i] & 0xF);
        }
        HuSprBankSet(grpId, 13, 10);
        sprintf(str, "%02d", time->hour);
        for(i=0; i<2; i++) {
            HuSprBankSet(grpId, i+11, str[i] & 0xF);
        }
        sprintf(str, "%02d", time->min);
        for(i=0; i<2; i++) {
            HuSprBankSet(grpId, i+14, str[i] & 0xF);
        }
        HuWinMesSet(boxNameWinId[boxNo], MESSNUM_PTR(&fileselBox[boxNo].name));
    }
}

void FileselBackUpdate(void)
{
    s16 angle = 0;
    while(1) {
        s16 i;
        for(i=0; i<3; i++) {
            HUSPRGRP *gp = &HuSprGrpData[boxGrpId[i]];
            HUSPRITE *sp = &HuSprData[gp->sprId[0]];
            HuVecF pos2D;
            HuVecF pos3D;
            pos2D.x = gp->pos.x;
            pos2D.y = gp->pos.y-(42*gp->scale.x);
            pos2D.z = 1600;
            Hu3D2Dto3D(&pos2D, HU3D_CAM1, &pos3D);
            Hu3DModelPosSetV(toueiMdlId[i], &pos3D);
            Hu3DModelScaleSet(toueiMdlId[i], gp->scale.x*0.7, gp->scale.y*0.7, gp->scale.x*0.2);
            HuWinPosSet(boxNameWinId[i], gp->pos.x-(80*gp->scale.x), gp->pos.y);
            HuWinScaleSet(boxNameWinId[i], gp->scale.x, gp->scale.y);
            if(sp->attr & HUSPR_ATTR_DISPOFF) {
                Hu3DModelDispOff(toueiMdlId[i]);
                HuWinDispOff(boxNameWinId[i]);
            } else {
                Hu3DModelDispOn(toueiMdlId[i]);
                HuWinDispOn(boxNameWinId[i]);
            }
        }
        HuSprScaleSet(backGrpId, 0, (0.1*HuSin(angle))+0.9, (0.1*HuSin(angle))+0.9);
        angle += 2;
        HuPrcVSleep();
    }
}

s32 FileselCopy(s16 srcBoxNo);
s32 FileselEraseConfirm(s16 boxNo);

s32 FileselSelect(void)
{
    s16 boxNo = 0;
    HUSPRGRP *gp;
    HUWIN *winP = &winData[infoWinId];
    
    s16 i;
    s16 j;
    int result;
    
    s16 diff;
    s16 prevBox;
    
    float weight;
    
    OSTime time;
    if((HuCardCheck() & 0x1) == 0) {
        HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARD_REMOVE);
        HuWinMesWait(infoWinId);
        HuWinHomeClear(infoWinId);
        return CARD_RESULT_NOCARD;
    }
    UnMountCnt = 0;
    HuAudFXPlay(MSM_SE_MENU_04);
    for(i=1; i<=10; i++) {
        weight = i/10.0;
        for(j=0; j<3; j++) {
            HuSprGrpScaleSet(boxGrpId[j], 0.7f, 0.7f);
            HuSprGrpPosSet(boxGrpId[j], 288.0f+((j*180)-180), 150-((1-HuSin(weight*90.0f))*300));
        }
        HuPrcVSleep();
    }
    for(i=1; i<=5; i++) {
         weight = ((i/5.0)*0.3)+0.7;
         HuSprGrpScaleSet(boxGrpId[boxNo], weight, weight);
         HuPrcVSleep();
    }
    prevBox = boxNo;
    repeat:
    HuWinMesSet(infoWinId, MESS_MPSYSTEM_SELECTFILE);
    HuWinMesWait(infoWinId);
    HuWinMesSet(helpWinId, MESS_SYSHELP_FILESEL);
    while(1) {
        if(UnMountCnt) {
            break;
        }
        diff = 0;
        if(HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            diff = -1;
        } else if(HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            diff = 1;
        }
        if(diff) {
            HuAudFXPlay(MSM_SE_MENU_05);
            prevBox = boxNo;
            boxNo += diff;
            if(boxNo < 0) {
                boxNo = SAVE_BOX_MAX-1;
            } else if(boxNo >= SAVE_BOX_MAX) {
                boxNo = 0;
            }
            for(i=1; i<=5; i++) {
                 weight = ((i/5.0)*0.3)+0.7;
                 HuSprGrpScaleSet(boxGrpId[boxNo], weight, weight);
                 weight = 1-((i/5.0)*0.3);
                 HuSprGrpScaleSet(boxGrpId[prevBox], weight, weight);
                 HuPrcVSleep();
            }
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_A) {
            HuAudFXPlay(MSM_SE_CMN_02);
            break;
        } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
            HuAudFXPlay(MSM_SE_CMN_04);
            HuWinHomeClear(helpWinId);
            return FILESEL_RESULT_CANCEL;
        }
        HuPrcVSleep();
    }
    HuWinHomeClear(infoWinId);
    HuWinHomeClear(helpWinId);
    if(UnMountCnt) {
        HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARD_REMOVE);
        HuWinMesWait(infoWinId);
        UnMountCnt = 0;
        goto readFail;
    } else {
        SLCurBoxNoSet(boxNo);
        if(!fileselBox[boxNo].validF) {
            UnMountCnt = 0;
            for(i=1; i<=10; i++) {
                 weight = 1-HuCos((i/10.0)*90.0);
                 HuSprGrpPosSet(boxGrpId[boxNo], 288.0f+((boxNo*180)-180), 150-(weight*300));
                 weight = 0.7*(1-(i/10.0));
                 for(j=0; j<3; j++) {
                     if(boxNo != j) {
                         HuSprGrpScaleSet(boxGrpId[j], weight, weight);
                     }
                 }
                 HuPrcVSleep();
            }
            FileCommonInit();
            result = NameEnterMain(boxNo);
            
            if(result == FILESEL_RESULT_CANCEL) {
                fileRepeat:
                for(i=1; i<=10; i++) {
                     weight = 1-HuSin((i/10.0)*90.0);
                     HuSprGrpPosSet(boxGrpId[boxNo], 288.0f+((boxNo*180)-180), 150-(weight*300));
                     weight = 0.7*(i/10.0);
                     for(j=0; j<3; j++) {
                         if(boxNo != j) {
                             HuSprGrpScaleSet(boxGrpId[j], weight, weight);
                         }
                     }
                     HuPrcVSleep();
                }
                goto repeat;
            } else {
                if(UnMountCnt) {
                    HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARD_REMOVE);
                    HuWinMesWait(infoWinId);
                    UnMountCnt = 0;
                    goto readFail;
                } else {
                    if(!FileTestOpen()) {
                        time = OSGetTime();
                        SLSaveDataMake(TRUE, &time);
                        SLCommonSet();
                        for(i=0; i<3; i++) {
                            if(i != boxNo) {
                                SLSaveEmptyMark(curSlotNo, i);
                            }
                        }
                        SLSdSave(SLCurSlotNoGet(), SLCurBoxNoGet(), &GwSdCommon);
                        result = FileClear(HUWIN_NONE);
                        if(result == FILESEL_RESULT_CANCEL || result == CARD_RESULT_NOFILE || result == CARD_RESULT_IOERROR) {
                            goto readFail;
                        }
                        if(result == CARD_RESULT_BUSY) {
                            goto readBusy;
                        }
                    } else {
                        time = OSGetTime();
                        SLSaveDataMake(FALSE, &time);
                        SLCommonSet();
                        SLSdSave(SLCurSlotNoGet(), SLCurBoxNoGet(), &GwSdCommon);
                        result = FileSaveMesOpen(HUWIN_NONE, MESS_MPSYSTEM_MES_SAVE_SAVE);
                        if(result == FILESEL_RESULT_CANCEL || result == CARD_RESULT_NOFILE || result == CARD_RESULT_IOERROR) {
                            goto readFail;
                        }
                        if(result == CARD_RESULT_BUSY) {
                            goto readBusy;
                        }
                    }
                    fileselBox[boxNo].validF = TRUE;
                    fileselBox[boxNo].charNo = 7;
                    OSTicksToCalendarTime(GwCommon.time, &fileselBox[boxNo].time);
                    memcpy(&fileselBox[boxNo].name, &GwCommon.name, 17);
                    FileselSprUpdate(boxNo);
                    goto fileRepeat;
                }
            }
        } else {
            UnMountCnt = 0;
            if(!fileselBox[0].validF || !fileselBox[1].validF || !fileselBox[2].validF) {
                HuWinMesSet(infoWinId, MESS_MPSYSTEM_FILE_CHOICE);
                HuWinMesWait(infoWinId);
                HuWinChoiceSet(infoWinId, -1);
                while(winP->stat != HUWIN_STAT_NONE) {
                    if(UnMountCnt) {
                        HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARD_REMOVE);
                        HuWinMesWait(infoWinId);
                        UnMountCnt = 0;
                        goto readFail;
                    }
                    HuPrcVSleep();
                }
                result = winP->choice;
                if(UnMountCnt) {
                    HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARD_REMOVE);
                    HuWinMesWait(infoWinId);
                    UnMountCnt = 0;
                    goto readFail;
                } else if(result == -1) {
                    goto repeat;
                } else if(result == 1) {
                    result = FileselCopy(boxNo);
                    if(result == CARD_RESULT_NOFILE || result == CARD_RESULT_IOERROR) {
                        goto readFail;
                    }
                    if(result == CARD_RESULT_BUSY) {
                        goto readBusy;
                    }
                    goto repeat;
                } else if(result == 2) {
                    result = FileselEraseConfirm(boxNo);
                    if(result == FILESEL_RESULT_CANCEL || result == CARD_RESULT_NOFILE || result == CARD_RESULT_IOERROR) {
                        goto readFail;
                    }
                    if(result == CARD_RESULT_BUSY) {
                        goto readBusy;
                    }
                    goto repeat;
                }
                
            } else {
                HuWinMesSet(infoWinId, MESS_MPSYSTEM_FILE_CHOICE_NOCOPY);
                HuWinMesWait(infoWinId);
                HuWinChoiceSet(infoWinId, -1);
                while(winP->stat != HUWIN_STAT_NONE) {
                    if(UnMountCnt) {
                        HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARD_REMOVE);
                        HuWinMesWait(infoWinId);
                        UnMountCnt = 0;
                        goto readFail;
                    }
                    HuPrcVSleep();
                }
                result = winP->choice;
                if(UnMountCnt) {
                    HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARD_REMOVE);
                    HuWinMesWait(infoWinId);
                    UnMountCnt = 0;
                    goto readFail;
                } else if(result == -1) {
                    goto repeat;
                } else if(result == 1) {
                    result = FileselEraseConfirm(boxNo);
                    if(result == FILESEL_RESULT_CANCEL || result == CARD_RESULT_NOFILE || result == CARD_RESULT_IOERROR) {
                        goto readFail;
                    }
                    if(result == CARD_RESULT_BUSY) {
                        goto readBusy;
                    }
                    goto repeat;
                }
            }
        }
    }
    SLCommonLoad();
    SLSdLoad(0, SLCurBoxNoGet(), &GwSdCommon);
    return boxNo;
    readFail:
    HuWinHomeClear(infoWinId);
    for(i=1; i<=10; i++) {
         weight = HuCos((i/10.0)*90.0);

         for(j=0; j<3; j++) {
             gp = &HuSprGrpData[boxGrpId[j]];
             if(!(gp->scale.x <= 0 && gp->scale.y <= 0)) {
                 if(boxNo == j) {
                     HuSprGrpScaleSet(boxGrpId[j], weight, weight);
                 } else {
                     HuSprGrpScaleSet(boxGrpId[j], weight*0.7, weight*0.7);
                 }
             }
         }
         HuPrcVSleep();
         
    }
    HuPrcSleep(30);
    return CARD_RESULT_NOCARD;
    readBusy:
    HuWinHomeClear(infoWinId);
    for(i=1; i<=10; i++) {
         weight = HuCos((i/10.0)*90.0);

         for(j=0; j<3; j++) {
             gp = &HuSprGrpData[boxGrpId[j]];
             if(!(gp->scale.x <= 0 && gp->scale.y <= 0)) {
                 if(boxNo == j) {
                     HuSprGrpScaleSet(boxGrpId[j], weight, weight);
                 } else {
                     HuSprGrpScaleSet(boxGrpId[j], weight*0.7, weight*0.7);
                 }
             }
         }
         HuPrcVSleep();
    }
    return CARD_RESULT_BUSY;
}

void FileselCopyDispSet(s16 boxNo, BOOL focusF);

s32 FileselCopy(s16 srcBoxNo)
{
    s16 i;
    s16 boxNo;
    s16 diff;
    s16 selNum;
    s32 result;
    s16 prevBox;
    int deleteSeNo;
    float scale;
    boxNo = 0;
    FileselCopyDispSet(srcBoxNo, TRUE);
    HuWinInsertMesSet(infoWinId, MESSNUM_PTR(&fileselBox[srcBoxNo].name), 0);
    HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARD_COPY);
    HuWinMesWait(infoWinId);
    HuWinMesSet(helpWinId, MESS_SYSHELP_MENU_SEL);
    for(boxNo = 0; boxNo<SAVE_BOX_MAX; boxNo++) {
        if(!fileselBox[boxNo].validF) {
            break;
        }
    }
    for(i=selNum=0; i<SAVE_BOX_MAX; i++) {
        if(!fileselBox[i].validF) {
            selNum++;
        }
    }
    for(i=1; i<=5; i++) {
        scale = 0.7+((i/5.0)*0.3);
        HuSprGrpScaleSet(boxGrpId[boxNo], scale, scale);
        HuPrcVSleep();
    }
    prevBox = boxNo;
    while(1) {
        if(selNum > 1) {
            diff = 0;
            if(HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                diff = -1;
            } else if(HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                diff = 1;
            }
            if(diff) {
                prevBox = boxNo;
                boxNo += diff;
                if(boxNo < 0) {
                    boxNo = SAVE_BOX_MAX-1;
                } else if(boxNo >= SAVE_BOX_MAX) {
                    boxNo = 0;
                }
                while(fileselBox[boxNo].validF) {
                    boxNo += diff;
                    if(boxNo < 0) {
                        boxNo = SAVE_BOX_MAX-1;
                    } else if(boxNo >= SAVE_BOX_MAX) {
                        boxNo = 0;
                    } 
                }
                for(i=1; i<=5; i++) {
                    scale = 0.7+((i/5.0)*0.3);
                    HuSprGrpScaleSet(boxGrpId[boxNo], scale, scale);
                    scale = 1-((i/5.0)*0.3);
                    HuSprGrpScaleSet(boxGrpId[prevBox], scale, scale);
                    HuPrcVSleep();
                }
            }
        }
        if(UnMountCnt) {
            UnMountCnt = 0;
            FileselCopyDispSet(srcBoxNo, FALSE);
            HuWinHomeClear(helpWinId);
            return CARD_RESULT_NOFILE;
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_A) {
            break;
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_B) {
            goto cancelFile;
        }
        HuPrcVSleep();
    }
    HuWinHomeClear(helpWinId);
    for(i=1; i<=20; i++) {
        scale = 1-HuSin((i/20.0)*90);
        HuSprGrpScaleSet(boxGrpId[boxNo], scale, scale);
        HuSprGrpZRotSet(boxGrpId[boxNo], (i*10));
        HuPrcVSleep();
    }
    HuSprGrpZRotSet(boxGrpId[boxNo], 0);
    memcpy(&saveBuf[curSlotNo][SLBoxDataOffsetGet(boxNo)], &saveBuf[curSlotNo][SLBoxDataOffsetGet(srcBoxNo)], SAVE_BOX_SIZE);
    deleteSeNo = HuAudFXPlay(MSM_SE_MENU_07);
    result = FileSaveMesOpen(HUWIN_NONE, MESS_MPSYSTEM_MES_SAVE_COPY);
    HuAudFXStop(deleteSeNo);
    HuAudFXPlay(MSM_SE_MENU_08);
    if(result != FILESEL_RESULT_CANCEL) {
        if(result == CARD_RESULT_READY) {
            fileselBox[boxNo].validF = TRUE;
            fileselBox[boxNo].charNo = fileselBox[srcBoxNo].charNo;
            fileselBox[boxNo].time = fileselBox[srcBoxNo].time;
            memcpy(&fileselBox[boxNo].name, &fileselBox[srcBoxNo].name, 17);
            FileselSprUpdate(boxNo);
            HuSprGrpScaleSet(boxGrpId[boxNo], 1, 1);
            for(i=1; i<=10; i++) {
                scale = i/10.0;
                HuSprGrpScaleSet(boxGrpId[boxNo], scale*0.7, scale*0.7);
                HuPrcVSleep();
            }
            HuWinMesSet(infoWinId, MESS_MPSYSTEM_COPY_END);
            HuWinMesWait(infoWinId);
            FileselCopyDispSet(srcBoxNo, FALSE);
        } else {
            FileselCopyDispSet(srcBoxNo, FALSE);
            for(i=1; i<=5; i++) {
                scale = ((i/5.0)*0.7);
                HuSprGrpScaleSet(boxGrpId[boxNo], scale, scale);
                HuPrcVSleep();
            }
        }
        return result;
    }
    cancelFile:
    FileselCopyDispSet(srcBoxNo, FALSE);
    HuWinHomeClear(helpWinId);
    for(i=1; i<=5; i++) {
        scale = 1-((i/5.0)*0.3);
        HuSprGrpScaleSet(boxGrpId[boxNo], scale, scale);
        HuPrcVSleep();
    }
    return FILESEL_RESULT_CANCEL;
}

void FileselCopyDispSet(s16 boxNo, BOOL focusF)
{
    s16 i;
    if(focusF) {
        HuSprDispOn(boxGrpId[boxNo], 17);
        for(i=1; i<=10; i++) {
            HuSprTPLvlSet(boxGrpId[boxNo], 17, i/10.0);
            HuPrcVSleep();
        }
    } else {
        for(i=1; i<=10; i++) {
            HuSprTPLvlSet(boxGrpId[boxNo], 17, 1-(i/10.0));
            HuPrcVSleep();
        }
        HuSprDispOff(boxGrpId[boxNo], 17);
    }
}

s32 FileselEraseConfirm(s16 boxNo)
{
    HUWIN *winP = &winData[infoWinId];
    int i;
    s32 result;
    float scale;
    HuWinInsertMesSet(infoWinId, MESSNUM_PTR(&fileselBox[boxNo].name), 0);
    HuWinMesSet(infoWinId, MESS_MPSYSTEM_ERASE_CONFIRM);
    HuWinMesWait(infoWinId);
    HuWinChoiceSet(infoWinId, -1);
    while(winP->stat != HUWIN_STAT_NONE) {
        if(UnMountCnt) {
            HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARD_REMOVE);
            HuWinMesWait(infoWinId);
            UnMountCnt = 0;
            return CARD_RESULT_NOFILE;
        }
        HuPrcVSleep();
    }
    result = winP->choice;
    HuWinHomeClear(infoWinId);
    if(UnMountCnt) {
        HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARD_REMOVE);
        HuWinMesWait(infoWinId);
        UnMountCnt = 0;
        return CARD_RESULT_NOFILE;
    }
    if(result == 0) {
        int deleteSeNo = HuAudFXPlay(MSM_SE_MENU_09);
        SLSaveEmptyMark(curSlotNo, boxNo);
        result = FileSaveMesOpen(HUWIN_NONE, MESS_MPSYSTEM_MES_SAVE_ERASE);
        HuAudFXStop(deleteSeNo);
        if(result == 0) {
            HuAudFXPlay(MSM_SE_MENU_10);
            for(i=1; i<=20; i++) {
                scale = 1-HuSin((i/20.0)*90);
                HuSprGrpScaleSet(boxGrpId[boxNo], scale, scale);
                HuSprGrpZRotSet(boxGrpId[boxNo], i*10);
                HuPrcVSleep();
            }
            HuPrcSleep(30);
            fileselBox[boxNo].validF = FALSE;
            FileselSprUpdate(boxNo);
            HuSprGrpZRotSet(boxGrpId[boxNo], 0);
            HuSprGrpScaleSet(boxGrpId[boxNo], 1, 1);
            for(i=1; i<=10; i++) {
                scale = i/10.0;
                HuSprGrpScaleSet(boxGrpId[boxNo], scale, scale);
                HuPrcVSleep();
            }
            HuWinMesSet(infoWinId, MESS_MPSYSTEM_ERASE_END);
            HuWinMesWait(infoWinId);
        }
    } else {
        result = 0;
    }
    return result;
}

void FileselWipeDraw(HU3DMODEL *modelP, Mtx *mtx);

void FileselFileLoad(s16 boxNo)
{
    s16 i;
    HUSPRGRP *gp;
    s16 j;
    s16 dispNum;
    HUSPRITE *sp;
    s32 statId;
    
    HuVecF posBoxBase;
    HuVecF posBox;
    s16 dispF[SAVE_BOX_MAX];
    
    float scale;
    HuDataDirClose(DATA_filesel);
    statId = HuDataDirReadAsync(DATA_mdsel);
    HuWinExClose(infoWinId);
    for(i=dispNum=0; i<SAVE_BOX_MAX; i++) {
        gp = &HuSprGrpData[boxGrpId[i]];
        sp = &HuSprData[gp->sprId[0]];
        if(gp->scale.x > 0.1 && gp->pos.x > 0 && !(sp->attr & HUSPR_ATTR_DISPOFF)) { 
            dispNum++;
            dispF[i] = TRUE;
        } else {
            dispF[i] = FALSE;
        }
    }
    if(dispNum) {
        for(i=1; i<=10; i++) {
            scale = (1-(i/10.0))*0.7;
            for(j=0; j<SAVE_BOX_MAX; j++) {
                if(dispF[j] && boxNo != j) {
                    HuSprGrpScaleSet(boxGrpId[j], scale, scale);
                }
            }
            HuPrcVSleep();
        }
        HuPrcSleep(10);
        gp = &HuSprGrpData[boxGrpId[boxNo]];
        posBoxBase.x = gp->pos.x;
        posBoxBase.y = gp->pos.y;
        scale = 1;
        HuAudFXPlay(MSM_SE_MENU_11);
        for(i=1; scale > 0.01; i++) {
            scale *= 0.9;
            HuSprGrpScaleSet(boxGrpId[boxNo], scale, scale);
            posBox.x = posBoxBase.x+((288-posBoxBase.x)*(1.0-scale));
            posBox.y = posBoxBase.y+((240-posBoxBase.y)*(1.0-scale));
            HuSprGrpPosSet(boxGrpId[boxNo], posBox.x, posBox.y);
            HuPrcVSleep();
        }
        HuSprGrpScaleSet(boxGrpId[boxNo], 0, 0);
        HuPrcSleep(15);
    } else {
        HuPrcSleep(15);
    }
    wipeTime = 0;
    HuAudFXPlay(MSM_SE_MENU_12);
    i = Hu3DHookFuncCreate(FileselWipeDraw);
    Hu3DModelLayerSet(i, 2);
    Hu3DModelCameraSet(i, HU3D_CAM1);
    HuAudSStreamAllFadeOut(1000);
    HuPrcSleep(30);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 30);
    WipeWait();
    while(!HuDataGetAsyncStat(statId)) {
        HuPrcVSleep();
    }
    omOvlCall(DLL_mdseldll, 0, 0);
}

void FileselWipeDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    GXColor colorN = { 255, 255, 255, 255 };
    Mtx44 proj;
    Mtx modelview;
    float angle, minAngle;
    MTXOrtho(proj, 0, 480, 0, 640, 0, 10);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    MTXIdentity(modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetViewport(0, 0, 640, 480, 0, 1);
    GXSetScissor(0, 0, 640, 480);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
    GXSetChanCtrl(GX_COLOR1A1, 0, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
    GXSetCullMode(GX_CULL_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColor(GX_TEVREG0, colorN);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, 12);
    minAngle = (1-HuCos((wipeTime/60.0)*90))*180;
    angle = (minAngle > 90) ? 90 : minAngle;
    GXPosition2f32(320, 240);
    GXPosition2f32((600*HuSin(angle))+320, (600*-HuCos(angle))+240);
    GXPosition2f32(320, -360);
    GXPosition2f32(320, 240);
    GXPosition2f32((600*-HuSin(angle))+320, (600*-HuCos(angle))+240);
    GXPosition2f32(320, -360);
    angle = (minAngle < 90) ? 0 : (minAngle-90);
    GXPosition2f32(320, 240);
    GXPosition2f32(920, 240);
    GXPosition2f32((600*HuCos(angle))+320, (600*HuSin(angle))+240);
    GXPosition2f32(320, 240);
    GXPosition2f32(-280, 240);
    GXPosition2f32((600*-HuCos(angle))+320, (600*HuSin(angle))+240);
    GXEnd();
    wipeTime++;
    if(wipeTime > 60.0) {
        wipeTime = 60;
    }
}

void FileselCharUpdate(void)
{
    s16 i;
    s16 time;
    float angle[10];
    float speed[10];
    HuVecF pos;
    for(i=0; i<10; i++) {
        angle[i] = frandmod(20)+(i*36);
        Hu3DMotionSet(charMdlId[i], charMotId[i][1]);
        Hu3DModelAttrSet(charMdlId[i], HU3D_MOTATTR_LOOP);
        Hu3DModelDispOff(charMdlId[i]);
        speed[i] = (0.01f*frandmod(30))+0.5;
    }
    speed[0] = 0.8f;
    time = 0;
    while(1) {
        if(time > 60) {
            for(i=0; i<10; i++) {
                pos.x = HuSin(angle[i])*600;
                pos.z = HuCos(angle[i])*100;
                Hu3DModelRotSet(charMdlId[i], 0, 90+angle[i], 0);
                pos.y = 0;
                Hu3DModelPosSetV(charMdlId[i], &pos);
                angle[i] += speed[i];
                if(angle[i] > 360) {
                    angle[i] -= 360;
                }
                if((angle[i] > 85 && angle[i] < 95) || (angle[i] > 265 && angle[i] < 275)) {
                    Hu3DModelDispOn(charMdlId[i]);
                }
            }
        } else {
            time++;
        }
        if(charHideF) {
            for(i=0; i<10; i++) {
                speed[i] = 1.5f;
                Hu3DMotionSpeedSet(charMdlId[i], 1.5f);
                if((angle[i] > 85 && angle[i] < 95) || (angle[i] > 265 && angle[i] < 275)) {
                    Hu3DModelDispOff(charMdlId[i]);
                }
            }
        }
        HuPrcVSleep();
    }
}


void CameraMove(void)
{
    Vec pos;
    Vec offset;
    Vec dir;
    Vec yOfs;

    f32 rotZ;
    s8 stickPos;
    static BOOL moveF = FALSE;

    if(moveF) {
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