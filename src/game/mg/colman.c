#include "game/mg/colman.h"
#include "game/hu3d.h"

typedef struct ColBroad_s {
    float t;
    COLMAP_ACTOR *actor1;
    COLMAP_ACTOR *actor2;
    s16 actor1Idx;
    s16 actor2Idx;
    int type;
} COLBROAD;

typedef struct ColNarrow_s {
    float t;
    COLBROAD *broadP;
} COLNARROW;

typedef struct ColWork_s {
    HU3DMODELID *mdlId;
    s16 mdlNum;
    s16 attr;
    COLBROAD *broadCol;
    int broadColNum;
    COLMAP_ATTR_PARAM attrParam[7];
    COLMAP_ATTR_PARAM attrParamHi[7];
    COLMAP_ACTOR *actor;
    int actorNum;
    COLNARROW *narrowCol;
    s16 *colOrder1;
    s16 *colOrder2;
} COLWORK;

typedef struct ColTri_s {
    HuVecF norm;
    HuVecF edgeNorm1;
    HuVecF edgeNorm2;
    HuVecF edgeNorm3;
    float d;
    HuVecF center;
    float dist;
} COLTRI;

typedef struct ColMesh_s {
    HSFOBJECT *obj;
    COLTRI *tri;
    HuVecF *actorPos;
    HuVecF *actorMove;
    u8 mdlNo;
    u32 mask;
    Mtx mtx;
    Mtx mtxOld;
    Mtx mtxInv;
    Mtx mtxInvOld;
} COLMESH;

static COLWORK colWork;

static BOOL colMapInitF;
static BOOL CancelTRXF;
static int colMeshCount;
static COLMESH *colMesh;

#define SWAP(a, b, type) \
do { \
    type temp; \
    temp = a; \
    a = b; \
    b = temp; \
} while(0)

static void SortCollisions(COLNARROW *narrowP, int num)
{
    s16 idx2; //r31
    s16 *order1; //r30
    s16 idx1; //r29
    s16 *order2; //r28
    int orderNum; //r27
    int orderIdx; //r26
    
    
    float t;
    
    order1 = colWork.colOrder1;
    order2 = colWork.colOrder2;
    order1[0] = 0;
    order2[0] = num-1;
    for(orderIdx=0, orderNum=1; orderIdx<orderNum; orderIdx++) {
        t = narrowP[order1[orderIdx]].t;
        idx1 = order1[orderIdx]+1;
        idx2 = order2[orderIdx];
        if(order2[orderIdx] > order1[orderIdx]) {
            while(idx1 < idx2) {
                if(narrowP[idx1].t < t) {
                    if(t <= narrowP[idx2].t) {
                        idx1++;
                        idx2--;
                    } else {
                        SWAP(narrowP[idx1+1], narrowP[idx2], COLNARROW);
                        idx1 += 2;
                    }
                } else {
                    if(t <= narrowP[idx2].t) {
                        SWAP(narrowP[idx2-1], narrowP[idx1], COLNARROW);
                        idx2 -= 2;
                    } else {
                        SWAP(narrowP[idx2], narrowP[idx1], COLNARROW);
                        idx1++;
                        idx2--;
                    }
                }
            }
            if(idx1 == idx2) {
                if(t <= narrowP[idx1].t) {
                    idx2--;
                } else {
                    idx1++;
                }
            }
            if(idx2 == order1[orderIdx]) {
                order1[orderNum] = idx1;
                order2[orderNum] = order2[orderIdx];
                orderNum++;
            } else {
                COLNARROW *col = &narrowP[order1[orderIdx]];
                SWAP(narrowP[idx2], *col, COLNARROW);
                order1[orderNum] = order1[orderIdx];
                order2[orderNum] = idx2-1;
                orderNum++;
                order1[orderNum] = idx1;
                order2[orderNum] = order2[orderIdx];
                orderNum++;
            }
        }
    }
}

static COLMESH *SearchColMesh(HSFOBJECT *obj)
{
    int no;
    for(no=colMeshCount; no--;) {
        if(colMesh[no].obj == obj) {
            return &colMesh[no];
        }
    }
    return NULL;
}

static void ColMtxTransApply(Mtx mtx, float x, float y, float z)
{
    mtx[0][3] += x;
    mtx[1][3] += y;
    mtx[2][3] += z;
}

static void ColMtxRot(Mtx mtx, float x, float y, float z)
{
    if(x != 0) {
        MTXRotDeg(mtx, 'X', x);
    } else {
        MTXIdentity(mtx);
    }
    if(y != 0) {
        Mtx temp;
        MTXRotDeg(temp, 'Y', y);
        MTXConcat(temp, mtx, mtx);
    }
    if(z != 0) {
        Mtx temp;
        MTXRotDeg(temp, 'Z', z);
        MTXConcat(temp, mtx, mtx);
    }
}

static void MakeIndexBuf(int *out, HSFFACE *hsfFaceP, int idx)
{
    switch(hsfFaceP->type) {
        case HSF_FACE_TRI:
            out[0] = hsfFaceP->index[0].vertex;
            out[1] = hsfFaceP->index[1].vertex;
            out[2] = hsfFaceP->index[2].vertex;
            break;
           
        case HSF_FACE_QUAD:
            if(idx & 0x1) {
                out[0] = hsfFaceP->index[0].vertex;
                out[1] = hsfFaceP->index[1].vertex;
                out[2] = hsfFaceP->index[3].vertex;
            } else {
                out[0] = hsfFaceP->index[0].vertex;
                out[1] = hsfFaceP->index[3].vertex;
                out[2] = hsfFaceP->index[2].vertex;
            }
            
            break;
        
        case HSF_FACE_TRISTRIP:
            out[0] = hsfFaceP->strip.data[idx].vertex;
            out[1] = hsfFaceP->strip.data[idx+1].vertex;
            out[2] = hsfFaceP->strip.data[idx+2].vertex;
            break;
    }
}

static u32 ColCodeGet(u32 matAttr)
{
    switch(matAttr & 0x3F0000) {
        case 0x10000:
            return (1 << 0);
       
        case 0x20000:
            return (1 << 1);
       
        case 0x30000:
            return (1 << 2);
        
        case 0x40000:
            return (1 << 3);
       
        case 0x50000:
            return (1 << 4);
       
        case 0x60000:
            return (1 << 5);
        
        case 0x70000:
            return (1 << 6);
       
        case 0x80000:
            return (1 << 7);
        
        case 0x100000:
            return (1 << 8);
       
        case 0x180000:
            return (1 << 9);
        
        case 0x200000:
            return (1 << 10);
       
        case 0x280000:
            return (1 << 11);
        
        case 0x300000:
            return (1 << 12);
       
        case 0x380000:
            return (1 << 13);
       
        default:
            return 0;
    }
}

static int ColCodeNoGet(u32 code)
{
    if(code & 0x407F) {
        if(code & 0x1) {
            return 0;
        } else if(code & 0x2) {
            return 1;
        } else if(code & 0x4) {
            return 2;
        } else if(code & 0x8) {
            return 3;
        } else if(code & 0x10) {
            return 4;
        } else if(code & 0x20) {
            return 5;
        } else if(code & 0x40) {
            return 6;
        }
    } else if(code & 0xBF80) {
        if(code & 0x80) {
            return 7;
        } else if(code & 0x100) {
            return 8;
        } else if(code & 0x200) {
            return 9;
        } else if(code & 0x400) {
            return 10;
        } else if(code & 0x800) {
            return 11;
        } else if(code & 0x1000) {
            return 12;
        } else if(code & 0x2000) {
            return 13;
        }
    }
    return -1;
}

static void ColMtxCalcHsfObject(HSFOBJECT *obj, Mtx mtx)
{
    HSFTRANSFORM *trxP;
    int no;
    BOOL processF = FALSE;
    COLMESH *meshP;
    Mtx objMtx;
    Mtx rot;
    Mtx final;
    switch(obj->type) {
        case HSF_OBJ_NULL1:
        case HSF_OBJ_MESH:
        case HSF_OBJ_ROOT:
        case HSF_OBJ_JOINT:
        case HSF_OBJ_NULL2:
        case HSF_OBJ_NULL3:
        case HSF_OBJ_MAP:
            processF = TRUE;
            break;
       
        case HSF_OBJ_REPLICA:
        case HSF_OBJ_CAMERA:
        case HSF_OBJ_LIGHT:
            break;
    }
    if(!processF) {
        return;
    }
    if(!CancelTRXF) {
        trxP = &obj->mesh.base;
    } else {
        trxP = &obj->mesh.curr;
    }
    MTXScale(final, trxP->scale.x, trxP->scale.y, trxP->scale.z);
    ColMtxRot(rot, trxP->rot.x, trxP->rot.y, trxP->rot.z);
    MTXConcat(rot, final, final);
    ColMtxTransApply(final, trxP->pos.x, trxP->pos.y, trxP->pos.z);
    MTXConcat(mtx, final, objMtx);
    meshP = SearchColMesh(obj);
    if(meshP) {
        memcpy(meshP->mtx, objMtx, sizeof(Mtx));
    }
    for(no=obj->mesh.childNum; no--;) {
        ColMtxCalcHsfObject(obj->mesh.child[no], objMtx);
    }
}

static void ColMtxCalcModelAll(void)
{
    HU3DMODEL *modelP;
    Mtx final;
    Mtx rot;
    int no;
    for(no=colWork.mdlNum; no--;) {
        modelP = &Hu3DData[colWork.mdlId[no]];
        ColMtxRot(rot, modelP->rot.x, modelP->rot.y, modelP->rot.z);
        MTXScale(final, modelP->scale.x, modelP->scale.y, modelP->scale.z);
        MTXConcat(modelP->mtx, rot, rot);
        MTXConcat(rot, final, final);
        ColMtxTransApply(final, modelP->pos.x, modelP->pos.y, modelP->pos.z);
        CancelTRXF = modelP->motId != HU3D_MOTID_NONE;
        ColMtxCalcHsfObject(modelP->hsf->root, final);
    }
}

static BOOL ColRectEject(HuVecF *center, HuVecF *size, COLTRI *tri, HuVecF *eject, BOOL flatF)
{
    HuVecF vtx[4];
    HuVecF temp;
    int i;
    BOOL result;
    int num;
    
    float d;
    float mind;
    result = FALSE;
    if(flatF) {
        vtx[0].x = -size->x;
        vtx[0].z = -size->z;
        vtx[1].x = size->x;
        vtx[1].z = size->z;
        vtx[2].x = -size->x;
        vtx[2].z =  -size->z;
        vtx[3].x = size->x;
        vtx[3].z = size->z;
        vtx[0].y = -size->y;
        vtx[1].y = -size->y;
        vtx[2].y = size->y;
        vtx[3].y = size->y;
        num = 4;
    } else {
        vtx[0].x = -size->x;
        vtx[0].z = -size->z;
        vtx[1].x = size->x;
        vtx[1].z = size->z;
        if(tri->norm.y >= 0) {
            vtx[0].y = -size->y;
            vtx[1].y = -size->y;
        } else {
            vtx[0].y = size->y;
            vtx[1].y = size->y;
        }
        num = 2;
    }
    for(i=0; i<num; i++) {
        VECAdd(center, &vtx[i], &temp);
        d = tri->d+VECDotProduct(&tri->norm, &temp);
        if(d >= -0.001f) {
            if(result == FALSE || d < mind) {
                result = TRUE;
                mind = d;
                *eject = vtx[i];
            }
        }
    }
    return result;
}

#define CLAMP_EPSILON(x) ((fabs(x) < 0.001f) ? 0 : (x))

static inline float GetSign(float x)
{
    if(x < 0) {
        return -1;
    } else if(x > 0) {
        return 1;
    } else {
        return 0;
    }
}

static inline BOOL SameSign(float x, float y)
{
    int signA = GetSign(x);
    int signB = GetSign(y);
    BOOL result = signA == signB;
    (void)signA;
    (void)signA;
    (void)signB;
    (void)signB;
    (void)result;
    
    return result;
}

static BOOL CheckPlane(HuVecF a, HuVecF b, HuVecF c, COLTRI *tri, BOOL extFlag)
{
    HuVecF ab;
    HuVecF ac;
    VECSubtract(&a, &b, &ab);
    VECSubtract(&a, &c, &ac);

    if(VECDotProduct(&ab, &tri->edgeNorm1) >= 0 && VECDotProduct(&ac, &tri->edgeNorm2) >= 0 && VECDotProduct(&ab, &tri->edgeNorm3) >= 0) {
        return TRUE;
    } else {
        //TODO: Find less hacky matching code
        if(extFlag) {
           int a;
           (void)a;
           (void)a;
        }
        return FALSE;
    }
}

static int ColEjectDistGet(HuVecF *start, HuVecF *end, HuVecF *dir, HuVecF *size, float height, float maxDist, float minDist, HuVecF *vtx, int *vtxIdx, COLTRI *tri, float *outDist)
{
    float dist;
    
    float startD;
    float roundStartD;
    float roundEndD;
    
    float endD;
    
    HuVecF a;
    HuVecF delta;
    HuVecF eject;
    HuVecF perp;
    
    startD = tri->d+VECDotProduct(&tri->norm, start)-height;
    endD = (tri->d+VECDotProduct(&tri->norm, end))-height;
    roundStartD = CLAMP_EPSILON(startD);
    roundEndD = CLAMP_EPSILON(endD);
    if(roundStartD < 0 || SameSign(roundStartD, roundEndD)) {
        float dot = (tri->d+VECDotProduct(&tri->norm, start))+height;
        if(dot > 0 && roundEndD < 0) {
            VECScale(&tri->norm, &perp, startD);
            VECSubtract(start, &perp, &a);
            if(!ColRectEject(&a, size, tri, &eject, FALSE)) {
                *outDist = startD;
                return -5;
            }
            VECAdd(&a, &eject, &a);
            if(!CheckPlane(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
                return -6;
            } else {
                *outDist = startD;
                return -5;
            }
        } else {
            return -1;
        }
    } else {
        float dot = VECDotProduct(&tri->norm, dir);
        if(fabs(dot) < 0.001) {
            return -3;
        }
        dist = -startD/dot;
        if((dist < minDist && (minDist-dist) > 0.001f) || (maxDist < dist && (dist-maxDist) > 0.001f)) {
            return -2;
        }
        if(dist < minDist) {
            dist = minDist;
        }
        VECScale(dir, &delta, dist);
        VECAdd(start, &delta, &a);
        ColRectEject(start, size, tri, &eject, FALSE);
        VECAdd(&a, &eject, &a);
        if(!CheckPlane(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
            return -4;
        } else {
            *outDist = dist;
            return -0;
        }
    }
}

static int ColPlaneDistGet(HuVecF *start, HuVecF *end, HuVecF *dir, float height, float maxDist, float minDist, HuVecF *vtx, int *vtxIdx, COLTRI *tri, float *outDist)
{
    float dist;
    float startD;
    float roundStartD;
    float roundEndD;
    
    float endD;
    
    HuVecF a;
    HuVecF delta;
    HuVecF perp;

    startD = (tri->d+VECDotProduct(&tri->norm, start))-height;
    endD = (tri->d+VECDotProduct(&tri->norm, end))-height;
    roundStartD = CLAMP_EPSILON(startD);
    roundEndD = CLAMP_EPSILON(endD);
    if(roundStartD < 0 || SameSign(roundStartD, roundEndD)) {
        float dot = (tri->d+VECDotProduct(&tri->norm, start))+height;
        if(dot > 0 && roundEndD < 0) {
            *outDist = startD;
            VECScale(&tri->norm, &perp, startD);
            VECSubtract(start, &perp, &a);
            if(!CheckPlane(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
                return -6;
            } else {
                return -5;
            }
        } else {
            return -1;
        }
    } else {
        float dot = VECDotProduct(&tri->norm, dir);
        if(fabs(dot) < 0.001) {
            return -3;
        }
        dist = -startD/dot;
        if((dist < minDist && (minDist-dist) > 0.001f) || (maxDist < dist && (dist-maxDist) > 0.001f)) {
            return -2;
        }
        if(dist < minDist) {
            dist = minDist;
        }
        VECScale(dir, &delta, dist);
        VECAdd(start, &delta, &a);
        if(!CheckPlane(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
            return -4;
        } else {
            *outDist = dist;
            return -0;
        }
    }
}

static void UpdateMeshMtx(void)
{
    COLMESH *meshP;
    int no;
    if(colWork.attr & 0x10) {
        return;
    }
    colWork.attr |= 0x10;
    meshP = colMesh;
    for(no=colMeshCount; no--; meshP++) {
        memcpy(meshP->mtxOld, meshP->mtx, sizeof(Mtx));
        memcpy(meshP->mtxInvOld, meshP->mtxInv, sizeof(Mtx));
    }
    ColMtxCalcModelAll();
    meshP = colMesh;
    for(no=colMeshCount; no--; meshP++) {
        MTXInverse(meshP->mtx, meshP->mtxInv);
    }
}

static void ClearColPoint(COLMAP_ACTOR *actorP)
{
    COLMAP_COLPOINT *colPointP = &actorP->colPoint[actorP->colPointNum];
    
    memset(&colPointP->colOfs, 0, sizeof(colPointP->colOfs));
    memset(&colPointP->normal, 0, sizeof(colPointP->normal));
    colPointP->polyAttr = 0;
    colPointP->meshNo = -1;
    colPointP->obj = NULL;
    colPointP->faceNo = -1;
    actorP->param.attr &= ~0x04000000;
}

static BOOL _ActorApplyColAttr(COLMAP_ACTOR *actorP, COLMAP_ATTR_PARAM *attrParam, int code);

static BOOL _ColCorrection(COLMAP_ACTOR *actorP)
{
    COLMAP_COLPOINT *colPointP = &actorP->colPoint[actorP->colPointNum];
    COLMAP_ATTR_PARAM *attrParam;
    int codeNo;
    BOOL ret;
    
    codeNo = ColCodeNoGet(colPointP->polyAttr);
    if(codeNo < 0) {
        OSReport("( colman.c : _ColCorrection ) | マップとあたってるはずなのにアトリビュートがありません\n");
        return FALSE;
    }
    attrParam = (codeNo >= 7) ? (&colWork.attrParamHi[codeNo-7]) : (&colWork.attrParam[codeNo]);
    actorP->groundAttr |= colPointP->polyAttr;
    colPointP->polyAttr = 0;
    ret = _ActorApplyColAttr(actorP, attrParam, codeNo);
    if(ret && actorP->param.actorColHook) {
        actorP->param.actorColHook(actorP, actorP->param.user);
    }
    return ret;
}

static void _ActorMeshCol(void);
static void _ActorMeshColAlt(void);
static int _ActorColBroadAlt(void);
static int _ActorColBroad(void);
static int _ActorColNarrow(void);


//static BOOL _ActorApplyColAttr(COLMAP_ACTOR *actorP, COLMAP_ATTR_PARAM *attrParam, int code);

static int _ActorColResponse(void)
{
    COLMAP_ACTOR *actorP;
    int posNo;
    int result;
    int i;
    actorP = colWork.actor;
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    result = 0;
    for(i=0; i<colWork.actorNum; i++, actorP++) {
        if(!(actorP->param.attr & 0x1)) {
            continue;
        }
        if(!(actorP->param.attr & 0x40000000)) {
            if(actorP->param.attr & 0x04000000) {
                actorP->unk130 = actorP->unk134;
                if((actorP->param.attr & 0x20000000) || _ColCorrection(actorP)) {
                    result = 1;
                }
                if(actorP->colPointNum >= 4) {
                    actorP->param.attr |= 0x02000000;
                } else {
                    actorP->colPointNum++;
                }
            }
            
            actorP->unk134 = 0;
        }
        
    }
    return result;
}

//Must be macro for stack ordering concerns
#define _ColPointUpdate() \
do { \
    COLMAP_ACTOR *actorP; \
    int no; \
    actorP = colWork.actor; \
    for(no=colWork.actorNum; no--; actorP++) { \
        if(actorP->param.attr & 0x1) { \
            if((actorP->param.attr & 0x0C000000) == 0) { \
                actorP->param.attr |= 0x80000000; \
            } else { \
                actorP->param.attr &= ~0x80000000; \
            } \
            if(actorP->param.attr & 0x20000000) { \
                if(actorP->colPointNum >= 4) { \
                    actorP->param.attr |= 0x02000000; \
                } else { \
                    actorP->colPointNum++; \
                } \
            } \
            actorP->param.attr &= 0x83FFFFFF; \
        } \
    } \
} while(0)

static void _ActorMeshUpdate(int posNo)
{
    COLMAP_ACTOR *actorP;
    COLMESH *meshP;
    int i;
    int no;
    meshP = colMesh;
    actorP = colWork.actor;
    for(no=colWork.actorNum; no--; actorP++) {
        VECAdd(&actorP->oldPos, &actorP->moveDir, &actorP->pos[posNo]);
    }
    for(no=colMeshCount; no--; meshP++) {
        for(actorP=colWork.actor, i=0; i<colWork.actorNum; i++, actorP++) {
            meshP->actorPos[i] = actorP->pos[posNo];
            MTXMultVec(meshP->mtxInv, &meshP->actorPos[i], &meshP->actorPos[i]);
            
        }
    }
    (void)actorP;
}


static void MakeNormal(HuVecF *a, HuVecF *b, HuVecF *c, COLTRI *out)
{
    HuVecF ba;
    HuVecF cb;
    HuVecF ac;
    
    
    VECSubtract(b, a, &ba);
    VECSubtract(c, b, &cb);
    VECSubtract(a, c, &ac);
    VECCrossProduct(&out->norm, &ba, &out->edgeNorm1);
    VECCrossProduct(&out->norm, &cb, &out->edgeNorm2);
    VECCrossProduct(&out->norm, &ac, &out->edgeNorm3);
    VECNormalize(&out->edgeNorm1, &out->edgeNorm1);
    VECNormalize(&out->edgeNorm2, &out->edgeNorm2);
    VECNormalize(&out->edgeNorm3, &out->edgeNorm3);
}

static void ColMakeTri(COLTRI *tri, Vec *vtxBuf, int *idx)
{
    HuVecF ba;
    HuVecF cb;
    HuVecF aCenter;
    float dist;
    VECSubtract(&vtxBuf[idx[1]], &vtxBuf[idx[0]], &ba);
    VECSubtract(&vtxBuf[idx[2]], &vtxBuf[idx[1]], &cb);
    VECCrossProduct(&ba, &cb, &tri->norm);
    VECNormalize(&tri->norm, &tri->norm);
    tri->d = -VECDotProduct(&tri->norm, &vtxBuf[idx[0]]);
    MakeNormal(&vtxBuf[idx[0]], &vtxBuf[idx[1]], &vtxBuf[idx[2]], tri);
    tri->center.x = (vtxBuf[idx[0]].x+vtxBuf[idx[1]].x+vtxBuf[idx[2]].x)/3;
    tri->center.y = (vtxBuf[idx[0]].y+vtxBuf[idx[1]].y+vtxBuf[idx[2]].y)/3;
    tri->center.z = (vtxBuf[idx[0]].z+vtxBuf[idx[1]].z+vtxBuf[idx[2]].z)/3;
    VECSubtract(&vtxBuf[idx[0]], &tri->center, &aCenter);
    tri->dist = VECMag(&aCenter);
    VECSubtract(&vtxBuf[idx[1]], &tri->center, &aCenter);
    dist = VECMag(&aCenter);
    if(tri->dist < dist) {
        tri->dist = dist;
    }
    VECSubtract(&vtxBuf[idx[2]], &tri->center, &aCenter);
    dist = VECMag(&aCenter);
    if(tri->dist < dist) {
        tri->dist = dist;
    }
    tri->dist = (tri->dist*tri->dist);

}

COLMAP_ACTOR *ColMapActorGet(int no)
{
    return &colWork.actor[no];
}

void ColMapClear(void)
{
    CancelTRXF = FALSE;
    colMapInitF = FALSE;
    colWork.mdlId = NULL;
    colWork.mdlNum = 0;
    colWork.attr = 0;
    colWork.broadCol = NULL;
    colWork.broadColNum = 0;
    colWork.actorNum = 0;
    colWork.actor = NULL;
    colWork.narrowCol = NULL;
    colWork.colOrder1 = NULL;
    colWork.colOrder2 = NULL;
    memset(colWork.attrParam, 0, sizeof(colWork.attrParam));
    memset(colWork.attrParamHi, 0, sizeof(colWork.attrParamHi));
    colMesh = NULL;
    colMeshCount = 0;
}


void ColMapInit(HU3DMODELID *mdlId, s16 mdlNum, int actorNum)
{
    COLTRI *triP; //r31
    HuVecF *vtxBuf; //r30
    HSFFACE *hsfFaceP; //r29
    int triNum; //r28
    COLTRI *triOther; //r27
    int no; //r26
    COLMAP_ACTOR *actorP; //r25
    int meshNum; //r24
    HSFOBJECT *objP; //r23
    int i; //r22
    COLMESH *meshP; //r21
    int objNum; //r20
    int faceNo; //r19
    
    int index[3]; //sp+0x154
    
    if(ColMapInitCheck()) {
        return;
    }
    colWork.mdlNum = mdlNum;
    colWork.mdlId = HuMemDirectMallocNum(HEAP_MODEL, mdlNum*sizeof(HU3DMODELID), HU_MEMNUM_OVL);
    colWork.attr = 1;
    colWork.actorNum = actorNum;
    if(!colWork.actor) {
        colWork.actor = HuMemDirectMallocNum(HEAP_MODEL, COLMAP_ACTOR_MAX*sizeof(COLMAP_ACTOR), HU_MEMNUM_OVL);
        memset(colWork.actor, 0, COLMAP_ACTOR_MAX*sizeof(COLMAP_ACTOR));
    }
    if(!colWork.narrowCol) {
        colWork.narrowCol = HuMemDirectMallocNum(HEAP_MODEL, COLMAP_ACTOR_MAX*4*sizeof(COLNARROW), HU_MEMNUM_OVL);
        memset(colWork.narrowCol, 0, COLMAP_ACTOR_MAX*4*sizeof(COLNARROW));
    }
    memcpy(colWork.mdlId, mdlId, mdlNum*sizeof(HU3DMODELID));
    for(no=7; no--;) {
        colWork.attrParam[no].type = 1;
        colWork.attrParam[no].speed = 1;
        colWork.attrParam[no].yDeviate = 1;
        colWork.attrParam[no].maxDot = 1;
        colWork.attrParam[no].attr = 0;
        colWork.attrParamHi[no].type = 1;
        colWork.attrParamHi[no].speed = 1;
        colWork.attrParamHi[no].yDeviate = 1;
        colWork.attrParamHi[no].maxDot = 1;
        colWork.attrParamHi[no].attr = 0;
    }
    colWork.broadCol = HuMemDirectMallocNum(HEAP_MODEL, ((colWork.actorNum*(colWork.actorNum+1))/2.0f)*sizeof(COLBROAD), HU_MEMNUM_OVL);
    colWork.colOrder1 = HuMemDirectMallocNum(HEAP_MODEL, COLMAP_ACTOR_MAX*4*sizeof(s16), HU_MEMNUM_OVL);
    colWork.colOrder2 = HuMemDirectMallocNum(HEAP_MODEL, COLMAP_ACTOR_MAX*4*sizeof(s16), HU_MEMNUM_OVL);
    for(no=actorNum; no--;) {
        actorP = &colWork.actor[no];
        actorP->param.paramB = 0;
        actorP->param.paramA = 0;
        actorP->param.type = 0;
        actorP->param.narrowHook = NULL;
        actorP->param.narrowHook2 = NULL;
        actorP->param.mask = -1;
        actorP->param.attr = 0;
        actorP->param.height = 0;
        actorP->param.radius = 0;
        actorP->unk130 = 0;
        actorP->unk134 = 1;
        actorP->colPointNum = 0;
    }
    for(meshNum=0, no=colWork.mdlNum; no--;) {
        HSFDATA *hsfP = Hu3DData[colWork.mdlId[no]].hsf;
        objP = hsfP->object;
        for(objNum=hsfP->objectNum; objNum--; objP++) {
            if(objP->type == HSF_OBJ_MESH) {
                meshNum++;
            }
        }
    }
    colMesh = HuMemDirectMallocNum(HEAP_MODEL, sizeof(COLMESH)*meshNum, HU_MEMNUM_OVL);
    colMeshCount = meshNum;
    for(meshNum=0, no=colWork.mdlNum; no--;) {
        HSFDATA *hsfP = Hu3DData[colWork.mdlId[no]].hsf;
        objP = hsfP->object;
        for(objNum=hsfP->objectNum; objNum--; objP++) {
            if(objP->type == HSF_OBJ_MESH) {
                for(triNum=0, hsfFaceP=objP->mesh.face->data, faceNo=objP->mesh.face->count; faceNo--; hsfFaceP++) {
                    switch(hsfFaceP->type) {
                        case HSF_FACE_QUAD:
                            triNum += 2;
                            break;
                           
                        case HSF_FACE_TRI:
                            triNum += 1;
                            break;
                        
                        case HSF_FACE_TRISTRIP:
                            triNum += hsfFaceP->strip.count;
                            break;
                    }
                }
                vtxBuf = objP->mesh.vertex->data;
                triP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(COLTRI)*triNum, HU_MEMNUM_OVL);
                memset(triP, 0, sizeof(COLTRI)*triNum);
                for(triNum=0, hsfFaceP=objP->mesh.face->data, faceNo=objP->mesh.face->count; faceNo--; hsfFaceP++) {
                    switch(hsfFaceP->type) {
                        case HSF_FACE_QUAD:
                            MakeIndexBuf(index, hsfFaceP, 0);
                            ColMakeTri(&triP[triNum], vtxBuf, index);
                            triNum++;
                            MakeIndexBuf(index, hsfFaceP, 1);
                            ColMakeTri(&triP[triNum], vtxBuf, index);
                            triNum++;
                            break;
                      
                        case HSF_FACE_TRI:
                            MakeIndexBuf(index, hsfFaceP, 0);
                            ColMakeTri(&triP[triNum], vtxBuf, index);
                            triNum++;
                            break;
                       
                        case HSF_FACE_TRISTRIP:
                            for(i=0; i<hsfFaceP->strip.count; i++) {
                                MakeIndexBuf(index, hsfFaceP, i);
                                triOther = &triP[triNum];
                                ColMakeTri(triOther, vtxBuf, index);
                                if(i & 0x1) {
                                    VECScale(&triOther->norm, &triOther->norm, -1);
                                }
                                triNum++;
                            }
                            break;
                    }
                }
                if(meshNum == 256) {
                    OSReport("( colman.c : ColMapInit ) | テンポラリのバッファをオーバーライトしています\n");
                }
                memset(&colMesh[meshNum], 0, sizeof(COLMESH));
                colMesh[meshNum].tri = triP;
                colMesh[meshNum].obj = objP;
                colMesh[meshNum].actorPos = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HuVecF)*colWork.actorNum*2, HU_MEMNUM_OVL);
                colMesh[meshNum].actorMove = colMesh[meshNum].actorPos+colWork.actorNum;
                colMesh[meshNum].mask = -1;
                colMesh[meshNum].mdlNo = no;
                memset(colMesh[meshNum].actorPos, 0, sizeof(HuVecF)*colWork.actorNum);
                memset(colMesh[meshNum].actorMove, 0, sizeof(HuVecF)*colWork.actorNum);
                memset(colMesh[meshNum].mtx, 0, sizeof(Mtx));
                memset(colMesh[meshNum].mtxOld, 0, sizeof(Mtx));
                memset(colMesh[meshNum].mtxInv, 0, sizeof(Mtx));
                memset(colMesh[meshNum].mtxInvOld, 0, sizeof(Mtx));
                meshNum++;
            }
        }
    }
    colWork.attr |= 0x10;
    ColMtxCalcModelAll();
    for(meshP=colMesh, no=colMeshCount; no--; meshP++) {
        MTXInverse(meshP->mtx, meshP->mtxInv);
        memcpy(meshP->mtxOld, meshP->mtx, sizeof(Mtx));
        memcpy(meshP->mtxInvOld, meshP->mtxInv, sizeof(Mtx));
    }
    colMapInitF = TRUE;
}

void ColMapMaskSet(int mdlNo, u32 mask)
{
    COLMESH *meshP;
    int no;
    if(!ColMapInitCheck()) {
        return;
    }
    for(meshP=colMesh, no=colMeshCount; no--; meshP++) {
        if(meshP->mdlNo == mdlNo) {
            meshP->mask = mask;
        }
    }
}

u32 ColMapMaskGet(int mdlNo)
{
    COLMESH *meshP;
    int no;
    if(!ColMapInitCheck()) {
        return;
    }
    for(meshP=colMesh, no=colMeshCount; no--; meshP++) {
        if(meshP->mdlNo == mdlNo) {
            return meshP->mask;
        }
    }
    return 0;
}

BOOL ColMapAltColReset(void)
{
    BOOL ret = (colWork.attr & 0x20) ? TRUE : FALSE;
    colWork.attr &= ~0x20;
    return ret;
}

BOOL ColMapAltColSet(void)
{
    BOOL ret = (colWork.attr & 0x20) ? FALSE : TRUE;
    colWork.attr |= 0x20;
    return ret;
}

void ColMapKill(void)
{
    int no;
    CancelTRXF = FALSE;
    colMapInitF = FALSE;
    if(colWork.broadCol) {
        HuMemDirectFree(colWork.broadCol);
    }
    if(colWork.mdlId) {
        HuMemDirectFree(colWork.mdlId);
    }
    if(colWork.actor) {
        HuMemDirectFree(colWork.actor);
    }
    if(colWork.narrowCol) {
        HuMemDirectFree(colWork.narrowCol);
    }
    if(colWork.colOrder1) {
        HuMemDirectFree(colWork.colOrder1);
    }
    if(colWork.colOrder2) {
        HuMemDirectFree(colWork.colOrder2);
    }
    colWork.mdlNum = 0;
    colWork.broadCol = NULL;
    colWork.mdlId = NULL;
    colWork.attr = 0;
    colWork.broadColNum = 0;
    colWork.actorNum = 0;
    colWork.actor = NULL;
    colWork.narrowCol = NULL;
    colWork.colOrder1 = NULL;
    colWork.colOrder2 = NULL;
    memset(colWork.attrParam, 0, sizeof(colWork.attrParam));
    memset(colWork.attrParamHi, 0, sizeof(colWork.attrParamHi));
    if(colMesh) {
        for(no=colMeshCount; no--;) {
            if(colMesh[no].tri) {
                HuMemDirectFree(colMesh[no].tri);
            }
            if(colMesh[no].actorPos) {
                HuMemDirectFree(colMesh[no].actorPos);
            }
        }
        HuMemDirectFree(colMesh);
    }
    colMesh = NULL;
    colMeshCount = 0;
}

BOOL ColMapInitCheck(void)
{
    return colMapInitF;
}

void ColMapDirtyClear(void)
{
    colWork.attr &= ~0x18;
}


void ColMapAttrParamSet(COLMAP_ATTR_PARAM *param, u32 polyAttr)
{
    int code = ColCodeNoGet(polyAttr);
    COLMAP_ATTR_PARAM *dst;
    if(code < 0 || !param) {
        return;
    }
    dst = (code >= 7) ? (&colWork.attrParamHi[code-7]) : (&colWork.attrParam[code]);
    *dst = *param;
}

void ColMapAttrParamGet(COLMAP_ATTR_PARAM *param, u32 polyAttr)
{
    int code = ColCodeNoGet(polyAttr);
    COLMAP_ATTR_PARAM *dst;
    if(code < 0 || !param) {
        return;
    }
    dst = (code >= 7) ? (&colWork.attrParamHi[code-7]) : (&colWork.attrParam[code]);
    *param = *dst;
}

void ColMapActorPosSet(HuVecF *pos, int no)
{
    COLMAP_ACTOR *actorP = &colWork.actor[no];
    int idx = (colWork.attr & 0x4) ? 1 : 0;
    HuVecF temp = *pos;
    actorP->pos[idx] = temp;
    if(actorP->param.attr & 0x1000000) {
        actorP->pos[idx^1] = temp;
    }
}

void ColMapActorPosGet(HuVecF *pos, int no)
{
    int idx = (colWork.attr & 0x4) ? 1 : 0;
    if(colWork.attr & 0x8) {
        idx ^= 1;
    }
    *pos = colWork.actor[no].pos[idx];
}

void ColMapActorParamSet(COLMAP_ACTOR_PARAM *param, int no)
{
    if(no >= 0 && no < colWork.actorNum) {
        colWork.actor[no].param = *param;
    }
}

COLMAP_ACTOR *ColMapActorGetSafe(int no)
{
    if(no >= 0 && no < colWork.actorNum) {
        return &colWork.actor[no];
    } else {
        return NULL;
    }
}

BOOL ColMapPolyGet(HuVecF *pos1, HuVecF *pos2, u32 mask, HuVecF *outPos, u32 *outCode, int *outMdlNo, HSFOBJECT **outObj, int *outTriNo)
{
    HSFFACE *hsfFaceP; //r31
    COLMESH *meshP; //r30
    COLTRI *triP; //r28
    int j; //r27
    int i; //r26
    int triNo; //r25
    int triNum; //r24
    u32 polyAttr; //r22
    int mdlNo; //r21
    HSFOBJECT *obj; //r20
    int no; //r19
    BOOL temp; //r18
    
    float dist; //f31
    float mag2; //f30
    
    HuVecF start; //sp+0x6C
    HuVecF end; //sp+0x60
    HuVecF delta; //sp+0x54
    
    int vtxIdx[3]; //sp+0x48
    HuVecF outDelta; //sp+0x3C
    HuVecF centerDist; //sp+0x30
    HuVecF *vtx; //sp+0x2C
    int colPlaneResult; //sp+0x28
    float outDist; //sp+0x24
    
    meshP = colMesh;
    dist = 1;
    polyAttr = 0;
    mdlNo = 0;
    obj = NULL;
    triNo = -1;
    for(no=colMeshCount; no--;  meshP++) {
        if(mask & meshP->mask) {
            start = *pos1;
            end = *pos2;
            MTXMultVec(meshP->mtxInv, &start, &start);
            MTXMultVec(meshP->mtxInv, &end, &end);
            VECSubtract(&end, &start, &delta);
            mag2 = VECSquareMag(&delta);
            vtx = meshP->obj->mesh.vertex->data;
            hsfFaceP = meshP->obj->mesh.face->data;
            triP = meshP->tri;
            for(i=0; i<meshP->obj->mesh.face->count; i++, hsfFaceP++) {
                switch(hsfFaceP->type) {
                    case HSF_FACE_QUAD:
                        triNum = 2;
                        break;
                   
                    case HSF_FACE_TRI:
                        triNum = 1;
                        break;
                   
                    case HSF_FACE_TRISTRIP:
                        triNum = hsfFaceP->strip.count;
                        break;
                }
                for(j=0; j<triNum; j++, triP++) {
                    if(triP->norm.x || triP->norm.y || triP->norm.z) {
                        switch(hsfFaceP->type) {
                            case HSF_FACE_TRI:
                                vtxIdx[0] = hsfFaceP->index[0].vertex;
                                vtxIdx[1] = hsfFaceP->index[1].vertex;
                                vtxIdx[2] = hsfFaceP->index[2].vertex;
                                break;
                               
                            case HSF_FACE_QUAD:
                                if(j & 0x1) {
                                    vtxIdx[0] = hsfFaceP->index[0].vertex;
                                    vtxIdx[1] = hsfFaceP->index[1].vertex;
                                    vtxIdx[2] = hsfFaceP->index[3].vertex;
                                } else {
                                    vtxIdx[0] = hsfFaceP->index[0].vertex;
                                    vtxIdx[1] = hsfFaceP->index[3].vertex;
                                    vtxIdx[2] = hsfFaceP->index[2].vertex;
                                }
                                
                                break;
                            
                            case HSF_FACE_TRISTRIP:
                                vtxIdx[0] = hsfFaceP->strip.data[j].vertex;
                                vtxIdx[1] = hsfFaceP->strip.data[j+1].vertex;
                                vtxIdx[2] = hsfFaceP->strip.data[j+2].vertex;
                                break;
                        }
                        VECSubtract(&triP->center, &start, &centerDist);
                        if(VECSquareMag(&centerDist) > triP->dist+mag2) {
                            temp = FALSE;
                        } else {
                            temp = TRUE;
                        }
                        if(temp != FALSE) {
                            colPlaneResult = ColPlaneDistGet(&start, &end, &delta, 0, dist, 0, vtx, vtxIdx, triP, &outDist);
                            if(colPlaneResult >= 0) {
                                dist = outDist;
                                polyAttr = ColCodeGet(meshP->obj->mesh.material[hsfFaceP->mat & 0xFFF].flags);
                                mdlNo = meshP->mdlNo;
                                obj = meshP->obj;
                                triNo = i;
                            }
                        }
                    }
                }
            }
        }
    }
    if(triNo < 0) {
        return FALSE;
    }
    if(outPos) {
        VECSubtract(pos2, pos1, &outDelta);
        VECScale(&outDelta, &outDelta, dist);
        VECAdd(pos1, &outDelta, outPos);
    }
    if(outCode) {
        *outCode = polyAttr;
    }
    if(outMdlNo) {
        *outMdlNo = mdlNo;
    }
    if(outObj) {
        *outObj = obj;
    }
    if(outTriNo) {
        *outTriNo = triNo;
    }
    return TRUE;
}

void ColMapActorExec(void)
{
    COLMAP_ACTOR *actorP; //r31
    COLMESH *meshP; //r30
    int i; //r27
    int no; //r22
    int colNum; //r21
    int posNo; //r18
    
    HuVecF temp; //sp+0x3C
    int otherPosNo; //sp+0x38
    
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    otherPosNo = posNo ^ 1;
    colNum = 1;
    if(colWork.attr & 0x8) {
        return;
    }
    UpdateMeshMtx();
    for(actorP=colWork.actor, no=colWork.actorNum; no--; actorP++) {
        if(actorP->param.attr & 0x1) {
            actorP->groundAttr = 0;
            actorP->oldPos = actorP->pos[otherPosNo];
            VECSubtract(&actorP->pos[posNo], &actorP->oldPos, &actorP->moveDir);
            actorP->pos[posNo] = actorP->oldPos;
            actorP->unk130 = 0;
            actorP->unk134 = 1;
            actorP->colPointNum = 0;
            actorP->paramAttr = 0;
            memset(&actorP->colBit[0], 0, sizeof(actorP->colBit));
            ClearColPoint(actorP);
        }
    }
    for(meshP=colMesh, no=colMeshCount; no--; meshP++) {
        for(actorP=colWork.actor, i=0; i<colWork.actorNum; i++, actorP++) {
            if(actorP->param.attr & 0x1) {
                if(actorP->param.attr & 0x01000000) {
                    if(actorP->param.attr & 0x8000) {
                        MTXMultVec(meshP->mtxInvOld, &actorP->pos[otherPosNo], &temp);
                        MTXMultVec(meshP->mtx, &temp, &temp);
                        VECSubtract(&temp, &actorP->pos[otherPosNo], &meshP->actorMove[i]);
                    } else {
                        meshP->actorMove[i].x = 0;
                        meshP->actorMove[i].y = 0;
                        meshP->actorMove[i].z = 0;
                    }
                } else {
                    MTXMultVec(meshP->mtx, &meshP->actorPos[i], &temp);
                    VECSubtract(&temp, &actorP->pos[otherPosNo], &meshP->actorMove[i]);
                }
                if(fabs(meshP->actorMove[i].x) < 0.001f) {
                    meshP->actorMove[i].x = 0;
                }
                if(fabs(meshP->actorMove[i].y) < 0.001f) {
                    meshP->actorMove[i].y = 0;
                }
                if(fabs(meshP->actorMove[i].z) < 0.001f) {
                    meshP->actorMove[i].z = 0;
                }
            }
        }
    }
    for(actorP=colWork.actor, no=colWork.actorNum; no--; actorP++) {
        actorP->param.attr &= 0xFFFF;
    }
    while(colNum) {
        if(colWork.attr & 0x20) {
            _ActorMeshColAlt();
            colNum = _ActorColBroadAlt();
        } else {
            _ActorMeshCol();
            colNum = _ActorColBroad();
        }
        colNum += _ActorColNarrow();
        colNum += _ActorColResponse();
        _ColPointUpdate();
    }
    _ActorMeshUpdate(posNo);
    colWork.attr ^= 0x4;
    colWork.attr |= 0x8;
}