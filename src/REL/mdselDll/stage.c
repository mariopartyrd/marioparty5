#include "REL/mdselDll.h"
#include "datanum/mdsel.h"

static unsigned int SpriteAnimFile[MDSPR_ANIM_MAX] = {
    MDSEL_ANM_win_frame,
    MDSEL_ANM_win_bg,
    MDSEL_ANM_cursor,
    MDSEL_ANM_cursor_glow,
    MDSEL_ANM_eff_glow,
    MDSEL_ANM_mdparty,
    MDSEL_ANM_mdstory,
    MDSEL_ANM_mdmini,
    MDSEL_ANM_sdroom,
    MDSEL_ANM_mdomake,
    MDSEL_ANM_mdoption
};

static MDSPR_INFO SpriteInfo[MDSPR_MAX] = {
    { MDSPR_ANIM_WIN_FRAME, 2, { 0, -28 }, { 72, 1 }, 1.0f },
    { MDSPR_ANIM_WIN_FRAME, 2, { 0, 32 }, { 72, 1 }, 1.0f },
    { MDSPR_ANIM_WIN_BG, 3, { 0, 0 }, { 1, 8 }, 0.6f },
    { MDSPR_ANIM_CURSOR, 7, { 64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 7, { -64, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_CURSOR, 5, { 64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 5, { -64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 5, { 64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 5, { -64, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_CURSOR, 4, { 64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 4, { -64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 4, { 64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 4, { -64, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_MDPARTY, 6, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_MDSTORY, 6, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_MDMINI, 6, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_SDROOM, 6, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_MDOMAKE, 6, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_MDOPTION, 6, { 0, 0 }, { 1, 1 }, 1.0f },
};

static MDSPR_GRP_INFO SpriteGrpInfo[MDSPR_GRP_MAX] = {
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_WIN, 288, 408),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_CURSOR_LR, 288, 240),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_CURSOR_UDLR, 288, 240),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_MODE, 288, 240),
};

static unsigned int StarMdlFileTbl[MDMODEL_STAR_NUM] = {
    MDSEL_HSF_star_choru,
    MDSEL_HSF_star_neru,
    MDSEL_HSF_star_teru,
    MDSEL_HSF_star_niru,
    MDSEL_HSF_star_haru,
    MDSEL_HSF_star_maru,
    MDSEL_HSF_star_daru
};

static unsigned int CapMdlFileTbl[MDMODEL_CAP_NUM] = {
    MDSEL_HSF_cap_particle,
    MDSEL_HSF_cap_choru,
    MDSEL_HSF_cap_neru,
    MDSEL_HSF_cap_niru,
    MDSEL_HSF_cap_haru,
    MDSEL_HSF_cap_maru,
    MDSEL_HSF_cap_daru
};


void MdSpriteCreate(MDSPRITE_WORK *sprite)
{
    HUSPRGRPID grpId;
    MDSPR_INFO *sprInfo;
    int i;
    MDSPR_GRP_INFO *grpInfo;
    int j;
    int grpNum;
    for(i=0; i<MDSPR_ANIM_MAX; i++) {
        sprite->animP[i] = HuSprAnimRead(HuDataReadNum(SpriteAnimFile[i], HU_MEMNUM_OVL));
        
    }
    for(i=0; i<MDSPR_MAX; i++) {
        sprInfo = &SpriteInfo[i];
        sprite->sprId[i] = HuSprCreate(sprite->animP[sprInfo->animNo], 0, 0);
    }
    for(grpInfo=&SpriteGrpInfo[0], i=0; i<MDSPR_GRP_MAX; i++, grpInfo++) {
        grpNum = grpInfo->end-grpInfo->start+1;
        grpId = HuSprGrpCreate(grpNum);
        sprite->grpId[i] = grpId;
        for(sprInfo=&SpriteInfo[grpInfo->start], j=0; j<grpNum; j++, sprInfo++) {
            HuSprGrpMemberSet(grpId, j, sprite->sprId[grpInfo->start+j]);
            HuSprPriSet(grpId, j, sprInfo->pri);
            HuSprDispOn(grpId, j);
            HuSprPosSet(grpId, j, sprInfo->pos.x, sprInfo->pos.y);
            HuSprZRotSet(grpId, j, 0);
            HuSprScaleSet(grpId, j, sprInfo->scale.x, sprInfo->scale.y);
            HuSprColorSet(grpId, j, 255, 255, 255);
            HuSprTPLvlSet(grpId, j, sprInfo->tpLvl);
        }
        HuSprGrpPosSet(grpId, grpInfo->posX, grpInfo->posY);
        HuSprGrpCenterSet(grpId, 0, 0);
        HuSprGrpZRotSet(grpId, 0);
        HuSprGrpScaleSet(grpId, 1, 1);
        HuSprGrpDrawNoSet(grpId, 0);
        HuSprGrpScissorSet(grpId, 0, 0, 640, 480);
    }
    grpId = sprite->grpId[MDSPR_GRP_CURSOR_LR];
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_LR, MDSPR_CURSOR_LR_R), 3);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_LR, MDSPR_CURSOR_LR_L), 2);
    grpId = sprite->grpId[MDSPR_GRP_CURSOR_UDLR];
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R), 3);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L), 2);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U), 0);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D), 1);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R), 3);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L), 2);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_U), 0);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_D), 1);
    HuSprAttrSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R), HUSPR_ATTR_ADDCOL);
    HuSprAttrSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L), HUSPR_ATTR_ADDCOL);
    HuSprAttrSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_U), HUSPR_ATTR_ADDCOL);
    HuSprAttrSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_D), HUSPR_ATTR_ADDCOL);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_LR, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_UDLR, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_WIN, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_MODE, FALSE);
}

void MdSpriteGrpDispSet(MDSPRITE_WORK *sprite, int grpNo, BOOL dispF)
{
    int i;
    MDSPR_GRP_INFO *grpInfo;
    HUSPRGRPID grpId;
    int grpNum;
    grpInfo = &SpriteGrpInfo[grpNo];
    grpId = sprite->grpId[grpNo];
    grpNum = grpInfo->end-grpInfo->start+1;
    for(i=0; i<grpNum; i++) {
        if(dispF == FALSE) {
            HuSprDispOff(grpId, i);
        } else if(dispF == TRUE) {
            HuSprDispOn(grpId, i);
        }
    }
}

HuVec2F *MdSpriteGrpPosGet(MDSPRITE_WORK *sprite, int grpNo)
{
    HUSPRGRP *grp = &HuSprGrpData[sprite->grpId[grpNo]];
    return &grp->pos;
}

HuVec2F *MdSpriteGrpScaleGet(MDSPRITE_WORK *sprite, int grpNo)
{
    HUSPRGRP *grp = &HuSprGrpData[sprite->grpId[grpNo]];
    return &grp->scale;
}

HuVec2F *MdSpritePosGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo)
{
    HUSPRGRP *grp = &HuSprGrpData[sprite->grpId[grpNo]];
    HUSPRITE *spr = &HuSprData[grp->sprId[memberNo]];
    return &spr->pos;
}

HuVec2F *MdSpriteScaleGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo)
{
    HUSPRGRP *grp = &HuSprGrpData[sprite->grpId[grpNo]];
    HUSPRITE *spr = &HuSprData[grp->sprId[memberNo]];
    return &spr->scale;
}

float *MdSpriteAlphaGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo)
{
    HUSPRGRP *grp = &HuSprGrpData[sprite->grpId[grpNo]];
    HUSPRITE *spr = &HuSprData[grp->sprId[memberNo]];
    return &spr->a;
}

static void FontBGFilter(HU3DMODEL *modelP, Mtx *mtx);
static void Layer2Hook(s16 layerNo);
static void Layer1Hook(s16 layerNo);

void MdModelCreate(MDMODEL_WORK *model, MDCAMERA_WORK *camera)
{
    int i;
    model->mdlId[MDMODEL_STAGE] = Hu3DModelCreateData(MDSEL_HSF_stage);
    Hu3DModelDispOff(model->mdlId[MDMODEL_STAGE]);
    Hu3DModelLayerSet(model->mdlId[MDMODEL_STAGE], 0);
    Hu3DModelCameraSet(model->mdlId[MDMODEL_STAGE], camera->camera[MDCAMERA_MAIN].bit);
    Hu3DModelPosSet(model->mdlId[MDMODEL_STAGE], 0, 0, 0);
    Hu3DModelRotSet(model->mdlId[MDMODEL_STAGE], 0, 0, 0);
    Hu3DModelScaleSet(model->mdlId[MDMODEL_STAGE], 1, 1, 1);
    Hu3DModelAttrSet(model->mdlId[MDMODEL_STAGE], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrReset(model->mdlId[MDMODEL_STAGE], HU3D_MOTATTR_PAUSE);
    Hu3DMotionSpeedSet(model->mdlId[MDMODEL_STAGE], 0.2f);
    for(i=0; i<MDMODEL_STAR_NUM; i++) {
        model->mdlId[MDMODEL_STAR_START+i] = Hu3DModelCreateData(StarMdlFileTbl[i]);
        Hu3DModelDispOff(model->mdlId[MDMODEL_STAR_START+i]);
        if(i == MDMODEL_STAR_TERU-MDMODEL_STAR_START) {
            Hu3DModelLayerSet(model->mdlId[MDMODEL_STAR_START+i], 0);
            Hu3DModelCameraSet(model->mdlId[MDMODEL_STAR_START+i], camera->camera[MDCAMERA_GUIDE].bit);
        } else {
            Hu3DModelLayerSet(model->mdlId[MDMODEL_STAR_START+i], 2);
            Hu3DModelCameraSet(model->mdlId[MDMODEL_STAR_START+i], camera->camera[MDCAMERA_MAIN].bit);
        }
        Hu3DModelPosSet(model->mdlId[MDMODEL_STAR_START+i], 0, 0, 0);
        Hu3DModelRotSet(model->mdlId[MDMODEL_STAR_START+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_STAR_START+i], 1, 1, 1);
        Hu3DModelAttrSet(model->mdlId[MDMODEL_STAR_START+i], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(model->mdlId[MDMODEL_STAR_START+i], HU3D_MOTATTR_PAUSE);
        Hu3DMotionSpeedSet(model->mdlId[MDMODEL_STAR_START+i], 1.0f);
    }
    for(i=0; i<MDMODEL_CAP_NUM; i++) {
        model->mdlId[MDMODEL_CAP_START+i] = Hu3DModelCreateData(CapMdlFileTbl[i]);
        Hu3DModelDispOff(model->mdlId[MDMODEL_CAP_START+i]);
        Hu3DModelLayerSet(model->mdlId[MDMODEL_CAP_START+i], 2);
        Hu3DModelCameraSet(model->mdlId[MDMODEL_CAP_START+i], camera->camera[MDCAMERA_MAIN].bit);
        Hu3DModelPosSet(model->mdlId[MDMODEL_CAP_START+i], 0, 0, 0);
        Hu3DModelRotSet(model->mdlId[MDMODEL_CAP_START+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_CAP_START+i], 1, 1, 1);
        Hu3DModelAttrReset(model->mdlId[MDMODEL_CAP_START+i], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(model->mdlId[MDMODEL_CAP_START+i], HU3D_MOTATTR_PAUSE);
        Hu3DMotionSpeedSet(model->mdlId[MDMODEL_CAP_START+i], 1.0f);
    }
    model->mdlId[MDMODEL_FONTBGFILTER] = Hu3DHookFuncCreate(FontBGFilter);
    Hu3DModelLayerSet(model->mdlId[MDMODEL_FONTBGFILTER], 7);
    Hu3DModelCameraSet(model->mdlId[MDMODEL_FONTBGFILTER], camera->camera[MDCAMERA_GUIDE].bit);
    Hu3DModelDispOn(model->mdlId[MDMODEL_FONTBGFILTER]);
    model->motId[MDMODEL_MOT_STAR_CHORU] = Hu3DJointMotionData(model->mdlId[MDMODEL_STAR_CHORU], MDSEL_HSF_star_choru_mot);
    model->motId[MDMODEL_MOT_STAR_NERU] = Hu3DJointMotionData(model->mdlId[MDMODEL_STAR_NERU], MDSEL_HSF_star_neru_mot);
    model->motId[MDMODEL_MOT_STAR_TERU] = Hu3DJointMotionData(model->mdlId[MDMODEL_STAR_TERU], MDSEL_HSF_star_teru_mot);
    model->motId[MDMODEL_MOT_STAR_TERU_TALK] = Hu3DJointMotionData(model->mdlId[MDMODEL_STAR_TERU], MDSEL_HSF_star_teru_mot_talk);
    model->motId[MDMODEL_MOT_STAR_NIRU] = Hu3DJointMotionData(model->mdlId[MDMODEL_STAR_NIRU], MDSEL_HSF_star_niru_mot);
    model->motId[MDMODEL_MOT_STAR_HARU] = Hu3DJointMotionData(model->mdlId[MDMODEL_STAR_HARU], MDSEL_HSF_star_haru_mot);
    model->motId[MDMODEL_MOT_STAR_MARU] = Hu3DJointMotionData(model->mdlId[MDMODEL_STAR_MARU], MDSEL_HSF_star_maru_mot);
    model->motId[MDMODEL_MOT_STAR_DARU] = Hu3DJointMotionData(model->mdlId[MDMODEL_STAR_DARU], MDSEL_HSF_star_daru_mot);
    Hu3DMotionSet(model->mdlId[MDMODEL_STAR_CHORU], model->motId[MDMODEL_MOT_STAR_CHORU]);
    Hu3DMotionSet(model->mdlId[MDMODEL_STAR_NERU], model->motId[MDMODEL_MOT_STAR_NERU]);
    Hu3DMotionSet(model->mdlId[MDMODEL_STAR_TERU], model->motId[MDMODEL_MOT_STAR_TERU]);
    Hu3DMotionSet(model->mdlId[MDMODEL_STAR_NIRU], model->motId[MDMODEL_MOT_STAR_NIRU]);
    Hu3DMotionSet(model->mdlId[MDMODEL_STAR_HARU], model->motId[MDMODEL_MOT_STAR_HARU]);
    Hu3DMotionSet(model->mdlId[MDMODEL_STAR_MARU], model->motId[MDMODEL_MOT_STAR_MARU]);
    Hu3DMotionSet(model->mdlId[MDMODEL_STAR_DARU], model->motId[MDMODEL_MOT_STAR_DARU]);
    for(i=0; i<MDMODEL_MAX; i++) {
        HU3DMODEL *modelP = &Hu3DData[model->mdlId[i]];
        model->posP[i] = &modelP->pos;
        model->rotP[i] = &modelP->rot;
        model->scaleP[i] = &modelP->scale;
    }
    Hu3DLayerHookSet(1, Layer2Hook);
    Hu3DLayerHookSet(0, Layer1Hook);
}

static void FontBGFilter(HU3DMODEL *modelP, Mtx *mtx)
{
    MDDEBUGFONT_WORK *debugFont = sysP->debugFont;
    GXColor color = { 0, 0, 0, 128 };
    s16 width = 0;
    if(debugFont->printF == TRUE) {
        Mtx44 proj;
        width = debugFont->maxLen*(int)(8.0f*debugFont->size);
        GXSetViewport(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
        MTXOrtho(proj, 0, 480, 0, 640, 0, -16777215);
        GXSetProjection(proj, GX_ORTHOGRAPHIC);
        MTXIdentity(*mtx);
        GXLoadPosMtxImm(*mtx, GX_PNMTX0);
        GXSetColorUpdate(TRUE);
        GXSetAlphaUpdate(FALSE);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetNumChans(1);
        GXSetNumTexGens(0);
        GXSetNumTevStages(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
        GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColor(GX_COLOR1, color);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3s16(debugFont->posX, debugFont->posY+debugFont->ofsY, 0);
        GXPosition3s16(debugFont->posX+width, debugFont->posY+debugFont->ofsY, 0);
        GXPosition3s16(debugFont->posX+width, debugFont->posY+debugFont->rowOfsY, 0);
        GXPosition3s16(debugFont->posX, debugFont->posY+debugFont->rowOfsY, 0);
        GXEnd();
    }
}

static void Layer2Hook(s16 layerNo)
{
    if(Hu3DCameraNo == 0) {
        Hu3DZClear();
    }
}

static void Layer1Hook(s16 layerNo)
{
    if(Hu3DCameraNo == 1) {
        Hu3DZClear();
    }
}

static void ParticleHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx);
static void ParticleHook2(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx);
static void ParticleUpdate(MDPARTICLE_BUFFER *buffer);

static void TickParticle(MDPARTICLE_BUFFER *buffer, int num);
static void UpdateParticleData(MDPARTICLE_BUFFER *buffer);


void MdParticleCreate(MDPARTICLE_WORK *particle)
{
    MDPARTICLE_BUFFER *buffer; //r31
    MDPARTICLE *data; //r30
    MDPARTICLE_PARAM *param; //r29
    int i; //r28
    MDCAMERA_WORK *camera; //r26
    MDSPRITE_WORK *sprite; //r25
    MDMODEL_WORK *model; //r24
    camera = sysP->camera;
    sprite = sysP->sprite;
    model = sysP->model;
    param = &particle->param;
    param->baseAlpha = 224;
    param->fadeInSpeed2 = 8;
    param->fadeOutSpeed1 = 128;
    param->fadeInSpeed1 = 4;
    param->fadeOutSpeed2 = 16;
    param->unk14 = 0.6f;
    param->scaleSpeed1 = 2;
    param->scaleSpeed2 = 5;
    param->unk20 = 0.3f;
    param->color.r = 80;
    param->color.g = 198;
    param->color.b = 255;
    for(data=&particle->data[0], i=0; i<MDPARTICLE_MAX; i++, data++) {
        data->mode = 0;
        data->delay = 0;
        data->angle = 0;
        data->radius = 0;
        data->tpLvl = 0;
        data->radiusSpeed = 0;
        data->angleSpeed = 0;
        data->velY = 0;
        data->scale = 0;
        data->zRot = 0;
        data->pos.x = 0;
        data->pos.y = 0;
        data->pos.z = 0;
        data->color.r = data->color.g = data->color.b = data->color.a = 0;
    }
    buffer = &particle->buffer[0];
    buffer->pauseF = FALSE;
    buffer->animP = sprite->animP[MDSPR_ANIM_EFF_GLOW];
    buffer->num = MDPARTICLE_MAX/2;
    buffer->particle = &particle->data[0];
    buffer->param = param;
    buffer->hook = ParticleHook;
    buffer->updateHook = ParticleUpdate;
    model->mdlId[MDMODEL_PARTICLE] = buffer->mdlId = Hu3DParticleCreate(buffer->animP, buffer->num);
    buffer->hookWork = Hu3DData[buffer->mdlId].hookData;
    buffer->dataBuf = buffer->hookWork->data;
    Hu3DParticleHookSet(buffer->mdlId, buffer->hook);
    Hu3DParticleBlendModeSet(buffer->mdlId, HU3D_PARTICLE_BLEND_ADDCOL );
    Hu3DModelLayerSet(buffer->mdlId, 0);
    Hu3DModelCameraSet(buffer->mdlId, camera->camera[MDCAMERA_MAIN].bit);
    
    TickParticle(buffer, 240);
    UpdateParticleData(buffer);
}

static void UpdateParticleData(MDPARTICLE_BUFFER *buffer)
{
    MDPARTICLE *particle; //r31
    HU3DPARTICLEDATA *dataBuf; //r30
    int i; //r28
    
    for(particle=&buffer->particle[0], dataBuf=&buffer->dataBuf[0], i=0; i<buffer->num; i++, particle++, dataBuf++) {
        dataBuf->scale = particle->scale;
        dataBuf->zRot = particle->zRot;
        dataBuf->pos = particle->pos;
        dataBuf->color = particle->color;
    }
    DCStoreRangeNoSync(&buffer->dataBuf[0], sizeof(HU3DPARTICLEDATA)*buffer->num);
}

static void TickParticle(MDPARTICLE_BUFFER *buffer, int num)
{
    int i;
    for(i=0; i<num; i++) {
        buffer->updateHook(buffer);
    }
}

void MdParticleDataUpdate(int bufIdx)
{
    MDPARTICLE_BUFFER *buffer = &sysP->particle->buffer[bufIdx];
    buffer->updateHook(buffer);
    UpdateParticleData(buffer);
}

static void ParticleHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx)
{
    MdParticleDataUpdate(0);
}

static void ParticleHook2(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx)
{
    MdParticleDataUpdate(1);
}

static void ParticleUpdate(MDPARTICLE_BUFFER *buffer)
{
    MDPARTICLE *particle; //r31
    MDPARTICLE_PARAM *param; //r30
    int i; //r29
    int third; //r27
    int border; //r26
    
    param = buffer->param;
    third = buffer->num*0.33f;
    border = buffer->num-16;
    if(buffer->pauseF != TRUE) {
        for(particle=buffer->particle, i=0; i<buffer->num; i++, particle++) {
            switch(particle->mode) {
                case 0:
                    particle->mode++;
                    break;
                
                case 1:
                    particle->delay = frandmod(256);
                    particle->mode++;
                    break;
                
                case 2:
                    if(--particle->delay < 0) {
                        particle->delay = 0;
                        particle->mode++;
                    }
                    break;
                
                case 3:
                    particle->tpLvl = 0;
                    particle->radius = 0;
                    particle->angle = frandmod(360);
                    if(i < third) {
                        particle->velY = 0.5f+((frandmod(100)+1)/500.0f);
                        particle->radiusSpeed = 0.5f+((frandmod(100)+1)/200.0f);
                        particle->angleSpeed = -0.5f+((frandmod(100)+1)/100.0f);
                    } else {
                        particle->velY = 0.5f+((frandmod(100)+1)/100.0f);
                        particle->radiusSpeed = 0.5f+((frandmod(100)+1)/200.0f);
                        particle->angleSpeed = -0.5f+((frandmod(100)+1)/100.0f);
                    }
                    if(i >= border) {
                        particle->radius = 384;
                        particle->velY = 0;
                        particle->radiusSpeed = 0;
                        particle->angleSpeed = -0.5f+((frandmod(100)+1)/50.0f);
                    }
                    if(i < third) {
                        particle->color.r = 128;
                        particle->color.g = 64;
                        particle->color.b = 255;
                    } else {
                        particle->color.r = param->color.r;
                        particle->color.g = param->color.g;
                        particle->color.b = param->color.b;
                    }
                    particle->color.a = 0;
                    particle->scale = 0;
                    particle->zRot = 0;
                    particle->pos.x = 0;
                    particle->pos.y = 0;
                    particle->pos.z = 0;
                    if(i >= border) {
                        particle->pos.y = -200.0f+((frandmod(4000)+1)/10.0f);
                    }
                    particle->mode++;
                    break;
                
                
                case 4:
                case 5:
                case 6:
                    if(particle->mode == 4) {
                        if(i < third) {
                            particle->scale += param->scaleSpeed2;
                        } else {
                            particle->scale += param->scaleSpeed1;
                        }
                    }
                    particle->pos.x = particle->radius*HuCos(particle->angle);
                    particle->pos.y += particle->velY;
                    particle->pos.z = particle->radius*HuSin(particle->angle);
                    particle->radius += particle->radiusSpeed;
                    particle->angle += particle->angleSpeed;
                    if(particle->mode == 5) {
                        if(i < third) {
                            if(particle->pos.y >= 10) {
                                particle->mode = 6;
                            }
                        } else if(i < border) {
                            if(particle->pos.y >= 250) {
                                particle->mode = 6;
                            }
                        }
                    }
                    if(particle->mode == 4) {
                        if(i < third) {
                            particle->tpLvl += 1.0f/param->fadeInSpeed1;
                        } else {
                            particle->tpLvl += 1.0f/param->fadeInSpeed2;
                        }
                        if(particle->tpLvl >= 1) {
                            particle->tpLvl = 1;
                            particle->mode = 5;
                        }
                    } else if(particle->mode == 6) {
                        if(i < third) {
                            particle->tpLvl -= 1.0f/param->fadeOutSpeed2;
                        } else {
                            particle->tpLvl -= 1.0f/param->fadeOutSpeed1;
                        }
                        if(particle->tpLvl <= 0) {
                            particle->tpLvl = 0;
                        }
                    }
                    particle->color.a = param->baseAlpha*particle->tpLvl;
                    if(particle->mode == 6) {
                        if(particle->tpLvl <= 0) {
                            particle->tpLvl = 0;
                            particle->mode = 3;
                        }
                    }
                    break;
            }
        }
    }
}

void fn_1_C6B4(void)
{
    
}