#include "game/board/main.h"
#include "game/board/model.h"
#include "game/board/audio.h"

#include "game/memory.h"

typedef struct ModelManWork_s {
    unsigned killF : 1;
} MODELMANWORK;

static s16 modelCnt;
static MBMODEL *modelMb;
static OMOBJ *modelManObj;

static void ModelManExec(OMOBJ *obj);
static BOOL InitData(MBMODEL *modelP, int dataNum, BOOL linkF);
static BOOL InitMotionData(MBMODEL *modelP, int motNum, int *motDataNum);

void MBModelInit(void)
{
    MBMODEL *modelP = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MBMODEL)*MB_MODEL_MAX, HU_MEMNUM_OVL);
    int i;
    MODELMANWORK *work;
    
    modelMb = modelP;
    memset(modelMb, 0, sizeof(MBMODEL)*MB_MODEL_MAX);
    modelCnt = 0;
    for(i=0; i<MB_MODEL_MAX; i++, modelP++) {
        modelP->mbId = MB_MODEL_NONE;
        modelP->disp = FALSE;
    }
    modelManObj = MBAddObj(32259, 0, 0, ModelManExec);
    work = omObjGetWork(modelManObj, MODELMANWORK);
    work->killF = FALSE;
    omSetStatBit(modelManObj, OM_STAT_NOPAUSE|OM_STAT_SPRPAUSE);
}

void MBModelClose(void)
{
    int i;
    if(modelManObj) {
        MODELMANWORK *work = omObjGetWork(modelManObj, MODELMANWORK);
        work->killF = TRUE;
    }
    for(i=1; i<MB_MODEL_MAX; i++) {
        MBModelKill(i);
    }
    if(modelMb) {
        HuMemDirectFree(modelMb);
        modelMb = NULL;
    }
}

static void ModelManExec(OMOBJ *obj)
{
    MODELMANWORK *work = omObjGetWork(modelManObj, MODELMANWORK);
    if(work->killF || MBKillCheck()) {
        modelManObj = NULL;
        MBDelObj(obj);
        return;
    } else {
        MBMODEL *modelP;
        int i;
        MBModelManDummy();
        for(modelP=&modelMb[0], i=0; i<MB_MODEL_MAX; i++, modelP++) {
            if(modelP->mbId != MB_MODEL_NONE) {
                u32 attr = Hu3DModelAttrGet(modelP->modelId);
                if(!modelP->disp) {
                    if(!(attr & HU3D_ATTR_DISPOFF)) {
                        Hu3DModelDispOff(modelP->modelId);
                    }
                } else {
                    if(attr & HU3D_ATTR_DISPOFF) {
                        Hu3DModelDispOn(modelP->modelId);
                    }
                }
            }
        }
    }
}

void MBModelManDummy(void)
{
    
}

void MBModelDispOffAll(BOOL dispOff)
{
    MBMODEL *modelP;
    int i;
    if(!modelMb) {
        return;
    }
    for(modelP=&modelMb[0], i=0; i<MB_MODEL_MAX; i++, modelP++) {
        if(modelP->mbId != MB_MODEL_NONE) {
            if(dispOff) {
                modelP->dispOld = modelP->disp;
                modelP->disp = FALSE;
            } else {
                modelP->disp = modelP->dispOld;
            }
        }
    }
}

void MBModelDispOffCamera(u16 cameraBit, BOOL dispOff)
{
    MBMODEL *modelP;
    int i;
    if(!modelMb) {
        return;
    }
    for(modelP=&modelMb[0], i=0; i<MB_MODEL_MAX; i++, modelP++) {
        if(modelP->mbId != MB_MODEL_NONE && (modelP->cameraBit & cameraBit)) {
            if(dispOff) {
                modelP->dispOld = modelP->disp;
                modelP->disp = FALSE;
            } else {
                modelP->disp = modelP->dispOld;
            }
        }
    }
}

void MBModelAmbSetAll(float r, float g, float b)
{
    MBMODEL *modelP;
    for(modelP=&modelMb[0]; modelP<&modelMb[MB_MODEL_MAX]; modelP++) {
        if(modelP->mbId != MB_MODEL_NONE) {
            MBModelAmbSet(modelP->mbId, r, g, b);
        }
    }
}

void MBModelCameraSetAll(u16 cameraBit)
{
    MBMODEL *modelP;
    for(modelP=&modelMb[0]; modelP<&modelMb[MB_MODEL_MAX]; modelP++) {
        if(modelP->mbId != MB_MODEL_NONE && modelP->charNo == CHARNO_NONE) {
            Hu3DModelCameraSet(modelP->modelId, cameraBit);
            modelP->cameraBit = cameraBit;
        }
    }
}

static MBMODELID CreateMBModel(int charNo, int dataNum, int *motDataNum, BOOL linkF)
{
    int i;
    for(i=0; i<MB_MODEL_MAX; i++) {
        if(modelMb[i].mbId == MB_MODEL_NONE) {
            break;
        }
    }
    if(i >= MB_MODEL_MAX) {
        return MB_MODEL_NONE;
    } else {
        MBMODEL *modelP = &modelMb[i];
        memset(modelP, 0, sizeof(MBMODEL));
        modelP->mbId = i+1;
        modelP->charNo = charNo;
        modelCnt++;
        if(InitData(modelP, dataNum, linkF)) {
            int motNum = 0;
            if(motDataNum != NULL) {
                while(motDataNum[motNum] >= 0) {
                    motNum++;
                }
                if(motNum >= MB_MOT_MAX) {
                    motNum = MB_MOT_MAX;
                }
            }
            for(i=0; i<MB_MOT_MAX; i++) {
                modelP->motId[i] = HU3D_MOTID_NONE;
            }
            if(InitMotionData(modelP, motNum, motDataNum)) {
                modelP->disp = TRUE;
                modelP->dispOld = modelP->disp;
                modelP->unkFlag = FALSE;
                modelP->motionOff = FALSE;
                modelP->unkSpeed = 100;
                modelP->scale.x = modelP->scale.y = modelP->scale.z = 1;
                modelP->motNo = 0;
                modelP->cameraBit = HU3D_CAM0;
                Hu3DModelCameraSet(modelP->modelId, modelP->cameraBit);
                Hu3DModelLayerSet(modelP->modelId, 0);
                modelP->unkFlag2 = FALSE;
                modelP->alpha = 255;
                MBModelPosSet(modelP->mbId, 0, 0, 0);
                MBModelRotSet(modelP->mbId, 0, 0, 0);
                MBModelScaleSet(modelP->mbId, 1, 1, 1);
                MBModelOffsetSet(modelP->mbId, 0, 0, 0);
                MBModelAmbSet(modelP->mbId, 1, 1, 1);
                return modelP->mbId;
            }
        }
        MBModelKill(modelP->mbId);
        return MB_MODEL_NONE;
    }
}

MBMODELID MBModelCreate(int dataNum, int *motDataNum, BOOL linkF)
{
    return CreateMBModel(CHARNO_NONE, dataNum, motDataNum, linkF);
}

MBMODELID MBModelCreateChar(int charNo, int dataNum, int *motDataNum, BOOL linkF)
{
    return CreateMBModel(charNo, dataNum, motDataNum, linkF);
}

MBMODELID MBModelCreateParam(MBMODELPARAM *param, HuVecF *pos, HuVecF *rot)
{
    MBMODELID modelId = MBModelCreate(param->dataNum, NULL, param->linkF);
    float unkSpeed;
    if(modelId == MB_MODEL_NONE) {
        return MB_MODEL_NONE;
    }
    MBMotionNoSet(modelId, 0, (param->loopF) ? HU3D_MOTATTR_LOOP : HU3D_MOTATTR_NONE);
    if(!param->motStartF) {
        MBMotionSpeedSet(modelId, 0);
    }
    MBModelStubVal2Set(modelId, param->unkVal);
    MBModelDispSet(modelId, param->dispF);
    if(pos) {
        MBModelPosSetV(modelId, pos);
    }
    if(rot) {
        MBModelRotSetV(modelId, rot);
    }
    OSs16tof32(&param->unkSpeed, &unkSpeed);
    MBModelUnkSpeedSet(modelId, unkSpeed);
    return modelId;
}

void MBModelKill(MBMODELID modelId)
{
    if(modelId <= 0) {
        return;
    } else {
        MBMODEL *modelP = MBModelGet(modelId);
        int i;
        if(!modelP) {
            return;
        }
        if(modelP->charNo == CHARNO_NONE) {
            modelP->motId[0] = HU3D_MOTID_NONE;
            if(modelP->modelId >= 0) {
                Hu3DModelKill(modelP->modelId);
                modelP->modelId = HU3D_MODELID_NONE;
            }
            for(i=1; i<MB_MOT_MAX; i++) {
                if(modelP->motId[i] >= 0) {
                    Hu3DMotionKill(modelP->motId[i]);
                    modelP->motId[i] = HU3D_MOTID_NONE;
                }
                modelP->motData[i] = NULL;
            }
            modelP->data = NULL;
            modelP->dataNum = 0;
        } else {
            for(i=0; i<MB_MOT_MAX; i++) {
                modelP->motId[i] = HU3D_MOTID_NONE;
                modelP->motData[i] = NULL;
            }
            if(modelP->data) {
                if(modelP->modelId >= 0) {
                    modelP->modelId = CHARNO_NONE;
                }
                modelP->data = NULL;
            }
            CharModelKill(modelP->charNo);
            modelP->charNo = CHARNO_NONE;
        }
        modelCnt--;
        modelP->mbId = MB_MODEL_NONE;
        modelP->disp = FALSE;
    }
}

HU3DMODELID MBModelIdGet(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return HU3D_MODELID_NONE;
    } else {
        return modelP->modelId;
    }
}

BOOL MBModelUnkFlagSet(MBMODELID modelId, BOOL flag)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        modelP->unkFlag2 = flag;
        return TRUE;
    }
}

BOOL MBModelZWriteOffSet(MBMODELID modelId, BOOL zWriteOff)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(zWriteOff) {
            Hu3DModelZWriteOff(modelP->modelId);
        } else {
            Hu3DModelZWriteOn(modelP->modelId);
        }
        return TRUE;
    }
}

BOOL MBModelAmbSet(MBMODELID modelId, float r, float g, float b)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        Hu3DModelAmbSet(modelP->modelId, r, g, b);
        return TRUE;
    }
}

BOOL MBModelStubValSet(MBMODELID modelId, int value)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL MBModelStubVal2Set(MBMODELID modelId, int value)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL MBModelUnkSpeedSet(MBMODELID modelId, float speed)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(speed < 0) {
            speed = -1;
        }
        modelP->unkSpeed = speed;
        return TRUE;
    }
}

int MBMotionCreate(MBMODELID modelId, int dataNum)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return MB_MOT_NONE;
    } else {
        int motNo;
        if(modelP->motNum >= MB_MOT_MAX) {
            return MB_MOT_NONE;
        }
        for(motNo=1; motNo<MB_MOT_MAX; motNo++) {
            if(modelP->motId[motNo] == HU3D_MOTID_NONE) {
                break;
            }
        }
        if(motNo >= MB_MOT_MAX) {
            return MB_MOT_NONE; 
        }
        if(modelP->charNo == CHARNO_NONE) {
            modelP->motData[motNo] = HuDataSelHeapReadNum(dataNum, HU_MEMNUM_OVL, HEAP_MODEL);
            if(!modelP->motData[motNo]) {
                return MB_MOT_NONE; 
            }
            modelId = Hu3DJointMotion(modelP->modelId, modelP->motData[motNo]);
        } else {
            modelId = CharMotionCreate(modelP->charNo, dataNum);
            CharMotionDataClose(modelP->charNo);
        }
        modelP->motId[motNo] = modelId;
        modelP->motNum++;
        return motNo;
    }
}

BOOL MBMotionKill(MBMODELID modelId, int motNo)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(motNo < 0 || motNo >= MB_MOT_MAX) {
            return MB_MOT_NONE;
        }
        if(motNo == modelP->motNo && modelP->motNo != 0) {
            MBMotionNoSet(modelId, 0, HU3D_MOTATTR_NONE);
        }
        if(modelP->motId[motNo] > 0) {
            if(modelP->charNo == CHARNO_NONE) {
                Hu3DMotionKill(modelP->motId[motNo]);
            } else {
                CharMotionKill(modelP->charNo, modelP->motId[motNo]);
            }
            modelP->motId[motNo] = HU3D_MOTID_NONE;
        }
        modelP->motData[motNo] = NULL;
        modelP->motNum--;
        return TRUE;
    }
}

BOOL MBMotionStartEndSet(MBMODELID modelId, s16 start, s16 end)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(start >= 0) {
            OSs16tof32(&start, &modelP->motStart);
        }
        if(end >= 0) {
            OSs16tof32(&end, &modelP->motEnd);
        }
        Hu3DMotionStartEndSet(modelP->modelId, modelP->motStart, modelP->motEnd);
        return TRUE;
    }
}

BOOL MBMotionLoopSet(MBMODELID modelId, BOOL loopF)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(loopF) {
            Hu3DModelAttrSet(modelP->modelId, HU3D_MOTATTR_LOOP);
        } else {
            Hu3DModelAttrReset(modelP->modelId, HU3D_MOTATTR_LOOP);
        }
        return TRUE;
    }
}

BOOL MBMotionVoiceOnSet(MBMODELID modelId, int motNo, BOOL voiceOn)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(motNo >= MB_MOT_MAX || motNo < 0) {
            return FALSE;
        }
        if(modelP->charNo == CHARNO_NONE) {
            return FALSE;
        }
        CharMotionVoiceOnSet(modelP->charNo, modelP->motId[motNo], voiceOn);
        return TRUE;
    }
}

BOOL MBMotionNoSet(MBMODELID modelId, int motNo, u32 attr)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(motNo >= MB_MOT_MAX || motNo < 0) {
            return FALSE;
        }
        Hu3DMotionSpeedSet(modelP->modelId, 1);
        if(modelP->charNo == CHARNO_NONE) {
            Hu3DMotionSet(modelP->modelId, modelP->motId[motNo]);
        } else {
            CharMotionSet(modelP->charNo, modelP->motId[motNo]);
        }
        modelP->motStart = 0;
        if(modelP->charNo == CHARNO_NONE) {
            modelP->motEnd = Hu3DMotionMaxTimeGet(modelP->modelId);
        } else {
            modelP->motEnd = CharMotionMaxTimeGet(modelP->charNo);
        }
        modelP->motionOff = FALSE;
        if(attr & (HU3D_MOTATTR_SHIFT_REV|HU3D_MOTATTR_REV)) {
            if(modelP->charNo == CHARNO_NONE) {
                Hu3DMotionTimeSet(modelP->modelId, modelP->motEnd);
            } else {
                CharMotionTimeSet(modelP->charNo, modelP->motEnd);
            }
        }
        Hu3DModelAttrReset(modelP->modelId, HU3D_MOTATTR_PAUSE|HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(modelP->modelId, attr);
        modelP->motNo = motNo;
        return TRUE;
    }
}

int MBMotionShiftIDGet(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return HU3D_MOTID_NONE;
    } else {
        return Hu3DMotionShiftIDGet(modelP->modelId);
    }
}

BOOL MBMotionShiftSet(MBMODELID modelId, int motNo, float start, float end, u32 attr)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(motNo >= MB_MOT_MAX || motNo < 0) {
            return HU3D_MOTID_NONE;
        }
        modelP->motStart = 0;
        if(modelP->charNo == CHARNO_NONE) {
            modelP->motEnd = Hu3DMotionMaxTimeGet(modelP->modelId);
        } else {
            modelP->motEnd = CharMotionMaxTimeGet(modelP->charNo);
        }
        modelP->motionOff = FALSE;
        if(modelP->charNo == CHARNO_NONE) {
            Hu3DMotionShiftSet(modelP->modelId, modelP->motId[motNo], start, end, attr);
        } else {
            CharMotionShiftSet(modelP->charNo, modelP->motId[motNo], start, end, attr);
        }
        modelP->motNo = motNo;
        return TRUE;
    }
}

BOOL MBModelAttrSet(MBMODELID modelId, u32 attr)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        Hu3DModelAttrSet(modelP->modelId, attr);
        return TRUE;
    }
}

BOOL MBModelAttrReset(MBMODELID modelId, u32 attr)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        Hu3DModelAttrReset(modelP->modelId, attr);
        return TRUE;
    }
}

BOOL MBMotionTimeSet(MBMODELID modelId, float time)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(modelP->charNo == CHARNO_NONE) {
            Hu3DMotionTimeSet(modelP->modelId, time);
        } else {
            CharMotionTimeSet(modelP->charNo, time);
        }
        return TRUE;
    }
}

float MBMotionTimeGet(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return -1;
    } else {
        return (modelP->charNo == CHARNO_NONE) ? Hu3DData[modelP->modelId].motWork.time : CharMotionTimeGet(modelP->charNo);
    }
}

float MBMotionMaxTimeGet(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return -1;
    } else {
        return (modelP->charNo == CHARNO_NONE) ? Hu3DMotionMaxTimeGet(modelP->modelId) : CharMotionMaxTimeGet(modelP->charNo);
    }
}

float MBMotionSpeedGet(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return -1;
    } else {
        HU3DMODEL *dataP = &Hu3DData[modelP->modelId];
        return dataP->motWork.speed;
    }
}

BOOL MBMotionSpeedSet(MBMODELID modelId, float speed)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        HU3DMODEL *dataP;
        if(modelP->charNo == CHARNO_NONE) {
            Hu3DMotionSpeedSet(modelP->modelId, speed);
        } else {
            CharMotionSpeedSet(modelP->charNo, speed);
        }
        dataP = &Hu3DData[modelP->modelId];
        dataP->motShiftWork.speed = speed;
        return TRUE;
    }
}

BOOL MBMotionEndCheck(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return TRUE;
    } else {
        return (modelP->charNo == CHARNO_NONE) ? Hu3DMotionEndCheck(modelP->modelId) : CharMotionEndCheck(modelP->charNo);
    }
}

BOOL MBMotionShapeSet(MBMODELID modelId, int motNo, u32 attr)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(modelP->charNo != CHARNO_NONE) {
            return FALSE;
        }
        if(motNo >= MB_MOT_MAX) {
            return FALSE;
        } else {
            HU3DMODEL *dataP = &Hu3DData[modelP->modelId];
            dataP->motShapeWork.speed = 1;
            Hu3DMotionShapeSet(modelP->modelId, modelP->motId[motNo]);
            if(attr & HU3D_MOTATTR_SHAPE_REV) {
                float time = Hu3DMotionMaxTimeGet(modelP->modelId);
                dataP->motShapeWork.time = time;
            }
            Hu3DModelAttrSet(modelP->modelId, attr);
            return TRUE;
        }
    }
}

BOOL MBMotionShapeTimeSet(MBMODELID modelId, double time)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(modelP->charNo != CHARNO_NONE) {
            return FALSE;
        } else {
            HU3DMODEL *dataP = &Hu3DData[modelP->modelId];
            dataP->motShapeWork.time = time;
            return TRUE;
        }
    }
}

float MBMotionShapeTimeGet(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return -1;
    } else {
        if(modelP->charNo != CHARNO_NONE) {
            return -1;
        } else {
            HU3DMODEL *dataP = &Hu3DData[modelP->modelId];
            return dataP->motShapeWork.time;
        }
    }
}

float MBMotionShapeMaxTimeGet(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return -1;
    } else {
        if(modelP->charNo != CHARNO_NONE) {
            return -1;
        } else {
            HU3DMODEL *dataP = &Hu3DData[modelP->modelId];
            if(dataP->motIdShape == HU3D_MOTID_NONE) {
                return 0;
            }
            dataP = &Hu3DData[modelP->modelId];
            return dataP->motShapeWork.end;
        }
    }
}

BOOL MBMotionShapeSpeedSet(MBMODELID modelId, float speed)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(modelP->charNo != CHARNO_NONE) {
            return FALSE;
        } else {
            HU3DMODEL *dataP = &Hu3DData[modelP->modelId];
            dataP->motShapeWork.speed = speed;
            return TRUE;
        }
    }
}

float MBMotionShapeSpeedGet(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return -1;
    } else {
        HU3DMODEL *dataP = &Hu3DData[modelP->modelId];
        return dataP->motShapeWork.speed;
    }
}

BOOL MBMotionShapeEndCheck(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return TRUE;
    } else {
        if(modelP->charNo != CHARNO_NONE) {
            return TRUE;
        } else {
            HU3DMODEL *dataP = &Hu3DData[modelP->modelId];
            if(dataP->attr & HU3D_MOTATTR_SHAPE_REV) {
                return dataP->motShapeWork.time <= 0.0f;
            } else {
                return MBMotionShapeMaxTimeGet(modelId) <= dataP->motShapeWork.time;
            }
        }
    }
}

BOOL MBMotionShapeStartEndSet(MBMODELID modelId, s16 start, s16 end)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP || modelP->charNo != CHARNO_NONE) {
        return FALSE;
    } else {
        if(start >= 0) {
            OSs16tof32(&start, &modelP->motStart);
        }
        if(end >= 0) {
            OSs16tof32(&end, &modelP->motEnd);
        }
        Hu3DMotionShapeStartEndSet(modelP->modelId, modelP->motStart, modelP->motEnd);
        return FALSE;
    }
}

BOOL MBModelAlphaSet(MBMODELID modelId, u8 alpha)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        float tpLvl;
        modelP->alpha = alpha;
        OSu8tof32(&alpha, &tpLvl);
        tpLvl = (1.0f/255.0f)*tpLvl;
        Hu3DModelTPLvlSet(modelP->modelId, tpLvl);
        return TRUE;
    }
}

BOOL MBModelMtxSet(MBMODELID modelId, Mtx *mtx)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(!mtx) {
            return FALSE;
        } else {
            HU3DMODEL *dataP = &Hu3DData[modelP->modelId];
            MTXCopy(*mtx, dataP->mtx);
            return TRUE;
        }
    }
}

BOOL MBModelMtxGet(MBMODELID modelId, Mtx *mtx)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        if(!mtx) {
            return MB_MODEL_NONE;
        } else {
            HU3DMODEL *dataP = &Hu3DData[modelP->modelId];
            MTXCopy(dataP->mtx, *mtx);
            return TRUE;
        }
    }
}

BOOL MBModelRotYSet(MBMODELID modelId, float rotY)
{
    HuVecF rot;
    MBModelRotGet(modelId, &rot);
    return MBModelRotSet(modelId, rot.x, MBAngleWrap(rotY), rot.z);
}

float MBModelRotYGet(MBMODELID modelId)
{
    HuVecF rot;
    MBModelRotGet(modelId, &rot);
    return rot.y;
}

BOOL MBModelPosSetV(MBMODELID modelId, HuVecF *pos)
{
    if(!pos) {
        return FALSE;
    } else {
        return MBModelPosSet(modelId, pos->x, pos->y, pos->z);
    }
}

BOOL MBModelPosSet(MBMODELID modelId, float x, float y, float z)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        modelP->pos.x = x;
        modelP->pos.y = y;
        modelP->pos.z = z;
        Hu3DModelPosSet(modelP->modelId,
            modelP->pos.x+modelP->offset.x,
            modelP->pos.y+modelP->offset.y,
            modelP->pos.z+modelP->offset.z);
        return TRUE;
    }
}

BOOL MBModelRotSetV(MBMODELID modelId, HuVecF *rot)
{
    if(!rot) {
        return -1;
    } else {
        return MBModelRotSet(modelId, rot->x, rot->y, rot->z);
    }
}

BOOL MBModelRotSet(MBMODELID modelId, float x, float y, float z)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        modelP->rot.x = x;
        modelP->rot.y = y;
        modelP->rot.z = z;
        Hu3DModelRotSetV(modelP->modelId, &modelP->rot);
        return TRUE;
    }
}

BOOL MBModelScaleSetV(MBMODELID modelId, HuVecF *scale)
{
    if(!scale) {
        return FALSE;
    } else {
        return MBModelScaleSet(modelId, scale->x, scale->y, scale->z);
    }
}

BOOL MBModelScaleSet(MBMODELID modelId, float x, float y, float z)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        modelP->scale.x = x;
        modelP->scale.y = y;
        modelP->scale.z = z;
        Hu3DModelScaleSetV(modelP->modelId, &modelP->scale);
        return TRUE;
    }
}

BOOL MBModelPosGet(MBMODELID modelId, HuVecF *pos)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP || !pos) {
        return FALSE;
    } else {
        *pos = modelP->pos;
        return TRUE;
    }
}

BOOL MBModelRotGet(MBMODELID modelId, HuVecF *rot)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP || !rot) {
        return FALSE;
    } else {
        *rot = modelP->rot;
        return TRUE;
    }
}

BOOL MBModelScaleGet(MBMODELID modelId, HuVecF *scale)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP || !scale) {
        return FALSE;
    } else {
        *scale = modelP->scale;
        return TRUE;
    }
}

void MBModelOffsetSet(MBMODELID modelId, float x, float y, float z)
{
    MBMODEL *modelP = MBModelGet(modelId);
    Vec pos;
    modelP->offset.x = x;
    modelP->offset.y = y;
    modelP->offset.z = z;
    MBModelPosGet(modelId, &pos);
    MBModelPosSetV(modelId, &pos);
}

void MBModelOffsetSetV(MBMODELID modelId, HuVecF *offset)
{
    MBModelOffsetSet(modelId, offset->x, offset->y, offset->z);
}

void MBModelOffsetGet(MBMODELID modelId, HuVecF *offset)
{
    MBMODEL *modelP = MBModelGet(modelId);
    *offset = modelP->offset;
}

BOOL MBModelDispSet(MBMODELID modelId, BOOL dispF)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        modelP->disp = dispF;
        return TRUE;
    }
}

BOOL MBModelDispGet(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        return modelP->disp;
    }
}

BOOL MBModelCameraSet(MBMODELID modelId, u16 cameraBit)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        Hu3DModelCameraSet(modelP->modelId, cameraBit);
        modelP->cameraBit = cameraBit;
        return TRUE;
    }
}

BOOL MBModelLayerSet(MBMODELID modelId, u8 layer)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        modelP->layer = layer;
        Hu3DModelLayerSet(modelP->modelId, layer);
        return TRUE;
    }
}

int MBModelLayerGet(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return -1;
    } else {
        return modelP->layer;
    }
}

void MBModelLayerSetAll(int layerNo)
{
    MBMODEL *modelP;
    for(modelP=&modelMb[0]; modelP<&modelMb[MB_MODEL_MAX]; modelP++) {
        if(modelP->mbId != MB_MODEL_NONE && modelP->charNo == CHARNO_NONE) {
            Hu3DModelLayerSet(modelP->modelId, layerNo);
        }
    }
}

BOOL MBModelHookSet(MBMODELID modelId, char *objName, MBMODELID hookMdlId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    MBMODEL *hookMdlP = MBModelGet(hookMdlId);
    if(!modelP || !hookMdlP || !objName) {
        return FALSE;
    } else {
        Hu3DModelHookSet(modelP->modelId, objName, hookMdlP->modelId);
        MBModelPosSet(hookMdlId, 0, 0, 0);
        return TRUE;
    }
}

BOOL MBModelHookReset(MBMODELID modelId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        Hu3DModelHookReset(modelP->modelId);
        return TRUE;
    }
}

BOOL MBModelHookObjReset(MBMODELID modelId, char *objName)
{
    MBMODEL *modelP = MBModelGet(modelId);
    if(!modelP) {
        return FALSE;
    } else {
        Hu3DModelHookObjReset(modelP->modelId, objName);
        return TRUE;
    }
}

MBMODEL *MBModelGet(MBMODELID modelId)
{
    MBMODEL *modelP;
    if(modelId <= 0 || modelId > MB_MODEL_MAX || !modelMb) {
        return NULL;
    }
    modelP = &modelMb[modelId-1];
    if(modelP->mbId != modelId) {
        return NULL;
    } else {
        return modelP;
    }
}

static MBMODEL *SearchModelDataNum(int dataNum)
{
    MBMODEL *modelP;
    for(modelP=&modelMb[0]; modelP<&modelMb[MB_MODEL_MAX]; modelP++) {
        if(modelP->dataNum == dataNum) {
            return modelP;
        }
    }
    return NULL;
}

static BOOL InitData(MBMODEL *modelP, int dataNum, BOOL linkF)
{
    MBMODEL *linkModelP = NULL;
    if(linkF && modelP->charNo == CHARNO_NONE) {
        linkModelP = SearchModelDataNum(dataNum);
        if(linkModelP) {
            modelP->modelId = Hu3DModelLink(linkModelP->modelId);
            modelP->data = NULL;
            modelP->dataNum = 0;
            return TRUE;
        }
    }
    if(modelP->charNo == CHARNO_NONE) {
        modelP->data = HuDataSelHeapReadNum(dataNum, HU_MEMNUM_OVL, HEAP_MODEL);
        if(!modelP->data) {
            s32 modelRestMem = HuRestMemGet(HEAP_MODEL);
            s32 dvdRestMem = HuRestMemGet(HEAP_DVD);
            return FALSE;
        }
    } else {
        modelP->data = NULL;
    }
    if(modelP->charNo == CHARNO_NONE) {
        modelP->modelId = Hu3DModelCreate(modelP->data);
    } else {
        modelP->modelId = CharModelCreate(modelP->charNo, CHAR_MODEL1);
    }
    if(modelP->modelId < 0) {
        return FALSE;
    } else {
        Hu3DModelAttrSet(modelP->modelId, HU3D_ATTR_NOCULL);
        modelP->dataNum = dataNum;
        return TRUE;
    }
}

static BOOL InitMotionData(MBMODEL *modelP, int motNum, int *motDataNum)
{
    void *dataP;
    modelP->motId[0] = Hu3DData[modelP->modelId].motId;
    if(motDataNum == NULL) {
        modelP->motNum = 1;
        return TRUE;
    } else {
        void *dataP = NULL;
        int i;
        HU3DMOTID motId;
        for(i=0; i<motNum; i++) {
            if(modelP->charNo == CHARNO_NONE) {
                dataP = HuDataSelHeapReadNum(motDataNum[i], HU_MEMNUM_OVL, HEAP_MODEL);
                if(dataP == NULL) {
                    return -1;
                }
                motId = Hu3DJointMotion(modelP->modelId, dataP);
            } else {
                motId = CharMotionCreate(modelP->charNo, motDataNum[i]);
            }
            if(motId < 0) {
                return -1;
            }
            modelP->motId[i+1] = motId;
            modelP->motData[i+1] = dataP;
        }
        modelP->motNum = motNum+1;
        return TRUE;
    }
}

void MBModelStub(void)
{
    
}

int MBModelSePlay(MBMODELID modelId, s16 seId)
{
    MBMODEL *modelP = MBModelGet(modelId);
    return MBAudFXPosPlay(seId, &modelP->pos);
}