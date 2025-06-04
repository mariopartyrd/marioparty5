#include <stdio.h>
#include <string.h>
#include "msm_se.h"
#include "dolphin/mtx/GeoTypes.h"
#include "game/esprite.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/wipe.h"
#include "game/board/camera.h"
#include "game/board/capmachine.h"
#include "game/board/capsule.h"
#include "game/board/com.h"
#include "game/board/masu.h"
#include "game/board/model.h"
#include "game/board/player.h"
#include "game/board/pad.h"
#include "game/board/pause.h"
#include "game/board/status.h"
#include "game/board/tutorial.h"
#include "game/board/window.h"

typedef struct _struct_capMachineWork_0x2C {
    /* 0x00 */ s32 unk_0;                           /* inferred */
    /* 0x04 */ s16 unk_4;                           /* inferred */
    /* 0x06 */ s16 unk_6;                           /* inferred */
    /* 0x08 */ s16 unk_8;                           /* inferred */
    /* 0x0A */ char pad_A[2];
    /* 0x0C */ Point3d unk_C;                       /* inferred */
    /* 0x18 */ s32 unk_18;                          /* inferred */
    /* 0x1C */ f32 unk_1C;                          /* inferred */
    /* 0x20 */ Point3d *unk_20;                     /* inferred */
    /* 0x24 */ f32 unk_24;                          /* inferred */
    /* 0x28 */ s32 unk_28;                          /* inferred */
} capMachineWork_s;  

static capMachineWork_s capMachineWork[CAP_MACHINE_WORK_MAX];
static HUPROCESS *capsuleSelProc[4] = { NULL, NULL, NULL, NULL };
static HUPROCESS *capMachineProc[4] = { NULL, NULL, NULL, NULL };

s32 capsuleCurNo;
s32 capMachineNum;

static HU3DPARMANPARAM storyEffParmanParam = {
    0x1E,
    0x108,
    50.0f,
    0.0f,
    65.0f,
    { 0.0f, 0.0f, 0.0f },
    15.0f,
    0.9f,
    60.0f,
    0.95f,
    1,
    { { 0xFF, 0xFF, 0xFF, 0xFF }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
    { { 0xFF, 0xFF, 0xFF, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
};

BOOL MBMotionShapeTimeSet(MBMODELID modelId, double time);
static Point3d capMachineOfs = { 0.0f, -140.0f, 0.0f };
void CapMachineExec(); 
void CapMachineStoryExec();

//Start The Stirring Sequence
void CapMachineKakimazeStart(capMachineWork_s *arg0) {
    arg0->unk_28 = 60;
    MBAudFXPlay(MSM_SE_CAPSULE_MACHINE_02);
}

//Check If We Can Stir
BOOL CapMachineKakimazeCheck(capMachineWork_s *arg0) {
    return arg0->unk_28 == 0;
}
void MBCapMachineInit(void) {
    capMachineNum = 0;
}

void MBCapMachineClose(void) {
    s32 i;
    capMachineWork_s *var_r30;

    for (i = 0; i < capMachineNum; i++) {
        var_r30 = &capMachineWork[i];
        HuMemDirectFree(var_r30->unk_20);
        var_r30->unk_20 = NULL;
    }
}

s32 CreateStoryEff(void) {
    s32 var_r31;
    s32 var_r30;
    ANIMDATA *var_r29;

    var_r29 = HuSprAnimRead(HuDataSelHeapReadNum(0x210003, 268435456, HEAP_MODEL));
    HuDataDirClose(0x210000);
    var_r31 = Hu3DParManCreate(var_r29, 0x32, &storyEffParmanParam);
    Hu3DParManTimeLimitSet(var_r31, 0x1E);
    var_r30 = Hu3DParManModelIDGet(var_r31);
    Hu3DModelLayerSet(var_r30, 3);
    Hu3DModelCameraSet(var_r30, 1);
    return var_r31;
}

//Todo: Change Type to properly reflect property.
void CapMachineDestroy(void) {
    s32 *var_r31 = HuPrcCurrentGet()->property;
    capMachineProc[*var_r31] = NULL;
    HuMemDirectFree(var_r31);
}

void MBCapMachineCreateAll(void) {
    MASU *var_r30;
    s32 i;
    for (i = 0; i < MBMasuNumGet(0); i++) {
        var_r30 = MBMasuGet(0, i + 1);
        if ((var_r30->flag & 0x01000000)) {
            //Add The Function TO Fix This
            MBCapMachineObjCreate(i + 1);
        }
    }
}


void MBCapMachineExec(int playerNo) {
    s32 *var_r30;
    s32 var_r29;
    s32 var_r28;

    var_r29 = GwSystem.partyF;
    if (var_r29 || !MBPlayerStoryComCheck(playerNo) || GwSystem.turnNo != GwSystem.turnMax) {
        if (MBPlayerCapsuleNumGet(playerNo) < MBPlayerCapsuleMaxGet()) {
            MBWalkNumDispSet(playerNo, 0);
            var_r28 = GwSystem.partyF;
            if (!var_r28  &&  MBPlayerStoryComCheck(playerNo) ) {
                capMachineProc[playerNo] = HuPrcChildCreate(CapMachineStoryExec, 0x2003, 0x4000, 0, mbMainProcess);
            } else {
                capMachineProc[playerNo] = HuPrcChildCreate(CapMachineExec, 0x2003, 0x4000, 0, mbMainProcess);
            }
            var_r30 = HuMemDirectMallocNum(HEAP_HEAP, 4, 0x10000000);
            capMachineProc[playerNo]->property = var_r30;
            HuPrcDestructorSet2(capMachineProc[playerNo], CapMachineDestroy);
            *var_r30 = playerNo;
            while (capMachineProc[playerNo]) {
                HuPrcVSleep();
            }
            MBWalkNumDispSet(playerNo, 1);
        }
    }
}


void MBCapMachineTutorialExec(void) {
    s32 i;
    const static u32 sprTbl[4] = { 0xD90006, 0xD90007, 0xD90008, 0xD90009 }; /* const */
    const static u32 mesTbl[4] = { 0x2A0018, 0x2A0019, 0x2A001A, 0x2A001B };
    s32 var_r29 = MBMasuFlagMatchFind(0, 0x01000000);
    capMachineWork_s * CapMachineWork_ref = capMachineWork;
    for (i = 0; i < capMachineNum; i++, CapMachineWork_ref++) {
        if (CapMachineWork_ref->unk_18 == var_r29) {
            break;
        }
    }

    MBCameraMasuViewSet(MBMasuFlagMatchFindLink(0, var_r29, 0x04000000), NULL, NULL, 2100.0f, -1.0f, 1);
    MBCameraMotionWait();
    if (!_CheckFlag(0x1000E) || !MBTutorialExitFlagGet()) {
        WipeCreate(1, 0x81, 30);
    }
    while(WipeCheckEnd()) {
        HuPrcVSleep();
    }
    MBWinCreate(6, 0x2A0017, -1);
    MBTopWinWait();
    MBAudFXPlay(MSM_SE_CAPSULE_MACHINE_01);
    MBModelAttrReset(CapMachineWork_ref->unk_4, 0x40000040);
    CapMachineWork_ref->unk_28 = 0x3C;
    MBAudFXPlay(MSM_SE_CAPSULE_MACHINE_02);
    while (CapMachineWork_ref->unk_28) {
        HuPrcVSleep();
    }
    MBModelDispSet(CapMachineWork_ref->unk_4, 0);
    MBModelDispSet(CapMachineWork_ref->unk_6, 1);
    MBMotionShapeTimeSet(CapMachineWork_ref->unk_6,  0.0);
    MBModelAttrReset(CapMachineWork_ref->unk_6, 0x40000080);
    HuPrcSleep(30);
    capsuleCurNo = 0;
    CapMachineWork_ref->unk_0 = 0;
    for (i = 0; i < 4; i++) {
        s16 var_r28 = MBTutorialSprDispOn(sprTbl[i]);
        MBWinCreate(6, mesTbl[i], -1);
        MBTopWinWait();
        MBTutorialSprDispOff(var_r28);
    }
    MBWinCreate(6, 0x2A001C, -1);
    MBTopWinWait();

    while (!MBMotionShapeEndCheck(CapMachineWork_ref->unk_6)) {
        HuPrcVSleep();
    }
    MBModelDispSet(CapMachineWork_ref->unk_4, 1);
    MBModelDispSet(CapMachineWork_ref->unk_6, 0);
    MBModelAttrSet(CapMachineWork_ref->unk_4, 0x40000080);
    MBMotionShapeTimeSet(CapMachineWork_ref->unk_4,  0.0);
    MBModelAttrSet(CapMachineWork_ref->unk_4, 0x40000080);
    WipeColorSet(0xFF, 0xFF, 0xFF);
    if (!_CheckFlag(0x1000E)|| !MBTutorialExitFlagGet()) {
        WipeCreate(2, 0x81, 1);
    }
    while (WipeCheckEnd()) {
        HuPrcVSleep();  
    }
}