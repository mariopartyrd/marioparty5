#include "REL/instDll.h"

HU3DMODELID StageMdlId[STAGE_MODEL_MAX];
HU3DMOTID StageMotId[STAGE_MOT_MAX];
static HU3DMODELID DummyMdlId;

STAGE_MODEL StageMdlTbl[] = {
    { INST_HSF_stage1, STAGE_MODEL_NONE, { -500, 0, 0 }, { 0, 0, 0 }, { 1, 2, 2 }, HU3D_CAM0 },
    { INST_HSF_stage2, STAGE_MODEL_NONE, { 500, 0, 0 }, { 0, 0, 0 }, { 1, 2, 2 }, HU3D_CAM0 },
    { INST_HSF_mgmovie, STAGE_MODEL_NONE, { 0, 157, 0 }, { 0, 0, 0 }, { 2.5f, 2.5f, 2.5f }, HU3D_CAM1 },
    { INST_HSF_touei, STAGE_MODEL_NONE, { -420, 157, 0 }, { 0, 0, 0 }, { 0.6f, 0.6f, 0.3f }, HU3D_CAM1 },
    { INST_HSF_touei, STAGE_MODEL_NONE, { -290, 157, 0 }, { 0, 0, 0 }, { 0.6f, 0.6f, 0.3f }, HU3D_CAM1 },
    { INST_HSF_touei, STAGE_MODEL_NONE, { 290, 157, 0 }, { 0, 0, 0 }, { 0.6f, 0.6f, 0.3f }, HU3D_CAM1 },
    { INST_HSF_touei, STAGE_MODEL_NONE, { 420, 157, 0 }, { 0, 0, 0 }, { 0.6f, 0.6f, 0.3f }, HU3D_CAM1 },
    { INST_HSF_touei, STAGE_MODEL_NONE, { -1000, 0, 0 }, { 0, 0, 0 }, { 1.1f, 1.1f, 0.3f }, HU3D_CAM1 },
    { INST_HSF_guide_choru, STAGE_MODEL_NONE, { -356, -240, 0 }, { 0, 5, 0 }, { 2.0f, 2.0f, 2.0f }, HU3D_CAM1 },
    { INST_HSF_sora1, STAGE_MODEL_NONE, { 0, 0, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, HU3D_CAM1 },
    { HU_DATANUM_NONE, STAGE_MODEL_NONE, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, HU3D_CAM1 },
};

void StageModelCreate(STAGE_MODEL *model)
{
    STAGE_MODEL *modelP;
    s16 i;
    for(modelP=model, i=0; modelP->dataNum != HU_DATANUM_NONE; modelP++, i++) {
        if(modelP->motMdl == STAGE_MODEL_NONE) {
            StageMdlId[i] = Hu3DModelCreateData(modelP->dataNum);
            Hu3DModelPosSetV(StageMdlId[i], &modelP->pos);
            Hu3DModelRotSetV(StageMdlId[i], &modelP->rot);
            Hu3DModelScaleSetV(StageMdlId[i], &modelP->scale);
            Hu3DModelCameraSet(StageMdlId[i], modelP->cameraBit);
            Hu3DModelLayerSet(StageMdlId[i], 1);
        } else {
            StageMotId[i] = Hu3DJointMotionData(StageMdlId[modelP->motMdl], modelP->dataNum);
        }
    }
}