#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/init.h"
#include "game/main.h"

#include "string.h"

HU3DTEXANIM Hu3DTexAnimData[HU3D_TEXANIM_MAX];
HU3DTEXSCROLL Hu3DTexScrData[HU3D_TEXSCROLL_MAX];

void Hu3DAnimInit(void)
{
    s16 i;
    HU3DTEXANIM *texAnimP;
    HU3DTEXSCROLL *texScrP;
    for(texAnimP = &Hu3DTexAnimData[0], i=0; i<HU3D_TEXANIM_MAX; i++, texAnimP++) {
        texAnimP->modelId = HU3D_MODELID_NONE;
    }
    for(texScrP = &Hu3DTexScrData[0], i=0; i<HU3D_TEXSCROLL_MAX; i++, texScrP++) {
        texScrP->modelId = HU3D_MODELID_NONE;
    }
}

HU3DANIMID Hu3DAnimCreate(void *dataP, HU3DMODELID modelId, char *bmpName)
{
    HU3DTEXANIM *texAnimP;
    HU3DANIMID animId;
    
    HSFDATA *hsf;
    HSFATTRIBUTE *attrP;
    s16 i;
    s16 bmpNum;
    
    for(texAnimP = &Hu3DTexAnimData[0], animId=0; animId<HU3D_TEXANIM_MAX; animId++, texAnimP++) {
        if(texAnimP->modelId == HU3D_MODELID_NONE) {
            break;
        }
    }
    if(animId == HU3D_TEXANIM_MAX) {
        OSReport("Error: TexAnim Over\n");
        return HU3D_ANIMID_NONE;
    }
    hsf = Hu3DData[modelId].hsf;
    for(attrP=hsf->attribute, i=bmpNum=0; i<hsf->attributeNum; i++, attrP++) {
        if(strcmp(bmpName, attrP->bitmap->name) == 0) {
            HU3DATTRANIM *attrAnimP;
            if(!attrP->animWorkP) {
                attrAnimP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3DATTRANIM), Hu3DData[modelId].mallocNo);
                attrP->animWorkP = attrAnimP;
                attrAnimP->attr = HU3D_ATTRANIM_ATTR_NONE;
            } else {
                attrAnimP = attrP->animWorkP;
                if((attrAnimP->attr & HU3D_ATTRANIM_ATTR_ANIM2D) && Hu3DTexAnimData[attrAnimP->animId].modelId != HU3D_MODELID_NONE) {
                    Hu3DTexAnimData[attrAnimP->animId].modelId = HU3D_MODEL_MAX;
                }
            }
            attrAnimP->attr |= HU3D_ATTRANIM_ATTR_ANIM2D;
            attrAnimP->animId = animId;
            attrAnimP->scale.x = attrAnimP->scale.y = 1;
            attrAnimP->trans.x = attrAnimP->trans.y = 0;
            bmpNum++;
        }
    }
    if(bmpNum == 0) {
        OSReport("Error: Not Found TexAnim Name\n");
        return HU3D_ANIMID_NONE;
    }
    if(!dataP) {
        texAnimP->anim = NULL;
    } else {
        texAnimP->anim = HuSprAnimRead(dataP);
        texAnimP->anim->useNum++;
    }
    texAnimP->modelId = modelId;
    texAnimP->time = 0;
    texAnimP->bank = 0;
    texAnimP->anmNo = 0;
    texAnimP->attr = 0;
    texAnimP->speed = 1;
    return animId;
}

HU3DANIMID Hu3DAnimLink(HU3DANIMID linkAnimId, HU3DMODELID modelId, char *bmpName)
{
    HU3DTEXANIM *texAnimP;
    HU3DANIMID animId;
    
    HSFDATA *hsf;
    HSFATTRIBUTE *attrP;
    s16 i;
    s16 bmpNum;
    
    HU3DTEXANIM *linkTexAnimP = &Hu3DTexAnimData[linkAnimId];
    
    for(texAnimP = &Hu3DTexAnimData[0], animId=0; animId<HU3D_TEXANIM_MAX; animId++, texAnimP++) {
        if(texAnimP->modelId == HU3D_MODELID_NONE) {
            break;
        }
    }
    if(animId == HU3D_TEXANIM_MAX) {
        OSReport("Error: TexAnim Over\n");
        return HU3D_ANIMID_NONE;
    }
    hsf = Hu3DData[modelId].hsf;
    for(attrP=hsf->attribute, i=bmpNum=0; i<hsf->attributeNum; i++, attrP++) {
        if(strcmp(bmpName, attrP->bitmap->name) == 0) {
            HU3DATTRANIM *attrAnimP;
            if(!attrP->animWorkP) {
                attrAnimP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3DATTRANIM), Hu3DData[modelId].mallocNo);
                attrP->animWorkP = attrAnimP;
                attrAnimP->attr = HU3D_ATTRANIM_ATTR_NONE;
            } else {
                attrAnimP = attrP->animWorkP;
                if((attrAnimP->attr & HU3D_ATTRANIM_ATTR_ANIM2D) && Hu3DTexAnimData[attrAnimP->animId].modelId != HU3D_MODELID_NONE) {
                    Hu3DTexAnimData[attrAnimP->animId].modelId = HU3D_MODEL_MAX;
                }
            }
            attrAnimP->attr |= HU3D_ATTRANIM_ATTR_ANIM2D;
            attrAnimP->animId = animId;
            attrAnimP->scale.x = attrAnimP->scale.y = 1;
            attrAnimP->trans.x = attrAnimP->trans.y = 0;
            bmpNum++;
        }
    }
    if(bmpNum == 0) {
        OSReport("Error: Not Found TexAnim Name\n");
        return HU3D_ANIMID_NONE;
    }
    texAnimP->anim = linkTexAnimP->anim;
    texAnimP->anim->useNum++;
    texAnimP->modelId = modelId;
    texAnimP->time = 0;
    texAnimP->bank = 0;
    texAnimP->anmNo = 0;
    texAnimP->attr = HU3D_ANIM_ATTR_NONE;
    texAnimP->speed = 1;
    return animId;
}

void Hu3DAnimKill(HU3DANIMID animId)
{
    HU3DTEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    if(texAnimP->modelId != HU3D_MODEL_MAX) {
        HSFDATA *hsf = Hu3DData[texAnimP->modelId].hsf;
        if(hsf) {
            HSFATTRIBUTE *attrP;
            s16 i;
            for(attrP=hsf->attribute, i=0; i<hsf->attributeNum; i++, attrP++) {
                if(attrP->animWorkP) {
                    HU3DATTRANIM *attrAnimP = attrP->animWorkP;
                    if(attrAnimP->animId == animId) {
                        attrAnimP->attr &= ~HU3D_ATTRANIM_ATTR_ANIM2D;
                        if(attrAnimP->attr == HU3D_ATTRANIM_ATTR_NONE) {
                            attrP->animWorkP = NULL;
                            HuMemDirectFree(attrAnimP);
                        }
                    }
                }
            }
        }
    }
    texAnimP->modelId = HU3D_MODELID_NONE;
    if(--texAnimP->anim->useNum <= 0) {
        HuMemDirectFree(texAnimP->anim);
    }
}

void Hu3DAnimModelKill(HU3DMODELID modelId)
{
    HU3DTEXANIM *texAnimP;
    HU3DANIMID animId;
    for(texAnimP = &Hu3DTexAnimData[0], animId=0; animId<HU3D_TEXANIM_MAX; animId++, texAnimP++) {
        if(texAnimP->modelId == modelId) {
            Hu3DAnimKill(animId);
        }
    }
}

void Hu3DAnimAllKill(void)
{
    HU3DTEXANIM *texAnimP;
    HU3DANIMID animId;
    for(texAnimP = &Hu3DTexAnimData[0], animId=0; animId<HU3D_TEXANIM_MAX; animId++, texAnimP++) {
        if(texAnimP->modelId != HU3D_MODELID_NONE) {
            Hu3DAnimKill(animId);
        }
    }
    Hu3DTexScrollAllKill();
}

void Hu3DAnimAttrSet(HU3DANIMID animId, u16 attr)
{
    HU3DTEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    texAnimP->attr |= attr;
}

void Hu3DAnimAttrReset(HU3DANIMID animId, u16 attr)
{
    HU3DTEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    texAnimP->attr &= ~attr;
}

void Hu3DAnimSpeedSet(HU3DANIMID animId, float speed)
{
    HU3DTEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    texAnimP->speed = speed;
}

void Hu3DAnimBankSet(HU3DANIMID animId, s32 bank)
{
    HU3DTEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    texAnimP->bank = bank;
    texAnimP->anmNo = texAnimP->time = 0;
}

void Hu3DAnmNoSet(HU3DANIMID animId, u16 anmNo)
{
    HU3DTEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    texAnimP->anmNo = anmNo;
    texAnimP->time = 0;
}

s32 Hu3DAnimSet(HU3DMODEL *modelP, HSFATTRIBUTE *attrP, s16 texSlotNo)
{
    ANIMPAT *pat;
    HU3DATTRANIM *attrAnimP = attrP->animWorkP;
    HU3DTEXANIM *texAnimP = &Hu3DTexAnimData[attrAnimP->animId];
    ANIMDATA *anim = texAnimP->anim;
    s16 patNo = anim->bank[texAnimP->bank].frame[texAnimP->anmNo].pat;
    ANIMLAYER *layer;
    ANIMBMP *bmp;
    if(patNo == -1) {
        return FALSE;
    } else {
        s16 wrapS = (attrP->wrapS == TRUE) ? TRUE : FALSE;
        s16 wrapT = (attrP->wrapT == TRUE) ? TRUE : FALSE;
        pat = &anim->pat[patNo];
        layer = &pat->layer[0];
        bmp = &anim->bmp[layer->bmpNo];
        HuSprTexLoad(texAnimP->anim, layer->bmpNo, texSlotNo, wrapS, wrapT, (modelP->attr & HU3D_ATTR_TEX_NEAR) ? GX_NEAR : GX_LINEAR);
        attrAnimP->scale.x = (float)layer->sizeX/bmp->sizeX;
        attrAnimP->scale.y = (float)layer->sizeY/bmp->sizeY;
        attrAnimP->trans.x = (float)layer->startX/bmp->sizeX;
        attrAnimP->trans.y = (float)layer->startY/bmp->sizeY;
        
        return TRUE;
    }
}

void Hu3DAnimExec(void)
{
    HU3DTEXSCROLL *texScrP;
    HU3DTEXANIM *texAnimP;
    s16 i;
    for(texAnimP=&Hu3DTexAnimData[0], i=0; i<HU3D_TEXANIM_MAX; i++, texAnimP++) {
        if(texAnimP->modelId == HU3D_MODELID_NONE) {
            continue;
        }
        if(Hu3DPauseF == FALSE || (texAnimP->attr & HU3D_ANIM_ATTR_PAUSE)) {
            ANIMDATA *anim = texAnimP->anim;
            ANIMBANK *bank = &anim->bank[texAnimP->bank];
            ANIMFRAME *frame = &bank->frame[texAnimP->anmNo];
            if(!(texAnimP->attr & HU3D_ANIM_ATTR_ANIMON)
             || (frame->time == -1 && (texAnimP->attr & HU3D_ANIM_ATTR_LOOP))) {
                s16 j;
                for(j=0; j<(int)texAnimP->speed*minimumVcount; j++) {
                    texAnimP->time++;
                    if(texAnimP->time >= frame->time) {
                        texAnimP->anmNo++;
                        texAnimP->time -= frame->time;
                        if(texAnimP->anmNo >= bank->timeNum) {
                            texAnimP->anmNo--;
                        } else if(frame[1].time == -1) {
                            if(texAnimP->attr & HU3D_ANIM_ATTR_LOOP) {
                                texAnimP->anmNo--;
                            } else {
                                texAnimP->anmNo = 0;
                            }
                        }
                        frame = &bank->frame[texAnimP->anmNo];
                    }
                }
                texAnimP->time += (texAnimP->speed*minimumVcount)-j;
                if(texAnimP->time >= frame->time) {
                    texAnimP->anmNo++;
                    texAnimP->time -= frame->time;
                    if(texAnimP->anmNo >= bank->timeNum) {
                        texAnimP->anmNo--;
                    } else if(frame[1].time == -1) {
                        if(texAnimP->attr & HU3D_ANIM_ATTR_LOOP) {
                            texAnimP->anmNo--;
                        } else {
                            texAnimP->anmNo = 0;
                        }
                    }
                }
            }
        }
    }
    for(texScrP=&Hu3DTexScrData[0], i=0; i<HU3D_TEXSCROLL_MAX; i++, texScrP++) {
        if(texScrP->modelId == HU3D_MODELID_NONE) {
            continue;
        }
        if(Hu3DPauseF && !(texScrP->attr & HU3D_TEXSCR_ATTR_PAUSEDISABLE)) {
            MTXRotDeg(texScrP->texMtx, 'Z', texScrP->rot);
            mtxTransCat(texScrP->texMtx, texScrP->pos.x, texScrP->pos.y, texScrP->pos.z);
        } else {
            if(texScrP->attr & HU3D_TEXSCR_ATTR_POSMOVE) {
                VECAdd(&texScrP->pos, &texScrP->posMove, &texScrP->pos);
                if(texScrP->pos.x > 1.0f) {
                    texScrP->pos.x -= 1.0f;
                }
                if(texScrP->pos.y > 1.0f) {
                    texScrP->pos.y -= 1.0f;
                }
                if(texScrP->pos.z > 1.0f) {
                    texScrP->pos.z -= 1.0f;
                }
                if(texScrP->pos.x < -1.0f) {
                    texScrP->pos.x += 1.0f;
                }
                if(texScrP->pos.y < -1.0f) {
                    texScrP->pos.y += 1.0f;
                }
                if(texScrP->pos.z < -1.0f) {
                    texScrP->pos.z += 1.0f;
                }
            }
            if(texScrP->attr & HU3D_TEXSCR_ATTR_ROTMOVE) {
                texScrP->rot += texScrP->rotMove;
                if(texScrP->rot > 360.0f) {
                    texScrP->rot -= 360.0f;
                }
                if(texScrP->rot < -360.0f) {
                    texScrP->rot += 360.0f;
                }
            }
            MTXRotDeg(texScrP->texMtx, 'Z', texScrP->rot);
            mtxTransCat(texScrP->texMtx, texScrP->pos.x, texScrP->pos.y, texScrP->pos.z);
        }
    }
}

HU3DTEXSCRID Hu3DTexScrollCreate(HU3DMODELID modelId, char *bmpName)
{
    HU3DTEXSCROLL *texScrP;
    HU3DTEXSCRID texScrId;
    
    HSFDATA *hsf;
    HSFATTRIBUTE *attrP;
    s16 i;
    s16 bmpNum;
    
    for(texScrP=&Hu3DTexScrData[0], texScrId=0; texScrId<HU3D_TEXSCROLL_MAX; texScrId++, texScrP++) {
        if(texScrP->modelId == HU3D_MODELID_NONE) {
            break;
        }
    }
    if(texScrId == HU3D_TEXSCROLL_MAX) {
        OSReport("Error: TexScroll Over\n");
        return HU3D_TEXSCRID_NONE;
    }
    hsf = Hu3DData[modelId].hsf;
    for(attrP=hsf->attribute, i=bmpNum=0; i<hsf->attributeNum; i++, attrP++) {
        if(strcmp(bmpName, attrP->bitmap->name) == 0) {
            HU3DATTRANIM *attrAnimP;
            if(!attrP->animWorkP) {
                attrAnimP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3DATTRANIM), Hu3DData[modelId].mallocNo);
                attrP->animWorkP = attrAnimP;
                attrAnimP->attr = HU3D_ATTRANIM_ATTR_NONE;
            } else {
                attrAnimP = attrP->animWorkP;
            }
            attrAnimP->attr |= HU3D_ATTRANIM_ATTR_TEXMTX;
            attrAnimP->texScrId = texScrId;
            attrAnimP->scale.x = attrAnimP->scale.y = 1;
            attrAnimP->trans.x = attrAnimP->trans.y = 0;
            bmpNum++;
        }
    }
    if(bmpNum == 0) {
        OSReport("Error: Not Found TexAnim Name\n");
        return HU3D_TEXSCRID_NONE;
    }
    texScrP->modelId = modelId;
    texScrP->attr = HU3D_TEXSCR_ATTR_NONE;
    texScrP->pos.x = texScrP->pos.y = texScrP->pos.z =  0;
    texScrP->rot = 0;
    MTXIdentity(texScrP->texMtx);
    return texScrId;
}

void Hu3DTexScrollKill(HU3DTEXSCRID texScrId)
{
    HU3DTEXSCROLL *texScrP = &Hu3DTexScrData[texScrId];
    HSFDATA *hsf = Hu3DData[texScrP->modelId].hsf;
    if(hsf) {
        HSFATTRIBUTE *attrP;
        s16 i;
        for(attrP=hsf->attribute, i=0; i<hsf->attributeNum; i++, attrP++) {
            if(attrP->animWorkP) {
                HU3DATTRANIM *attrAnimP = attrP->animWorkP;
                if(attrAnimP->texScrId == texScrId) {
                    attrAnimP->attr &= ~HU3D_ATTRANIM_ATTR_TEXMTX;
                    if(attrAnimP->attr == HU3D_ATTRANIM_ATTR_NONE) {
                        attrP->animWorkP = NULL;
                        HuMemDirectFree(attrAnimP);
                    }
                }
            }
        }
    }
    texScrP->modelId = HU3D_MODELID_NONE;
}

void Hu3DTexScrollAllKill(void)
{
    HU3DTEXSCROLL *texScrP;
    HU3DTEXSCRID texScrId;
    for(texScrP=&Hu3DTexScrData[0], texScrId=0; texScrId<HU3D_TEXSCROLL_MAX; texScrId++, texScrP++) {
        if(texScrP->modelId != HU3D_MODELID_NONE) {
            Hu3DTexScrollKill(texScrId);
        }
    }
}

void Hu3DTexScrollPosSet(HU3DTEXSCRID texScrId, float posX, float posY, float posZ)
{
    HU3DTEXSCROLL *texScrP = &Hu3DTexScrData[texScrId];
    texScrP->attr &= ~HU3D_TEXSCR_ATTR_POSMOVE;
    texScrP->pos.x = posX;
    texScrP->pos.y = posY;
    texScrP->pos.z = posZ;
}

void Hu3DTexScrollPosMoveSet(HU3DTEXSCRID texScrId, float posX, float posY, float posZ)
{
    HU3DTEXSCROLL *texScrP = &Hu3DTexScrData[texScrId];
    texScrP->attr |= HU3D_TEXSCR_ATTR_POSMOVE;
    texScrP->posMove.x = posX*minimumVcount;
    texScrP->posMove.y = posY*minimumVcount;
    texScrP->posMove.z = posZ*minimumVcount;
}

void Hu3DTexScrollRotSet(HU3DTEXSCRID texScrId, float rot)
{
    HU3DTEXSCROLL *texScrP = &Hu3DTexScrData[texScrId];
    texScrP->attr &= ~HU3D_TEXSCR_ATTR_ROTMOVE;
    texScrP->rot = rot;
}

void Hu3DTexScrollRotMoveSet(HU3DTEXSCRID texScrId, float rot)
{
    HU3DTEXSCROLL *texScrP = &Hu3DTexScrData[texScrId];
    texScrP->attr |= HU3D_TEXSCR_ATTR_ROTMOVE;
    texScrP->rotMove = rot*minimumVcount;
}

void Hu3DTexScrollPauseDisableSet(HU3DTEXSCRID texScrId, BOOL pauseDiableF)
{
    HU3DTEXSCROLL *texScrP = &Hu3DTexScrData[texScrId];
    if(pauseDiableF) {
        texScrP->attr |= HU3D_TEXSCR_ATTR_PAUSEDISABLE;
    } else {
        texScrP->attr &= ~HU3D_TEXSCR_ATTR_PAUSEDISABLE;
    }
}

static void particleFunc(HU3DMODEL *modelP, Mtx *mtx);

HU3DMODELID Hu3DParticleCreate(ANIMDATA *anim, s16 maxCnt)
{
    HU3DMODELID modelId = Hu3DHookFuncCreate(particleFunc);
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP;
    HU3DPARTICLEDATA *particleDataP;
    s16 i;
    HuVecF *vtxBuf;
    void *dlBuf;
    Hu3DModelAttrSet(modelId, HU3D_ATTR_PARTICLE);
    modelP->hookData = particleP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3DPARTICLE), modelP->mallocNo);
    particleP->anim = anim;
    anim->useNum++;
    particleP->maxCnt = maxCnt;
    particleP->blendMode = HU3D_PARTICLE_BLEND_NORMAL;
    particleP->hook = NULL;
    particleP->count = 0;
    particleP->attr = HU3D_PARTICLE_ATTR_NONE;
    particleP->prevCount = 0;
    particleP->dataCnt = particleP->emitCnt = 0;
    particleP->data = particleDataP = HuMemDirectMallocNum(HEAP_MODEL, maxCnt*sizeof(HU3DPARTICLEDATA), modelP->mallocNo);
    particleP->prevCounter = -1;
    for(i=0; i<maxCnt; i++, particleDataP++) {
        particleDataP->scale = 0.0f;
        particleDataP->unk04 = 0;
        particleDataP->cameraBit = HU3D_CAM_ALL;
        particleDataP->zRot = 0;
        particleDataP->pos.x = ((frand()&0x7F)-64)*20;
        particleDataP->pos.y = ((frand()&0x7F)-64)*30;
        particleDataP->pos.z = ((frand()&0x7F)-64)*20;
        particleDataP->color.r = particleDataP->color.g = particleDataP->color.b = particleDataP->color.a = 255;
    }
    particleP->vtxBuf = vtxBuf = HuMemDirectMallocNum(HEAP_MODEL, maxCnt*sizeof(HuVecF)*4, modelP->mallocNo);
    for(i=0; i<maxCnt*4; i++, vtxBuf++) {
        vtxBuf->x = vtxBuf->y = vtxBuf->z = 0;
    }
    particleP->dlBuf = dlBuf =  HuMemDirectMallocNum(HEAP_MODEL, (maxCnt*96)+128, modelP->mallocNo);
    DCInvalidateRange(dlBuf, (maxCnt*96)+128);
    GXBeginDisplayList(dlBuf, 0x20000);
    GXBegin(GX_QUADS, GX_VTXFMT0, maxCnt*4);
    for(i=0; i<maxCnt; i++) {
        GXPosition1x16(i*4);
        GXColor1x16(i);
        GXTexCoord1x16(0);
        GXPosition1x16((i*4)+1);
        GXColor1x16(i);
        GXTexCoord1x16(1);
        GXPosition1x16((i*4)+2);
        GXColor1x16(i);
        GXTexCoord1x16(2);
        GXPosition1x16((i*4)+3);
        GXColor1x16(i);
        GXTexCoord1x16(3);
    }
    GXEnd();
    particleP->dlSize = GXEndDisplayList();
    return modelId;
}


void Hu3DParticleScaleSet(HU3DMODELID modelId, float scale)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP = modelP->hookData;
    HU3DPARTICLEDATA *particleDataP;
    s16 i;
    for(particleDataP=particleP->data, i=0; i<particleP->maxCnt; i++, particleDataP++) {
        particleDataP->scale = scale;
    }
}

void Hu3DParticleZRotSet(HU3DMODELID modelId, float zRot)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP = modelP->hookData;
    HU3DPARTICLEDATA *particleDataP;
    s16 i;
    for(particleDataP=particleP->data, i=0; i<particleP->maxCnt; i++, particleDataP++) {
        particleDataP->zRot = zRot;
    }
}

void Hu3DParticleColSet(HU3DMODELID modelId, u8 r, u8 g, u8 b)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP = modelP->hookData;
    HU3DPARTICLEDATA *particleDataP;
    s16 i;
    for(particleDataP=particleP->data, i=0; i<particleP->maxCnt; i++, particleDataP++) {
        particleDataP->color.r = r;
        particleDataP->color.g = g;
        particleDataP->color.b = b;
    }
}

void Hu3DParticleTPLvlSet(HU3DMODELID modelId, float tpLvl)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP = modelP->hookData;
    HU3DPARTICLEDATA *particleDataP;
    u8 alpha;
    s16 i;
    for(particleDataP=particleP->data, alpha=tpLvl*255, i=0; i<particleP->maxCnt; i++, particleDataP++) {
        particleDataP->color.a = alpha;
    }
}

void Hu3DParticleBlendModeSet(HU3DMODELID modelId, u8 blendMode)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP = modelP->hookData;
    particleP->blendMode = blendMode;
}

void Hu3DParticleHookSet(HU3DMODELID modelId, HU3DPARTICLEHOOK hook)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP = modelP->hookData;
    particleP->hook = hook;
}

void Hu3DParticleAttrSet(HU3DMODELID modelId, u8 attr)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP = modelP->hookData;
    particleP->attr |= attr;
}

void Hu3DParticleAttrReset(HU3DMODELID modelId, u8 attr)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP = modelP->hookData;
    particleP->attr &= ~attr;
}

void Hu3DParticleCntSet(HU3DMODELID modelId, s16 count)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP = modelP->hookData;
    particleP->count = count;
}

void Hu3DParticleAnimModeSet(HU3DMODELID modelId, s16 animBank)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DPARTICLE *particleP = modelP->hookData;
    particleP->attr |= HU3D_PARTICLE_ATTR_ANIMON;
    particleP->animBank = animBank;
    particleP->animTime = 0;
    particleP->animNo = 0;
    particleP->animSpeed = 1;
}

static Vec basePos[] = {
    { -0.5f,  0.5f, 0.0f },
    {  0.5f,  0.5f, 0.0f },
    {  0.5f, -0.5f, 0.0f },
    { -0.5f, -0.5f, 0.0f }
};

static HuVec2F baseST[] = {
    { 0.0f, 0.0f },
    { 1.0f, 0.0f },
    { 1.0f, 1.0f },
    { 0.0f, 1.0f },
};

static void particleFunc(HU3DMODEL *modelP, Mtx *mtx)
{
    HuVecF *vtxBuf;
    float scale;
    float x;
    float y;
    s16 bmpFmt;
    s16 dispF;
    s32 i;
    ANIMFRAME *animFrame;
    ANIMPAT *animPat;
    ANIMDATA *anim;
    ANIMBANK *animBank;
    ANIMBMP *animBmp;
    ANIMLAYER *animLayer;
    HU3DPARTICLE *particleP;
    HU3DPARTICLEDATA *particleDataP;
    Mtx mtxInv;
    Mtx mtxPos;
    Mtx mtxRot;
    Vec scaleVtx[4];
    Vec finalVtx[4];
    Vec initVtx[4];
    ROMtx basePosMtx;

    particleP = modelP->hookData;
    anim = particleP->anim;
    if(HmfInverseMtxF3X3(*mtx, mtxInv) == FALSE) {
        PSMTXIdentity(mtxInv);
    }
    PSMTXReorder(mtxInv, basePosMtx);
    if((Hu3DPauseF == FALSE || (modelP->attr & HU3D_ATTR_NOPAUSE))) {
        if(particleP->hook && particleP->prevCounter != GlobalCounter) {
            HU3DPARTICLEHOOK hook = particleP->hook;
            hook(modelP, particleP, *mtx);
        }
    } else if(particleP->prevCounter == -1) {
        return;
    }
    particleDataP = particleP->data;
    vtxBuf = particleP->vtxBuf;
    PSMTXROMultVecArray(basePosMtx, &basePos[0], initVtx, 4);
    for(i=0, dispF=FALSE; i<particleP->maxCnt; i++, particleDataP++) {
        if(particleDataP->scale && (particleDataP->cameraBit & Hu3DCameraBit)) {
            if(!particleDataP->zRot) {
                scale = particleDataP->scale;
                vtxBuf->x = initVtx[0].x * scale + particleDataP->pos.x;
                vtxBuf->y = initVtx[0].y * scale + particleDataP->pos.y;
                vtxBuf->z = initVtx[0].z * scale + particleDataP->pos.z;
                vtxBuf++;
                vtxBuf->x = initVtx[1].x * scale + particleDataP->pos.x;
                vtxBuf->y = initVtx[1].y * scale + particleDataP->pos.y;
                vtxBuf->z = initVtx[1].z * scale + particleDataP->pos.z;
                vtxBuf++;
                vtxBuf->x = initVtx[2].x * scale + particleDataP->pos.x;
                vtxBuf->y = initVtx[2].y * scale + particleDataP->pos.y;
                vtxBuf->z = initVtx[2].z * scale + particleDataP->pos.z;
                vtxBuf++;
                vtxBuf->x = initVtx[3].x * scale + particleDataP->pos.x;
                vtxBuf->y = initVtx[3].y * scale + particleDataP->pos.y;
                vtxBuf->z = initVtx[3].z * scale + particleDataP->pos.z;
                vtxBuf++;
            } else {
                VECScale(&basePos[0], &scaleVtx[0], particleDataP->scale);
                VECScale(&basePos[1], &scaleVtx[1], particleDataP->scale);
                VECScale(&basePos[2], &scaleVtx[2], particleDataP->scale);
                VECScale(&basePos[3], &scaleVtx[3], particleDataP->scale);
                PSMTXRotRad(mtxRot, 'Z', particleDataP->zRot);
                PSMTXConcat(mtxInv, mtxRot, mtxPos);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &particleDataP->pos, vtxBuf++);
                VECAdd(&finalVtx[1], &particleDataP->pos, vtxBuf++);
                VECAdd(&finalVtx[2], &particleDataP->pos, vtxBuf++);
                VECAdd(&finalVtx[3], &particleDataP->pos, vtxBuf++);
            }
            dispF = TRUE;
        } else {
            vtxBuf->x = vtxBuf->y = vtxBuf->z = 0.0f;
            vtxBuf++;
            vtxBuf->x = vtxBuf->y = vtxBuf->z = 0.0f;
            vtxBuf++;
            vtxBuf->x = vtxBuf->y = vtxBuf->z = 0.0f;
            vtxBuf++;
            vtxBuf->x = vtxBuf->y = vtxBuf->z = 0.0f;
            vtxBuf++;
        }
    }
    if(dispF) {
        DCFlushRangeNoSync(particleP->vtxBuf, particleP->maxCnt * sizeof(Vec) * 4);
        GXLoadPosMtxImm(*mtx, 0);
        GXSetNumTevStages(1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        if(shadowModelDrawF != 0) {
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ONE, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
            GXSetZMode(0, GX_LEQUAL, 0);
        } else {
            bmpFmt = (particleP->anim->bmp->dataFmt & 0xF);
            if(bmpFmt == ANIM_BMP_I8 || bmpFmt == ANIM_BMP_I4) {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_RASC, GX_CC_ZERO);
            } else {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
            }
            if(modelP->attr & HU3D_ATTR_ZWRITE_OFF) {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
            } else {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
            }
        }
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
        if(particleP->attr & HU3D_PARTICLE_ATTR_ANIMON) {
            animBank = &anim->bank[particleP->animBank];
            animFrame = &animBank->frame[particleP->animNo];
            animPat = &anim->pat[animFrame->pat];
            HuSprTexLoad(particleP->anim, animPat->layer->bmpNo, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
            if(Hu3DPauseF == FALSE || (modelP->attr & HU3D_ATTR_NOPAUSE)) {
                for(i=0; i<(s32)particleP->animSpeed*minimumVcount; i++) {
                    particleP->animTime += 1.0f;
                    if(particleP->animTime >= animFrame->time) {
                        particleP->animNo++;
                        particleP->animTime -= animFrame->time;
                        if(particleP->animNo >= animBank->timeNum || animFrame[1].time == -1) {
                            particleP->animNo = 0;
                        }
                    }
                    animFrame = &animBank->frame[particleP->animNo];
                }
                particleP->animTime += particleP->animSpeed * minimumVcount - i;
                if(particleP->animTime >= animFrame->time) {
                    particleP->animNo++;
                    particleP->animTime -= animFrame->time;
                    if(particleP->animNo >= animBank->timeNum || animFrame[1].time == -1) {
                        particleP->animNo = 0;
                    }
                }
            }
            animLayer = animPat->layer;
            animBmp = &anim->bmp[animLayer->bmpNo];
            x = (float) animLayer->sizeX / animBmp->sizeX;
            y = (float) animLayer->sizeY / animBmp->sizeY;
            PSMTXScale(mtxInv, x, y, 1.0f);
            x = (float) animLayer->startX / animBmp->sizeX;
            y = (float) animLayer->startY / animBmp->sizeY;
            mtxTransCat(mtxInv, x, y, 0.0f);
            GXLoadTexMtxImm(mtxInv, GX_TEXMTX0, GX_MTX2x4);
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
        } else {
            HuSprTexLoad(particleP->anim, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
        }
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(0);
        switch (particleP->blendMode) {
            case HU3D_PARTICLE_BLEND_NORMAL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_ADDCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_INVCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTCLR, GX_LO_NOOP);
                break;
        }
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetArray(GX_VA_POS, particleP->vtxBuf, sizeof(HuVecF));
        GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GXSetArray(GX_VA_CLR0, &particleP->data->color, sizeof(HU3DPARTICLEDATA));
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetArray(GX_VA_TEX0, baseST, sizeof(HuVec2F));
        GXCallDisplayList(particleP->dlBuf, particleP->dlSize);
        totalPolyCnt += particleP->maxCnt;
    }
    if(shadowModelDrawF == FALSE) {
        if(!(particleP->attr & HU3D_PARTICLE_ATTR_STOPCNT) && Hu3DPauseF == 0) {
            particleP->count++;
        }
        if(particleP->prevCount != 0 && particleP->prevCount <= particleP->count) {
            if(particleP->attr & HU3D_PARTICLE_ATTR_RESETCNT) {
                particleP->count = 0;
            }
            particleP->count = particleP->prevCount;
        }
        particleP->prevCounter = GlobalCounter;
    }
}

#include "game/process.h"

static HUPROCESS *parManProc[HU3D_PARMAN_MAX];

static float jitterTbl[] = {
    1.0f, 0.9f, 0.7f, 0.5f,
    0.5f, 0.7f, 0.9f, 1.0f
};


static void ParManFunc();
static void ParManHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx);

void Hu3DParManInit(void)
{
    s16 i;
    for(i=0; i<HU3D_PARMAN_MAX; i++) {
        parManProc[i] = NULL;
    }
}

static void Hu3DParManParticleInit(HU3DMODELID modelId, s16 dataCnt, float scale)
{
    HU3DMODEL *modelP;
    HU3DPARTICLE *particleP;
    HU3DPARTICLEDATA *particleDataP;
    s16 i;
    Hu3DParticleHookSet(modelId, ParManHook);
    modelP = &Hu3DData[modelId];
    particleP = modelP->hookData;
    particleP->dataCnt = dataCnt;
    for(particleDataP=particleP->data, i=0; i<particleP->maxCnt; i++, particleDataP++) {
        particleDataP->scale = scale;
    }
}

HU3DPARMANID Hu3DParManCreate(ANIMDATA *anim, s16 maxCnt, HU3DPARMANPARAM *param)
{
    HU3DPARMANID parManId;
    HU3DMODELID modelId;
    HU3DPARMAN *parManP;
    for(parManId=0; parManId<HU3D_PARMAN_MAX; parManId++) {
        if(!parManProc[parManId]) {
            break;
        }
    }
    if(parManId == HU3D_PARMAN_MAX) {
        return HU3D_PARMANID_NONE;
    }
    modelId = Hu3DParticleCreate(anim, maxCnt);
    
    Hu3DParManParticleInit(modelId, parManId, 0.0f);
    parManProc[parManId] = HuPrcCreate(ParManFunc, 0, 4096, 0);
    parManProc[parManId]->property = parManP = HuMemDirectMallocNum(HEAP_HEAP, sizeof(HU3DPARMAN), HU_MEMNUM_OVL);
    parManP->modelId = modelId;
    parManP->param = param;
    parManP->attr = HU3D_PARMAN_ATTR_NONE;
    parManP->pos.x = parManP->pos.y = parManP->pos.z = 0;
    parManP->vec.x = 0;
    parManP->vec.y = 1;
    parManP->vec.z = 1;
    parManP->vacuum.x = 0;
    parManP->vacuum.y = 0;
    parManP->vacuum.z = 0;
    parManP->vacuumSpeed = 1;
    parManP->accel = 0;
    parManP->timeLimit = 0;
    parManP->parManId = parManId;
    return parManId;
}

HU3DPARMANID Hu3DParManLink(HU3DPARMANID linkParManId, HU3DPARMANPARAM *param)
{
    HU3DPARMANID parManId;
    HU3DPARMAN *parManP;
    HU3DPARMAN *linkParManP;
    for(parManId=0; parManId<HU3D_PARMAN_MAX; parManId++) {
        if(!parManProc[parManId]) {
            break;
        }
    }
    if(parManId == HU3D_PARMAN_MAX) {
        return HU3D_PARMANID_NONE;
    }
    linkParManP = parManProc[linkParManId]->property;
    parManProc[parManId] = HuPrcCreate(ParManFunc, 100, 4096, 0);
    parManProc[parManId]->property = parManP = HuMemDirectMallocNum(HEAP_HEAP, sizeof(HU3DPARMAN), HU_MEMNUM_OVL);
    parManP->modelId = linkParManP->modelId;
    parManP->param = param;
    parManP->attr = HU3D_PARMAN_ATTR_NONE;
    parManP->pos.x = parManP->pos.y = parManP->pos.z = 0;
    parManP->vec.x = 0;
    parManP->vec.y = 1;
    parManP->vec.z = 1;
    parManP->vacuum.x = 0;
    parManP->vacuum.y = 0;
    parManP->vacuum.z = 0;
    parManP->vacuumSpeed = 1;
    parManP->accel = 0;
    parManP->timeLimit = 0;
    parManP->parManId = parManId;
    return parManId;
}

void Hu3DParManKill(HU3DPARMANID parManId)
{
    HU3DPARMAN *parManLinkP;
    if(parManProc[parManId]) {
        HU3DPARMAN *parManP = parManProc[parManId]->property;
        HU3DMODEL *modelP = &Hu3DData[parManP->modelId];
        HU3DPARTICLE *particleP = modelP->hookData;
        HU3DPARTICLEDATA *particleDataP;
        s16 i;
        for(particleDataP=particleP->data, i=0; i<particleP->maxCnt; i++, particleDataP++) {
            if(particleDataP->parManId == parManId) {
                particleDataP->scale = 0;
            }
        }
        for(i=0; i<HU3D_PARMAN_MAX; i++) {
            if(!parManProc[i]) {
                continue;;
            }
            if(i != parManId) {
                parManLinkP = parManProc[i]->property;
                if(parManLinkP->modelId == parManP->modelId) {
                    break;
                }
            }
        }
        if(i == HU3D_PARMAN_MAX) {
            Hu3DModelKill(parManP->modelId);
        }
        HuPrcKill(parManProc[parManId]);
        parManProc[parManId] = NULL;
        HuMemDirectFree(parManP);
    }
    
}

void Hu3DParManAllKill(void)
{
    HU3DPARMANID parManId;
    for(parManId=0; parManId<HU3D_PARMAN_MAX; parManId++) {
        if(parManProc[parManId]) {
            Hu3DParManKill(parManId);
        }
    }
}

HU3DPARMAN *Hu3DParManPtrGet(HU3DPARMANID parManId)
{
    return parManProc[parManId]->property;
}

void Hu3DParManPosSet(HU3DPARMANID parManId, float posX, float posY, float posZ)
{
    HU3DPARMAN *parManP = parManProc[parManId]->property;
    parManP->pos.x = posX;
    parManP->pos.y = posY;
    parManP->pos.z = posZ;
}

void Hu3DParManVecSet(HU3DPARMANID parManId, float x, float y, float z)
{
    HU3DPARMAN *parManP = parManProc[parManId]->property;
    parManP->vec.x = x;
    parManP->vec.y = y;
    parManP->vec.z = z;
}

void Hu3DParManRotSet(HU3DPARMANID parManId, float rotX, float rotY, float rotZ)
{
    HU3DPARMAN *parManP = parManProc[parManId]->property;
    Mtx rotMtx;
    mtxRot(rotMtx, rotX, rotY, rotZ);
    parManP->vec.x = rotMtx[0][2];
    parManP->vec.y = rotMtx[1][2];
    parManP->vec.z = rotMtx[2][2];
}

void Hu3DParManAttrSet(HU3DPARMANID parManId, s32 attr)
{
    HU3DPARMAN *parManP = parManProc[parManId]->property;
    parManP->attr |= attr;
}

void Hu3DParManAttrReset(HU3DPARMANID parManId, s32 attr)
{
    HU3DPARMAN *parManP = parManProc[parManId]->property;
    parManP->attr &= ~attr;
}

HU3DMODELID Hu3DParManModelIDGet(HU3DPARMANID parManId)
{
    HU3DPARMAN *parManP = parManProc[parManId]->property;
    return parManP->modelId;
}

void Hu3DParManTimeLimitSet(HU3DPARMANID parManId, s32 timeLimit)
{
    HU3DPARMAN *parManP = parManProc[parManId]->property;
    parManP->timeLimit = timeLimit;
}

void Hu3DParManVacumeSet(HU3DPARMANID parManId, float x, float y, float z, float speed)
{
    HU3DPARMAN *parManP;
    Hu3DParManAttrSet(parManId, HU3D_PARMAN_ATTR_VACUUM);
    parManP = parManProc[parManId]->property;
    parManP->vacuum.x = x;
    parManP->vacuum.y = y;
    parManP->vacuum.z = z;
    parManP->vacuumSpeed = speed;
}

void Hu3DParManColorSet(HU3DPARMANID parManId, s16 color)
{
    HU3DPARMAN *parManP;
    Hu3DParManAttrSet(parManId, HU3D_PARMAN_ATTR_SETCOLOR);
    parManP = parManProc[parManId]->property;
    parManP->color = color;
}

void Hu3DParManLayerSet(HU3DPARMANID parManId, s16 layer)
{
    Hu3DModelLayerSet(Hu3DParManModelIDGet(parManId), layer);
}

static void ParManFunc()
{
    HUPROCESS *processP;
    HU3DPARMAN *parManP;
    HU3DPARMANPARAM *param;
    HU3DMODEL *modelP;
    HU3DPARTICLE *particleP;
    HU3DPARTICLEDATA *particleDataP;
    HU3DPARTICLEDATA *particleDataEnd;
    Vec vecDir;
    Vec vel;
    Vec dir;
    Vec up;
    float c;
    float s;
    float angleStart;
    float upRot;
    float accelVal;
    float rot;
    s16 colorIdx;
    s16 circleIdx;

    processP = HuPrcCurrentGet();
    parManP = processP->property;
    param = parManP->param;
    modelP = &Hu3DData[parManP->modelId];
    while(1) {
        if(Hu3DPauseF && !(modelP->attr & HU3D_ATTR_NOPAUSE)) {
            HuPrcVSleep();
            continue;
        }
        particleP = modelP->hookData;
        particleDataP = particleP->data;
        if(parManP->attr & HU3D_PARMAN_ATTR_RANDTIME90) {
            accelVal = param->accelRange*0.9+frandmod((u32)param->accelRange*0.1*1000.0)/1000.0f;
        } else if(parManP->attr & HU3D_PARMAN_ATTR_RANDTIME70) {
            accelVal = param->accelRange*0.7+frandmod((u32)param->accelRange*0.3*1000.0)/1000.0f;
        } else {
            accelVal = param->accelRange;
        }
        parManP->accel += accelVal;
        circleIdx = 0;
        particleDataEnd = &particleP->data[particleP->maxCnt];
        if(parManP->attr & HU3D_PARMAN_ATTR_RANDANGLE) {
            angleStart = frandmod((u32)(360.0f/param->accelRange)*100)/100;
        }
        while(parManP->accel >= 1.0f) {
            if(parManP->attr & HU3D_PARMAN_ATTR_TIMEUP) {
                parManP->accel -= 1.0f;
            } else {
                while(particleDataP < particleDataEnd) {
                    if(!particleDataP->scale) {
                        s = param->scaleBase;
                        if(parManP->attr & HU3D_PARMAN_ATTR_RANDSCALE90) {
                            s = s*0.9+(frandmod((u32)(s*0.1*1000.0))/1000.0f);
                        } else if(parManP->attr & HU3D_PARMAN_ATTR_RANDSCALE70) {
                            s = s*0.7+(frandmod((u32)(s*0.3*1000.0))/1000.0f);
                        }
                        particleDataP->scaleBase = s;
                        particleDataP->scale = s;
                        particleDataP->pos = parManP->pos;
                        vel.x = frandmod((u32)(param->scaleRange*2.0f))-param->scaleRange;
                        vel.y = frandmod((u32)(param->scaleRange*2.0f))-param->scaleRange;
                        vel.z = frandmod((u32)(param->scaleRange*2.0f))-param->scaleRange;
                        VECNormalize(&vel, &vel);
                        VECScale(&vel, &vel, param->scaleRange);
                        VECAdd(&vel, &particleDataP->pos, &particleDataP->pos);
                        VECNormalize(&parManP->vec, &vecDir);
                        if(parManP->attr & HU3D_PARMAN_ATTR_RANDANGLE) {
                            upRot = angleStart+(360.0f/param->accelRange)*circleIdx;
                            rot = param->angleRange;
                        } else {
                            upRot = frandmod(360);
                            if(param->angleRange) {
                                rot = frandmod((u32)param->angleRange);
                            } else {
                                rot = 0.0f;
                            }
                        }
                        if(vecDir.x * vecDir.x < 0.000001 && vecDir.z * vecDir.z < 0.000001) {
                            up.x = 1.0f;
                            up.y = up.z = 0.0f;
                        } else {
                            if(vecDir.y * vecDir.y > 0.000001) {
                                dir.x = vecDir.x;
                                dir.y = 0.0f;
                                dir.z = vecDir.z;
                            } else {
                                dir.x = vecDir.x;
                                dir.y = 1.0f;
                                dir.z = vecDir.z;
                            }
                            VECCrossProduct(&dir, &vecDir, &up);
                        }
                        VECNormalize(&up, &up);
                        s = HuSin(upRot);
                        c = HuCos(upRot);
                        dir.x = up.x * (vecDir.x * vecDir.x + c * (1.0f - vecDir.x * vecDir.x))
                            + up.y * (vecDir.x * vecDir.y * (1.0f - c) - vecDir.z * s)
                            + up.z * (vecDir.x * vecDir.z * (1.0f - c) + vecDir.y * s);
                        dir.y = up.x * (vecDir.x * vecDir.y * (1.0f - c) + vecDir.z * s)
                            + up.y * (vecDir.y * vecDir.y + c * (1.0f - vecDir.y * vecDir.y))
                            + up.z * (vecDir.y * vecDir.z * (1.0f - c) - vecDir.x * s);
                        dir.z = up.x * (vecDir.x * vecDir.z * (1.0f - c) - vecDir.y * s)
                            + up.y * (vecDir.y * vecDir.z * (1.0f - c) + vecDir.x * s)
                            + up.z * (vecDir.z * vecDir.z + c * (1.0f - vecDir.z * vecDir.z));
                        VECCrossProduct(&dir, &vecDir, &up);
                        s = HuSin(rot);
                        c = HuCos(rot);
                        dir.x = vecDir.x * (up.x * up.x + c * (1.0f - up.x * up.x))
                            + vecDir.y * (up.x * up.y * (1.0f - c) - up.z * s)
                            + vecDir.z * (up.x * up.z * (1.0f - c) + up.y * s);
                        dir.y = vecDir.x * (up.x * up.y * (1.0f - c) + up.z * s)
                            + vecDir.y * (up.y * up.y + c * (1.0f - up.y * up.y))
                            + vecDir.z * (up.y * up.z * (1.0f - c) - up.x * s);
                        dir.z = vecDir.x * (up.x * up.z * (1.0f - c) - up.y * s)
                            + vecDir.y * (up.y * up.z * (1.0f - c) + up.x * s)
                            + vecDir.z * (up.z * up.z + c * (1.0f - up.z * up.z));
                        VECNormalize(&dir, &dir);
                        s = param->speedBase;
                        if(parManP->attr & HU3D_PARMAN_ATTR_RANDSPEED90) {
                            s = s*0.9+frandmod((u32)(s*0.1*1000.0))/1000.0f;
                        } else if(parManP->attr & HU3D_PARMAN_ATTR_RANDSPEED70) {
                            s = s*0.7+frandmod((u32)(s*0.3*1000.0))/1000.0f;
                        } else if(parManP->attr & HU3D_PARMAN_ATTR_RANDSPEED100) {
                            s = frandmod((u32)(s*1000.0f))/1000.0f;
                        }
                        VECScale(&dir, &particleDataP->vel, s);
                        particleDataP->accel = param->gravity;
                        particleDataP->speedDecay = param->speedDecay;
                        if(parManP->attr & HU3D_PARMAN_ATTR_SETCOLOR) {
                            particleDataP->colorIdx = colorIdx = parManP->color;
                        } else {
                            particleDataP->colorIdx = colorIdx = frandmod(param->colorNum);
                        }
                        particleDataP->color = param->colorStart[colorIdx];
                        particleDataP->time = 0;
                        particleDataP->parManId = parManP->parManId;
                        break;
                    } else {
                        particleDataP++;
                    }
                }
                parManP->accel -= 1.0f;
                circleIdx++;
            }
        }
        if(parManP->timeLimit != 0) {
            parManP->timeLimit--;
            if(parManP->timeLimit == 0) {
                parManP->attr |= HU3D_PARMAN_ATTR_TIMEUP;
            }
        }
        HuPrcVSleep();
    }
}

static void ParManHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx)
{
    HU3DPARMANPARAM *param;
    HU3DPARMAN *parManP;
    HU3DPARTICLEDATA *particleDataP;
    GXColor *colorEnd;
    GXColor *colorStart;
    Vec vacuumAccel;
    Vec vacuumDist;
    float weight;
    s16 colorIdx;
    s16 i;

    if(Hu3DPauseF == FALSE || (modelP->attr & HU3D_ATTR_NOPAUSE)) {
        particleDataP = particleP->data;
        for(i=0; i<particleP->maxCnt; i++, particleDataP++) {
            if(particleDataP->scale) {
                parManP = parManProc[particleDataP->parManId]->property;
                param = parManP->param;
                if(parManP->attr & HU3D_PARMAN_ATTR_SCALEJITTER) {
                    particleDataP->scale = particleDataP->scaleBase*jitterTbl[(parManP->jitterNo+i)&7];
                } else {
                    particleDataP->scale = particleDataP->scaleBase;
                }
                if(!(parManP->attr & HU3D_PARMAN_ATTR_PAUSE)) {
                    s16 time = particleDataP->time;
                    particleDataP->pos.x += particleDataP->vel.x+particleDataP->accel.x;
                    particleDataP->pos.y += particleDataP->vel.y+particleDataP->accel.y;
                    particleDataP->pos.z += particleDataP->vel.z+particleDataP->accel.z;
                    VECScale(&particleDataP->vel, &particleDataP->vel, particleDataP->speedDecay);
                    VECAdd(&param->gravity, &particleDataP->accel, &particleDataP->accel);
                    if(parManP->attr & HU3D_PARMAN_ATTR_VACUUM) {
                        VECSubtract(&parManP->vacuum, &particleDataP->pos, &vacuumAccel);
                        VECNormalize(&vacuumAccel, &vacuumAccel);
                        VECScale(&vacuumAccel, &vacuumAccel, parManP->vacuumSpeed);
                        VECAdd(&vacuumAccel, &particleDataP->accel, &particleDataP->accel);
                        VECAdd(&particleDataP->vel, &particleDataP->accel, &vacuumAccel);
                        VECSubtract(&parManP->vacuum, &particleDataP->pos, &vacuumDist);
                        if(VECSquareMag(&vacuumDist) <= VECSquareMag(&vacuumAccel)) {
                            particleDataP->scale = 0.0f;
                            continue;
                        }
                    }
                    particleDataP->scaleBase *= param->scaleDecay;
                    weight = (float) particleDataP->time / param->maxTime;
                    if(weight > 1.0f) {
                        weight = 1.0f;
                    }
                    OSf32tos16(&particleDataP->colorIdx, &colorIdx);
                    colorStart = &param->colorStart[colorIdx];
                    colorEnd = &param->colorEnd[colorIdx];
                    particleDataP->color.r = colorStart->r+(weight*(colorEnd->r-colorStart->r));
                    particleDataP->color.g = colorStart->g+(weight*(colorEnd->g-colorStart->g));
                    particleDataP->color.b = colorStart->b+(weight*(colorEnd->b-colorStart->b));
                    particleDataP->color.a = colorStart->a+(weight*(colorEnd->a-colorStart->a));
                    if(particleDataP->scale < 0.01 || particleDataP->time >= param->maxTime) {
                        particleDataP->scale = 0.0f;
                    }
                    particleDataP->time++;
                }
            }
        }
        parManP = parManProc[particleP->dataCnt]->property;
        parManP->jitterNo++;
        DCStoreRangeNoSync(particleP->data, particleP->maxCnt*sizeof(HU3DPARTICLEDATA));
    }
}

#include "datanum/effect.h"
#include "game/disp.h"

HU3DWATER Hu3DWaterData;

static float waterTexMtx[2][3] = {
    0.05f, 0, 0,
    0, 0.05f, 0
};

static float waterWaveTexMtx[2][3] = {
    0.1f, 0, 0,
    0, 0.1f, 0
};

static void CopyWaterFb(s16 layerNo);
static void WaterLayerHook(s16 layerNo);

void Hu3DWaterCreate(s16 layerNo, void *animBump, void *animSurface, void *animSky, BOOL mipMapF, HuVecF *posMin, HuVecF *posMax)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    s16 fbWaterW;
    s16 fbWaterH;
    s16 i, j;
    waterP->glowMode = 0;
    if(mipMapF) {
        fbWaterW = HU_FB_WIDTH/2;
        fbWaterH = HU_FB_HEIGHT/2;
    } else {
        fbWaterW = HU_FB_WIDTH;
        fbWaterH = HU_FB_HEIGHT;
    }
    waterP->fbWater = HuMemDirectMallocNum(HEAP_MODEL, GXGetTexBufferSize(fbWaterW, fbWaterH, GX_TF_RGB565, GX_FALSE, 0), HU_MEMNUM_OVL);
    waterP->fbDisp = HuMemDirectMallocNum(HEAP_MODEL, GXGetTexBufferSize(HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, GX_FALSE, 0), HU_MEMNUM_OVL);
    if(layerNo >= HU3D_LAYER_HOOK_POST) {
        layerNo -= HU3D_LAYER_HOOK_POST;
    }
    Hu3DLayerHookSet(layerNo, CopyWaterFb);
    waterP->mipMapF = mipMapF;
    waterP->fbWaterW = fbWaterW;
    waterP->fbWaterH = fbWaterH;
    if(animBump == HU3D_WATER_ANIM_NONE) {
        waterP->animBump = HuSprAnimDataRead(EFFECT_ANM_waterBump);
    } else if(animBump) {
        waterP->animBump = HuSprAnimRead(animBump);
    } else {
        waterP->animBump = animBump;
    }
    if(animSurface == HU3D_WATER_ANIM_NONE) {
        waterP->animSurface = HuSprAnimDataRead(EFFECT_ANM_waterSurface);
    } else if(animSurface) {
        waterP->animSurface = HuSprAnimRead(animSurface);
    } else {
        waterP->animSurface = animSurface;
    }
    if(animSky == HU3D_WATER_ANIM_NONE) {
        waterP->animSky = HuSprAnimDataRead(EFFECT_ANM_waterSky);
    } else if(animSky) {
        waterP->animSky = HuSprAnimRead(animSky);
    } else {
        waterP->animSky = animSky;
    }
    waterP->posMin = *posMin;
    waterP->posMax = *posMax;
    waterP->hiliteCol.r = waterP->hiliteCol.g = waterP->hiliteCol.b = 76;
    waterP->hiliteCol.a = 25;
    waterP->glowCol.r = waterP->glowCol.g = waterP->glowCol.b = waterP->glowCol.a = 255;
    waterP->padY = 5;
    waterP->texPos.x = waterP->texPos.y = waterP->texPos.z = 0;
    waterP->texScale.x = 1;
    waterP->texScale.y = 1;
    waterP->texScale.z = 1;
    for(i=0; i<3; i++) {
        for(j=0; j<2; j++) {
            waterP->texMtx[j][i] = waterTexMtx[j][i];
        }
    }
    layerNo += HU3D_LAYER_HOOK_POST;
    waterP->layerNo = layerNo;
    waterP->cameraBit = HU3D_CAM0;
    waterP->unk7C = 0;
    waterP->animWave = NULL;
    waterP->maxTime = 60;
    for(i=0; i<32; i++) {
        waterP->wave[i].time = -1;
    }
    Hu3DLayerHookSet(layerNo, WaterLayerHook);
}

void Hu3DWaterTexPosSet(float posX, float posY, float posZ)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    waterP->texPos.x = posX;
    waterP->texPos.y = posY;
    waterP->texPos.z = posZ;
}

void Hu3DWaterTexScaleSet(float scaleX, float scaleY, float scaleZ)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    waterP->texScale.x = scaleX;
    waterP->texScale.y = scaleY;
    waterP->texScale.z = scaleZ;
}

void Hu3DWaterPadYSet(float padY)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    waterP->padY = padY;
}

void Hu3DWaterGlowSet(s16 glowMode, GXColor *glowCol)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    waterP->glowCol.r = glowCol->r;
    waterP->glowCol.g = glowCol->g;
    waterP->glowCol.b = glowCol->b;
    waterP->glowCol.a = glowCol->a;
    waterP->glowMode = glowMode;
}

void Hu3DWaterIndTexMtxSet(float texMtx[2][3])
{
    HU3DWATER *waterP = &Hu3DWaterData;
    s16 i;
    s16 j;
    for(i=0; i<3; i++) {
        for(j=0; j<2; j++) {
            waterP->texMtx[j][i] = texMtx[j][i];
        }
    }
}

void Hu3DWaterHiliteSet(float level)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    waterP->hiliteCol.r = waterP->hiliteCol.g = waterP->hiliteCol.b = level*255;
}

void Hu3DWaterHiliteTPLvlSet(float tpLvl)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    waterP->hiliteCol.a = tpLvl*255;
}

void Hu3DWaterCameraSet(u16 cameraBit)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    waterP->cameraBit = cameraBit;
}

void Hu3DWaterWaveCreate(HuVecF *pos, float radius, float radiusMax)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    s16 i;
    s16 j;
    if(!waterP->animWave) {
        waterP->animWave = HuSprAnimDataRead(EFFECT_ANM_waterWave);
        HuDataDirClose(DATA_effect);
    }
    for(i=0; i<HU3D_WAVE_MAX; i++) {
        if(waterP->wave[i].time == -1) {
            break;
        }
    }
    if(i == HU3D_WAVE_MAX) {
        return;
    }
    waterP->wave[i].time = 0;
    waterP->wave[i].pos = *pos;
    waterP->wave[i].radius = radius;
    waterP->wave[i].radiusMax = radiusMax;
    for(i=0; i<2; i++) {
        for(j=0; j<3; j++) {
            waterP->wave[i].texMtx[i][j] = waterWaveTexMtx[i][j];
        }
    }
}

static void CopyWaterFb(s16 layerNo)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    if(!(waterP->cameraBit & Hu3DCameraBit)) {
        return;
    }
    GXSetTexCopySrc(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
    if(waterP->mipMapF) {
        GXSetTexCopyDst(HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_TRUE);
    } else {
        GXSetTexCopyDst(HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, GX_FALSE);
    }
    GXCopyTex(waterP->fbWater, GX_FALSE);
}

static void DrawQuad(HuVecF *min, HuVecF *max);
static void DrawWave(HU3DWATER *waterP, s16 waveNo);

static void WaterLayerHook(s16 layerNo)
{
    HU3DWATER *waterP = &Hu3DWaterData;
    HU3DCAMERA *cameraP = &Hu3DCamera[Hu3DCameraNo];
    Mtx lookAt;
    Mtx texTrans;
    Mtx texScale;
    Mtx texMtx;
    Mtx proj;
    Vec diff;
    Vec dir;
    Vec posMin;
    Vec posMax;
    s32 tevStage;
    float diffLen;
    s32 i;
    
    if(!(waterP->cameraBit & Hu3DCameraBit)) {
        return;
    }
    Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, GX_FALSE, waterP->fbDisp);
    Hu3DTexLoad(waterP->fbWater, waterP->fbWaterW, waterP->fbWaterH, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, TRUE, GX_TEXMAP0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    MTXLightPerspective(proj, cameraP->fov, cameraP->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
    MTXConcat(proj, Hu3DCameraMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX1, GX_MTX3x4);
    GXSetTexCoordGen(GX_TEXMAP0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXMAP0, GX_TEXCOORD0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXLoadPosMtxImm(Hu3DCameraMtx, GX_PNMTX0);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(GX_TRUE, GX_GREATER, GX_FALSE);
    posMin = waterP->posMin;
    posMax = waterP->posMax;
    posMin.y += waterP->padY;
    posMax.y += waterP->padY;
    DrawQuad(&posMin, &posMax);
    Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, waterP->mipMapF, waterP->fbWater);
    Hu3DTexLoad(waterP->fbDisp, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, TRUE, GX_TEXMAP0);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    DrawQuad(&posMin, &posMax);
    Hu3DTexLoad(waterP->fbWater, waterP->fbWaterW, waterP->fbWaterH, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, TRUE, GX_TEXMAP0);
    HuSprTexLoad(waterP->animBump, 0, GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    if(waterP->animSky) {
        HuSprTexLoad(waterP->animSky, 0, GX_TEXMAP2, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    }
    if(waterP->animSurface) {
        HuSprTexLoad(waterP->animSurface, 0, GX_TEXMAP3, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    }
    tevStage = 0;
    MTXLightPerspective(proj, cameraP->fov, cameraP->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
    MTXConcat(proj, Hu3DCameraMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen(tevStage, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
    GXSetTevOrder(tevStage, GX_TEXMAP0, GX_TEXCOORD0, GX_COLOR0);
    GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevColor(GX_TEVREG0, waterP->hiliteCol);
    if(waterP->animSky) {
        tevStage++;
        VECSubtract(&cameraP->target, &cameraP->pos, &diff);
        diffLen = VECMag(&diff);
        dir.x = 0;
        dir.y = -1;
        dir.z = -0.3f;
        VECNormalize(&dir, &dir);
        VECHalfAngle(&diff, &dir, &diff);
        VECScale(&diff, &diff, diffLen);
        VECAdd(&cameraP->target, &diff, &dir);
        MTXLookAt(lookAt, &dir, &cameraP->up, &cameraP->target);
        MTXLightPerspective(proj, cameraP->fov, cameraP->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
        MTXConcat(proj, lookAt, texMtx);
        GXLoadTexMtxImm(texMtx, GX_TEXMTX1, GX_MTX3x4);
        GXSetTexCoordGen(tevStage, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);
        GXSetTevOrder(tevStage, GX_TEXMAP1, GX_TEXCOORD2, GX_COLOR0);
        GXSetTevColorIn(tevStage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
        GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    if(waterP->glowMode & 0xF) {
        tevStage++;
        GXSetTevColor(GX_TEVREG1, waterP->glowCol);
        GXSetTevOrder(tevStage, GX_TEXMAP_NULL, GX_TEXCOORD_NULL, GX_COLOR0);
        if(waterP->glowMode & 0x1) {
            GXSetTevColorIn(tevStage, GX_CC_C1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
            GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        } else if(waterP->glowMode & 0x2) {
            GXSetTevColorIn(tevStage, GX_CC_C1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
            GXSetTevColorOp(tevStage, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        } else if(waterP->glowMode & 0x4) {
            GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_C1, GX_CC_CPREV, GX_CC_ZERO);
            GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        } else if(waterP->glowMode & 0x8) {
            GXSetTevColorIn(tevStage, GX_CC_C1, GX_CC_CPREV, GX_CC_A1, GX_CC_ZERO);
            GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        }
        GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    if(waterP->animSurface) {
        tevStage++;
        GXSetTevOrder(tevStage, GX_TEXMAP2, GX_TEXCOORD3, GX_COLOR0);
        GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_CPREV);
        GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    tevStage++;
    GXSetNumTexGens(3);
    GXSetNumTevStages(tevStage);
    MTXTrans(texTrans, waterP->texPos.x, waterP->texPos.y, waterP->texPos.z);
    MTXScale(texScale, waterP->texScale.x, waterP->texScale.y, waterP->texScale.z);
    MTXConcat(texScale, texTrans, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX2, GX_MTX2x4);
    GXSetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, waterP->texMtx, -1);
    if(waterP->animSky) {
        GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD2, GX_TEXMAP1);
        GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_1, GX_ITS_1);
        GXSetTevIndWarp(GX_TEVSTAGE1, GX_INDTEXSTAGE1, GX_TRUE, GX_FALSE, GX_ITM_0);
        GXSetNumIndStages(2);
    } else {
        GXSetNumIndStages(1);
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXLoadPosMtxImm(Hu3DCameraMtx, GX_PNMTX0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    DrawQuad(&waterP->posMin, &waterP->posMax);
    for(i=0; i<HU3D_WAVE_MAX; i++) {
        if(waterP->wave[i].time != -1) {
            DrawWave(waterP, i);
        }
    }
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
}

void Hu3DTexLoad(void *buf, s16 w, s16 h, u32 format, GXTexWrapMode wrapS, GXTexWrapMode wrapT, BOOL filterF, GXTexMapID texMapId)
{
    GXTexObj texObj;
    GXInitTexObj(&texObj, buf, w, h, format, wrapS, wrapT, GX_FALSE);
    GXInitTexObjLOD(&texObj, (filterF) ? GX_LINEAR : GX_NEAR, (filterF) ? GX_LINEAR : GX_NEAR, 0, 0, 0, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, texMapId);
}

static void DrawQuad(HuVecF *min, HuVecF *max)
{
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetZCompLoc(GX_TRUE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(min->x, min->y, min->z);
    GXTexCoord2f32(0, 0);
    GXPosition3f32(max->x, max->y, min->z);
    GXTexCoord2f32(1, 0);
    GXPosition3f32(max->x, max->y, max->z);
    GXTexCoord2f32(1, 1);
    GXPosition3f32(min->x, min->y, max->z);
    GXTexCoord2f32(0, 1);
    GXEnd();
}

static void DrawWave(HU3DWATER *waterP, s16 waveNo)
{
    HU3DWATERWAVE *waveP = &waterP->wave[waveNo];
    HU3DCAMERA *cameraP = &Hu3DCamera[Hu3DCameraNo];
    s32 tevStage;
    Mtx lookAt;
    Mtx texTrans;
    Mtx texScale;
    Mtx texMtx;
    Mtx proj;
    Vec diff;
    Vec dir;
    HuVecF min;
    HuVecF max;
    float size;
    float len;
    
    GXColor color;
    
    Hu3DTexLoad(waterP->fbWater, waterP->fbWaterW, waterP->fbWaterH, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, TRUE, GX_TEXMAP0);
    HuSprTexLoad(waterP->animWave, 0, GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    if(waterP->animSky) {
        HuSprTexLoad(waterP->animSky, 0, GX_TEXMAP2, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    }
    tevStage = 0;
    MTXLightPerspective(proj, cameraP->fov, cameraP->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
    MTXConcat(proj, Hu3DCameraMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen(tevStage, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
    GXSetTevOrder(tevStage, GX_TEXMAP0, GX_TEXCOORD0, GX_COLOR0);
    GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevColor(GX_TEVREG0, waterP->hiliteCol);
    if(waterP->animSky) {
        tevStage++;
        VECSubtract(&cameraP->target, &cameraP->pos, &diff);
        len = VECMag(&diff);
        dir.x = 0;
        dir.y = -1;
        dir.z = -0.3f;
        VECNormalize(&dir, &dir);
        VECHalfAngle(&diff, &dir, &diff);
        VECScale(&diff, &diff, len);
        VECAdd(&cameraP->target, &diff, &dir);
        MTXLookAt(lookAt, &dir, &cameraP->up, &cameraP->target);
        MTXLightPerspective(proj, cameraP->fov, cameraP->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
        MTXConcat(proj, lookAt, texMtx);
        GXLoadTexMtxImm(texMtx, GX_TEXMTX1, GX_MTX3x4);
        GXSetTexCoordGen(tevStage, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);
        GXSetTevOrder(tevStage, GX_TEXMAP1, GX_TEXCOORD2, GX_COLOR0);
        GXSetTevColorIn(tevStage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
        GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    tevStage++;
    if(waveP->time > 0.6*waterP->maxTime) {
        color.a = 230*(1-((waveP->time-(0.6*waterP->maxTime))/(0.4*waterP->maxTime)));
    } else {
        color.a = 230;
    }
    color.r = color.g = color.b = 16;
    GXSetTevColor(GX_TEVREG1, color);
    GXSetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(tevStage, GX_TEXMAP2, GX_TEXCOORD1, GX_COLOR0);
    GXSetTevColorIn(tevStage, GX_CC_C1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A1, GX_CA_ZERO);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    tevStage++;
    GXSetNumTexGens(4);
    GXSetNumTevStages(tevStage);
    MTXTrans(texTrans, waterP->texPos.x, waterP->texPos.y, waterP->texPos.z);
    MTXScale(texScale, waterP->texScale.x, waterP->texScale.y, waterP->texScale.z);
    MTXConcat(texScale, texTrans, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX3, GX_MTX2x4);
    GXSetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD3, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, waveP->texMtx, -1);
    if(waterP->animSky) {
        GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD3, GX_TEXMAP1);
        GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_1, GX_ITS_1);
        GXSetTevIndWarp(GX_TEVSTAGE1, GX_INDTEXSTAGE1, GX_TRUE, GX_FALSE, GX_ITM_0);
        GXSetNumIndStages(2);
    } else {
        GXSetNumIndStages(1);
    }
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXLoadPosMtxImm(Hu3DCameraMtx, GX_PNMTX0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    len = HuSin(((float)waveP->time/waterP->maxTime)*90.0);
    max = waveP->pos;
    min = max;
    size = (0.5*waveP->radius)+(0.5*(len*(waveP->radiusMax-waveP->radius)));
    min.x -= size;
    min.z -= size;
    max.x += size;
    max.z += size;
    DrawQuad(&min, &max);
    waveP->time++;
    if(waveP->time > waterP->maxTime) {
        waveP->time = -1;
    }
}