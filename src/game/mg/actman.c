#include "game/mg/actman.h"
#include "game/mg/colman.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/frand.h"

#define ACTOR_MAX COLBODY_MAX
#define PLAYER_MAX GW_PLAYER_MAX

typedef struct ActManWork_s {
    MGPLAYER *player;
    MGACTOR *actor;
    MGACTOR *effPlayer[4];
    BOOL colCylF;
    OMOBJMAN *objman;
} ACTMAN_WORK;

static ACTMAN_WORK actmanWork;

static void PlayerModeWalk(MGPLAYER *playerP);
static void PlayerModeJump(MGPLAYER *playerP);
static void PlayerModeFall(MGPLAYER *playerP);
static void PlayerModePunch(MGPLAYER *playerP);
static void PlayerModeKick(MGPLAYER *playerP);
static void PlayerModeHipDrop(MGPLAYER *playerP);
static void PlayerModeHit(MGPLAYER *playerP);
static void PlayerModeKnockback(MGPLAYER *playerP);
static void PlayerModeSquish(MGPLAYER *playerP);
static void PlayerModeSquishHard(MGPLAYER *playerP);
static void PlayerModeDefault(MGPLAYER *playerP);
static void PlayerModeJumpAlt(MGPLAYER *playerP);

static MGPLAYER_MODE_FUNC playerModeFunc[MGPLAYER_MODE_MAX] = {
    PlayerModeWalk,
    PlayerModeJump,
    PlayerModeFall,
    PlayerModePunch,
    PlayerModeKick,
    PlayerModeHipDrop,
    PlayerModeHit,
    PlayerModeKnockback,
    PlayerModeSquish,
    PlayerModeSquishHard,
    PlayerModeDefault,
    PlayerModeDefault,
    PlayerModeDefault,
    PlayerModeDefault,
    PlayerModeDefault,
    PlayerModeDefault,
    PlayerModeDefault,
    PlayerModeDefault
};

//Must be macro for stack ordering reasons in MgActorExec
#define SafeNormalize(in, out) do {\
    BOOL result; \
    if(VECSquareMag(in) < 1e-6) { \
        (out)->x = (frandmod(20)-10.0f)*0.01f; \
        (out)->z = (frandmod(20)-10.0f)*0.01f; \
        (out)->y = (frandmod(1) != 0) ? 0.01f : -0.01f; \
        VECNormalize(out, out); \
        result = FALSE; \
    } else { \
        VECNormalize(in, out); \
        result = TRUE; \
    } \
} while(0)

//Must be also present as full function for sdata2 ordering
static void SafeNormalize2(HuVecF *in, HuVecF *out)
{
    BOOL result;
    if(VECSquareMag(in) < 1e-6) {
        (out)->x = (frandmod(20)-10.0f)*0.01f;
        (out)->z = (frandmod(20)-10.0f)*0.01f;
        (out)->y = (frandmod(1) != 0) ? 0.01f : -0.01f;
        VECNormalize(out, out);
        result = FALSE;
    } else { \
        VECNormalize(in, out);
        result = TRUE;
    }
}

static void ExecVibrate(OMOBJ *obj)
{
    MGPLAYER *playerP = (MGPLAYER *)obj->work[0];
    if(MgPlayerVibAttrCheck(playerP, MGPLAYER_VIBATTR_SQUISH_FAST)) {
        omVibrate(playerP->playerNo, 12, 4, 2);
    } else if(MgPlayerVibAttrCheck(playerP, MGPLAYER_VIBATTR_SQUISH)) {
        omVibrate(playerP->playerNo, 12, 4, 2);
    } else if(MgPlayerVibAttrCheck(playerP, MGPLAYER_VIBATTR_HEADJUMP)) {
        omVibrate(playerP->playerNo, 12, 4, 2);
    } else if(MgPlayerVibAttrCheck(playerP, MGPLAYER_VIBATTR_HIT_SRC)) {
        omVibrate(playerP->playerNo, 12, 4, 2);
    } else if(MgPlayerVibAttrCheck(playerP, MGPLAYER_VIBATTR_KNOCKBACK_SRC)) {
        omVibrate(playerP->playerNo, 12, 4, 2);
    } else if(MgPlayerVibAttrCheck(playerP, 0x200)) { //Should Check MGPLAYER_VIBATTR_SQUISH_HARD_SRC
        omVibrate(playerP->playerNo, 12, 4, 2);
    }
    switch(playerP->stunType) {
        case MGPLAYER_STUN_HIT:
            omVibrate(playerP->playerNo, 12, 4, 2);
            break;
           
        case MGPLAYER_STUN_KNOCKBACK:
            omVibrate(playerP->playerNo, 12, 4, 2);
            break;
        
        case MGPLAYER_STUN_SQUISH_HARD:
            omVibrate(playerP->playerNo, 12, 4, 2);
            break;
    }
    if(MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_VIBKILL)) {
        MgPlayerAttrReset(playerP, MGPLAYER_ATTR_VIBKILL);
        obj->objFunc = NULL;
        omDelObj(actmanWork.objman, obj);
    }
}

BOOL MgPlayerComStkOn(MGPLAYER *playerP)
{
    BOOL ret = MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK) ? TRUE : FALSE;
    MgPlayerAttrSet(playerP, MGPLAYER_ATTR_COMSTK);
    return ret;
}

void MgPlayerComStkOff(MGPLAYER *playerP)
{
    MgPlayerAttrReset(playerP, MGPLAYER_ATTR_COMSTK);
}

BOOL MgPlayerVecChase(MGPLAYER *playerP, HuVecF *pos, float velZ, float radius)
{
    float r2 = 0.001+(radius*radius);
    float angle;
    HuVecF dir;
    if(MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_MOVEOFF)) {
        return FALSE;
    }
    VECSubtract(pos, &playerP->actor->pos, &dir);
    dir.y = 0;
    if(VECSquareMag(&dir) < r2) {
        return TRUE;
    }
    VECNormalize(&dir, &dir);
    if(dir.z > 1) {
        dir.z = 1;
    } else if(dir.z < -1) {
        dir.z = -1;
    }
    angle = acosf(dir.z);
    if(dir.x < 0) {
        angle = -angle;
    }
    playerP->actor->rotY = (angle*180)/M_PI;
    playerP->actor->vel.x = playerP->actor->vel.y = 0;
    playerP->actor->vel.z = velZ;    
    return FALSE;
}

BOOL MgActorVecChase(MGACTOR *actorP, HuVecF *pos, float velZ, float radius)
{
    float r2 = 0.001+(radius*radius);
    float angle;
    HuVecF dir;
    VECSubtract(pos, &actorP->pos, &dir);
    dir.y = 0;
    if(VECSquareMag(&dir) < r2) {
        return TRUE;
    }
    VECNormalize(&dir, &dir);
    if(dir.z > 1) {
        dir.z = 1;
    } else if(dir.z < -1) {
        dir.z = -1;
    }
    angle = acosf(dir.z);
    if(dir.x < 0) {
        angle = -angle;
    }
    actorP->rotY = (angle*180)/M_PI;
    actorP->vel.x = actorP->vel.y = 0;
    actorP->vel.z = velZ;
    return FALSE;
}

void MgPlayerVibrateCreate(MGPLAYER *playerP)
{
    OMOBJ *obj = omAddObj(actmanWork.objman, 3000, 0, 0, ExecVibrate);
    obj->work[0] = (u32)playerP;
}

void MgPlayerVibrateKill(MGPLAYER *playerP)
{
    MgPlayerAttrSet(playerP, MGPLAYER_ATTR_VIBKILL);
}

static void KillEffectPlayer(int playerNo);

BOOL MgPlayerModeLandSet(MGPLAYER *playerP, int mode)
{
    if(playerP->mode == mode) {
        return FALSE;
    }
    if(playerP->stunType == MGPLAYER_STUN_FREEZE) {
        return FALSE;
    }
    switch(mode) {
        case MGPLAYER_MODE_KICK:
        case MGPLAYER_MODE_HIPDROP:
            if(playerP->mode != MGPLAYER_MODE_JUMP && playerP->mode != MGPLAYER_MODE_FALL) {
                return FALSE;
            }
            break;
           
        case MGPLAYER_MODE_JUMP:
        case MGPLAYER_MODE_FALL:
        case MGPLAYER_MODE_PUNCH:
            if(playerP->mode != MGPLAYER_MODE_WALK) {
                return FALSE;
            }
            break;
    }
    KillEffectPlayer(playerP->actor->no);
    playerP->mode = mode;
    playerP->subMode = 0;
    return TRUE;
}

BOOL MgPlayerModeSet(MGPLAYER *playerP, int mode)
{
    if(playerP->mode == mode) {
        return FALSE;
    }
    if(playerP->stunType == MGPLAYER_STUN_FREEZE) {
        return FALSE;
    }
    KillEffectPlayer(playerP->actor->no);
    playerP->mode = mode;
    playerP->subMode = 0;
    return TRUE;
}

static float WrapAngle(float angle)
{
    angle = fmod(angle, 360);
    if(angle < -180) {
        angle += 360;
    } else if(angle > 180) {
        angle -= 360;
    }
    return angle;
}

static MGACTOR *SetupActor(int no)
{
    MGACTOR *actorP = &actmanWork.actor[no];
    memset(actorP, 0, sizeof(MGACTOR));
    memset(&actorP->oldPos, 0, sizeof(HuVecF));
    memset(&actorP->vel, 0, sizeof(HuVecF));
    memset(&actorP->pos, 0, sizeof(HuVecF));
    memset(&actorP->moveDir, 0, sizeof(HuVecF));
    memset(&actorP->colNorm, 0, sizeof(HuVecF));
    memset(&actorP->forceA, 0, sizeof(HuVecF));
    memset(&actorP->forceB, 0, sizeof(HuVecF));
    actorP->no = no;
    actorP->mdlId = 0;
    actorP->param = 0;
    actorP->type = 0;
    actorP->rotY = 0;
    actorP->gravity = 150;
    actorP->velY = 0;
    actorP->terminalVelY = 0;
    actorP->colMesh = 0;
    actorP->colObj = NULL;
    actorP->colFace = 0;
    actorP->colGroundAttr = 0;
    actorP->correctHookParam = 0;
    actorP->correctHook = NULL;
    return actorP;
}

static void GetStickMtx(Mtx out, int camBit)
{
    HuVecF pos, target, up;
    HuVecF row1, row2, row3;
    HuVecF temp;
    HuVecF dir;
    HuVecF camPos, camTarget, camUp;
    Hu3DCameraPosGet(camBit, &camPos, &camUp, &camTarget);
    pos.x = camPos.x;
    pos.y = camPos.y;
    pos.z = camPos.z;
    target.x = camTarget.x;
    target.y = camTarget.y;
    target.z = camTarget.z;
    up.x = camUp.x;
    up.y = camUp.y;
    up.z = camUp.z;
    row1.x = 1;
    row1.y = 0;
    row1.z = 0;
    row2.x = 0;
    row2.y = 1;
    row2.z = 0;
    row3.x = 0;
    row3.y = 0;
    row3.z = 1;
    VECSubtract(&target, &pos, &dir);
    temp = dir;
    temp.y = 0;
    if(VECSquareMag(&temp) < 0.001) {
        temp = dir;
        dir = up;
        VECScale(&temp, &up, -1);
    }
    dir.y = 0;
    VECNormalize(&dir, &dir);
    if(VECDotProduct(&row2, &up) < 0) {
        row2.y *= -1;
        row3.z *= -1;
    }
    VECCrossProduct(&dir, &row2, &row1);
    row3 = dir;
    out[0][0] = row1.x;
    out[0][1] = row1.y;
    out[0][2] = row1.z;
    out[0][3] = 0;
    out[1][0] = row2.x;
    out[1][1] = row2.y;
    out[1][2] = row2.z;
    out[1][3] = 0;
    out[2][0] = row3.x;
    out[2][1] = row3.y;
    out[2][2] = row3.z;
    out[2][3] = 0;
}

static int AllocPlayer(void)
{
    int i;
    for(i=0; i<PLAYER_MAX; i++) {
        COLBODY *actorP = ColBodyGetRaw(i);
        if(!(actorP->param.attr & COLBODY_ATTR_ACTIVE)) {
            actorP->param.attr |= COLBODY_ATTR_ACTIVE;
            actorP->param.attr |= COLBODY_ATTR_RESET;
            break;
        }
    }
    if(i == PLAYER_MAX) {
        return -1;
    }
    SetupActor(i);
    return i;
}


static MGPLAYER *CreatePlayer(s16 playerNo, s16 model, s16 camBit, u32 actionFlag, unsigned int *motDataNum)
{
    int actorNo;
    if(!ColMapInitCheck()) {
        return NULL;
    }
    actorNo = AllocPlayer();
    if(actorNo < 0) {
        return NULL;
    } else {
        MGPLAYER *playerP;
        OMOBJ *obj;
        int mdlId;
        playerP = &actmanWork.player[actorNo];
        memset(playerP, 0, sizeof(MGPLAYER));
        obj = omAddObj(actmanWork.objman, playerNo, 1, 16, NULL);
        playerP->actor = &actmanWork.actor[actorNo];
        playerP->playerNo = playerNo;
        playerP->charNo = GwPlayerConf[playerNo].charNo;
        playerP->padNo = GwPlayerConf[playerNo].padNo;
        playerP->camBit = camBit;
        playerP->omObj = obj;
        playerP->actionFlag = actionFlag;
        playerP->motNo = 0;
        playerP->stunType = MGPLAYER_STUN_NONE;
        playerP->stunAngle = 0;
        playerP->stunTime = 0;
        playerP->squishTime = 0;
        playerP->attr = 0;
        playerP->modeAttr = 0;
        playerP->vibAttr = 0;
        playerP->subMode = 0;
        playerP->mode = 0;
        playerP->timer = 0;
        playerP->squishOldHeight = 0;
        playerP->work[0] = 0;
        playerP->work[1] = 0;
        playerP->work[2] = 0;
        memcpy(&playerP->modeFunc[0], &playerModeFunc[0], sizeof(playerModeFunc));
        mdlId = CharModelMotListCreate(playerP->charNo, model, motDataNum, obj->mtnId);
        playerP->actor->mdlId = mdlId;
        obj->work[0] = obj->work[1] = obj->work[2] = obj->work[3] = 0;
        Hu3DModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
        CharMotionSet(playerP->charNo, playerP->omObj->mtnId[0]);
        playerP->motNo = 0;
        return playerP;
    }
}

static MGACTOR *CreateActor(int mdlId)
{
    MGACTOR *actorP;
    int i;
    if(!ColMapInitCheck()) {
        return NULL;
    }
    for(i=PLAYER_MAX; i<ACTOR_MAX; i++) {
        COLBODY *body = ColBodyGetRaw(i);
        if(!(body->param.attr & COLBODY_ATTR_ACTIVE)) {
            body->param.attr |= COLBODY_ATTR_ACTIVE;
            body->param.attr |= COLBODY_ATTR_RESET;
            break;
        }
    }
    if(i == ACTOR_MAX) {
        return NULL;
    }
    actorP = SetupActor(i);
    actorP->mdlId = mdlId;
    return actorP;
}

static void KillEffectPlayer(int playerNo)
{
    if(actmanWork.effPlayer[playerNo]) {
        MgActorKill(actmanWork.effPlayer[playerNo]);
        actmanWork.effPlayer[playerNo] = NULL;
    }
}

extern inline float fabsf2(float x)
{
   return (float)fabs((float)x);
}

#define GET_ACTOR_VELY(actorP, speed) \
    ((actorP->gravity) ? (actorP->gravity+(actorP->gravity*((-1+sqrtf(1+(8*(speed/actorP->gravity))))/2))) : 0)

static int PlayerColHook(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    MGPLAYER *playerP1;
    MGACTOR *actorP; //sp+0x30
    
    playerP1 = &actmanWork.player[a->paramA];
    actorP = &actmanWork.actor[b->paramA];
    switch(b->type) {
        case 0:
        case 1:
        {
            HuVecF dir;
            HuVecF up;
            MGPLAYER *playerP2;
            BOOL result;
            float forceZ;
            float dot;
            
            
            playerP2 = &actmanWork.player[b->paramA];
            result = TRUE;
            VECSubtract(&b->point, &a->point, &dir);
            SafeNormalize(&dir, &dir);
            up.x = 0;
            up.y = 1;
            up.z = 0;
            dot = VECDotProduct(&up, &dir);
            if(dot > cos(M_PI/3)) {
                if(MgPlayerModeAttrCheck(playerP2, MGPLAYER_MODE_ATTR_AIR)) {
                    if(fabsf2(playerP1->actor->velY) <= 1000.0f) {
                        if(playerP2->actor->velY < 0 || MgPlayerVibAttrCheck(playerP2, MGPLAYER_VIBATTR_HEADJUMP)) {
                            if(MgPlayerStunSet(playerP1, MGPLAYER_STUN_SQUISH, 0, -1)) {
                                MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_SQUISH);
                            }
                        }
                    } else {
                        MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_SQUISH_FAST);
                    }
                }
            } else if(dot < -cos(M_PI/3)) {
                if(!MgPlayerAttrCheck(playerP1, MGPLAYER_ATTR_HEADJUMP_OFF)) {
                    HuVecF norm = a->normPos;
                    float velY;
                    Mtx rotMtx;
                    
                    if(playerP1->actor->velY > 0) {
                        break;
                    }
                    if(!MgPlayerModeAttrCheck(playerP1, MGPLAYER_MODE_ATTR_AIR)) {
                        break;
                    }
                    norm.y = 0;
                    playerP1->actor->velY = GET_ACTOR_VELY(playerP1->actor, 15000);
                    playerP1->actor->rotY += frandmod(90)-45;
                    if(!MgPlayerModeAttrCheck(playerP1, MGPLAYER_MODE_ATTR_HEADJUMP)) {
                        forceZ = VECMag(&norm)/2;
                    } else {
                        forceZ = VECMag(&playerP1->actor->forceB);
                    }
                    if(forceZ < 4) {
                        forceZ = 4;
                    }
                    playerP1->actor->forceB.x = playerP1->actor->forceB.y = 0;
                    playerP1->actor->forceB.z = forceZ;
                    MTXRotDeg(rotMtx, 'Y', playerP1->actor->rotY);
                    MTXMultVec(rotMtx, &playerP1->actor->forceB, &playerP1->actor->forceB);
                    MgPlayerAttrSet(playerP1, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    MgPlayerModeAttrSet(playerP1, MGPLAYER_MODE_ATTR_HEADJUMP);
                    MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_HEADJUMP);
                    
                } else {
                    HuVecF norm = dir;
                    norm.y = 0;
                    if(fabsf2(VECSquareMag(&norm)) < 0.001) {
                        norm.x = 1;
                    } else {
                        VECNormalize(&norm, &norm);
                    }
                    VECScale(&norm, &norm, -50);
                    a->normPos = norm;
                    result = FALSE;
                }
            }
            (void)playerP2;
            if(!result) {
                break;
            }
            if(MgPlayerVibAttrCheck(playerP1, 0x7)) {
                break;
            }
            {
                HuVecF temp;
                VECAdd(&a->normPos, &b->normPos, &temp);
                VECScale(&temp, &temp, 0.3f);
                temp.y = a->normPos.y;
                a->normPos = temp;
            }
        }
        break;

        case 2:
        {
            MGPLAYER *playerP2;
            HuVecF dir;
            float angle;

            playerP2 = &actmanWork.player[b->paramB & 0xFF];
            if(a->paramA == (b->paramB & 0xFF)) {
               return FALSE;
            }
            VECSubtract(&a->point, &b->point, &dir);
            SafeNormalize(&dir, &dir);
            if(dir.z > 1) {
                dir.z = 1;
            } else if(dir.z < -1) {
                dir.z = -1;
            }
            angle = acosf(dir.z);
            if(dir.x < 0) {
                angle = -angle;
            }
            angle = (angle*180)/M_PI;
            if(!MgPlayerStunSet(playerP1, MGPLAYER_STUN_HIT, angle, -1)) {
                return FALSE;
            }
            MgPlayerVibAttrSet(playerP2, MGPLAYER_VIBATTR_HIT_SRC);
            MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_HIT);
            (void)playerP2;
            return FALSE;
        }
        break;
        
        case 3:
        {
            MGPLAYER *playerP2;
            playerP2 = &actmanWork.player[b->paramB & 0xFF];
            if(a->paramA == (b->paramB & 0xFF)) {
               return FALSE;
            }
            if(!MgPlayerStunSet(playerP1, MGPLAYER_STUN_KNOCKBACK, actmanWork.actor[b->paramA].rotY, -1)) {
                return FALSE;
            }
            MgPlayerVibAttrSet(playerP2, MGPLAYER_VIBATTR_KNOCKBACK_SRC);
            MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_KNOCKBACK);
            (void)playerP2;
            return FALSE;
        }
        break;
        
        case 4:
        {
            MGPLAYER *playerP2;
            HuVecF dir;
            float angle;
            
            playerP2 = &actmanWork.player[b->paramB & 0xFF];
            if(a->paramA == (b->paramB & 0xFF)) {
               return FALSE;
            }
            VECSubtract(&a->point, &b->point, &dir);
            dir.y = 0;
            SafeNormalize(&dir, &dir);
            if(dir.z > 1) {
                dir.z = 1;
            } else if(dir.z < -1) {
                dir.z = -1;
            }
            angle = acosf(dir.z);
            if(dir.x < 0) {
                angle = -angle;
            }
            angle = (angle*180)/M_PI;
            if(!MgPlayerStunSet(playerP1, MGPLAYER_STUN_SQUISH_HARD, angle, -1)) {
                return FALSE;
            }
            MgPlayerVibAttrSet(playerP2, MGPLAYER_VIBATTR_SQUISH_HARD_SRC);
            MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_SQUISH_HARD);
            (void)playerP2;
            return FALSE;
        }
        break;
        
        default:
            return FALSE;
    }
    return TRUE;
}

static void ActorColCorrectHook(COLBODY *body, void *user)
{
    MGACTOR *actorP;
    COLBODY_POINT *point;
    
    point = &body->colPoint[body->colPointNum];
    actorP = user;
    
    SafeNormalize(&body->moveDir, &actorP->moveDir);
    actorP->colGroundAttr = body->groundAttr;
    actorP->colNorm = point->normal;
    actorP->colMesh = point->meshNo;
    actorP->colObj = point->obj;
    actorP->colFace = point->faceNo;
    if(actorP->correctHook) {
        actorP->correctHook(actorP, actorP->correctHookParam);
    }
}

void MgActorInit(void)
{
    int no;
    actmanWork.objman = NULL;
    actmanWork.player = NULL;
    actmanWork.actor = NULL;
    actmanWork.colCylF = FALSE;
    for(no=4; no--; ) {
        actmanWork.effPlayer[no] = NULL;
    }
    ColInit();
}

void MgActorClose(void)
{
    int no;
    if(actmanWork.player) {
        HuMemDirectFree(actmanWork.player);
    }
    if(actmanWork.actor) {
        HuMemDirectFree(actmanWork.actor);
    }
    actmanWork.player = NULL;
    actmanWork.actor = NULL;
    actmanWork.objman = NULL;
    actmanWork.colCylF = FALSE;
    for(no=4; no--; ) {
        actmanWork.effPlayer[no] = NULL;
    }
    ColKill();
}

OMOBJMAN *MgActorObjectSetup(void)
{
    actmanWork.objman = omInitObjMan(256, 1000);
    omGameSysInit(actmanWork.objman);
    actmanWork.player = HuMemDirectMallocNum(HEAP_MODEL, PLAYER_MAX*sizeof(MGPLAYER), HU_MEMNUM_OVL);
    actmanWork.actor = HuMemDirectMallocNum(HEAP_MODEL, ACTOR_MAX*sizeof(MGACTOR), HU_MEMNUM_OVL);
    memset(actmanWork.player, 0, PLAYER_MAX*sizeof(MGPLAYER));
    memset(actmanWork.actor, 0, ACTOR_MAX*sizeof(MGACTOR));
    actmanWork.colCylF = FALSE;
    return actmanWork.objman;
}

void MgActorColMapInit(HU3DMODELID *mdlId, s16 mdlNum, int bodyMax)
{
    ColMapInit(mdlId, mdlNum, bodyMax);
}

void MgPlayerPadSet(MGPLAYER *playerP, int stkX, int stkY, int btnDown, int btn)
{
    HuPadStkX[playerP->padNo] = stkX;
    HuPadStkY[playerP->padNo] = stkY;
    HuPadBtnDown[playerP->padNo] &= (PAD_BUTTON_START|PAD_TRIGGER_Z);
    HuPadBtn[playerP->padNo] &= (PAD_BUTTON_START|PAD_TRIGGER_Z);
    HuPadBtnDown[playerP->padNo] |= btnDown;
    HuPadBtn[playerP->padNo] |= btn;
}

void MgActorColAttrSet(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = ColBodyGetRaw(actorP->no);
    body->param.attr |= mask&(COLBODY_ATTR_RESET|0xFFFE);
}

void MgActorColAttrReset(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = ColBodyGetRaw(actorP->no);
    body->param.attr &= (~mask|0xFEFF0000|COLBODY_ATTR_ACTIVE);
}

u32 MgActorColAttrGet(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = ColBodyGetRaw(actorP->no);
    return body->param.attr & ~(mask|0xFEFF0000);
}

void MgPlayerModeFuncSet(MGPLAYER *playerP, MGPLAYER_MODE_FUNC *funcTbl)
{
    int no;
    for(no=MGPLAYER_MODE_MAX; no--;) {
        if(funcTbl[no]) {
            playerP->modeFunc[no] = funcTbl[no];
        }
    }
}

static unsigned int defMotDataNum[17] = {
    CHARMOT_HSF_c000m1_300,
    CHARMOT_HSF_c000m1_301,
    CHARMOT_HSF_c000m1_302,
    CHARMOT_HSF_c000m1_303,
    CHARMOT_HSF_c000m1_304,
    CHARMOT_HSF_c000m1_305,
    CHARMOT_HSF_c000m1_404,
    CHARMOT_HSF_c000m1_308,
    CHARMOT_HSF_c000m1_310,
    CHARMOT_HSF_c000m1_309,
    CHARMOT_HSF_c000m1_315,
    CHARMOT_HSF_c000m1_316,
    CHARMOT_HSF_c000m1_318,
    CHARMOT_HSF_c000m1_319,
    CHARMOT_HSF_c000m1_323,
    CHARMOT_HSF_c000m1_323,
    0
};

MGPLAYER *MgPlayerCreate(s16 playerNo, MGACTOR_PARAM *param, s16 model, s16 camBit, u32 actionFlag, unsigned int *motDataNum)
{
    MGPLAYER *playerP;
    COLBODY_PARAM colParam;
    if(motDataNum) {
        playerP = CreatePlayer(playerNo, model, camBit, actionFlag, motDataNum);
    } else {
        playerP = CreatePlayer(playerNo, model, camBit, actionFlag, defMotDataNum);
    }
    colParam.height = param->height;
    colParam.radius = param->radius;
    colParam.paramA = playerP->actor->no;
    colParam.paramB = param->param;
    colParam.type = param->type;
    colParam.mask = -1;
    colParam.narrowHook = PlayerColHook;
    colParam.narrowHook2 = param->narrowHook;
    colParam.colCorrectHook = ActorColCorrectHook;
    colParam.user = playerP->actor;
    colParam.attr = param->attr|COLBODY_ATTR_ACTIVE|COLBODY_ATTR_RESET;
    ColBodyParamSet(&colParam, playerP->actor->no);
    playerP->actor->param = param->param;
    playerP->actor->type = param->type;
    playerP->actor->correctHookParam = param->correctHookParam;
    playerP->actor->correctHook = param->correctHook;
    return playerP;
}

MGPLAYER *MgPlayerCreateJumpAlt(s16 playerNo, MGACTOR_PARAM *param, s16 model, s16 camBit, u32 actionFlag, unsigned int *motDataNum, unsigned int flag)
{
    MGPLAYER *playerP = MgPlayerCreate(playerNo, param, model, camBit, actionFlag, motDataNum);
    
    if(!playerP) {
        return NULL;
    }
    if(flag & 0x1) {
        MGPLAYER_MODE_FUNC modeFunc[MGPLAYER_MODE_MAX];
        int no;
        for(no=MGPLAYER_MODE_MAX; no--;) {
            modeFunc[no] = NULL;
            if(no == MGPLAYER_MODE_JUMP) {
                modeFunc[no] = PlayerModeJumpAlt;
            }
        }
        MgPlayerModeFuncSet(playerP, modeFunc);
    }
    return playerP;
}

MGACTOR *MgActorCreate(MGACTOR_PARAM *param, int mdlId)
{
    MGACTOR *actorP = CreateActor(mdlId);
    COLBODY_PARAM colParam;
    
    colParam.height = param->height;
    colParam.radius = param->radius;
    colParam.paramA = actorP->no;
    colParam.paramB = param->param;
    colParam.type = param->type;
    colParam.mask = -1;
    colParam.narrowHook = NULL;
    colParam.narrowHook2 = param->narrowHook;
    colParam.colCorrectHook = ActorColCorrectHook;
    colParam.user = actorP;
    colParam.attr = param->attr|COLBODY_ATTR_ACTIVE|COLBODY_ATTR_RESET;
    ColBodyParamSet(&colParam, actorP->no);
    actorP->param = param->param;
    actorP->type = param->type;
    actorP->correctHookParam = param->correctHookParam;
    actorP->correctHook = param->correctHook;
    return actorP;
}


void MgActorColMapMaskSet(int mdlNo, u32 mask)
{
    ColMapMaskSet(mdlNo, mask);
}

u32 MgActorColMapMaskGet(int mdlNo)
{
    return ColMapMaskGet(mdlNo);
}

void MgActorColMaskSet(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = ColBodyGet(actorP->no);
    if(!body) {
        return;
    }
    body->param.mask = mask;
    ColBodyParamSet(&body->param, actorP->no);
}

u32 MgActorColMaskGet(MGACTOR *actorP)
{
    COLBODY *body = ColBodyGet(actorP->no);
    if(!body) {
        return 0;
    }
    return body->param.mask;
}

void MgActorBodyColOff(MGACTOR *actorP)
{
    COLBODY *body = ColBodyGetRaw(actorP->no);
    body->param.attr |= COLBODY_ATTR_BODYCOL_OFF;
}

void MgActorBodyColOn(MGACTOR *actorP)
{
    COLBODY *body = ColBodyGetRaw(actorP->no);
    body->param.attr &= ~COLBODY_ATTR_BODYCOL_OFF;
}

void MgActorColCylReset(void)
{
    actmanWork.colCylF = FALSE;
    ColCylReset();
}

void MgActorColCylSet(void)
{
    actmanWork.colCylF = TRUE;
    ColCylSet();
}

void MgActorKill(MGACTOR *actorP)
{
    COLBODY *body = ColBodyGetRaw(actorP->no);
    body->param.attr &= ~COLBODY_ATTR_ACTIVE;
}

#define GET_STICK_SPEED(playerP, out) do { \
    HuVecF dir; \
    dir.x = HuPadStkX[playerP->padNo]/5.6f; \
    dir.y = 0; \
    dir.z = HuPadStkY[playerP->padNo]/5.6f; \
    out = HuMag2Point3D(dir.x, dir.y, dir.z); \
} while(0)

static void PlayerSetMotion(MGPLAYER *playerP, u16 motNo, float start, float end, u32 attr)
{
    if(playerP->motNo != motNo) {
        CharMotionShiftSet(playerP->charNo, playerP->omObj->mtnId[motNo], start, end, attr);
        playerP->motNo = motNo;
    }
}

static void PlayerModeDefault(MGPLAYER *playerP)
{
    MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
    PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
}

static void PlayerModeWalk(MGPLAYER *playerP)
{
    if(playerP->actor->colGroundAttr & 0x407F) {
        if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
            && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)) {
            if(playerP->actionFlag & MGPLAYER_ACTFLAG_JUMP) {
                MgPlayerModeSet(playerP, MGPLAYER_MODE_JUMP);
            }
        } else if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
            && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_B)) {
            if(playerP->actionFlag & MGPLAYER_ACTFLAG_PUNCH) {
                MgPlayerModeSet(playerP, MGPLAYER_MODE_PUNCH);
            }
        } else {
            if(playerP->actionFlag & MGPLAYER_ACTFLAG_WALK) {
                float speed2 = HuMag2Point2D(playerP->actor->vel.x, playerP->actor->vel.z);
                if(speed2 > 25) {
                    PlayerSetMotion(playerP, 2, 0, 5, HU3D_MOTATTR_LOOP);
                } else if(speed2 > 0.001) {
                    PlayerSetMotion(playerP, 1, 0, 5, HU3D_MOTATTR_LOOP);
                } else {
                    PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
                }
            }
        }
    } else {
        if(fabsf2(playerP->actor->velY) > 2000) {
            MgPlayerModeSet(playerP, MGPLAYER_MODE_FALL);
        }
    }
}

static void PlayerModeJump(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            PlayerSetMotion(playerP, 3, 0, 1, HU3D_MOTATTR_NONE);
            playerP->actor->velY = GET_ACTOR_VELY(playerP->actor, 25000);
            MgPlayerModeAttrSet(playerP, MGPLAYER_MODE_ATTR_AIR);
            playerP->timer = 1;
            playerP->subMode = 1;
            break;
           
        case 1:
            playerP->actor->velY = GET_ACTOR_VELY(playerP->actor, 25000)-(playerP->timer*playerP->actor->gravity);
            if(playerP->timer >= 10) {
                playerP->timer = 0;
                playerP->subMode = 2;
            } else {
                if(((playerP->actor->colGroundAttr & 0x7F)
                    && playerP->actor->colNorm.y > 0
                    || (playerP->actor->colGroundAttr & 0x4000))
                    && playerP->actor->velY < 0.001) {
                    float speed;
                    GET_STICK_SPEED(playerP, speed);
                    if(speed < 25.0f) {
                        PlayerSetMotion(playerP, 4, 0, 5, HU3D_MOTATTR_NONE);
                        MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                        playerP->timer = 10;
                        playerP->subMode = 3;
                    } else {
                        MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                        PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
                    }
                } else {
                    if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                        && !(HuPadBtn[playerP->padNo] & PAD_BUTTON_A)) {
                        playerP->actor->velY = GET_ACTOR_VELY(playerP->actor, 4000);
                        playerP->timer = 0;
                        playerP->subMode = 2;
                    }
                }
            }
            playerP->timer++;
            break;
       
        case 2:
            PlayerSetMotion(playerP, 3, 0, 1, HU3D_MOTATTR_NONE);
            if(((playerP->actor->colGroundAttr & 0x7F)
                && playerP->actor->colNorm.y > 0
                || (playerP->actor->colGroundAttr & 0x4000))
                && playerP->actor->velY < 0.001) {
                float speed;
                GET_STICK_SPEED(playerP, speed);
                if(speed < 25.0f) {
                    PlayerSetMotion(playerP, 4, 0, 5, HU3D_MOTATTR_NONE);
                    MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    playerP->timer = 10;
                    playerP->subMode = 3;
                } else {
                    MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                    PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
                    CharModelLandDustCreate(playerP->charNo, &playerP->actor->pos);
                }
            } else {
                if(playerP->motNo == 3) {
                    if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                        && !MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_HEADJUMP)
                        && ((HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)
                        || (HuPadBtnDown[playerP->padNo] & PAD_TRIGGER_L))) {
                        if(playerP->actionFlag & MGPLAYER_ACTFLAG_HIPDROP) {
                            MgPlayerModeSet(playerP, MGPLAYER_MODE_HIPDROP);
                        }
                    } else if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                        && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_B)) {
                        if(playerP->actionFlag & MGPLAYER_ACTFLAG_KICK) {
                            MgPlayerModeSet(playerP, MGPLAYER_MODE_KICK);
                        }
                    }
                }
            }
            break;
        
        case 3:
            if(playerP->timer <= 9 && !MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD)) {
                MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            }
            if(playerP->timer > 0) {
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                    && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)) {
                    playerP->subMode = 0;
                    if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD)) {
                        MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    }
                    if(playerP->timer >= 6.0f) {
                        CharModelLandDustCreate(playerP->charNo, &playerP->actor->pos);
                    }
                }
                playerP->timer--;
            } else {
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
            }
            break;
    }
}

static void PlayerModeFall(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            PlayerSetMotion(playerP, 3, 0, 1, HU3D_MOTATTR_NONE);
            MgPlayerModeAttrSet(playerP, MGPLAYER_MODE_ATTR_AIR);
            if(playerP->actor->colGroundAttr & 0x407F) {
                float speed;
                GET_STICK_SPEED(playerP, speed);
                if(speed < 25) {
                    PlayerSetMotion(playerP, 4, 0, 5, HU3D_MOTATTR_NONE);
                    MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    playerP->timer = 10;
                    playerP->subMode = 1;
                } else {
                    MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                    PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
                    CharModelLandDustCreate(playerP->charNo, &playerP->actor->pos);
                }
            } else {
                if(playerP->motNo == 3) {
                    if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                        && ((HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)
                        || (HuPadBtnDown[playerP->padNo] & PAD_TRIGGER_L))) {
                        if(playerP->actionFlag & MGPLAYER_ACTFLAG_HIPDROP) {
                            MgPlayerModeSet(playerP, MGPLAYER_MODE_HIPDROP);
                        }
                    } else if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                        && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_B)) {
                        if(playerP->actionFlag & MGPLAYER_ACTFLAG_KICK) {
                            MgPlayerModeSet(playerP, MGPLAYER_MODE_KICK);
                        }
                    }
                }
            }
            break;
      
        case 1:
            if(playerP->timer <= 9 && !MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD)) {
                MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            }
            if(playerP->timer > 0) {
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                    && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)) {
                    if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD)) {
                        MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    }
                    if(playerP->timer >= 6.0f) {
                        CharModelLandDustCreate(playerP->charNo, &playerP->actor->pos);
                    }
                    MgPlayerModeSet(playerP, MGPLAYER_MODE_JUMP);
                }
                playerP->timer--;
            } else {
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
            }
            break;
    }
}

static void PlayerModePunch(MGPLAYER *playerP)
{
    MGACTOR_PARAM param;
    switch(playerP->subMode) {
        case 0:
        {
            int no;
            COLBODY *body;
            PlayerSetMotion(playerP, 5, 0, 5, HU3D_MOTATTR_NONE);
            playerP->timer = 20;
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            no = playerP->actor->no;
            body = ColBodyGet(no);
            param.param = body->param.paramA+0x100;
            param.type = 2;
            param.height = 120;
            param.radius = 60;
            param.attr = COLBODY_ATTR_ACTIVE|COLBODY_ATTR_RESET|0xA;
            param.narrowHook = NULL;
            param.correctHook = NULL;
            actmanWork.effPlayer[no] = MgActorCreate(&param, HU3D_MODELID_NONE);
            MgActorColMaskSet(actmanWork.effPlayer[no], MgActorColMaskGet(playerP->actor));
            actmanWork.effPlayer[no]->rotY = playerP->actor->rotY;
            actmanWork.effPlayer[no]->pos = playerP->actor->pos;
            actmanWork.effPlayer[no]->vel.x = 0;
            actmanWork.effPlayer[no]->vel.y = 0;
            actmanWork.effPlayer[no]->vel.z = 0;
            actmanWork.effPlayer[no]->gravity = 0;
            playerP->subMode = 1;
        }
            
            break;

        case 1:
        {
            int no;
            no = playerP->actor->no;
            if(playerP->timer > 0) {
                playerP->timer--;
                if(playerP->timer >= 10) {
                    actmanWork.effPlayer[no]->vel.x = 0;
                    actmanWork.effPlayer[no]->vel.y = 0;
                    actmanWork.effPlayer[no]->vel.z = 10;
                } else {
                    actmanWork.effPlayer[no]->vel.x = 0;
                    actmanWork.effPlayer[no]->vel.y = 0;
                    actmanWork.effPlayer[no]->vel.z = 0;
                }
            } else {
                KillEffectPlayer(no);
                MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                playerP->subMode = 2;
            }
        }
            break;

        case 2:
            MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
            PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
            break;
    }
}

static void PlayerModeKick(MGPLAYER *playerP)
{
    
    switch(playerP->subMode) {
        case 0:
        {
            int no;
            COLBODY *body;
            MGACTOR_PARAM param;
            PlayerSetMotion(playerP, 6, 0, 5, HU3D_MOTATTR_NONE);
            playerP->timer = 20;
            no = playerP->actor->no;
            body = ColBodyGet(no);
            param.param = body->param.paramA+0x100;
            param.type = 3;
            param.height = 120;
            param.radius = 60;
            param.attr = COLBODY_ATTR_ACTIVE|COLBODY_ATTR_RESET|0xA;
            param.narrowHook = NULL;
            param.correctHook = NULL;
            actmanWork.effPlayer[no] = MgActorCreate(&param, HU3D_MODELID_NONE);
            MgActorColMaskSet(actmanWork.effPlayer[no], MgActorColMaskGet(playerP->actor));
            actmanWork.effPlayer[no]->rotY = playerP->actor->rotY;
            actmanWork.effPlayer[no]->pos = playerP->actor->pos;
            actmanWork.effPlayer[no]->vel.x = 0;
            actmanWork.effPlayer[no]->vel.y = -20;
            actmanWork.effPlayer[no]->vel.z = 60;
            actmanWork.effPlayer[no]->gravity = 0;
            playerP->timer = 20;
            playerP->subMode = 1;
        }
            
            break;

        case 1:
        {
            int no;
            no = playerP->actor->no;
            if(playerP->actor->colGroundAttr & 0x407F) {
                PlayerSetMotion(playerP, 4, 0, 5, HU3D_MOTATTR_NONE);
                KillEffectPlayer(no);
                playerP->timer = 10;
                playerP->subMode = 2;
            } else {
                actmanWork.effPlayer[no]->rotY = playerP->actor->rotY;
                actmanWork.effPlayer[no]->pos = playerP->actor->pos;
                actmanWork.effPlayer[no]->vel.x = 0;
                actmanWork.effPlayer[no]->vel.y = -20;
                actmanWork.effPlayer[no]->vel.z = 60;
            }
        }
            break;

        case 2:
            if(playerP->timer > 0) {
                playerP->timer = -1;
            } else {
                PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
            }
            
            break;
    }
}

static void PlayerModeHipDrop(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            PlayerSetMotion(playerP, 7, 0, 5, HU3D_MOTATTR_NONE);
            playerP->timer = 20;
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->subMode = 1;
            break;
        
        case 1:
            if(playerP->timer > 0) {
                playerP->timer--;
                playerP->actor->velY = playerP->actor->gravity;
            } else {
                int no;
                COLBODY *body;
                MGACTOR_PARAM param;
                no = playerP->actor->no;
                body = ColBodyGet(no);
                param.param = body->param.paramA+0x100;
                param.type = 4;
                param.height = 10;
                param.radius = 80;
                param.attr = COLBODY_ATTR_ACTIVE|COLBODY_ATTR_RESET|0xA;
                param.narrowHook = NULL;
                param.correctHook = NULL;
                actmanWork.effPlayer[no] = MgActorCreate(&param, HU3D_MODELID_NONE);
                MgActorColMaskSet(actmanWork.effPlayer[no], MgActorColMaskGet(playerP->actor));
                actmanWork.effPlayer[no]->rotY = playerP->actor->rotY;
                actmanWork.effPlayer[no]->pos = playerP->actor->pos;
                actmanWork.effPlayer[no]->pos.y -= 10;
                actmanWork.effPlayer[no]->vel.x = 0;
                actmanWork.effPlayer[no]->vel.y = 0;
                actmanWork.effPlayer[no]->vel.z = 0;
                actmanWork.effPlayer[no]->gravity = 0;
                playerP->actor->velY = -1000;
                playerP->subMode = 2;
            }
            break;
        
        case 2:
        {
            int no;
            no = playerP->actor->no;
            actmanWork.effPlayer[no]->pos = playerP->actor->pos;
            actmanWork.effPlayer[no]->pos.y -= 10;
            if(playerP->actor->colGroundAttr & 0x407F) {
                playerP->timer = 20;
                playerP->subMode = 3;
                MgPlayerVibAttrSet(playerP, 0x200);
                PlayerSetMotion(playerP, 8, 0, 5, HU3D_MOTATTR_NONE);
                KillEffectPlayer(playerP->actor->no);
            }
        }
            break;
        
        case 3:
            if(playerP->timer > 0) {
                playerP->timer--;
            } else {
                playerP->timer = 20;
                playerP->subMode = 4;
                PlayerSetMotion(playerP, 9, 0, 5, HU3D_MOTATTR_NONE);
            }
            break;
        
        case 4:
            if(playerP->timer > 0) {
                playerP->timer--;
            } else {
                if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD)) {
                    MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                }
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
            }
            break;
    }
}

static void PlayerModeHit(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
        {
            Mtx rotMtx;
            HuVecF up;
            HuVecF rotDir;
            HuVecF stunDir;
            playerP->stunAngle = WrapAngle(playerP->stunAngle);
            playerP->actor->rotY = WrapAngle(playerP->actor->rotY);
            up.x = 0;
            up.y = 0;
            up.z = 1;
            MTXRotDeg(rotMtx, 'Y', playerP->actor->rotY);
            MTXMultVec(rotMtx, &up, &rotDir);
            MTXRotDeg(rotMtx, 'Y', playerP->stunAngle);
            MTXMultVec(rotMtx, &up, &stunDir);
            if(VECDotProduct(&rotDir, &stunDir) < 0) {
                PlayerSetMotion(playerP, 10, 0, 5, HU3D_MOTATTR_NONE);
                playerP->actor->rotY = WrapAngle(playerP->stunAngle+180);
            } else {
                PlayerSetMotion(playerP, 11, 0, 5, HU3D_MOTATTR_NONE);
                playerP->actor->rotY = playerP->stunAngle;
            }
            playerP->timer = 45;
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->stunType = MGPLAYER_STUN_FREEZE;
            playerP->subMode = 1;
        }
            
            break;
        
        case 1:
            if(playerP->timer > 0) {
                playerP->actor->vel.x = 0;
                playerP->actor->vel.y = 0;
                if(playerP->motNo == 10) {
                    if(playerP->timer > 25) {
                        playerP->actor->vel.z = -(playerP->timer/1.7f)*((playerP->timer-25)/20);
                    }
                } else if(playerP->motNo == 11) {
                    if(playerP->timer > 0) {
                        playerP->actor->vel.z = (playerP->timer/3.0f)*(playerP->timer/45);
                    }
                }
                playerP->timer--;
            } else {
                playerP->subMode = 2;
            }
            break;
        
        case 2:
            playerP->stunType = MGPLAYER_STUN_BLINK;
            MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
            PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
            break;
            
    }
}

static void PlayerModeKnockback(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
        {
            Mtx rotMtx;
            HuVecF up;
            HuVecF rotDir;
            HuVecF stunDir;
            playerP->stunAngle = WrapAngle(playerP->stunAngle);
            playerP->actor->rotY = WrapAngle(playerP->actor->rotY);
            up.x = 0;
            up.y = 0;
            up.z = 1;
            MTXRotDeg(rotMtx, 'Y', playerP->actor->rotY);
            MTXMultVec(rotMtx, &up, &rotDir);
            MTXRotDeg(rotMtx, 'Y', playerP->stunAngle);
            MTXMultVec(rotMtx, &up, &stunDir);
            if(VECDotProduct(&rotDir, &stunDir) < 0) {
                PlayerSetMotion(playerP, 12, 0, 5, HU3D_MOTATTR_NONE);
                playerP->actor->rotY = WrapAngle(playerP->stunAngle+180);
            } else {
                PlayerSetMotion(playerP, 13, 0, 5, HU3D_MOTATTR_NONE);
                playerP->actor->rotY = playerP->stunAngle;
            }
            playerP->timer = 45;
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->stunType = MGPLAYER_STUN_FREEZE;
            playerP->subMode = 1;
        }
            break;
       
        case 1:
            if(playerP->timer > 0) {
                playerP->actor->vel.x = 0;
                playerP->actor->vel.y = 0;
                if(playerP->timer > 25) {
                    if(playerP->motNo == 12) {
                        playerP->actor->vel.z = -(playerP->timer/1.4f)*((playerP->timer-25)/20);
                    } else if(playerP->motNo == 13) {
                        playerP->actor->vel.z = (playerP->timer/1.4f)*((playerP->timer-25)/20);
                    }
                }
                playerP->timer--;
            } else {
                if(playerP->motNo == 12) {
                    PlayerSetMotion(playerP, 14, 0, 5, HU3D_MOTATTR_NONE);
                } else if(playerP->motNo == 13) {
                    PlayerSetMotion(playerP, 15, 0, 5, HU3D_MOTATTR_NONE);
                }
                playerP->timer = 30;
                playerP->subMode = 2;
            }
            break;
        
        case 2:
            if(playerP->timer > 0) {
                playerP->timer--;
            } else {
                playerP->stunType = MGPLAYER_STUN_BLINK;
                MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
            }
            
            break;
    }
}

static void PlayerModeSquish(MGPLAYER *playerP)
{
    COLBODY *body;
    float t, scaleXZ, scaleY;
    
    switch(playerP->subMode) {
        case 0:
            body = ColBodyGetRaw(playerP->actor->no);
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->timer = 8;
            if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH)) {
                MgPlayerModeAttrSet(playerP, MGPLAYER_MODE_ATTR_SQUISH);
                playerP->squishOldHeight = body->param.height;
                body->param.height = playerP->squishOldHeight*0.5f;
            }
            playerP->stunType = MGPLAYER_STUN_FREEZE;
            playerP->subMode = 1;
            break;
        
        case 1:
            t = playerP->timer/8;
            scaleXZ = 1.25f-(t*0.25f);
            scaleY = 0.5f+(t*0.5f);
            if(playerP->timer > 0) {
                playerP->timer--;
                Hu3DModelScaleSet(playerP->actor->mdlId, scaleXZ, scaleY, scaleXZ);
            } else {
                Hu3DModelScaleSet(playerP->actor->mdlId, 1.25f, 0.5f, 1.25f);
                playerP->subMode = 2;
            }
            break;
        
        case 2:
            MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->stunType = MGPLAYER_STUN_BLINK;
            playerP->squishTime = 90;
            MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
            PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
            break;
    }
}

static void PlayerModeSquishHard(MGPLAYER *playerP)
{
    COLBODY *body;
    
    switch(playerP->subMode) {
        case 0:
            MgActorBodyColOff(playerP->actor);
            MgPlayerModeAttrSet(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD);
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH)) {
                body = ColBodyGetRaw(playerP->actor->no);
                MgPlayerModeAttrReset(playerP, MGPLAYER_MODE_ATTR_SQUISH);
                body->param.height = playerP->squishOldHeight;
            }
            playerP->timer = 5;
            PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
            playerP->stunType = MGPLAYER_STUN_FREEZE;
            playerP->subMode = 1;
            break;
        
        case 1:
            if(playerP->timer > 0) {
                float scale;
                playerP->timer--;
                scale = 1+((10*(5-playerP->timer))/100);
                Hu3DModelScaleSet(playerP->actor->mdlId, scale, 0.05f+((20*playerP->timer)/100), scale);
            } else {
                playerP->timer = 120;
                playerP->subMode = 2;
            }
            break;
        
        case 2:
            if(playerP->timer > 0) {
                playerP->timer--;
            } else {
                MgActorBodyColOn(playerP->actor);
                playerP->timer = 100;
                playerP->subMode = 3;
            }
            break;
        
        case 3:
        {
            float t = playerP->timer/100;
            float angle = 270+((playerP->timer*2880)/100);
            float scale;
            t = ((0.5*(t*HuSin(angle)))+1)-(t*0.45f);
            scale = 1+(0.08f*(playerP->timer-93));
            if(Hu3DModelAttrGet(playerP->actor->mdlId) & HU3D_ATTR_DISPOFF) {
                Hu3DModelDispOn(playerP->actor->mdlId);
            } else {
                Hu3DModelDispOff(playerP->actor->mdlId);
            }
            if(playerP->timer > 0) {
                if(playerP->timer > 93) {
                    Hu3DModelScaleSet(playerP->actor->mdlId, scale, t, scale);
                } else {
                    Hu3DModelScaleSet(playerP->actor->mdlId, 1, t, 1);
                }
                playerP->timer--;
            } else {
                Hu3DModelScaleSet(playerP->actor->mdlId, 1, 1, 1);
                playerP->subMode = 4;
            }
        }
            break;
        
        case 4:
            if(Hu3DModelAttrGet(playerP->actor->mdlId) & HU3D_ATTR_DISPOFF) {
                Hu3DModelDispOn(playerP->actor->mdlId);
            } else {
                Hu3DModelDispOff(playerP->actor->mdlId);
            }
            MgPlayerModeAttrReset(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD);
            MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->stunType = MGPLAYER_STUN_BLINK;
            MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
            PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
            break;
    }
}

static void PlayerModeJumpAlt(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            MgPlayerModeAttrSet(playerP, MGPLAYER_MODE_ATTR_AIR);
            PlayerSetMotion(playerP, 3, 0, 1, HU3D_MOTATTR_NONE);
            playerP->actor->velY = GET_ACTOR_VELY(playerP->actor, 25000);
            playerP->subMode = 1;
            break;
        
        case 1:
        case 2:
            PlayerSetMotion(playerP, 3, 0, 1, HU3D_MOTATTR_NONE);
            if(((playerP->actor->colGroundAttr & 0x7F)
                    && playerP->actor->colNorm.y > 0
                    || (playerP->actor->colGroundAttr & 0x4000))
                    && playerP->actor->velY < 0.001) {
                        float speed = VECSquareMag(&playerP->actor->vel);
                        if(speed < 25) {
                            PlayerSetMotion(playerP, 4, 0, 5, HU3D_MOTATTR_NONE);
                            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                            playerP->timer = 10;
                            playerP->subMode = 3;
                        } else {
                            MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                            PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
                        }
                    } else {
                        playerP->motNo == 3;
                    }
            break;
        
        case 3:
            if(playerP->timer <= 9 && !MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD)) {
                MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            }
            if(playerP->timer > 0) {
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                    && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)) {
                        playerP->subMode = 0;
                        if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD)) {
                           MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                        }
                    }
                    playerP->timer--;
            } else {
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                PlayerSetMotion(playerP, 0, 0, 5, HU3D_MOTATTR_LOOP);
            }
            break;
    }
}

static void InitColPoint(MGACTOR *actorP, COLBODY *colBody)
{
    COLBODY_POINT *point; //r27
    if(colBody->colPointNum) {
        point = &colBody->colPoint[colBody->colPointNum-1];
        SafeNormalize(&colBody->moveDir, &actorP->moveDir);
        actorP->colNorm = point->normal;
        actorP->colMesh = point->meshNo;
        actorP->colObj = point->obj;
        actorP->colFace = point->faceNo;
        actorP->terminalVelY = (point->colOfs.y < 0) ? (100*point->colOfs.y) : 0;
    } else {
        actorP->colMesh = 0xFFFF;
        actorP->colObj = NULL;
        actorP->colFace = -1;
        actorP->terminalVelY = 0;
    }
}

void MgActorExec(void)
{
    MGPLAYER *playerP; //r31
    MGACTOR *actorP; //r30
    COLBODY *colBody; //r29
    int i; //r28
    
    
    float stickSpeed; //f30
    float t; //f29
    float scaleXZ; //f28
    float angle; //f27
    float scaleY; //f26
    float speedY; //f22
    
    Mtx stickMtx; //sp+0xB4
    Mtx rotMtx; //sp+0x84
    HuVecF stickVec; //sp+0x78
    HuVecF stickDir; //sp+0x6C
    HuVecF vel; //sp+0x60
    HuVecF gravityForce; //sp+0x54
    HuVecF normForce; //sp+0x48
    
    if(!ColMapInitCheck()) {
        return;
    }
    for(i=0; i<PLAYER_MAX; i++) {
        colBody = ColBodyGetRaw(i);
        if(colBody->param.attr & COLBODY_ATTR_ACTIVE) {
            playerP = &actmanWork.player[i];
            if(MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_PAUSE)) {
                continue;
            }
            if((playerP->actionFlag & MGPLAYER_ACTFLAG_WALK) && !MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)) {
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_MOVEOFF)) {
                    stickVec.x = HuPadStkX[playerP->padNo]/5.6f;
                    stickVec.y = 0;
                    stickVec.z = HuPadStkY[playerP->padNo]/5.6f;
                    GetStickMtx(stickMtx, playerP->camBit);
                    MTXMultVec(stickMtx, &stickVec, &stickVec);
                }
                stickSpeed = HuMag2Point3D(stickVec.x, stickVec.y, stickVec.z);
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_ANGLELOCK)) {
                    if(stickSpeed >= 0.001) {
                        VECNormalize(&stickVec, &stickDir);
                        if(stickDir.z > 1) {
                            stickDir.z = 1;
                        } else if(stickDir.z < -1) {
                            stickDir.z = -1;
                        }
                        angle = acosf(stickDir.z);
                        if(stickDir.x < 0) {
                            angle = -angle;
                        }
                        playerP->actor->rotY = (angle*180)/M_PI;
                    }
                }
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_MOVEOFF)) {
                    playerP->actor->vel.x = 0;
                    playerP->actor->vel.y = 0;
                    playerP->actor->vel.z = sqrtf(stickSpeed);
                }
            } else if(MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_MOVEOFF)) {
                playerP->actor->vel.x = playerP->actor->vel.y = playerP->actor->vel.z = 0;
            }
            playerP->vibAttr = 0;
            if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH)) {
                if(playerP->stunType == MGPLAYER_STUN_BLINK || playerP->stunType == MGPLAYER_STUN_NONE) {
                    playerP->squishTime--;
                    if(playerP->squishTime < 0) {
                        if(playerP->squishTime < -20) {
                            colBody = ColBodyGetRaw(playerP->actor->no);
                            MgPlayerModeAttrReset(playerP, MGPLAYER_MODE_ATTR_SQUISH);
                            Hu3DModelScaleSet(playerP->actor->mdlId, 1, 1, 1);
                            colBody->param.height = playerP->squishOldHeight;
                            playerP->squishTime = 0;
                        } else {
                           if(playerP->squishTime > (-20.0f/3.0f)) {
                                t = playerP->squishTime/(-20.0f/3.0f);
                                scaleY = 0.5f+(0.3f*t);
                                scaleXZ = 1+(0.25f-(t*0.25f));
                            } else if(playerP->squishTime > (-40.0f/3.0f)) {
                                t = (playerP->squishTime-(-20.0f/3.0f))/(-20.0f/3.0f);
                                scaleY = 0.8f-(0.1f*t);
                                scaleXZ = 1;
                            } else {
                                t = (playerP->squishTime-(-40.0f/3.0f))/(-20.0f/3.0f);
                                scaleXZ = 1;
                                scaleY = 0.7f+(t*0.3f);
                            }
                            Hu3DModelScaleSet(playerP->actor->mdlId, scaleXZ, scaleY, scaleXZ);
                        }
                    }
                }
            }
            if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH)) {
                VECScale(&playerP->actor->vel, &playerP->actor->vel, 0.5f);
            }
            if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_HEADJUMP)) {
                if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_STKLOCK)) {
                    if(playerP->actor->velY <= 0) {
                        if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD)
                            && playerP->stunType == MGPLAYER_STUN_NONE
                            && playerP->mode != MGPLAYER_MODE_HIPDROP) {
                            MgPlayerModeAttrReset(playerP, MGPLAYER_MODE_ATTR_STKLOCK);
                            MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                        }
                    } else {
                        MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    }
                }
                if(playerP->actor->colGroundAttr & 0x407F) {
                    MgPlayerModeAttrReset(playerP, MGPLAYER_MODE_ATTR_HEADJUMP);
                    if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODE_ATTR_SQUISH_HARD)
                        && playerP->mode != MGPLAYER_MODE_HIPDROP
                        && (playerP->stunType == MGPLAYER_STUN_NONE || playerP->stunType == MGPLAYER_STUN_BLINK)) {
                            MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                        }
                    memset(&playerP->actor->forceB, 0, sizeof(HuVecF));
                }
            }
            if(playerP->actor->colGroundAttr & 0x407F) {
                MgPlayerModeAttrReset(playerP, MGPLAYER_MODE_ATTR_AIR);
            } else if(playerP->actor->velY < -((playerP->actor->gravity*5)+(playerP->actor->gravity*5))) {
                MgPlayerModeAttrSet(playerP, MGPLAYER_MODE_ATTR_AIR);
            }
            if(playerP->actionFlag & MGPLAYER_ACTFLAG_STUN) {
                switch(playerP->stunType) {
                    case MGPLAYER_STUN_NONE:
                        break;

                    case MGPLAYER_STUN_HIT:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        MgPlayerModeSet(playerP, MGPLAYER_MODE_HIT);
                        break;
                    
                    case MGPLAYER_STUN_KNOCKBACK:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        MgPlayerModeSet(playerP, MGPLAYER_MODE_KNOCKBACK);
                        break;
                    
                    case MGPLAYER_STUN_SQUISH:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        MgPlayerModeSet(playerP, MGPLAYER_MODE_SQUISH);
                        break;
                    
                    case MGPLAYER_STUN_SQUISH_HARD:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        MgPlayerModeSet(playerP, MGPLAYER_MODE_SQUISH_HARD);
                        break;
                    
                    case MGPLAYER_STUN_BLINK:
                        if(Hu3DModelAttrGet(playerP->actor->mdlId) & HU3D_ATTR_DISPOFF) {
                            Hu3DModelDispOn(playerP->actor->mdlId);
                        } else {
                            Hu3DModelDispOff(playerP->actor->mdlId);
                        }
                        if(playerP->stunTime > 0) {
                            playerP->stunTime--;
                        } else {
                            Hu3DModelDispOn(playerP->actor->mdlId);
                            playerP->stunType = MGPLAYER_STUN_NONE;
                        }
                        break;
                }
            }
            playerP->modeFunc[playerP->mode](playerP);
        }
    }
    ColDirtyClear();
    for(i=0; i<ACTOR_MAX; i++) {
        colBody = ColBodyGetRaw(i);
        if(colBody->param.attr & COLBODY_ATTR_ACTIVE) {
            actorP = &actmanWork.actor[i];
            if(actorP->attr & 0x1) {
                continue;
            }
            if(actorP->mdlId >= 0) {
                Hu3DModelRotSet(actorP->mdlId, 0, actorP->rotY, 0);
            }
            if(actorP->colGroundAttr & 0x407F) {
                if(actorP->velY < 0.001 && actorP->terminalVelY > -(actorP->gravity*5)) {
                    actorP->velY = -(actorP->gravity*5);
                }
            }
            actorP->velY -= actorP->gravity;
            speedY = actorP->velY/100;
            actorP->vel.y += speedY;
            vel = actorP->vel;
            MTXRotDeg(rotMtx, 'Y', actorP->rotY);
            MTXMultVec(rotMtx, &vel, &vel);
            VECAdd(&vel, &actorP->forceB, &vel);
            VECAdd(&vel, &actorP->forceA, &vel);
            VECAdd(&actorP->pos, &vel, &actorP->pos);
            actorP->vel.x = actorP->vel.y =  actorP->vel.z = 0;
            ColBodyPosSet(&actorP->pos, i);
            actorP->oldPos = actorP->pos;
        }
    }
    ColBodyExec();
    for(i=0; i<ACTOR_MAX; i++) {
        colBody = ColBodyGetRaw(i);
        if(colBody->param.attr & COLBODY_ATTR_ACTIVE) {
            actorP = &actmanWork.actor[i];
            if(actorP->attr & 0x1) {
                continue;
            }
            if(actorP->oldPos.x != actorP->pos.x || actorP->oldPos.y != actorP->pos.y || actorP->oldPos.z != actorP->pos.z) {
                Hu3DModelPosSet(actorP->mdlId, actorP->pos.x, actorP->pos.y-(colBody->param.height/2), actorP->pos.z);
            } else {
                ColBodyPosGet(&actorP->pos, i);
                if(actorP->mdlId >= 0) {
                    if(actmanWork.colCylF) {
                        Hu3DModelPosSet(actorP->mdlId, actorP->pos.x, actorP->pos.y-(colBody->param.height/2), actorP->pos.z);
                    } else {
                        Hu3DModelPosSet(actorP->mdlId, actorP->pos.x, actorP->pos.y-colBody->param.radius, actorP->pos.z);
                    }
                }
                actorP->colGroundAttr = colBody->groundAttr;
                InitColPoint(actorP, colBody);
                if(actorP->colGroundAttr && (actorP->colGroundAttr & 0x407F)) {
                    if(colBody->paramAttr & 0x2) {
                        gravityForce.x = gravityForce.z = 0;
                        gravityForce.y = actorP->velY/100;
                        VECScale(&actorP->colNorm, &normForce, VECDotProduct(&actorP->colNorm, &gravityForce));
                        VECSubtract(&gravityForce, &normForce, &gravityForce);
                        VECScale(&gravityForce, &gravityForce, 0.04f);
                        if(VECSquareMag(&gravityForce) > 0.001) {
                            VECAdd(&actorP->forceA, &gravityForce, &actorP->forceA);
                        } else {
                            memset(&actorP->forceA, 0, sizeof(HuVecF));
                        }
                    } else {
                        memset(&actorP->forceA, 0, sizeof(HuVecF));
                    }
                    if(actorP->velY < actorP->terminalVelY) {
                        actorP->velY = actorP->terminalVelY;
                    } else {
                        actorP->velY = 0;
                    }
                }
            }
        }
    }
}

BOOL MgActorColMapPolyGet(HuVecF *pos1, HuVecF *pos2, u32 mask, MGACTOR_COLMAP_POLY *outPoly)
{
    if(!ColMapInitCheck()) {
        return FALSE;
    }
    return ColPolyGet(pos1, pos2, mask, &outPoly->pos, &outPoly->code, &outPoly->mdlNo, &outPoly->obj, &outPoly->triNo);
}

void MgActorPosUpdate(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody = ColBodyGetRaw(actorP->no);
    colBody->param.attr |= COLBODY_ATTR_RESET;
    MgActorPosSet(actorP, pos);
}

void MgActorPosSet(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody;
    if(actorP->no < 0) {
        return;
    }
    colBody = ColBodyGetRaw(actorP->no);
    if(colBody->param.attr & COLBODY_ATTR_ACTIVE) {
        actorP->pos = *pos;
        if(actmanWork.colCylF) {
            actorP->pos.y += colBody->param.height/2;
        } else {
            actorP->pos.y += colBody->param.radius;
        }
    }
}

void MgActorPosGet(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody;
    if(actorP->no < 0) {
        return;
    }
    colBody = ColBodyGetRaw(actorP->no);
    if(colBody->param.attr & COLBODY_ATTR_ACTIVE) {
        *pos = actorP->pos;
        if(actmanWork.colCylF) {
            pos->y -= colBody->param.height/2;
        } else {
            pos->y -= colBody->param.radius;
        }
    }
}

void MgPlayerPauseOn(MGPLAYER *playerP)
{
    KillEffectPlayer(playerP->actor->no);
    MgPlayerAttrSet(playerP, MGPLAYER_ATTR_PAUSE);
    MgActorPauseOn(playerP->actor);
}

void MgPlayerPauseOff(MGPLAYER *playerP, HuVecF *pos)
{
    MgPlayerAttrReset(playerP, MGPLAYER_ATTR_PAUSE);
    MgActorPauseOff(playerP->actor, pos);
}

void MgActorPauseOn(MGACTOR *actorP)
{
    COLBODY *colBody;
    actorP->attr |= 0x1;
    colBody = ColBodyGetRaw(actorP->no);
    colBody->param.attr |= COLBODY_ATTR_COL_OFF;
}

void MgActorPauseOff(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody;
    actorP->attr &= ~0x1;
    colBody = ColBodyGetRaw(actorP->no);
    colBody->param.attr &= ~COLBODY_ATTR_COL_OFF;
    MgActorPosUpdate(actorP, pos);
}

void MgActorColAttrParamSet(COL_ATTRPARAM *param, u32 polyAttr)
{
    ColAttrParamSet(param, polyAttr);
}

void MgActorColAttrParamGet(COL_ATTRPARAM *param, u32 polyAttr)
{
    ColAttrParamGet(param, polyAttr);
}

void MgPlayerVibAttrSet(MGPLAYER *playerP, u16 attr)
{
    playerP->vibAttr |= attr;
}

void MgPlayerVibAttrReset(MGPLAYER *playerP, u16 attr)
{
    playerP->vibAttr &= ~attr;
}

u16 MgPlayerVibAttrCheck(MGPLAYER *playerP, u16 attr)
{
    return playerP->vibAttr & attr;
}

void MgPlayerModeAttrSet(MGPLAYER *playerP, u16 attr)
{
    playerP->modeAttr |= attr;
}

void MgPlayerModeAttrReset(MGPLAYER *playerP, u16 attr)
{
    playerP->modeAttr &= ~attr;
}

u16 MgPlayerModeAttrCheck(MGPLAYER *playerP, u16 attr)
{
    return playerP->modeAttr & attr;
}

void MgPlayerAttrSet(MGPLAYER *playerP, u16 attr)
{
    playerP->attr |= attr;
}

void MgPlayerAttrReset(MGPLAYER *playerP, u16 attr)
{
    playerP->attr &= ~attr;
}

u16 MgPlayerAttrCheck(MGPLAYER *playerP, u16 attr)
{
    return playerP->attr & attr;
}

BOOL MgPlayerStunSet(MGPLAYER *playerP, int stunType, float angle, int maxTime)
{
    if(!(playerP->actionFlag & MGPLAYER_ACTFLAG_STUN) || MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_STUNOFF)) {
        return FALSE;
    }
    if(playerP->stunType != MGPLAYER_STUN_NONE) {
        return FALSE;
    }
    if(maxTime < 0) {
        maxTime = 90;
    }
    playerP->stunType = stunType;
    playerP->stunAngle = angle;
    playerP->stunTime = maxTime;
    return TRUE;
}