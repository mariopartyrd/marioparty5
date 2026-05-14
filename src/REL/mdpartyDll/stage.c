#include "REL/mdpartyDll.h"
#include "datanum/mdparty.h"

static void CapChr0MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void CapChr1MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void CapChr2MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void CapChr3MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void CapChr4MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void CapChr5MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void CapChr6MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void CapChr7MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void CapChr8MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void CapChr9MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);

static void Ring1PMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void Ring2PMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void Ring3PMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
static void Ring4PMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);

static unsigned int SpriteAnimFile[MDSPR_ANIM_MAX] = {
    MDPARTY_ANM_win_frame,
    MDPARTY_ANM_win_bg,
    MDPARTY_ANM_cursor,
    MDPARTY_ANM_cursor_glow,
    MDPARTY_ANM_playerno,
    MDPARTY_ANM_comdif,
    MDPARTY_ANM_playerno_finger,
    MDPARTY_ANM_turn,
    MDPARTY_ANM_bonus,
    MDPARTY_ANM_team_red,
    MDPARTY_ANM_team_blue,
    MDPARTY_ANM_map1_name,
    MDPARTY_ANM_map2_name,
    MDPARTY_ANM_map3_name,
    MDPARTY_ANM_map4_name,
    MDPARTY_ANM_map5_name,
    MDPARTY_ANM_map6_name,
    MDPARTY_ANM_map7_name,
    MDPARTY_ANM_handicap_num,
    MDPARTY_ANM_handicap,
    MDPARTY_ANM_effect_dot,
    MDPARTY_ANM_effect_glow,
    MDPARTY_ANM_mode_name
};

static MDSPR_INFO SpriteInfo[MDSPR_MAX] = {
    { MDSPR_ANIM_WIN_FRAME, 4, { 0, -28 }, { 72, 1 }, 1.0f },
    { MDSPR_ANIM_WIN_FRAME, 4, { 0, 32 }, { 72, 1 }, 1.0f },
    
    { MDSPR_ANIM_WIN_BG, 5, { 0, 0 }, { 1, 8 }, 0.6f },
    
    { MDSPR_ANIM_CURSOR, 14, { 64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 14, { -64, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_CURSOR, 7, { 64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 7, { -64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 7, { 64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 7, { -64, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_CURSOR, 6, { 64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 6, { -64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 6, { 64, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_CURSOR, 6, { -64, 0 }, { 1, 1 }, 1.0f },

    { MDSPR_ANIM_PLAYERNO, 8, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_PLAYERNO, 8, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_PLAYERNO, 8, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_PLAYERNO, 8, { 0, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_COMDIF, 9, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_COMDIF, 9, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_COMDIF, 9, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_COMDIF, 9, { 0, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_TEAM_RED, 10, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_TEAM_BLUE, 10, { 0, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_MAP1_NAME, 11, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_MAP2_NAME, 11, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_MAP3_NAME, 11, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_MAP4_NAME, 11, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_MAP5_NAME, 11, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_MAP6_NAME, 11, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_MAP7_NAME, 11, { 0, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_HANDICAP_NUM, 12, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_HANDICAP_NUM, 12, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_HANDICAP_NUM, 12, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_HANDICAP_NUM, 12, { 0, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_HANDICAP, 13, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_HANDICAP, 13, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_HANDICAP, 13, { 0, 0 }, { 1, 1 }, 1.0f },
    { MDSPR_ANIM_HANDICAP, 13, { 0, 0 }, { 1, 1 }, 1.0f },
    
    { MDSPR_ANIM_MODE_NAME, 15, { 0, 0 }, { 1, 1 }, 1.0f },
};

static MDSPR_GRP_INFO SpriteGrpInfo[MDSPR_GRP_MAX] = {
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_WIN, 288, 408),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_CURSOR_LR, 288, 240),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_CURSOR_UDLR, 288, 240),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_PLAYERNO, 0, 0),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_COMDIF, 0, 0),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_TEAM, 0, 0),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_MAP_NAME, 0, 0),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_HANDICAP, 0, 0),
    MDSPR_GRP_INFO_DEFINE(MDSPR_GRP_NAME, 288, 240),
};

static GXColor RingColorTbl[8] = {
    { 255, 128, 0, 0 },
    { 255, 128, 0, 0 },
    { 255, 128, 0, 0 },
    { 255, 128, 0, 0 },
    { 255, 128, 0, 0 },
    { 0, 255, 255, 0 },
    { 255, 0, 0, 0 },
    { 0, 128, 255, 0 },
};

static HU3DMATHOOK CapChrMatHookTbl[10] = {
    CapChr0MatHook,
    CapChr1MatHook,
    CapChr2MatHook,
    CapChr3MatHook,
    CapChr4MatHook,
    CapChr5MatHook,
    CapChr6MatHook,
    CapChr7MatHook,
    CapChr8MatHook,
    CapChr9MatHook
};

static HU3DMATHOOK RingMatHookTbl[4] = {
    Ring1PMatHook,
    Ring2PMatHook,
    Ring3PMatHook,
    Ring4PMatHook
};

static unsigned int ChrModelFileTbl[10] = {
    MDPARTY_HSF_mario,
    MDPARTY_HSF_luigi,
    MDPARTY_HSF_peach,
    MDPARTY_HSF_yoshi,
    MDPARTY_HSF_wario,
    MDPARTY_HSF_daisy,
    MDPARTY_HSF_waluigi,
    MDPARTY_HSF_kinopio,
    MDPARTY_HSF_teresa,
    MDPARTY_HSF_minikoopa
};

static HuVecF ChrModelPosTbl[10] = {
    { -250, 0, 0 },
    { -125, 0, 0 },
    { 0, 0, 0 },
    { 125, 0, 0 },
    { 250, 0, 0 },
    { -250, -250, 0 },
    { -125, -250, 0 },
    { 0, -250, 0 },
    { 125, -250, -1800 },
    { 250, -250, -1000 }
};

static unsigned int CapPlayerFileTbl[2] = {
    MDPARTY_HSF_cap_man,
    MDPARTY_HSF_cap_com
};

static unsigned int CapGameTypeFileTbl[2] = {
    MDPARTY_HSF_cap_broyal,
    MDPARTY_HSF_cap_tmatch
};

static unsigned int MapFileTbl[7] = {
    MDPARTY_HSF_map1,
    MDPARTY_HSF_map2,
    MDPARTY_HSF_map3,
    MDPARTY_HSF_map4,
    MDPARTY_HSF_map5,
    MDPARTY_HSF_map6,
    MDPARTY_HSF_map7
};

static unsigned int CapPackFileTbl[5] = {
    MDPARTY_HSF_cap_pack0,
    MDPARTY_HSF_cap_pack1,
    MDPARTY_HSF_cap_pack2,
    MDPARTY_HSF_cap_pack3,
    MDPARTY_HSF_cap_pack4
};

static unsigned int StarMotionFileTbl[3] = {
    MDPARTY_HSF_star_idle,
    MDPARTY_HSF_star_talk,
    MDPARTY_HSF_star_talk_fast
};

static unsigned int ChrMotionFileTbl[20] = {
    MDPARTY_HSF_mario_idle,
    MDPARTY_HSF_mario_win,
    
    MDPARTY_HSF_luigi_idle,
    MDPARTY_HSF_luigi_win,
    
    MDPARTY_HSF_peach_idle,
    MDPARTY_HSF_peach_win,
    
    MDPARTY_HSF_yoshi_idle,
    MDPARTY_HSF_yoshi_win,
    
    MDPARTY_HSF_wario_idle,
    MDPARTY_HSF_wario_win,
    
    MDPARTY_HSF_daisy_idle,
    MDPARTY_HSF_daisy_win,
    
    MDPARTY_HSF_waluigi_idle,
    MDPARTY_HSF_waluigi_win,
    
    MDPARTY_HSF_kinopio_idle,
    MDPARTY_HSF_kinopio_win,
    
    MDPARTY_HSF_teresa_idle,
    MDPARTY_HSF_teresa_win,
    
    MDPARTY_HSF_minikoopa_idle,
    MDPARTY_HSF_minikoopa_win,
};

static s16 ModelSprAnimTbl[6] = {
    MDSPR_ANIM_PLAYERNO_FINGER,
    MDSPR_ANIM_PLAYERNO_FINGER,
    MDSPR_ANIM_PLAYERNO_FINGER,
    MDSPR_ANIM_PLAYERNO_FINGER,
    MDSPR_ANIM_TURN,
    MDSPR_ANIM_BONUS
};

static s16 ModelAnimModelTbl[6] = {
    MDMODEL_FINGER_1P,
    MDMODEL_FINGER_2P,
    MDMODEL_FINGER_3P,
    MDMODEL_FINGER_4P,
    MDMODEL_CAP_TURN,
    MDMODEL_CAP_BONUS
};

static char *ModelAnimBmpNameTbl[6] = {
    "dummy",
    "dummy",
    "dummy",
    "dummy",
    "IA8_usa10",
    "bonusyes"
};

static HuVecF TexBufChrOfs[10] = {
    { 0, 15, 0 },
    { 0, 18, 0 },
    { 0, 12, 0 },
    { 0, 20, 0 },
    { 0, 15, 0 },
    { 0, 6, 0 },
    { 0, -5, 0 },
    { 0, 20, 0 },
    { 0, 70, 0 },
    { 0, 10, 0 },
};

static char *TexBufChrObjName[10] = {
    "root_head",
    "root_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
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
    
    grpId = sprite->grpId[MDSPR_GRP_HANDICAP];
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_HANDICAP, MDSPR_HANDICAP_NUM_1P), 0);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_HANDICAP, MDSPR_HANDICAP_NUM_2P), 0);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_HANDICAP, MDSPR_HANDICAP_NUM_3P), 0);
    HuSprBankSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_HANDICAP, MDSPR_HANDICAP_NUM_4P), 0);
    
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_WIN, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_LR, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_UDLR, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_PLAYERNO, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_COMDIF, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_TEAM, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_MAP_NAME, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_HANDICAP, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_NAME, FALSE);
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

HuVec2f *MdSpriteGrpPosGet(MDSPRITE_WORK *sprite, int grpNo)
{
    HUSPRGRP *grp = &HuSprGrpData[sprite->grpId[grpNo]];
    return &grp->pos;
}

HuVec2f *MdSpriteGrpScaleGet(MDSPRITE_WORK *sprite, int grpNo)
{
    HUSPRGRP *grp = &HuSprGrpData[sprite->grpId[grpNo]];
    return &grp->scale;
}

HuVec2f *MdSpritePosGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo)
{
    HUSPRGRP *grp = &HuSprGrpData[sprite->grpId[grpNo]];
    HUSPRITE *spr = &HuSprData[grp->sprId[memberNo]];
    return &spr->pos;
}

HuVec2f *MdSpriteScaleGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo)
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
static void CameraBgLayerHook(s16 layerNo);
static void CameraMainLayerHook(s16 layerNo);
static void CameraFgLayerHook(s16 layerNo);

void MdModelCreate(MDMODEL_WORK *model, MDCAMERA_WORK *camera)
{
    int i;
    model->mdlId[MDMODEL_STAGE_CHR] = Hu3DModelCreateData(MDPARTY_HSF_stage_chr);
    Hu3DModelDispOff(model->mdlId[MDMODEL_STAGE_CHR]);
    Hu3DModelLayerSet(model->mdlId[MDMODEL_STAGE_CHR], 0);
    Hu3DModelCameraSet(model->mdlId[MDMODEL_STAGE_CHR], camera->camera[MDCAMERA_BG].bit);
    Hu3DModelPosSet(model->mdlId[MDMODEL_STAGE_CHR], 0, -300, 0);
    Hu3DModelRotSet(model->mdlId[MDMODEL_STAGE_CHR], 0, 0, 0);
    Hu3DModelScaleSet(model->mdlId[MDMODEL_STAGE_CHR], 2, 2, 1);
    Hu3DModelAttrSet(model->mdlId[MDMODEL_STAGE_CHR], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrReset(model->mdlId[MDMODEL_STAGE_CHR], HU3D_MOTATTR_PAUSE);
    Hu3DMotionSpeedSet(model->mdlId[MDMODEL_STAGE_CHR], 0.5f);
    for(i=0; i<10; i++) {
        model->mdlId[MDMODEL_MARIO+i] = Hu3DModelCreateData(ChrModelFileTbl[i]);
        Hu3DModelDispOff(model->mdlId[MDMODEL_MARIO+i]);
        Hu3DModelLayerSet(model->mdlId[MDMODEL_MARIO+i], 1);
        Hu3DModelCameraSet(model->mdlId[MDMODEL_MARIO+i], camera->camera[MDCAMERA_BG].bit);
        Hu3DModelPosSet(model->mdlId[MDMODEL_MARIO+i], ChrModelPosTbl[i].x, ChrModelPosTbl[i].y, ChrModelPosTbl[i].z);
        Hu3DModelRotSet(model->mdlId[MDMODEL_MARIO+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_MARIO+i], 0.8f, 0.8f, 0.8f);
        Hu3DModelAttrSet(model->mdlId[MDMODEL_MARIO+i], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(model->mdlId[MDMODEL_MARIO+i], HU3D_MOTATTR_PAUSE);
        Hu3DMotionSpeedSet(model->mdlId[MDMODEL_MARIO+i], 1.0f);
    }
    model->mdlId[MDMODEL_STAGE] = Hu3DModelCreateData(MDPARTY_HSF_stage);
    Hu3DModelDispOff(model->mdlId[MDMODEL_STAGE]);
    Hu3DModelLayerSet(model->mdlId[MDMODEL_STAGE], 0);
    Hu3DModelCameraSet(model->mdlId[MDMODEL_STAGE], camera->camera[MDCAMERA_MAIN].bit);
    Hu3DModelPosSet(model->mdlId[MDMODEL_STAGE], 0, 0, 0);
    Hu3DModelRotSet(model->mdlId[MDMODEL_STAGE], 0, 0, 0);
    Hu3DModelScaleSet(model->mdlId[MDMODEL_STAGE], 1, 1, 1);
    Hu3DModelAttrSet(model->mdlId[MDMODEL_STAGE], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrReset(model->mdlId[MDMODEL_STAGE], HU3D_MOTATTR_PAUSE);
    Hu3DMotionSpeedSet(model->mdlId[MDMODEL_STAGE], 1.0f);
    for(i=0; i<4; i++) {
        if(i == 0) {
            model->mdlId[MDMODEL_CAP_MAN_1P+i] = Hu3DModelCreateData(CapPlayerFileTbl[0]);
        } else {
            model->mdlId[MDMODEL_CAP_MAN_1P+i] = Hu3DModelLink(model->mdlId[MDMODEL_CAP_MAN_1P]);
        }
        Hu3DModelDispOff(model->mdlId[MDMODEL_CAP_MAN_1P+i]);
        Hu3DModelLayerSet(model->mdlId[MDMODEL_CAP_MAN_1P+i], 2);
        Hu3DModelCameraSet(model->mdlId[MDMODEL_CAP_MAN_1P+i], camera->camera[MDCAMERA_MAIN].bit);
        Hu3DModelPosSet(model->mdlId[MDMODEL_CAP_MAN_1P+i], 0, 0, 0);
        Hu3DModelRotSet(model->mdlId[MDMODEL_CAP_MAN_1P+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_CAP_MAN_1P+i], 1, 1, 1);
        Hu3DModelAttrSet(model->mdlId[MDMODEL_CAP_MAN_1P+i], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(model->mdlId[MDMODEL_CAP_MAN_1P+i], HU3D_MOTATTR_PAUSE);
        Hu3DMotionSpeedSet(model->mdlId[MDMODEL_CAP_MAN_1P+i], 1.0f);
        if(i == 0) {
            model->mdlId[MDMODEL_CAP_COM_1P+i] = Hu3DModelCreateData(CapPlayerFileTbl[1]);
        } else {
            model->mdlId[MDMODEL_CAP_COM_1P+i] = Hu3DModelLink(model->mdlId[MDMODEL_CAP_COM_1P]);
        }
        Hu3DModelDispOff(model->mdlId[MDMODEL_CAP_COM_1P+i]);
        Hu3DModelLayerSet(model->mdlId[MDMODEL_CAP_COM_1P+i], 2);
        Hu3DModelCameraSet(model->mdlId[MDMODEL_CAP_COM_1P+i], camera->camera[MDCAMERA_MAIN].bit);
        Hu3DModelPosSet(model->mdlId[MDMODEL_CAP_COM_1P+i], 0, 0, 0);
        Hu3DModelRotSet(model->mdlId[MDMODEL_CAP_COM_1P+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_CAP_COM_1P+i], 1, 1, 1);
        Hu3DModelAttrSet(model->mdlId[MDMODEL_CAP_COM_1P+i], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(model->mdlId[MDMODEL_CAP_COM_1P+i], HU3D_MOTATTR_PAUSE);
        Hu3DMotionSpeedSet(model->mdlId[MDMODEL_CAP_COM_1P+i], 1.0f);
    }
    for(i=0; i<10; i++) {
        if(i == 0) {
            model->mdlId[MDMODEL_CAP_CHR_00+i] = Hu3DModelCreateData(MDPARTY_HSF_cap_chr);
        } else {
            model->mdlId[MDMODEL_CAP_CHR_00+i] = Hu3DModelLink(model->mdlId[MDMODEL_CAP_CHR_00]);
        }
        Hu3DModelDispOff(model->mdlId[MDMODEL_CAP_CHR_00+i]);
        Hu3DModelLayerSet(model->mdlId[MDMODEL_CAP_CHR_00+i], 2);
        Hu3DModelCameraSet(model->mdlId[MDMODEL_CAP_CHR_00+i], camera->camera[MDCAMERA_MAIN].bit);
        Hu3DModelPosSet(model->mdlId[MDMODEL_CAP_CHR_00+i], 0, 0, 0);
        Hu3DModelRotSet(model->mdlId[MDMODEL_CAP_CHR_00+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_CAP_CHR_00+i], 1, 1, 1);
        Hu3DModelMatHookSet(model->mdlId[MDMODEL_CAP_CHR_00+i], CapChrMatHookTbl[i]);
    }
    for(i=0; i<2; i++) {
        model->mdlId[MDMODEL_CAP_BROYAL+i] = Hu3DModelCreateData(CapGameTypeFileTbl[i]);
        Hu3DModelDispOn(model->mdlId[MDMODEL_CAP_BROYAL+i]);
        Hu3DModelLayerSet(model->mdlId[MDMODEL_CAP_BROYAL+i], 2);
        Hu3DModelCameraSet(model->mdlId[MDMODEL_CAP_BROYAL+i], camera->camera[MDCAMERA_MAIN].bit);
        Hu3DModelPosSet(model->mdlId[MDMODEL_CAP_BROYAL+i], 0, 0, 0);
        Hu3DModelRotSet(model->mdlId[MDMODEL_CAP_BROYAL+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_CAP_BROYAL+i], 1, 1, 1);
        Hu3DModelAttrSet(model->mdlId[MDMODEL_CAP_BROYAL+i], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(model->mdlId[MDMODEL_CAP_BROYAL+i], HU3D_MOTATTR_PAUSE);
        Hu3DMotionSpeedSet(model->mdlId[MDMODEL_CAP_BROYAL+i], 1.0f);
    }
    model->mdlId[MDMODEL_VS] = Hu3DModelCreateData(MDPARTY_HSF_vs);
    Hu3DModelDispOn(model->mdlId[MDMODEL_VS]);
    Hu3DModelLayerSet(model->mdlId[MDMODEL_VS], 2);
    Hu3DModelCameraSet(model->mdlId[MDMODEL_VS], camera->camera[MDCAMERA_MAIN].bit);
    Hu3DModelPosSet(model->mdlId[MDMODEL_VS], 0, 0, 0);
    Hu3DModelRotSet(model->mdlId[MDMODEL_VS], 0, 0, 0);
    Hu3DModelScaleSet(model->mdlId[MDMODEL_VS], 1, 1, 1);
    for(i=0; i<7; i++) {
        model->mdlId[MDMODEL_MAP1+i] = Hu3DModelCreateData(MapFileTbl[i]);
        Hu3DModelDispOn(model->mdlId[MDMODEL_MAP1+i]);
        Hu3DModelLayerSet(model->mdlId[MDMODEL_MAP1+i], 0);
        Hu3DModelCameraSet(model->mdlId[MDMODEL_MAP1+i], camera->camera[MDCAMERA_MAIN].bit);
        Hu3DModelPosSet(model->mdlId[MDMODEL_MAP1+i], 0, 0, 0);
        Hu3DModelRotSet(model->mdlId[MDMODEL_MAP1+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_MAP1+i], 1, 1, 1);
    }
    for(i=0; i<5; i++) {
        model->mdlId[MDMODEL_CAP_PACK0+i] = Hu3DModelCreateData(CapPackFileTbl[i]);
        Hu3DModelDispOn(model->mdlId[MDMODEL_CAP_PACK0+i]);
        Hu3DModelLayerSet(model->mdlId[MDMODEL_CAP_PACK0+i], 2);
        Hu3DModelCameraSet(model->mdlId[MDMODEL_CAP_PACK0+i], camera->camera[MDCAMERA_MAIN].bit);
        Hu3DModelPosSet(model->mdlId[MDMODEL_CAP_PACK0+i], 0, 0, 0);
        Hu3DModelRotSet(model->mdlId[MDMODEL_CAP_PACK0+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_CAP_PACK0+i], 1, 1, 1);
        Hu3DModelAttrSet(model->mdlId[MDMODEL_CAP_PACK0+i], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(model->mdlId[MDMODEL_CAP_PACK0+i], HU3D_MOTATTR_PAUSE);
        Hu3DMotionSpeedSet(model->mdlId[MDMODEL_CAP_PACK0+i], 1.0f);
    }
    model->mdlId[MDMODEL_CAP_TURN] = Hu3DModelCreateData(MDPARTY_HSF_cap_turn);
    Hu3DModelDispOn(model->mdlId[MDMODEL_CAP_TURN]);
    Hu3DModelLayerSet(model->mdlId[MDMODEL_CAP_TURN], 2);
    Hu3DModelCameraSet(model->mdlId[MDMODEL_CAP_TURN], camera->camera[MDCAMERA_MAIN].bit);
    Hu3DModelPosSet(model->mdlId[MDMODEL_CAP_TURN], 0, 0, 0);
    Hu3DModelRotSet(model->mdlId[MDMODEL_CAP_TURN], 0, 0, 0);
    Hu3DModelScaleSet(model->mdlId[MDMODEL_CAP_TURN], 1, 1, 1);
    model->mdlId[MDMODEL_CAP_BONUS] = Hu3DModelCreateData(MDPARTY_HSF_cap_bonus);
    Hu3DModelDispOn(model->mdlId[MDMODEL_CAP_BONUS]);
    Hu3DModelLayerSet(model->mdlId[MDMODEL_CAP_BONUS], 2);
    Hu3DModelCameraSet(model->mdlId[MDMODEL_CAP_BONUS], camera->camera[MDCAMERA_MAIN].bit);
    Hu3DModelPosSet(model->mdlId[MDMODEL_CAP_BONUS], 0, 0, 0);
    Hu3DModelRotSet(model->mdlId[MDMODEL_CAP_BONUS], 0, 0, 0);
    Hu3DModelScaleSet(model->mdlId[MDMODEL_CAP_BONUS], 1, 1, 1);
    for(i=0; i<4; i++) {
        if(i == 0) {
            model->mdlId[MDMODEL_FINGER_1P+i] = Hu3DModelCreateData(MDPARTY_HSF_finger);
        } else {
            model->mdlId[MDMODEL_FINGER_1P+i] = Hu3DModelLink(model->mdlId[MDMODEL_FINGER_1P]);
        }
        Hu3DModelDispOff(model->mdlId[MDMODEL_FINGER_1P+i]);
        Hu3DModelLayerSet(model->mdlId[MDMODEL_FINGER_1P+i], 2);
        Hu3DModelCameraSet(model->mdlId[MDMODEL_FINGER_1P+i], camera->camera[MDCAMERA_MAIN].bit);
        Hu3DModelPosSet(model->mdlId[MDMODEL_FINGER_1P+i], 0, 0, 0);
        Hu3DModelRotSet(model->mdlId[MDMODEL_FINGER_1P+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_FINGER_1P+i], 1, 1, 1);
        Hu3DModelAttrSet(model->mdlId[MDMODEL_FINGER_1P+i], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(model->mdlId[MDMODEL_FINGER_1P+i], HU3D_MOTATTR_PAUSE);
        Hu3DMotionSpeedSet(model->mdlId[MDMODEL_FINGER_1P+i], 1.0f);
    }
    for(i=0; i<4; i++) {
        if(i == 0) {
            model->mdlId[MDMODEL_RING_1P+i] = Hu3DModelCreateData(MDPARTY_HSF_ring);
        } else {
            model->mdlId[MDMODEL_RING_1P+i] = Hu3DModelLink(model->mdlId[MDMODEL_RING_1P]);
        }
        Hu3DModelDispOff(model->mdlId[MDMODEL_RING_1P+i]);
        Hu3DModelLayerSet(model->mdlId[MDMODEL_RING_1P+i], 3);
        Hu3DModelCameraSet(model->mdlId[MDMODEL_RING_1P+i], camera->camera[MDCAMERA_MAIN].bit);
        Hu3DModelPosSet(model->mdlId[MDMODEL_RING_1P+i], 0, 0, 0);
        Hu3DModelRotSet(model->mdlId[MDMODEL_RING_1P+i], 0, 0, 0);
        Hu3DModelScaleSet(model->mdlId[MDMODEL_RING_1P+i], 1, 1, 1);
        Hu3DModelMatHookSet(model->mdlId[MDMODEL_RING_1P+i], RingMatHookTbl[i]);
    }
    model->mdlId[MDMODEL_STAR] = Hu3DModelCreateData(MDPARTY_HSF_star);
    Hu3DModelDispOff(model->mdlId[MDMODEL_STAR]);
    Hu3DModelLayerSet(model->mdlId[MDMODEL_STAR], 0);
    Hu3DModelCameraSet(model->mdlId[MDMODEL_STAR], camera->camera[MDCAMERA_FG].bit);
    Hu3DModelPosSet(model->mdlId[MDMODEL_STAR], 0, 0, 0);
    Hu3DModelRotSet(model->mdlId[MDMODEL_STAR], 0, 0, 0);
    Hu3DModelScaleSet(model->mdlId[MDMODEL_STAR], 1, 1, 1);
    Hu3DModelAttrSet(model->mdlId[MDMODEL_STAR], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrReset(model->mdlId[MDMODEL_STAR], HU3D_MOTATTR_PAUSE);
    Hu3DMotionSpeedSet(model->mdlId[MDMODEL_STAR], 1.0f);
    
    model->mdlId[MDMODEL_FONTBGFILTER] = Hu3DHookFuncCreate(FontBGFilter);
    Hu3DModelLayerSet(model->mdlId[MDMODEL_FONTBGFILTER], 7);
    Hu3DModelCameraSet(model->mdlId[MDMODEL_FONTBGFILTER], camera->camera[MDCAMERA_FG].bit);
    Hu3DModelDispOn(model->mdlId[MDMODEL_FONTBGFILTER]);
    for(i=0; i<3; i++) {
        model->motId[MDMODEL_MOT_STAR_IDLE+i] = Hu3DJointMotionData(model->mdlId[MDMODEL_STAR], StarMotionFileTbl[i]);
    }
    Hu3DMotionSet(model->mdlId[MDMODEL_STAR], model->motId[MDMODEL_MOT_STAR_IDLE]);
    for(i=0; i<20; i++) {
        model->motId[MDMODEL_MOT_MARIO_IDLE+i] = Hu3DJointMotionData(model->mdlId[MDMODEL_MARIO+(i/2)], ChrMotionFileTbl[i]);
        if((i%2) == 0) {
            Hu3DMotionSet(model->mdlId[MDMODEL_MARIO+(i/2)], model->motId[MDMODEL_MOT_MARIO_IDLE+i]);
        }
    }
    for(i=0; i<MDMODEL_MAX; i++) {
        HU3DMODEL *modelP = &Hu3DData[model->mdlId[i]];
        model->posP[i] = &modelP->pos;
        model->rotP[i] = &modelP->rot;
        model->scaleP[i] = &modelP->scale;
    }
    Hu3DLayerHookSet(1+HU3D_LAYER_HOOK_POST, NULL);
    Hu3DLayerHookSet(0+HU3D_LAYER_HOOK_POST, CameraBgLayerHook);
    Hu3DLayerHookSet(2, CameraMainLayerHook);
    Hu3DLayerHookSet(4, CameraMainLayerHook);
    Hu3DLayerHookSet(0, CameraFgLayerHook);
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
        GXSetTevColor(GX_TEVREG0, color);
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

static void CameraBgLayerHook(s16 layerNo)
{
    if(Hu3DCameraNo == MDCAMERA_BG) {
        Hu3DZClear();
    }
}

static void CameraMainLayerHook(s16 layerNo)
{
    if(Hu3DCameraNo == MDCAMERA_MAIN) {
        Hu3DZClear();
    }
}

static void CameraFgLayerHook(s16 layerNo)
{
    if(Hu3DCameraNo == MDCAMERA_FG) {
        Hu3DZClear();
    }
}

static void TexBufFBCopy(int unitNo);
static void TexBufZClear(void);

void MdTexBufLayerHook(s16 layerNo)
{
    int i;
    if(Hu3DCameraNo == MDCAMERA_BG) {
        for(i=0; i<10; i++) {
            TexBufFBCopy(i+MDPARTY_TEXBUF_MIRROR);
        }
        TexBufZClear();
    }
}

static void TexBufUnitSet(int unitNo)
{
    MDTEXBUF_WORK *texbuf = sysP->texbuf;
    GXColor color = { 16, 16, 16, 128 };
    MDTEXBUF_UNIT *unit;
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    unit = &texbuf->unit[unitNo];
    GXLoadTexObj(&unit->texObj, GX_TEXMAP0);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

static void CapChr0MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    TexBufUnitSet(MDPARTY_TEXBUF_MIRROR+0);
}

static void CapChr1MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    TexBufUnitSet(MDPARTY_TEXBUF_MIRROR+1);
}

static void CapChr2MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    TexBufUnitSet(MDPARTY_TEXBUF_MIRROR+2);
}

static void CapChr3MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    TexBufUnitSet(MDPARTY_TEXBUF_MIRROR+3);
}

static void CapChr4MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    TexBufUnitSet(MDPARTY_TEXBUF_MIRROR+4);
}

static void CapChr5MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    TexBufUnitSet(MDPARTY_TEXBUF_MIRROR+5);
}

static void CapChr6MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    TexBufUnitSet(MDPARTY_TEXBUF_MIRROR+6);
}

static void CapChr7MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    TexBufUnitSet(MDPARTY_TEXBUF_MIRROR+7);
}

static void CapChr8MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    TexBufUnitSet(MDPARTY_TEXBUF_MIRROR+8);
}

static void CapChr9MatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    TexBufUnitSet(MDPARTY_TEXBUF_MIRROR+9);
}

void MdModelAnimInit(void)
{
    MDSPRITE_WORK *sprite = sysP->sprite;
    MDMODEL_WORK *model = sysP->model;
    int i; //r30
    
    for(i=0; i<MDMODEL_ANIM_MAX; i++) {
        HU3DANIMID animId = Hu3DAnimCreate(sprite->animP[ModelSprAnimTbl[i]], model->mdlId[ModelAnimModelTbl[i]], ModelAnimBmpNameTbl[i]);
        model->animId[i] = animId;
        Hu3DAnimSpeedSet(animId, 1);
        Hu3DAnimAttrSet(animId, HU3D_ANIM_ATTR_ANIMON);
        Hu3DAnimAttrSet(animId, HU3D_ANIM_ATTR_LOOP);
    }
    Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_FINGER_1P], 0);
    Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_FINGER_2P], 0);
    Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_FINGER_3P], 0);
    Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_FINGER_4P], 0);
    Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_CAP_TURN], 0);
    Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_CAP_BONUS], 0);
}

void MdTexBufInit(void)
{
    MDTEXBUF_WORK *texbuf = sysP->texbuf;
    MDMODEL_WORK *model = sysP->model;
    int i;
    for(i=0; i<10; i++) {
        MDTEXBUF_UNIT *unit = &texbuf->unit[MDPARTY_TEXBUF_MIRROR+i];
        unit->w = MDPARTY_TEXBUF_W;
        unit->h = MDPARTY_TEXBUF_H;
        unit->texFmt = GX_TF_RGB565;
        MdTexBufObjPosSet(unit, model->mdlId[i+MDMODEL_MARIO], TexBufChrObjName[i], sysP->camera->camera[MDCAMERA_BG].bit, &TexBufChrOfs[i]);
        unit->addr = HuMemDirectMallocNum(HEAP_MODEL, GXGetTexBufferSize(unit->w, unit->h, unit->texFmt, GX_FALSE, 0), HU_MEMNUM_OVL);
        
        if(!unit->addr) {
            OSReport("sys->texbuf->unit[MDPARTY_TEXBUF_MIRROR + %d].addr = NULL...\n", i);
        }
        GXInitTexObj(&unit->texObj, unit->addr, unit->w, unit->h, unit->texFmt, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&unit->texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    }
}

static void TexBufFBCopy(int unitNo)
{
    MDTEXBUF_UNIT *unit = &sysP->texbuf->unit[unitNo];
    Hu3DFbCopyExec(unit->posX, unit->posY, unit->w, unit->h, unit->texFmt,GX_FALSE, unit->addr);
}

void MdTexBufObjPosSet(MDTEXBUF_UNIT *unit, HU3DMODELID modelId, char *objName, u16 cameraBit, HuVecF *ofsP)
{
    HuVecF pos3D; //sp+0x20
    HuVecF pos2D; //sp+0x14
    
    s16 x;
    s16 y;
    
    Hu3DModelObjPosGet(modelId, objName, &pos3D);
    pos3D.x += ofsP->x;
    pos3D.y += ofsP->y;
    pos3D.z += ofsP->z;
    Hu3D3Dto2D(&pos3D, cameraBit, &pos2D);
    x = (int)((pos2D.x/576)*640) & 0xFFFE;
    y = (int)(pos2D.y) & 0xFFFE;
    unit->posX = x-(unit->w/2);
    unit->posY = y-(unit->h/2);
}

void MdTexBufPosSet(MDTEXBUF_UNIT *unit, HuVecF *posP, u16 cameraBit)
{
    HuVecF pos2D; //sp+0x14
    
    s16 x;
    s16 y;
    Hu3D3Dto2D(posP, cameraBit, &pos2D);
    x = (int)((pos2D.x/576)*640) & 0xFFFE;
    y = (int)(pos2D.y) & 0xFFFE;
    unit->posX = x-(unit->w/2);
    unit->posY = y-(unit->h/2);
}

static void TexBufZClear(void)
{
    HU3DCAMERA *cameraP = &Hu3DCamera[Hu3DCameraNo];
    Mtx44 proj;
    Mtx modelview;
    Vec pos;
    Vec target;
    Vec up;
    GXColor color;
    float z;
    
    MTXPerspective(proj, cameraP->fov, cameraP->aspect, cameraP->near, cameraP->far);
    GXSetProjection(proj, GX_PERSPECTIVE);
    GXSetViewport(0, 0, 640, 480, 0, 1);
    GXSetScissor(0, 0, 640, 480);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    color.r = color.g = color.b = 0;
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaCompare(GX_GEQUAL, 0, GX_AOP_AND, GX_GEQUAL, 0);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);

    pos.x = pos.y = pos.z = 0;
    target.x = target.y = 0;
    target.z = -100;
    up.x = up.z = 0;
    up.y = 1;
    MTXLookAt(modelview, &pos, &up, &target);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    pos.x = (10000*-sin(cameraP->fov/2))*1.2f;
    pos.y = 10000*-sin(cameraP->fov/2)*(1/1.2f);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-pos.x, -pos.y, -10000);
    GXPosition3f32(pos.x, -pos.y, -10000);
    GXPosition3f32(pos.x, pos.y, -10000);
    GXPosition3f32(-pos.x, pos.y, -10000);
}

void MdRingScaleSet(int no, float scale);

void MdRingCreate(void)
{
    MDRING_WORK *ringWork = sysP->ring;
    MDMODEL_WORK *model = sysP->model;
    MDRING *ring;
    int i;
    for(ring=&ringWork->ring[0], i=0; i<4; i++, ring++) {
        ring->disp = FALSE;
        ring->mode = 0;
        ring->mdlNo = i+MDMODEL_RING_1P;
        ring->baseMdlNo = -1;
        ring->posP = &model->posP[ring->mdlNo];
        ring->rotP = &model->rotP[ring->mdlNo];
        ring->scaleP = &model->scaleP[ring->mdlNo];
        MdRingBaseMdlSet(i, -1);
        MdRingBaseOfsSet(i, 0, 0, 0);
        if(i <= 3) {
            ring->color = RingColorTbl[i];
        }
        MdRingColorReset(i);
    }
    ringWork->scale = 0.5f;
}

void MdRingMain(void)
{
    MDRING *ring; //r31
    int i; //r30
    HuVecF *posP; //r29
    HuVecF *basePosP; //r28
    int colorIdx; //r27
    MDRING_WORK *ringWork; //r26
    MDMODEL_WORK *model; //r25
    MDCAMERA_WORK *cameraWork; //r24
    
    HuVecF *rotP; //sp+0x10
    HuVecF *scaleP; //sp+0xC
    MDCAMERA *camera; //sp+0x8
    
    ringWork = sysP->ring;
    model = sysP->model;
    cameraWork = sysP->camera;
    camera = &cameraWork->camera[MDCAMERA_MAIN];
    for(ring=&ringWork->ring[0], i=0; i<4; i++, ring++) {
        posP = *ring->posP;
        rotP = *ring->rotP;
        scaleP = *ring->scaleP;
        if(ring->baseMdlNo != -1) {
            basePosP = model->posP[ring->baseMdlNo];
            *posP = *basePosP;
            posP->x = basePosP->x+ring->baseOfs.x;
            posP->y = basePosP->y+ring->baseOfs.y;
            posP->z = basePosP->z+ring->baseOfs.z;
        }
        switch(ring->mode) {
            case 1:
                ring->baseOfs.x = 0;
                ring->baseOfs.y = 0;
                ring->baseOfs.z = 0;
                MdRingDispSet(i, TRUE);
                MdRingColorReset(i);
                ring->color.r = RingColorTbl[0].r;
                ring->color.g = RingColorTbl[0].g;
                ring->color.b = RingColorTbl[0].b;
                ring->color.a = 0;
                ring->mode = 2;
                break;
            
            case 2:
                MdMathOscillate(MDMATH_OSC_90_REV, &ring->time, &ring->t, 16);
                MdMathLerpColor(&ring->color.a, 0, 192, ring->t);
                if(ring->time >= 1) {
                    MdRingColorReset(i);
                    ring->mode = 3;
                }
                break;
            
            case 3:
                MdMathOscillate(MDMATH_OSC_180, &ring->time, &ring->t, 32);
                MdMathLerpColor(&ring->color.a, 192, 144, ring->t);
                if(ring->time >= 1) {
                    MdRingColorReset(i);
                }
                break;
            
            case 4:
                MdRingColorReset(i);
                ring->endColor = ring->color;
                ring->mode = 5;
                break;
                
            case 5:
                MdMathOscillate(MDMATH_OSC_90, &ring->time, &ring->t, 8);
                MdMathLerpColor(&ring->color.a, ring->endColor.a, 0, ring->t);
                if(ring->time >= 1) {
                    MdRingDispSet(i, FALSE);
                    MdRingColorReset(i);
                    ring->color.r = RingColorTbl[0].r;
                    ring->color.g = RingColorTbl[0].g;
                    ring->color.b = RingColorTbl[0].b;
                    ring->color.a = 0;
                    ring->mode = 0;
                }
                break;
            
            case 6:
                MdRingDispSet(i, TRUE);
                MdRingColorReset(i);
                
                ring->color.a = 0;
                MdRingScaleSet(i, ringWork->scale);
                ring->mode = 7;
                break;
            
            case 7:
                MdMathOscillate(MDMATH_OSC_90, &ring->time, &ring->t, 4);
                MdMathOscillate(MDMATH_OSC_90, &ring->ofsTime, &ring->ofsT, 36);
                MdMathLerpColor(&ring->color.a, 0, 128, ring->t);
                MdMathLerpScale(*ring->scaleP, 0.8f, 1.8f, ring->ofsT);
                if(ring->time >= 1) {
                    ring->time = 0;
                    ring->t = 0;
                    ring->mode = 8;
                }
                break;
            
            case 8:
                MdMathOscillate(MDMATH_OSC_90, &ring->time, &ring->t, 32);
                MdMathOscillate(MDMATH_OSC_90, &ring->ofsTime, &ring->ofsT, 36);
                MdMathLerpColor(&ring->color.a, 128, 0, ring->t);
                MdMathLerpScale(*ring->scaleP, 0.8f, 1.8f, ring->ofsT);
                if(ring->time >= 1) {
                    MdRingDispSet(i, FALSE);
                    MdRingColorReset(i);
                    ring->color.a = 0;
                    ring->mode = 0;
                }
                break;
            
            case 9:
                MdRingColorReset(i);
                ring->endColor = ring->color;
                ring->mode = 10;
                break;
            
            case 10:
                MdMathOscillate(MDMATH_OSC_90, &ring->time, &ring->t, 8);
                MdMathLerpColor(&ring->color.r, ring->endColor.r, RingColorTbl[6].r, ring->t);
                MdMathLerpColor(&ring->color.g, ring->endColor.g, RingColorTbl[6].g, ring->t);
                MdMathLerpColor(&ring->color.b, ring->endColor.b, RingColorTbl[6].b, ring->t);
                MdMathLerpColor(&ring->color.a, ring->endColor.a, 192, ring->t);
                if(ring->time >= 1) {
                    MdRingColorReset(i);
                    ring->mode = 3;
                }
                break;
            
            case 11:
                MdRingColorReset(i);
                ring->endColor = ring->color;
                ring->mode = 12;
                break;
            
            case 12:
                MdMathOscillate(MDMATH_OSC_90, &ring->time, &ring->t, 8);
                MdMathLerpColor(&ring->color.r, ring->endColor.r, RingColorTbl[7].r, ring->t);
                MdMathLerpColor(&ring->color.g, ring->endColor.g, RingColorTbl[7].g, ring->t);
                MdMathLerpColor(&ring->color.b, ring->endColor.b, RingColorTbl[7].b, ring->t);
                MdMathLerpColor(&ring->color.a, ring->endColor.a, 192, ring->t);
                if(ring->time >= 1) {
                    MdRingColorReset(i);
                    ring->mode = 3;
                }
                break;
            
            case 13:
                MdRingColorReset(i);
                ring->endColor = ring->color;
                ring->mode = 14;
                break;
            
            case 14:
                colorIdx = 0;
                colorIdx = i;
                MdMathOscillate(MDMATH_OSC_90, &ring->time, &ring->t, 15);
                MdMathLerpColor(&ring->color.r, ring->endColor.r, RingColorTbl[colorIdx].r, ring->t);
                MdMathLerpColor(&ring->color.g, ring->endColor.g, RingColorTbl[colorIdx].g, ring->t);
                MdMathLerpColor(&ring->color.b, ring->endColor.b, RingColorTbl[colorIdx].b, ring->t);
                MdMathLerpColor(&ring->color.a, ring->endColor.a, 192, ring->t);
                if(ring->time >= 1) {
                    MdRingColorReset(i);
                    ring->mode = 3;
                }
                break;
            
            case 15:
                ring->color = RingColorTbl[i];
                ring->mode = 0;
                break;
        }
    }
    
}

void MdRingModeSet(int no, int mode)
{
    MDRING *ring = &sysP->ring->ring[no];
    ring->mode = mode;
}

void MdRingDispSet(int no, BOOL dispF)
{
    MDRING *ring = &sysP->ring->ring[no];
    MDMODEL_WORK *model = sysP->model;
    ring->disp = dispF;
    if(dispF == FALSE) {
        Hu3DModelDispOff(model->mdlId[ring->mdlNo]);
    } else if(dispF == TRUE) {
        Hu3DModelDispOn(model->mdlId[ring->mdlNo]);
    }
}

void MdRingScaleSet(int no, float scale)
{
    MDRING *ring = &sysP->ring->ring[no];
    (*ring->scaleP)->x = scale;
    (*ring->scaleP)->y = scale;
    (*ring->scaleP)->z = scale;
    
}

void MdRingBaseMdlSet(int no, int mdlNo)
{
    MDRING *ring = &sysP->ring->ring[no];
    ring->baseMdlNo = mdlNo;
}

void MdRingBaseOfsSet(int no, float x, float y, float z)
{
    MDRING *ring = &sysP->ring->ring[no];
    ring->baseOfs.x = x;
    ring->baseOfs.y = y;
    ring->baseOfs.z = z;
}

void MdRingColorReset(int no)
{
    MDRING *ring = &sysP->ring->ring[no];
    ring->unk10 = 0;
    ring->time = 0;
    ring->ofsTime = 0;
    ring->t = 0;
    ring->ofsT = 0;
}


static void RingMatSetup(int no)
{
    MDRING *ring = &sysP->ring->ring[no];
    Mtx trans;
    Mtx scale;
    Mtx texMtx;
    if(ring->disp) {
        GXSetColorUpdate(TRUE);
        GXSetAlphaUpdate(TRUE);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_INVCOPY);
        GXSetNumTevStages(1);
        GXSetTevColor(GX_TEVREG0, ring->color);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
        MTXTrans(trans, 0.02f, 0.02f, 0);
        MTXScale(scale, 0.99f, 0.99f, 1);
        MTXConcat(scale, trans, texMtx);
        GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
}

static void Ring1PMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    RingMatSetup(0);
}

static void Ring2PMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    RingMatSetup(1);
}

static void Ring3PMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    RingMatSetup(2);
}

static void Ring4PMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    RingMatSetup(3);
}