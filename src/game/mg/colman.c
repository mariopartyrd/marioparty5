#include "game/mg/colman.h"
#include "game/hu3d.h"
#include "game/frand.h"

typedef struct ColBroad_s {
    float t;
    COLMAP_ACTOR *actor1;
    COLMAP_ACTOR *actor2;
    s16 actor1Idx;
    s16 actor2Idx;
    int colResult;
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

extern inline float fabsf2(float x)
{
   return (float)fabs((float)x);
}

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

static void RemoveMtxTrans(Mtx out, Mtx in)
{
    HuVecF row1, row2, row3;
    row1.x = in[0][0];
    row1.y = in[0][1];
    row1.z = in[0][2];
    row2.x = in[1][0];
    row2.y = in[1][1];
    row2.z = in[1][2];
    row3.x = in[2][0];
    row3.y = in[2][1];
    row3.z = in[2][2];
    
    out[0][0] = row1.x;
    out[0][1] = row1.y;
    out[0][2] = row1.z;
    out[0][3] = 0;
    out[1][0] = row2.x;
    out[1][1] = row2.y;
    out[1][2] = row2.z;
    out[1][3] = 0;
    out[2][0] = row2.x;
    out[2][1] = row2.y;
    out[2][2] = row2.z;
    out[2][3] = 0;
}

static void MakeScaleMtx(Mtx out, Mtx in)
{
    HuVecF row1, row2, row3;
    row1.x = in[0][0];
    row1.y = in[0][1];
    row1.z = in[0][2];
    row2.x = in[1][0];
    row2.y = in[1][1];
    row2.z = in[1][2];
    row3.x = in[2][0];
    row3.y = in[2][1];
    row3.z = in[2][2];
    out[0][0] = VECMag(&row1);
    out[0][1] = 0;
    out[0][2] = 0;
    out[0][3] = 0;
    out[1][0] = 0;
    out[1][1] = VECMag(&row2);
    out[1][2] = 0;
    out[1][3] = 0;
    out[2][0] = 0;
    out[2][1] = 0;
    out[2][2] = VECMag(&row3);
    out[2][3] = 0;
}

static void UseFloat()
{
    (void)0.001f;
    (void)0.5f;
    (void)-1.0f;
    (void)1.0f;
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

static BOOL ColPlaneEdgeCheck(HuVecF a, HuVecF b, HuVecF c, COLTRI *tri, BOOL extFlag)
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
           (void)0.0001f;
        }
        return FALSE;
    }
}

static BOOL ColPlaneYCheck(HuVecF *center, HuVecF *dir, COLTRI *tri, HuVecF *out, BOOL flatF)
{
    HuVecF temp;
    HuVecF vtx[4];
    
    int i;
    BOOL result;
    int num;
    
    float d;
    float mind;
    result = FALSE;
    
    if(flatF) {
        vtx[0].x = 0;
        vtx[0].z = 0;
        vtx[1].x = 0;
        vtx[1].z = 0;
        vtx[0].y = dir->y;
        vtx[1].y = -dir->y;
        num = 2;
    } else {
        vtx[0].x = 0;
        vtx[0].z = 0;
        if(tri->norm.y >= 0) {
            vtx[0].y = -dir->y;
        } else {
            vtx[0].y = dir->y;
        }
        num=1;
    }
    for(i=0; i<num; i++) {
        VECAdd(center, &vtx[i], &temp);
        d = tri->d+VECDotProduct(&tri->norm, &temp);
        if(d >= -0.001f) {
            if(result == FALSE || d < mind) {
                result = TRUE;
                mind = mind = d;
                *out = vtx[i];
            }
        }
    }
    
    return result;
}

static BOOL ColPlaneCheck(HuVecF *center, HuVecF *size, COLTRI *tri, HuVecF *eject, BOOL flatF)
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
            if(!ColPlaneCheck(&a, size, tri, &eject, FALSE)) {
                *outDist = startD;
                return -5;
            }
            VECAdd(&a, &eject, &a);
            if(!ColPlaneEdgeCheck(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
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
        if(fabs(dot) < 0.001f) {
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
        ColPlaneCheck(start, size, tri, &eject, FALSE);
        VECAdd(&a, &eject, &a);
        if(!ColPlaneEdgeCheck(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
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
            if(!ColPlaneEdgeCheck(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
                return -6;
            } else {
                return -5;
            }
        } else {
            return -1;
        }
    } else {
        float dot = VECDotProduct(&tri->norm, dir);
        if(fabs(dot) < 0.001f) {
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
        if(!ColPlaneEdgeCheck(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
            return -4;
        } else {
            *outDist = dist;
            return -0;
        }
    }
}

typedef struct ColLine_s {
    float endA;
    float startA;
    float endB;
    float startB;
} COLLINE;

static float _GetColLineTime(COLLINE *a, COLLINE *b, float t, BOOL *result)
{
    COLLINE *maxP;
    COLLINE *minP;
    float ret;
    float delta;
    
    if((a->endB-a->startB) > (b->endB-b->startB)) {
        maxP = a;
        minP = b;
    } else {
        maxP = b;
        minP = a;
    }
    *result = FALSE;
    if(maxP->startB <= minP->endB && minP->endB <= maxP->endB) {
        *result = TRUE;
    } else if(maxP->startB <= minP->startB && minP->startB <= maxP->endB) {
        *result = TRUE;
    }
    if(*result) {
        return 0;
    }
    if(fabsf2(t) < 0.0001f)  {
        return -1;
    }
    if(a->endA > b->endA) {
        if(t > 0) {
            return -1;
        } else {
            delta = a->startA-b->endA;
            ret = -(delta/t);
        }
    } else {
        if(t < 0) {
            return -1;
        }
        delta = a->endA-b->startA;
        ret = -(delta/t);
    }
    
    return ret;
}

static BOOL _EdgeSphereColInline(HuVecF *startPos, HuVecF *endPos, HuVecF *vtxStart, HuVecF *vtxEnd, HuVecF *dir, float *arg5, float *arg6)
{
    float scale;
    float endMag2;
    float endDirDot;
    float dirMag2;
    float endDot;
    float dirDot;
    
    HuVecF temp;
    
    VECSubtract(vtxStart, startPos, &temp);
    
    endMag2 = VECDotProduct(endPos, endPos);
    endDirDot = VECDotProduct(endPos, dir);
    dirMag2 = VECDotProduct(dir, dir);
    endDot = VECDotProduct(&temp, endPos);
    dirDot = VECDotProduct(&temp, dir);
    scale = (endMag2*dirMag2)-(endDirDot*endDirDot);
    if(fabsf2(scale) < 0.0001f) {
        return FALSE;
    }
    *arg5 = ((dirMag2*endDot)-(endDirDot*dirDot))/scale;
    *arg6 = ((endDirDot*endDot)-(endMag2*dirDot))/scale;
    return TRUE;
}

static inline double GetVecDot(HuVecF a, HuVecF b)
{
    return ((double)a.x*b.x)+((double)a.y*b.y)+((double)a.z*b.z);
    
}

static BOOL _EdgeSphereCol(HuVecF *startPos, HuVecF *endPos, float radius, HuVecF vtxStart, HuVecF vtxEnd, float *colT, BOOL *validColF)
{
    BOOL endZero;
    BOOL vtxDeltaZero;
    
    double temp_f31;
    double mag2;
    double temp_f26;
    double t;
    
    float startT;
    float r2;
    float temp_f21;
    double edgeMag;
    
    double vtxDeltaNorm[3];
    HuVecF vtxDelta;
    HuVecF sp274;
    HuVecF sp268;
    HuVecF sp25C;
    HuVecF sp250;
    HuVecF sp244;
    HuVecF sp238;
    HuVecF sp22C;
    
    double endMag2;
    double sp138;
    double sp130;
    double vtxDeltaMag;
    
    float sp58;
    float sp54;
    float sp50;
    float sp4C;
    float sp48;
    
    vtxStart.y = 0;
    vtxEnd.y = 0;
    *validColF = FALSE;
    VECSubtract(&vtxEnd, &vtxStart, &vtxDelta);
    r2 = radius*radius;
    
    if(_EdgeSphereColInline(startPos, endPos, &vtxStart, &vtxEnd, &vtxDelta, &sp58, &sp54)) {
        VECScale(endPos, &sp274, sp58);
        VECAdd(startPos, &sp274, &sp274);
        VECScale(&vtxDelta, &sp268, sp54);
        VECAdd(&vtxStart, &sp268, &sp268);
        VECSubtract(&sp274, &sp268, &sp25C);
        if(VECSquareMag(&sp25C) > r2) {
            return FALSE;
        }
    } else {
        endZero = VECSquareMag(endPos) < 0.0001f;
        vtxDeltaZero = VECSquareMag(&vtxDelta) < 0.0001f;
        if(endZero && vtxDeltaZero) {
            VECSubtract(&vtxStart, startPos, &sp250);
            if(VECSquareMag(&sp250) > r2) {
                return FALSE;
            } else {
                *colT = 0;
                *validColF = TRUE;
                return TRUE;
            }
        }
        if(!endZero && !vtxDeltaZero) {
            sp54 = (VECDotProduct(startPos, &vtxDelta)-VECDotProduct(&vtxStart, &vtxDelta))/VECSquareMag(&vtxDelta);
            VECScale(&vtxDelta, &sp250, sp54);
            VECAdd(&vtxStart, &sp250, &sp250);
            VECSubtract(&sp250, startPos, &sp250);
            if(VECSquareMag(&sp250) > r2) {
                return FALSE;
            }
            startT = (VECDotProduct(&vtxStart, endPos)-VECDotProduct(startPos, endPos))/VECSquareMag(endPos);
            temp_f21 = (VECDotProduct(&vtxEnd, endPos)-VECDotProduct(startPos, endPos))/VECSquareMag(endPos);
            if(VECDotProduct(&vtxDelta, endPos) > 0) {
                if(0 <= startT && startT <= 1.0f) {
                    *colT = startT;
                     return TRUE;
                } else if(0.0f <= temp_f21 && temp_f21 <= 1.0f) {
                    *colT = 0;
                    *validColF = TRUE;
                    return TRUE;
                }
            } else {
                if(0 <= temp_f21 && temp_f21 <= 1.0f) {
                    *colT = temp_f21;
                     return TRUE;
                } else if(0 <= startT && startT <= 1.0f) {
                    *colT = 0;
                    *validColF = TRUE;
                    return TRUE;
                }
            }
            startT = (VECDotProduct(startPos, &vtxDelta)-VECDotProduct(&vtxStart, &vtxDelta))/VECSquareMag(&vtxDelta);
            if(0 <= startT && startT <= 1.0f) {
                *colT = 0;
                *validColF = TRUE;
                return TRUE;
            } else {
                return FALSE;
            }
        } else if(endZero) {
            VECSubtract(&vtxStart, startPos, &sp244);
            mag2 = GetVecDot(vtxDelta, vtxDelta);
            temp_f26 = GetVecDot(vtxDelta, sp244);
            edgeMag = GetVecDot(sp244, sp244)-r2;
            
        } else if(vtxDeltaZero) {
            VECSubtract(startPos, &vtxStart, &sp244);
            mag2 = GetVecDot(*endPos, *endPos);
            temp_f26 = GetVecDot(*endPos, sp244);
            edgeMag = GetVecDot(sp244, sp244)-r2;
        }
        temp_f31 = (temp_f26*temp_f26)-(mag2*edgeMag);
        if(temp_f31 < 0) {
            if(temp_f31 < -fabsf2(temp_f31*0.0001f)) {
                return FALSE;
            } else {
                temp_f31 = 0;
            }
        } else {
            temp_f31 = sqrt(temp_f31);
        }
        if(fabsf2(mag2) < 0.0001f) {
            return FALSE;
        }
        t = (-temp_f26-temp_f31)/mag2;
        if(t < 0) {
            sp50 = (-temp_f26+temp_f31)/mag2;
            if(sp50 < 0) {
                return FALSE;
            } else {
                t = 0;
                *validColF = TRUE;
            }
        }
        if(t < -0.001f || 1.001f < t) {
            return FALSE;
        }
        if(endZero) {
            *colT = 0;
            *validColF = TRUE;
        } else {
            *colT = t;
        }
        return TRUE;
    }
    VECSubtract(startPos, &vtxStart, &sp238);
    endMag2 = GetVecDot(*endPos, *endPos);
    vtxDeltaMag = sqrt(((double)vtxDelta.x*vtxDelta.x)+((double)vtxDelta.z*vtxDelta.z));
    vtxDeltaNorm[0] = vtxDelta.x/vtxDeltaMag;
    vtxDeltaNorm[1] = 0;
    vtxDeltaNorm[2] = vtxDelta.z/vtxDeltaMag;
    sp130 = (vtxDeltaNorm[0]*endPos->x)+(vtxDeltaNorm[2]*endPos->z);
    sp138 = (vtxDeltaNorm[0]*sp238.x)+(vtxDeltaNorm[2]*sp238.z);
    mag2 = endMag2-(sp130*sp130);
    temp_f26 = GetVecDot(sp238, *endPos)-(sp130*sp138);
    edgeMag = (GetVecDot(sp238, sp238)-(sp138*sp138))-r2;
    temp_f31 = (temp_f26*temp_f26)-(mag2*edgeMag);
    if(temp_f31 < 0) {
        if(temp_f31 < -fabsf2(temp_f31*0.0001f)) {
            return FALSE;
        } else {
            temp_f31 = 0;
        }
    } else {
        temp_f31 = sqrt(temp_f31);
    }
    if(fabsf2(mag2) < 0.0001f) {
        return FALSE;
    }
    t = (-temp_f26-temp_f31)/mag2;
    if(t < 0) {
        sp48 = (-temp_f26+temp_f31)/mag2;
        if(sp48 < 0) {
            return FALSE;
        } else {
            t = 0;
            *validColF = TRUE;
        }
    }
    VECScale(endPos, &sp22C, t);
    VECAdd(startPos, &sp22C, &sp22C);
    sp4C = (VECDotProduct(&sp22C, &vtxDelta)-VECDotProduct(&vtxStart, &vtxDelta))/VECSquareMag(&vtxDelta);
    if(sp4C < -0.001f || sp4C > 1.001f) {
        return FALSE;
    }
    *colT = t;
    return TRUE;
}

static BOOL _ActorEdgeSphereCol(HuVecF *startPos, HuVecF *endPos, COLMAP_ACTOR *actorP, HuVecF *vtxStart, HuVecF *vtxEnd, float *colT, BOOL *colValidF, HuVecF *out)
{
    BOOL lineValid;

    float t;
    float dot;
    float halfH;
    float t2;
    
    HuVecF vtxDelta;
    HuVecF ejectPos;
    HuVecF endPos2;
    HuVecF dir;
    HuVecF ejectVec;
    HuVecF start;
    HuVecF delta;
    HuVecF lineStart;
    HuVecF lineEnd;
    COLLINE lineA;
    COLLINE lineB;
    HuVecF endColPos;
    HuVecF startColPos;
    
    float edgeColT;
    BOOL edgeColF;
    
    
    
    VECSubtract(vtxEnd, vtxStart, &vtxDelta);
    if(VECSquareMag(&vtxDelta) < 0.0001f) {
        return FALSE;
    }
    halfH = actorP->param.height/2;
    lineA.endA = startPos->y+halfH;
    lineA.startA = startPos->y-halfH;
    lineA.endB = startPos->y+halfH;
    lineA.startB = startPos->y-halfH;
    dir = vtxDelta;
    dir.y = 0;
    
    if(VECSquareMag(&dir) < 0.001f) {
        if(vtxStart->y > vtxEnd->y) {
            lineB.endA = vtxStart->y;
            lineB.startA = vtxEnd->y;
            lineB.endB = vtxStart->y;
            lineB.startB = vtxEnd->y;
        } else {
            lineB.endA = vtxEnd->y;
            lineB.startA = vtxStart->y;
            lineB.endB = vtxEnd->y;
            lineB.startB = vtxStart->y;
        }
        t = _GetColLineTime(&lineA, &lineB, endPos->y, &lineValid);
        if(t < 0 || 1 < t) {
            return FALSE;
        }
    } else {
        VECNormalize(&dir, &dir);
        VECScale(&dir, &ejectVec, actorP->param.radius);
        VECAdd(startPos, &ejectVec, &ejectPos);
        start = *vtxStart;
        delta = vtxDelta;
        start.y = 0;
        ejectPos.y = 0;
        delta.y = 0;
        dot = (VECDotProduct(&ejectPos, &delta)-VECDotProduct(&start, &delta))/VECSquareMag(&delta);
        VECScale(&vtxDelta, &lineStart, dot);
        VECAdd(vtxStart, &lineStart, &lineStart);
        VECAdd(startPos, &ejectVec, &ejectPos);
        VECAdd(&ejectPos, endPos, &ejectPos);
        ejectPos.y = 0;
        dot = (VECDotProduct(&ejectPos, &delta)-VECDotProduct(&start, &delta))/VECSquareMag(&delta);
        VECScale(&vtxDelta, &lineEnd, dot);
        VECAdd(vtxStart, &lineEnd, &lineEnd);
        if(lineStart.y > lineEnd.y) {
            lineB.endA = lineStart.y;
            lineB.startA = lineEnd.y;
            lineB.endB = lineStart.y;
            lineB.startB = lineEnd.y;
        } else {
            lineB.endA = lineEnd.y;
            lineB.startA = lineStart.y;
            lineB.endB = lineEnd.y;
            lineB.startB = lineStart.y;
        }
        t = _GetColLineTime(&lineA, &lineB, endPos->y, &lineValid);
        if(t < 0 || 1 < t) {
            t = -1;
        }
        VECSubtract(startPos, &ejectVec, &ejectPos);
        start = *vtxStart;
        delta = vtxDelta;
        start.y = 0;
        ejectPos.y = 0;
        delta.y = 0;
        dot = (VECDotProduct(&ejectPos, &delta)-VECDotProduct(&start, &delta))/VECSquareMag(&delta);
        VECScale(&vtxDelta, &lineStart, dot);
        VECAdd(vtxStart, &lineStart, &lineStart);
        PSVECSubtract(startPos, &ejectVec, &ejectPos);
        VECAdd(&ejectPos, endPos, &ejectPos);
        ejectPos.y = 0;
        dot = (VECDotProduct(&ejectPos, &delta)-VECDotProduct(&start, &delta))/VECSquareMag(&delta);
        VECScale(&vtxDelta, &lineEnd, dot);
        VECAdd(vtxStart, &lineEnd, &lineEnd);
        if(lineStart.y > lineEnd.y) {
            lineB.endA = lineStart.y;
            lineB.startA = lineEnd.y;
            lineB.endB = lineStart.y;
            lineB.startB = lineEnd.y;
        } else {
            lineB.endA = lineEnd.y;
            lineB.startA = lineStart.y;
            lineB.endB = lineEnd.y;
            lineB.startB = lineStart.y;
        }
        t2 = _GetColLineTime(&lineA, &lineB, endPos->y, &lineValid);
        if(0.0f <= t2 && t2 <= 1.0f && (t < 0 || t2 < t)) {
            t = t2;
        }
        if(t < 0 || 1 < t) {
            return FALSE;
        }
    }
    ejectPos = *startPos;
    endPos2 = *endPos;
    ejectPos.y = 0;
    endPos2.y = 0;
    if(!_EdgeSphereCol(&ejectPos, &endPos2, actorP->param.radius, *vtxStart, *vtxEnd, &edgeColT, &edgeColF)) {
        return FALSE;
    }
    *colT = (edgeColT > t) ? edgeColT : t;
    *colValidF = lineValid && edgeColF;
    if(out) {
        VECScale(endPos, &endColPos, *colT);
        VECAdd(startPos, &endColPos, &endColPos);
        dot = (VECDotProduct(&endColPos, &vtxDelta)-VECDotProduct(vtxStart, &vtxDelta))/VECSquareMag(&vtxDelta);
        if(dot < 0) {
            dot = 0;
        } else if(dot > 1) {
            dot = 1;
        }
        VECScale(&vtxDelta, &startColPos, dot);
        VECAdd(vtxStart, &startColPos, &startColPos);
        VECSubtract(&endColPos, &startColPos, out);
    }
    return TRUE;
}

static BOOL _ActorTriSphereCol(HuVecF *startPos, HuVecF *delta, COLMAP_ACTOR *actorP, HuVecF *vtxBuf, int *index, float *colT, HuVecF *out)
{
    HuVecF temp;
    HuVecF *outP;
    int ret;
    BOOL result;
    int sp44;
    float t;

    int i;
    
    result = _ActorEdgeSphereCol(startPos, delta, actorP, &vtxBuf[index[2]], &vtxBuf[index[0]], colT, &sp44, out);
    if(result) {
        ret = 2;
    } else {
        ret = -1;
    }
    outP = (out) ? &temp : NULL;
    for(i=0; i<2; i++) {
        result = _ActorEdgeSphereCol(startPos, delta, actorP, &vtxBuf[index[i]], &vtxBuf[index[i+1]], &t, &sp44, outP);
        if(result) {
            if(ret < 0 || sp44 || (t >= 0 && *colT > t)) {
                if(out) {
                    *out = temp;
                }
                *colT = t;
                ret = i;
            }
        }
    }
    return ret;
}

static BOOL _ColCylEdgeCalc(HuVecF *pos, HuVecF *posNew, HuVecF *posDelta, float radius, float startT, float colT, HuVecF *vtxBuf, int *index, float *outT, HuVecF *adjust)
{
    HuVecF *vtx; //r29
    int no; //r28
    HuVecF *vtx2; //r24
    
    double temp_f29;
    float t;
    double temp_f26;
    double temp_f25;
    double temp_f22;
    double temp_f21;
    double edgeMag;
    float temp_f19;
    float temp_f18;
    
    
    double edgeNorm[3];
    HuVecF edge;
    HuVecF spD8;
    HuVecF vtxDir;
    HuVecF spC0;
    

    double deltaMag2;
    double r2;
    double sp60;

    *outT = startT;
    deltaMag2 = GetVecDot(*posDelta, *posDelta);
    r2 = radius*radius;
    for(no=3; no--;) {
        vtx = &vtxBuf[index[no]];
        vtx2 = &vtxBuf[index[(no+1)%3]];
        VECSubtract(vtx2, vtx, &edge);
        VECSubtract(pos, vtx, &vtxDir);
        if(VECSquareMag(&edge) < 0.0001f) {
            continue;
        }
        edgeMag = sqrt(((double)edge.x*edge.x)+((double)edge.y*edge.y)+((double)edge.z*edge.z));
        edgeNorm[0] = edge.x/edgeMag;
        edgeNorm[1] = edge.y/edgeMag;
        edgeNorm[2] = edge.z/edgeMag;
        temp_f21 = (edgeNorm[0]*posDelta->x)+(edgeNorm[1]*posDelta->y)+(edgeNorm[2]*posDelta->z);
        temp_f22 = (edgeNorm[0]*vtxDir.x)+(edgeNorm[1]*vtxDir.y)+(edgeNorm[2]*vtxDir.z);
        temp_f26 = deltaMag2-(temp_f21*temp_f21);
        temp_f25 = GetVecDot(vtxDir, *posDelta)-(temp_f21*temp_f22);
        sp60 = (GetVecDot(vtxDir, vtxDir)-(temp_f22*temp_f22))-r2;
        if(0.0f == temp_f26) {
            if(VECSquareMag(&edge) > deltaMag2) {
                t = (VECDotProduct(pos, &edge)-VECDotProduct(vtx, &edge))/VECSquareMag(&edge);
                if(t >= 0 && t <= 1) {
                    VECScale(&edge, &spC0, t);
                    VECAdd(vtx, &spC0, &spC0);
                    VECSubtract(&spC0, pos, &spC0);
                    if(VECSquareMag(&spC0) > r2) {
                        continue;
                    }
                    *outT = colT;
                }
            }
        } else {
            temp_f29 = (temp_f25*temp_f25)-(temp_f26*sp60);
            if(temp_f29 < 0) {
                if(temp_f29 < -fabsf2(temp_f29*0.0001f)) {
                    continue;
                }
                temp_f29 = 0;
            } else {
                temp_f29 = sqrt(temp_f29);
            }
            if(fabsf2(temp_f26) < 0.0001f) {
                continue;
            }
            t = (-temp_f25-temp_f29)/temp_f26;
            if(t < 0) {
                temp_f18 = (-temp_f25+temp_f29)/temp_f26;
                if(temp_f18 < 0) {
                    continue;
                }
                if(1 < temp_f18) {
                    continue;
                }
                t = temp_f18;
            } else {
                if(*outT < t) {
                    continue;
                }
                if(colT > t) {
                    continue;
                }
            }
            VECScale(posDelta, &spD8, t);
            VECAdd(&spD8, pos, &spD8);
            temp_f19 = (VECDotProduct(&spD8, &edge)-VECDotProduct(vtx, &edge))/VECSquareMag(&edge);
            if(temp_f19 < -0.001f) {
                continue;
            }
            if(temp_f19 > 1.001f) {
                continue;
            }
            *outT = t;
            VECScale(&edge, &vtxDir, temp_f19);
            VECAdd(&vtxDir, vtx, &vtxDir);
            VECSubtract(&spD8, &vtxDir, adjust);
        }
    }
    return *outT < startT;
}

static BOOL _ColCylVtxCalc(HuVecF *pos, HuVecF *posDelta, float radius, float startT, float colT, HuVecF *vtxBuf, int *index, float *outT, HuVecF *adjust)
{
    HuVecF *vtx;
    int no;
    
    float outR;
    float dtMag;
    float t;
    float deltaMag;
    float r2;
    float temp;
    
    HuVecF dt;
    HuVecF vtxDir;
    
    r2 = radius*radius;
    deltaMag = VECMag(posDelta);
    *outT = startT;
    if(deltaMag < 0.0001f) {
        return 0;
    }
    for(no=3; no--;) {
        vtx = &vtxBuf[index[no]];
        t = (VECDotProduct(vtx, posDelta)-VECDotProduct(pos, posDelta))/VECSquareMag(posDelta);
        VECScale(posDelta, &dt, t);
        dtMag = VECSquareMag(&dt);
        VECSubtract(pos, vtx, &vtxDir);
        outR = dtMag-(VECSquareMag(&vtxDir)-r2);
        if(outR < 0) {
            if(outR < -fabsf2(outR*0.0001f)) {
                continue;
            }
            outR = 0;
        } else {
            outR = sqrtf(outR);
        }
        dtMag = sqrtf(dtMag);
        temp = dtMag-outR;
        t = temp/deltaMag;
        if(t < 0) {
            t = dtMag+outR;
            t /= deltaMag;
        }
        if(colT < t && t < *outT) {
            VECScale(posDelta, adjust, t);
            VECAdd(adjust, pos, adjust);
            VECSubtract(adjust, vtx, adjust);
            *outT = t;
        }
    }
    return *outT != startT;
}

static float _ColCylAxisCheck(HuVecF *startA, HuVecF *velA, float radiusA, HuVecF *startB, HuVecF *velB, float radiusB)
{
    HuVecF deltaStart;
    HuVecF deltaVel;
    
    float startVelDot;
    float r;
    float colT;
    float ret;
    
    float velMag2;
    float maxR;
    float startMag2;

    VECSubtract(startA, startB, &deltaStart);
    VECSubtract(velA, velB, &deltaVel);
    r = radiusA+radiusB;
    r = r*r;
    startVelDot = VECDotProduct(&deltaStart, &deltaVel);
    velMag2 = VECDotProduct(&deltaVel, &deltaVel);
    if(velMag2 < 0.001f) {
        return -1;
    }
    ret = -startVelDot/velMag2;
    startMag2 = VECSquareMag(&deltaStart);
    maxR = startMag2+(startVelDot*ret);
    if(r < maxR) {
        return -1;
    }
    colT = (startVelDot*startVelDot)-(velMag2*(startMag2-r));
    if(colT < 0) {
        if(colT < -0.001f) {
            return -1;
        } else {
            colT = 0;
        }
    } else {
        colT = sqrt(colT);
    }
    ret = (-startVelDot-colT)/velMag2;
    if(ret < 0) {
        return -1;
    }
    if(ret > 1) {
        if(ret < 1.001f) {
            ret = 1;
        } else {
            return -1;
        }
    }
    return ret;
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

#define InitColPoint(actorP, actorNo, norm, hsfFaceP, meshP, face) \
do { \
    COLMAP_COLPOINT *colPointP = &actorP->colPoint[actorP->colPointNum]; \
    colPointP->colOfs = meshP->actorMove[actorNo]; \
    colPointP->normal = norm; \
    colPointP->polyAttr = ColCodeGet(meshP->obj->mesh.material[hsfFaceP->mat & 0xFFF].flags); \
    colPointP->meshNo = meshP->mdlNo; \
    colPointP->obj = meshP->obj; \
    colPointP->faceNo = face; \
} while(0)


static BOOL _ActorApplyColAttr(COLMAP_ACTOR *actorP, COLMAP_ATTR_PARAM *attrParam, int code);

static BOOL _ColCorrection(COLMAP_ACTOR *actorP)
{
    COLMAP_COLPOINT *colPointP = &actorP->colPoint[actorP->colPointNum];
    COLMAP_ATTR_PARAM *attrParam;
    int codeNo;
    BOOL ret;
    u32 attr;
    
    
    codeNo = ColCodeNoGet(colPointP->polyAttr);
    if(codeNo < 0) {
        OSReport("( colman.c : _ColCorrection ) | マップとあたってるはずなのにアトリビュートがありません\n");
        return FALSE;
    }
    attrParam = (codeNo >= 7) ? (&colWork.attrParamHi[codeNo-7]) : (&colWork.attrParam[codeNo]);
    actorP->groundAttr |= colPointP->polyAttr;
    colPointP->polyAttr = 0;
    ret = _ActorApplyColAttr(actorP, attrParam, codeNo);
    if(ret && actorP->param.colCorrectHook) {
        actorP->param.colCorrectHook(actorP, actorP->param.user);
    }
    return ret;
}

typedef struct ColCylinder_s {
    HuVecF startPos;
    HuVecF vel;
    float t;
    float radius;
    float height;
    HuVecF outPos;
} COLCYLINDER;

static float _ColSphereTest(COLCYLINDER *a, COLCYLINDER *b, int *result)
{
    HuVecF endVec;
    HuVecF relVel;
    HuVecF endPosA;
    HuVecF endPosB;
    COLLINE lineA;
    COLLINE lineB;
    HuVecF startA;
    HuVecF startB;
    HuVecF velA;
    HuVecF velB;
    
    float endMag2;
    float colTime;
    float aRadius;
    float bRadius;

    float colLineTime;
    float ret;
    float endRadius;
    float endDist;
    float maxRadius2;
    
    BOOL lineColResult;
    BOOL endColF;
    
    VECSubtract(&a->vel, &b->vel, &relVel);
    *result = 4;
    VECScale(&a->vel, &endPosA, a->t);
    VECAdd(&a->startPos, &endPosA, &endPosA);
    VECScale(&b->vel, &endPosB, b->t);
    VECAdd(&b->startPos, &endPosB, &endPosB);
    aRadius = a->height/2;
    bRadius = b->height/2;

    
    lineA.endA = a->startPos.y+aRadius;
    lineA.startA = a->startPos.y-aRadius;
    lineA.endB = endPosA.y+aRadius;
    lineA.startB = endPosA.y-aRadius;
    
    lineB.endA = b->startPos.y+bRadius;
    lineB.startA = b->startPos.y-bRadius;
    lineB.endB = endPosB.y+bRadius;
    lineB.startB = endPosB.y-bRadius;
    colLineTime = _GetColLineTime(&lineA, &lineB, relVel.y, &lineColResult);
    if(lineColResult) {
        endDist = (bRadius+aRadius)-fabsf2(b->startPos.y-a->startPos.y);
    }
    startA = a->startPos;
    startA.y = 0;
    startB = b->startPos;
    startB.y = 0;
    velA = a->vel;
    velA.y = 0;
    velB = b->vel;
    velB.y = 0;
    endPosA.y = 0;
    endPosB.y = 0;
    VECSubtract(&endPosA, &endPosB, &endVec);
    maxRadius2 = (a->radius+b->radius);
    maxRadius2 = maxRadius2*maxRadius2;
    endMag2 = VECSquareMag(&endVec);
    if(endMag2 < maxRadius2) {
        endRadius = (a->radius+b->radius)-sqrtf(endMag2);
        endColF = TRUE;
        colTime = 0;
    } else {
        colTime = _ColCylAxisCheck(&startA, &velA, a->radius, &startB, &velB, b->radius);
        endColF = FALSE;
    }
    if(colLineTime < 0 || colTime < 0) {
        return -1;
    }
    if(endColF && lineColResult) {
        if(endDist > endRadius) {
            *result = 2;
        } else {
            *result = 3;
        }
    }
    if(colTime > colLineTime) {
        if(*result == 4) {
            *result = 0;
        }
        ret = colTime;
    } else {
        if(*result == 4) {
            *result = 1;
        }
        ret = colLineTime;
    }
    if(ret < 0 || 1 < ret) {
        *result = 4;
        return -1;
    }
    return ret;
}

static float _ColCylinderTest(COLCYLINDER *a, COLCYLINDER *b, int *result)
{
    HuVecF endVec;
    HuVecF relVel;
    HuVecF endPosA;
    HuVecF endPosB;
    COLLINE lineA;
    COLLINE lineB;
    HuVecF startA;
    HuVecF startB;
    HuVecF velA;
    HuVecF velB;
    
    float endMag2;
    float colTime;
    float ret;
    float maxRadius2;
    
    float colLineTime;
    float endRadius;
    float endDist;
    
    int lineColResult;
    int endColF;
    
    VECSubtract(&a->vel, &b->vel, &relVel);
    *result = 4;
    VECScale(&a->vel, &endPosA, a->t);
    VECAdd(&a->startPos, &endPosA, &endPosA);
    VECScale(&b->vel, &endPosB, b->t);
    VECAdd(&b->startPos, &endPosB, &endPosB);
    endPosA.y -= a->radius;
    endPosB.y -= b->radius;
    
    lineA.endA = a->height+(a->startPos.y-a->radius);
    lineA.startA = a->startPos.y-a->radius;
    lineA.endB = endPosA.y+a->height;
    lineA.startB = endPosA.y;
    
    lineB.endA = b->height+(b->startPos.y-b->radius);
    lineB.startA = b->startPos.y-b->radius;
    lineB.endB = endPosB.y+b->height;
    lineB.startB = endPosB.y;
    colLineTime = _GetColLineTime(&lineA, &lineB, relVel.y, &lineColResult);
    if(lineColResult) {
        endDist = (0.5f*(a->height+b->height))-fabsf2(endPosA.y-endPosB.y);
    }
    startA = a->startPos;
    startA.y = 0;
    startB = b->startPos;
    startB.y = 0;
    velA = a->vel;
    velA.y = 0;
    velB = b->vel;
    velB.y = 0;
    endPosA.y = 0;
    endPosB.y = 0;
    VECSubtract(&endPosA, &endPosB, &endVec);
    maxRadius2 = (a->radius+b->radius);
    maxRadius2 = maxRadius2*maxRadius2;
    endMag2 = VECSquareMag(&endVec);
    if(endMag2 < maxRadius2) {
        endRadius = (a->radius+b->radius)-sqrtf(endMag2);
        endColF = 1;
        colTime = 0;
    } else {
        colTime = _ColCylAxisCheck(&startA, &velA, a->radius, &startB, &velB, b->radius);
        endColF = 0;
    }
    if(colLineTime < 0 || colTime < 0) {
        return -1;
    }
    if(endColF && lineColResult) {
        if(endDist > endRadius) {
            *result = 2;
        } else {
            *result = 3;
        }
    }
    if(colTime > colLineTime) {
        if(*result == 4) {
            *result = 0;
        }
        ret = colTime;
    } else {
        if(*result == 4) {
            *result = 1;
        }
        ret = colLineTime;
    }
    if(ret < 0 || 1 < ret) {
        *result = 4;
        return -1;
    }
    return ret;
}

static BOOL CheckFacePoint(COLTRI *triP, HuVecF *p, float maxDist)
{
    HuVecF pCenter;
    VECSubtract(&triP->center, p, &pCenter);
    if(VECSquareMag(&pCenter) > triP->dist+maxDist) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void CalcCross(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *out)
{
    HuVecF ba;
    HuVecF cb;
    VECSubtract(b, a, &ba);
    VECSubtract(c, b, &cb);
    VECCrossProduct(&ba, &cb, out);
    VECNormalize(out, out);
}

static void CalcMeshNorm(COLMESH *meshP, HuVecF a, HuVecF b, HuVecF c, HuVecF *out)
{
    HuVecF ba;
    HuVecF cb;
    
    MTXMultVec(meshP->mtx, &a, &a);
    MTXMultVec(meshP->mtx, &b, &b);
    MTXMultVec(meshP->mtx, &c, &c);
    VECSubtract(&b, &a, &ba);
    VECSubtract(&c, &b, &cb);
    VECCrossProduct(&ba, &cb, out);
    VECNormalize(out, out);
}

static BOOL CheckPoint(COLMAP_ACTOR *actorP, COLMESH *meshP, float arg, int *index, HuVecF *vtxBuf, HuVecF *out)
{
    int posNo;
    HuVecF temp;
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    if(actorP->colT != 0) {
        (void)arg;
        return FALSE;
    }
    CalcMeshNorm(meshP, vtxBuf[index[0]], vtxBuf[index[1]], vtxBuf[index[2]], out);
    VECScale(out, &temp, arg-0.0001f);
    VECSubtract(&actorP->oldPos, &temp, &actorP->pos[posNo]);
    actorP->oldPos = actorP->pos[posNo];
    actorP->oldColT = 1;
    actorP->param.attr &= ~0x10000000;
    return TRUE;
}

static void _ActorMeshCol(void)
{
    COLMAP_ACTOR *actorP; //r31
    COLMESH *meshP; //r30
    HSFFACE *hsfFaceP; //r29
    HuVecF *vtxBuf; //r28
    COLTRI *triP; //r27
    COLMAP_ACTOR *actorP2; //r24
    int triNo; //r23
    BOOL doneF; //r22
    int colResult; //r21
    int j; //r19
    int posNo; //r17
    
    float radius; //f31
    float maxDist; //f30
    float temp_f29;
    float temp_f28;
    float temp_f27;
    float temp_f26;
    
    HuVecF pos;
    HuVecF posNew;
    HuVecF posDelta;
    HuVecF sp21C;
    int index[3]; //sp+0x210
    HuVecF sp204;
    HuVecF sp1F8;
    HuVecF sp1EC;
    HuVecF sp1E0;
    HuVecF posAdjust;
    HuVecF norm;
    
    
    
    int no; //sp+0x64
    int i; //sp+0x60
    float colT; //sp+0x5C
    int triNum; //sp+0x58
    
    meshP = colMesh;
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    for(no=colMeshCount; no--; meshP++) {
        for(actorP=colWork.actor, i=0; i<colWork.actorNum; i++, actorP++) {
            actorP2 = actorP;
            if((actorP->param.attr & 0x1) && (actorP->param.attr & 0x80000012) == 0) {
                if(!(actorP2->param.mask & meshP->mask)) {
                    continue;
                } else {
                    doneF = TRUE;
                    while(doneF) {
                        doneF = FALSE;
                        pos = actorP->oldPos;
                        VECAdd(&pos, &actorP->moveDir, &posNew);
                        MTXMultVec(meshP->mtxInvOld, &pos, &pos);
                        MTXMultVec(meshP->mtxInv, &posNew, &posNew);
                        VECSubtract(&posNew, &pos, &posDelta);
                        temp_f26 = VECSquareMag(&posDelta);
                        vtxBuf = meshP->obj->mesh.vertex->data;
                        hsfFaceP = meshP->obj->mesh.face->data;
                        triP = meshP->tri;
                        for(j=0; j<meshP->obj->mesh.face->count; j++, hsfFaceP++) {
                            if(doneF) {
                                break;
                            }
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
                            for(triNo=0; triNo<triNum; triNo++, triP++) {
                                if(triP->norm.x || triP->norm.y || triP->norm.z) {
                                    MakeIndexBuf(index, hsfFaceP, triNo);
                                    colT = actorP->oldColT;
                                    radius = actorP2->param.radius;
                                    maxDist = radius+(actorP2->param.height/2);
                                    maxDist = temp_f26+(maxDist*maxDist);
                                    (void)temp_f26;
                                    if(CheckFacePoint(triP, &pos, maxDist)) {
                                        colResult = ColPlaneDistGet(&pos, &posNew, &posDelta, radius, colT, actorP->colT, vtxBuf, index, triP, &colT);
                                        if(colResult == -6) {
                                            temp_f29 = actorP2->param.radius;
                                            temp_f28 = actorP2->param.radius;
                                            norm = triP->norm;
                                            sp1EC.x = norm.x;
                                            sp1EC.y = 0;
                                            sp1EC.z = norm.z;
                                            if(fabsf2(sp1EC.x) > 0.001f || fabsf2(sp1EC.z) > 0.001f) {
                                                VECNormalize(&sp1EC, &sp1EC);
                                            }
                                            temp_f29 /= 2;
                                            sp1EC.x *= temp_f28;
                                            if(fabsf2(norm.y) < 0.001f) {
                                                sp1EC.y = 0;
                                            } else {
                                                if(norm.y > 0) {
                                                    temp_f27 = temp_f29;
                                                } else {
                                                    temp_f27 = -temp_f29;
                                                }
                                                sp1EC.y = temp_f27;
                                            }
                                            sp1EC.z *= temp_f28;
                                            if(!ColPlaneCheck(&pos, &sp1EC, triP, &sp1F8, FALSE)) {
                                                colResult = -1;
                                                if(ColPlaneCheck(&pos, &sp1EC, triP, &sp1F8, TRUE)) {
                                                    VECAdd(&pos, &sp1F8, &sp204);
                                                    if(ColPlaneEdgeCheck(sp204, vtxBuf[index[0]], vtxBuf[index[1]], triP, FALSE)) {
                                                        colResult = -6;
                                                    }
                                                }
                                            } else {
                                                colResult = -4;
                                            }
                                        }
                                        switch(colResult) {
                                            case -4:
                                            case -6:
                                                if(VECSquareMag(&posDelta) < 0.0001f) {
                                                    break;
                                                } else {
                                                    COLMAP_COLPOINT *colPointP;
                                                    int no;
                                                    BOOL ret;
                                                    
                                                    ret = FALSE;
                                                    for(no=actorP->colPointNum; no--;) {
                                                        colPointP = &actorP->colPoint[no];
                                                        if(colPointP->obj == meshP->obj &&  colPointP->faceNo == j) {
                                                            ret = TRUE;
                                                            break;
                                                        }
                                                    }
                                                    if(!ret) {
                                                        if(!_ColCylEdgeCalc(&pos, &posNew, &posDelta, radius, actorP->oldColT, actorP->colT, vtxBuf, index, &colT, &posAdjust)
                                                            && !_ColCylVtxCalc(&pos, &posDelta, radius, actorP->oldColT, actorP->colT, vtxBuf, index, &colT, &posAdjust)) {
                                                            if(colResult == -6) {
                                                                if(CheckPoint(actorP, meshP, colT, index, vtxBuf, &sp21C)) {
                                                                    doneF = TRUE;
                                                                    goto colPoint;
                                                                }
                                                            }
                                                        } else {
                                                            VECScale(&actorP->moveDir, &sp1E0, colT);
                                                            VECAdd(&sp1E0, &actorP->oldPos, &sp1E0);
                                                            actorP->pos[posNo] = sp1E0;
                                                            if(VECSquareMag(&posAdjust) > 0.001f) {
                                                                VECNormalize(&posAdjust, &sp21C);
                                                            } else {
                                                                CalcMeshNorm(meshP, vtxBuf[index[0]], vtxBuf[index[1]], vtxBuf[index[2]], &sp21C);
                                                            }
                                                            actorP->oldColT = colT;
                                                            if(actorP->oldColT < 0) {
                                                                actorP->oldColT = actorP->colT;
                                                            }
                                                            actorP->param.attr |= 0x10000000;
                                                            goto colPoint;
                                                        }
                                                    }
                                                }
                                                break;
                                            
                                            case 0:
                                                CalcMeshNorm(meshP, vtxBuf[index[0]], vtxBuf[index[1]], vtxBuf[index[2]], &sp21C);
                                                VECScale(&actorP->moveDir, &actorP->pos[posNo], colT);
                                                VECAdd(&actorP->pos[posNo], &actorP->oldPos, &actorP->pos[posNo]);
                                                actorP->oldColT = colT;
                                                actorP->param.attr &= ~0x10000000;
                                                goto colPoint;

                                            case -5:
                                                if(CheckPoint(actorP, meshP, colT, index, vtxBuf, &sp21C)) {
                                                    doneF = TRUE;
                                                    default:
                                                    colPoint:
                                                    InitColPoint(actorP, i, sp21C, hsfFaceP, meshP, j);
                                                    actorP->param.attr |= 0x04000000;
                                                    if(doneF) {
                                                        actorP->param.attr |= 0x20000000;
                                                        actorP->param.attr &= ~0x14000000;
                                                        _ColCorrection(actorP);
                                                        goto skipTri;
                                                    }
                                                    
                                                    actorP->param.attr &= ~0x20000000;
                                                }
                                                break;
                                            
                                            case -1:
                                            case -2:
                                            case -3:
                                                break;
                                            
                                            
                                        }
                                    }
                                }
                            }
                            skipTri:
                            (void)actorP;
                        }
                    }
                }
            }
        }
    }
}


static void _ActorMeshSphereCol(void)
{
    COLMAP_ACTOR *actorP; //r31
    HSFFACE *hsfFaceP; //r30
    COLMESH *meshP; //r29
    COLTRI *triP; //r28
    HuVecF *vtxBuf; //r27
    COLMAP_ACTOR *actorP2; //r24
    int triNo; //r23
    int posNo; //r22
    BOOL doneF; //r19
    int j; //r18
    
    float maxDist; //f31
    float height; //f30
    float ejectDot; //f28
    float radius; //f27
    float dirSign; //f26
    float deltaMag2; //f25
    float axisLen; //f24
    
    HuVecF pos; //sp+0x224
    HuVecF posNew; //sp+0x218
    HuVecF posDelta; //sp+0x20C
    HuVecF meshNorm; //sp+0x200
    int index[3]; //sp+0x1F4
    HuVecF ejectDir; //sp+0x1E8
    HuVecF planeA; //sp+0x1DC
    HuVecF axisVec; //sp+0x1D0
    HuVecF finalPos; //sp+0x1C4
    HuVecF posAdjust; //sp+0x1B8
    HuVecF norm; //sp+0x1AC
    
    int no; //sp+0x80
    int i; //sp+0x7C
    int colResult; //sp+0x78
    float colT; //sp+0x74
    int triNum; //sp+0x70
    int sphereColResult; //sp+0xC
    
    meshP = colMesh;
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    for(no=colMeshCount; no--; meshP++) {
        for(actorP=colWork.actor, i=0; i<colWork.actorNum; i++, actorP++) {
            actorP2 = actorP;
            if((actorP->param.attr & 0x1) && (actorP->param.attr & 0x80000012) == 0) {
                if(!(actorP2->param.mask & meshP->mask)) {
                    continue;
                } else {
                    doneF = TRUE;
                    while(doneF) {
                        doneF = FALSE;
                        pos = actorP->oldPos;
                        VECAdd(&pos, &actorP->moveDir, &posNew);
                        MTXMultVec(meshP->mtxInvOld, &pos, &pos);
                        MTXMultVec(meshP->mtxInv, &posNew, &posNew);
                        VECSubtract(&posNew, &pos, &posDelta);
                        deltaMag2 = VECSquareMag(&posDelta);
                        vtxBuf = meshP->obj->mesh.vertex->data;
                        hsfFaceP = meshP->obj->mesh.face->data;
                        triP = meshP->tri;
                        for(j=0; j<meshP->obj->mesh.face->count; j++, hsfFaceP++) {
                            if(doneF) {
                                break;
                            }
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
                            for(triNo=0; triNo<triNum; triNo++, triP++) {
                                if(triP->norm.x || triP->norm.y || triP->norm.z) {
                                    MakeIndexBuf(index, hsfFaceP, triNo);
                                    colT = actorP->oldColT;
                                    height = actorP2->param.height;
                                    radius = actorP2->param.radius;
                                    norm = triP->norm;
                                    ejectDir.x = norm.x;
                                    ejectDir.y = 0;
                                    ejectDir.z = norm.z;
                                    if(fabsf2(ejectDir.x) > 0.001f || fabsf2(ejectDir.z) > 0.001f) {
                                        VECNormalize(&ejectDir, &ejectDir);
                                    }
                                    height /= 2;
                                    ejectDir.x *= radius;
                                    if(fabsf2(norm.y) < 0.001f) {
                                        ejectDir.y = 0;
                                    } else {
                                        if(norm.y > 0) {
                                            dirSign = height;
                                        } else {
                                            dirSign = -height;
                                        }
                                        ejectDir.y = dirSign;
                                    }
                                    ejectDir.z *= radius;
                                    ejectDot = VECDotProduct(&triP->norm, &ejectDir);
                                    maxDist = ejectDot+(actorP2->param.height/2);
                                    maxDist = deltaMag2+(maxDist*maxDist);
                                    (void)deltaMag2;
                                    (void)axisLen;
                                    if(CheckFacePoint(triP, &pos, maxDist)) {
                                        colResult = ColEjectDistGet(&pos, &posNew, &posDelta, &ejectDir, ejectDot, colT, actorP->colT, vtxBuf, index, triP, &colT);
                                        switch(colResult) {
                                            case -6:
                                                if(ColPlaneYCheck(&pos, &ejectDir, triP, &axisVec, FALSE)) {
                                                    ColPlaneCheck(&pos, &ejectDir, triP, &axisVec, FALSE);
                                                    VECAdd(&pos, &axisVec, &planeA);
                                                    axisLen = triP->d+VECDotProduct(&triP->norm, &planeA);
                                                    VECScale(&triP->norm, &axisVec, axisLen);
                                                    VECSubtract(&planeA, &axisVec, &planeA);
                                                    if(ColPlaneEdgeCheck(planeA, vtxBuf[index[0]], vtxBuf[index[1]], triP, FALSE)) {
                                                        case -4:
                                                        {
                                                            COLMAP_COLPOINT *colPointP;
                                                            int no;
                                                            BOOL ret;
                                                            
                                                            ret = FALSE;
                                                            for(no=actorP->colPointNum; no--;) {
                                                                colPointP = &actorP->colPoint[no];
                                                                if(colPointP->obj == meshP->obj &&  colPointP->faceNo == j) {
                                                                    ret = TRUE;
                                                                    break;
                                                                }
                                                            }
                                                            if(ret || VECSquareMag(&posDelta) < 0.0001f) {
                                                                break;
                                                            }
                                                            sphereColResult = _ActorTriSphereCol(&pos, &posDelta, actorP2, vtxBuf, index, &colT, NULL);
                                                            if(!(sphereColResult < 0 || colT < actorP->colT || actorP->oldColT < colT)){
                                                                VECScale(&actorP->moveDir, &finalPos, colT);
                                                                VECAdd(&finalPos, &actorP->oldPos, &finalPos);
                                                                actorP->pos[posNo] = finalPos;
                                                                CalcMeshNorm(meshP, vtxBuf[index[0]], vtxBuf[index[1]], vtxBuf[index[2]], &meshNorm);
                                                                actorP->oldColT = colT;
                                                                actorP->param.attr |= 0x10000000;
                                                                goto colPoint;
                                                            }
                                                        }
                                                        
                                                    }
                                                }
                                                break;
                                            
                                            case 0:
                                                CalcMeshNorm(meshP, vtxBuf[index[0]], vtxBuf[index[1]], vtxBuf[index[2]], &meshNorm);
                                                VECScale(&actorP->moveDir, &actorP->pos[posNo], colT);
                                                VECAdd(&actorP->pos[posNo], &actorP->oldPos, &actorP->pos[posNo]);
                                                actorP->oldColT = colT;
                                                actorP->param.attr &= ~0x10000000;
                                                goto colPoint;
                                            
                                            case -5:
                                                if(actorP->colT == 0.0f) {
                                                    CalcMeshNorm(meshP, vtxBuf[index[0]], vtxBuf[index[1]], vtxBuf[index[2]], &meshNorm);
                                                    VECScale(&meshNorm, &posAdjust, colT-0.0001f);
                                                    VECSubtract(&actorP->oldPos, &posAdjust, &actorP->pos[posNo]);
                                                    actorP->oldPos = actorP->pos[posNo];
                                                    actorP->oldColT = 1;
                                                    doneF = TRUE;
                                                    actorP->param.attr &= ~0x10000000;
                                                    default:
                                                    colPoint:
                                                    InitColPoint(actorP, i, meshNorm, hsfFaceP, meshP, j);
                                                    actorP->param.attr |= 0x04000000;
                                                    if(doneF) {
                                                        actorP->param.attr |= 0x20000000;
                                                        actorP->param.attr &= ~0x14000000;
                                                        _ColCorrection(actorP);
                                                        goto skipTri;
                                                    }
                                                    actorP->param.attr &= ~0x20000000;
                                                }
                                                break;

                                            case -1:
                                            case -2:
                                            case -3:
                                                break;
                                        }
                                    }
                                }
                            }
                            skipTri:
                            (void)actorP;
                        }
                    }
                }
            }
        }
    }
}

static int _ActorColSphereBroad(void)
{
    COLMAP_ACTOR *actorP; //r31
    COLMAP_ACTOR *actorP2; //r30
    COLBROAD *broadP; //r29
    int i; //r26
    int j; //r25
    BOOL resetPoint; //r24
    BOOL ret; //r23
    u8 clearPoint1; //r22
    u8 clearPoint2; //r21
    
    
    float t; //f31
    
    COLCYLINDER cylA; //sp+0x40
    COLCYLINDER cylB; //sp+0x10
    int colResult; //sp+0xC
    
    int posIdx; //sp+0x8
    
    ret = FALSE;
    colWork.broadColNum = 0;
    posIdx = (colWork.attr & 0x4) ? 1 : 0;
    
    for(actorP=colWork.actor, i=0; i<colWork.actorNum; i++, actorP++) {
        if((actorP->param.attr & 0x1) && (actorP->param.attr & 0x14) == 0) {
            cylA.startPos = actorP->oldPos;
            cylA.vel = actorP->moveDir;
            cylA.t = actorP->colT;
            cylA.radius = actorP->param.radius;
            cylA.height = actorP->param.height;
            for(actorP2=actorP+1, j=i+1; j<colWork.actorNum; j++, actorP2++) {
                if((actorP2->param.attr & 0x1) && (actorP2->param.attr & 0x14) == 0 && (actorP->param.mask & actorP2->param.mask)) {
                    if(!(actorP->colBit[j >> 5] & (1 << (j & 0x1F))) || !(actorP2->colBit[i >> 5] & (1 << (i & 0x1F)))) {
                        cylB.startPos = actorP2->oldPos;
                        cylB.vel = actorP2->moveDir;
                        cylB.t = actorP2->colT;
                        cylB.radius = actorP2->param.radius;
                        cylB.height = actorP2->param.height;
                        t = _ColSphereTest(&cylA, &cylB, &colResult);
                        if(colResult != 4) {
                            if(colResult == 2 || colResult == 3) {
                                if(actorP->colT > actorP2->colT) {
                                    t = actorP->colT;
                                } else {
                                    t = actorP2->colT;
                                }
                            } else {
                                if(t < actorP->colT && (actorP->colT-t) >0.001f) {
                                    continue;
                                }
                                if(t < actorP2->colT && (actorP2->colT-t) >0.001f) {
                                    continue;
                                }
                                resetPoint = FALSE;
                                clearPoint1 = FALSE;
                                clearPoint2 = FALSE;
                                if(t > actorP->oldColT && (t-actorP->oldColT) > 0.001f) {
                                    resetPoint = TRUE;
                                } else {
                                    clearPoint1 = TRUE;
                                }
                                if(t > actorP2->oldColT && (t-actorP2->oldColT) > 0.001f) {
                                    resetPoint = TRUE;
                                } else {
                                    clearPoint2 = TRUE;
                                }
                                if(resetPoint) {
                                    if(clearPoint1) {
                                        ClearColPoint(actorP);
                                    }
                                    if(clearPoint2) {
                                        ClearColPoint(actorP2);
                                    }
                                    ret = TRUE;
                                    continue;
                                }
                            }
                            broadP = &colWork.broadCol[colWork.broadColNum];
                            broadP->t = t;
                            broadP->actor1 = actorP;
                            broadP->actor2 = actorP2;
                            broadP->colResult = colResult;
                            broadP->actor1Idx = i;
                            broadP->actor2Idx = j;
                            
                            colWork.broadColNum++;
                            actorP->param.attr |= 0x08000000;
                            actorP2->param.attr |= 0x08000000;
                        }
                    }
                }
            }
        }
    }
    return ret;
}

static int _ActorColBroad(void)
{
    COLMAP_ACTOR *actorP; //r31
    COLMAP_ACTOR *actorP2; //r30
    COLBROAD *broadP; //r29
    int i; //r26
    int j; //r25
    BOOL resetPoint; //r24
    BOOL ret; //r23
    u8 clearPoint1; //r22
    u8 clearPoint2; //r21
    
    
    float t; //f31
    
    COLCYLINDER cylA; //sp+0x40
    COLCYLINDER cylB; //sp+0x10
    int colResult; //sp+0xC
    
    int posIdx; //sp+0x8
    
    ret = FALSE;
    colWork.broadColNum = 0;
    posIdx = (colWork.attr & 0x4) ? 1 : 0;
    
    for(actorP=colWork.actor, i=0; i<colWork.actorNum; i++, actorP++) {
        if((actorP->param.attr & 0x1) && (actorP->param.attr & 0x14) == 0) {
            cylA.startPos = actorP->oldPos;
            cylA.vel = actorP->moveDir;
            cylA.t = actorP->colT;
            cylA.radius = actorP->param.radius;
            cylA.height = actorP->param.height;
            for(actorP2=actorP+1, j=i+1; j<colWork.actorNum; j++, actorP2++) {
                if((actorP2->param.attr & 0x1) && (actorP2->param.attr & 0x14) == 0 && (actorP->param.mask & actorP2->param.mask)) {
                    if(!(actorP->colBit[j >> 5] & (1 << (j & 0x1F))) || !(actorP2->colBit[i >> 5] & (1 << (i & 0x1F)))) {
                        cylB.startPos = actorP2->oldPos;
                        cylB.vel = actorP2->moveDir;
                        cylB.t = actorP2->colT;
                        cylB.radius = actorP2->param.radius;
                        cylB.height = actorP2->param.height;
                        t = _ColCylinderTest(&cylA, &cylB, &colResult);
                        if(colResult != 4) {
                            if(colResult == 2 || colResult == 3) {
                                if(actorP->colT > actorP2->colT) {
                                    t = actorP->colT;
                                } else {
                                    t = actorP2->colT;
                                }
                            } else {
                                if(t < actorP->colT && (actorP->colT-t) >0.001f) {
                                    continue;
                                }
                                if(t < actorP2->colT && (actorP2->colT-t) >0.001f) {
                                    continue;
                                }
                                resetPoint = FALSE;
                                clearPoint1 = FALSE;
                                clearPoint2 = FALSE;
                                if(t > actorP->oldColT && (t-actorP->oldColT) >0.001f) {
                                    resetPoint = TRUE;
                                } else {
                                    clearPoint1 = TRUE;
                                }
                                if(t > actorP2->oldColT && (t-actorP2->oldColT) >0.001f) {
                                    resetPoint = TRUE;
                                } else {
                                    clearPoint2 = TRUE;
                                }
                                if(resetPoint) {
                                    if(clearPoint1) {
                                        ClearColPoint(actorP);
                                    }
                                    if(clearPoint2) {
                                        ClearColPoint(actorP2);
                                    }
                                    ret = TRUE;
                                    continue;
                                }
                            }
                            broadP = &colWork.broadCol[colWork.broadColNum];
                            broadP->t = t;
                            broadP->actor1 = actorP;
                            broadP->actor2 = actorP2;
                            broadP->colResult = colResult;
                            broadP->actor1Idx = i;
                            broadP->actor2Idx = j;
                            
                            colWork.broadColNum++;
                            actorP->param.attr |= 0x08000000;
                            actorP2->param.attr |= 0x08000000;
                        }
                    }
                }
            }
        }
    }
    return ret;
}

static int _ActorColNarrow(void)
{
    COLMAP_ACTOR *actorP1; //r31
    COLMAP_ACTOR *actorP2; //r30
    COLBROAD *broadP; //r29
    int colResult; //r28
    int i; //r25
    int cbResult1; //r24
    int cbResult2; //r23
    COLNARROW *narrowP; //r22
    COLNARROW *narrow; //r21
    int ret; //r20
    int posNo; //r19
    
    float h1; //f31
    float h2; //f30
    float r1; //f29
    float r2; //f28
    float dot; //f27
    
    COLMAP_NARROW_PARAM col1;
    COLMAP_NARROW_PARAM col2;
    HuVecF fixDir;
    HuVecF pointOfs;
    HuVecF colDeltaNorm;
    HuVecF colDelta;
    HuVecF colXZDelta;
    HuVecF norm1;
    HuVecF norm2;
    HuVecF newMove1;
    HuVecF newMove2;
    
    narrowP = colWork.narrowCol;
    if(colWork.broadColNum == 0) {
        return FALSE;
    }
    ret = FALSE;
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    for(broadP=colWork.broadCol, i=0; i<colWork.broadColNum; i++, broadP++) {
        narrowP[i].t = broadP->t;
        narrowP[i].broadP = broadP;
    }
    SortCollisions(narrowP, colWork.broadColNum);
    for(narrow=narrowP, i=0; i<colWork.broadColNum; i++, narrow++) {
        broadP = narrow->broadP;
        actorP1 = broadP->actor1;
        actorP2 = broadP->actor2;
        if(!(actorP1->param.attr & 0x40000000) && !(actorP2->param.attr & 0x40000000)) {
            actorP1->colBit[broadP->actor2Idx >> 5] |= (1 << (broadP->actor2Idx & 0x1F));
            actorP2->colBit[broadP->actor1Idx >> 5] |= (1 << (broadP->actor1Idx & 0x1F));
            r1 = broadP->actor1->param.radius;
            r2 = broadP->actor2->param.radius;
            h1 = broadP->actor1->param.height/2;
            h2 = broadP->actor2->param.height/2;
            norm1 = actorP1->moveDir;
            col1.paramA = actorP1->param.paramA;
            col1.paramB = actorP1->param.paramB;
            col1.type = actorP1->param.type;
            col1.normPos = norm1;
            col1.colResult = broadP->colResult;
            VECScale(&actorP1->moveDir, &col1.point, broadP->t);
            VECAdd(&col1.point, &actorP1->oldPos, &col1.point);
            norm2 = actorP2->moveDir;
            col2.paramA = actorP2->param.paramA;
            col2.paramB = actorP2->param.paramB;
            col2.type = actorP2->param.type;
            col2.normPos = norm2;
            col2.colResult = broadP->colResult;
            
            VECScale(&actorP2->moveDir, &col2.point, broadP->t);
            VECAdd(&col2.point, &actorP2->oldPos, &col2.point);
            if(!(colWork.attr & 0x20)) {
                col1.point.y = (actorP1->param.height*0.5f)+(col1.point.y-actorP1->param.radius);
                col2.point.y = (actorP2->param.height*0.5f)+(col2.point.y-actorP2->param.radius);
            }
            cbResult1 = 0;
            if(actorP1->param.narrowHook) {
                cbResult1 = actorP1->param.narrowHook(&col1, &col2);
            }
            if(actorP1->param.narrowHook2) {
                cbResult1 |= actorP1->param.narrowHook2(&col1, &col2);
            }
            newMove1 = col1.normPos;
            col1.normPos = norm1;
            col2.normPos = norm2;
            cbResult2 = 0;
            if(actorP2->param.narrowHook) {
                cbResult2 = actorP2->param.narrowHook(&col2, &col1);
            }
            if(actorP2->param.narrowHook2) {
                cbResult2 |= actorP2->param.narrowHook2(&col2, &col1);
            }
            newMove2 = col2.normPos;
            col1.normPos = norm1;
            col2.normPos = norm2;
            if(actorP1->colPointNum >= 4) {
                actorP1->moveDir.x = 0;
                actorP1->moveDir.y = 0;
                actorP1->moveDir.z = 0;
                actorP1->oldPos = col1.point;
                cbResult1 = 0;
            }
            if(actorP2->colPointNum >= 4) {
                actorP2->param.attr |= 0x02000000;
                actorP2->moveDir.x = 0;
                actorP2->moveDir.y = 0;
                actorP2->moveDir.z = 0;
                actorP2->oldPos = col2.point;
                cbResult2 = 0;
            }
            VECSubtract(&col2.point, &col1.point, &colDelta);
            if(VECSquareMag(&colDelta) < 0.0001f) {
                colDelta.x = (frandmod(20)-10.0f)*0.01f;
                colDelta.z = (frandmod(20)-10.0f)*0.01f;
                colDelta.y = 0.01f;
            }
            VECNormalize(&colDelta, &colDeltaNorm);
            colXZDelta = colDeltaNorm;
            colXZDelta.y = 0;
            if(VECSquareMag(&colXZDelta) > 0.0001f) {
                VECNormalize(&colXZDelta, &colXZDelta);
            }
            if(!(colWork.attr & 0x20)) {
                col1.point.y = actorP1->param.radius+(col1.point.y-(0.5f*actorP1->param.height));
                col2.point.y = actorP2->param.radius+(col2.point.y-(0.5f*actorP2->param.height));
            }
            if(cbResult1) {
                ret = TRUE;
                colResult = broadP->colResult;
                if((actorP1->param.attr & 0x40) && colResult == 3) {
                    colResult = 2;
                } else if((actorP1->param.attr & 0x80) && colResult == 2) {
                    colResult = 3;
                }
                if(!(actorP1->param.attr & 0x8)) {
                    if(colResult == 2 && r1+r2 > 0.0001f) {
                        if(cbResult2) {
                            VECScale(&colDelta, &fixDir, r1/(r1+r2));
                            VECScale(&colXZDelta, &pointOfs, 0.001f+r1);
                            VECSubtract(&pointOfs, &fixDir, &fixDir);
                        } else {
                            VECScale(&colXZDelta, &pointOfs, 0.002f+(r1+r2));
                            VECSubtract(&pointOfs, &colDelta, &fixDir);
                        }
                        fixDir.y = 0;
                        if(VECSquareMag(&fixDir) > 0.001f) {
                            VECScale(&fixDir, &fixDir, -1);
                            if(fixDir.x < 0) {
                                if(newMove1.x > fixDir.x) {
                                    newMove1.x = fixDir.x;
                                }
                                
                            } else if(newMove1.x < fixDir.x) {
                                newMove1.x = fixDir.x;
                            }
                            if(fixDir.z < 0) {
                                if(newMove1.z > fixDir.z) {
                                    newMove1.z = fixDir.z;
                                }
                            } else {
                                if(newMove1.z < fixDir.z) {
                                    newMove1.z = fixDir.z;
                                }
                            }
                        }
                    } else if(colResult == 3 && h1+h2 > 0.0001f) {
                        if(cbResult2) {
                            VECScale(&colDelta, &fixDir, h1/(h1+h2));
                            pointOfs.x = pointOfs.z = 0;
                            if(colDeltaNorm.y >= 0) {
                                pointOfs.y = 0.001f+h1;
                            } else {
                                pointOfs.y = -(0.001f+h1);
                            }
                            VECSubtract(&pointOfs, &fixDir, &fixDir);
                        } else {
                            pointOfs.x = pointOfs.z = 0;
                            if(colDeltaNorm.y >= 0) {
                                pointOfs.y = 0.002f+(h1+h2);
                            } else {
                                pointOfs.y = -(0.002f+(h1+h2));
                            }
                            VECSubtract(&pointOfs, &colDelta, &fixDir);
                        }
                        fixDir.x = fixDir.z = 0;
                        if(VECSquareMag(&fixDir) > 0.001f) {
                            VECScale(&fixDir, &fixDir, -1);
                            if(fixDir.y < 0) {
                                if(newMove1.y > fixDir.y) {
                                    newMove1.y = fixDir.y;
                                }
                            } else if(newMove1.y < fixDir.y) {
                                newMove1.y = fixDir.y;
                            }
                        }
                    }
                }
                fixDir.x = fixDir.y = fixDir.z  = 0;
                switch(colResult) {
                    case 0:
                    case 2:
                        dot = VECDotProduct(&colXZDelta, &newMove1);
                        if(dot > 0) {
                            VECScale(&colXZDelta, &fixDir, dot);
                        }
                        if(actorP2->param.attr & 0x20) {
                            actorP1->groundAttr |= 0x8000;
                        }
                        break;
                    
                    case 1:
                    case 3:
                        if((colDeltaNorm.y < 0 && col1.normPos.y < 0) || (colDeltaNorm.y > 0 && col1.normPos.y > 0)) {
                            fixDir.y = col1.normPos.y;
                        }
                        if(actorP2->param.attr & 0x20) {
                            actorP1->groundAttr |= 0x4000;
                        }
                        break;
                }
                VECSubtract(&newMove1, &fixDir, &newMove1);
                if(VECSquareMag(&newMove1) > 0.001f) {
                    VECNormalize(&newMove1, &fixDir);
                } else {
                    fixDir = newMove1;
                }
                VECScale(&fixDir, &fixDir, 0.002f);
                VECScale(&newMove1, &pointOfs, broadP->t);
                VECSubtract(&col1.point, &pointOfs, &actorP1->oldPos);
                VECSubtract(&actorP1->oldPos, &fixDir, &actorP1->oldPos);
                actorP1->moveDir = newMove1;
                actorP1->pos[posNo] = col1.point;
                actorP1->param.attr |= 0x40000000;
                actorP1->colT = broadP->t;
                actorP1->oldColT = 1;
                ClearColPoint(actorP1);
            }
            VECScale(&colDeltaNorm, &colDeltaNorm, -1);
            VECScale(&colXZDelta, &colXZDelta, -1);
            VECScale(&colDelta, &colDelta, -1);
            if(cbResult2) {
                ret = TRUE;
                colResult = broadP->colResult;
                if((actorP1->param.attr & 0x40) && colResult == 3) {
                    colResult = 2;
                } else if((actorP1->param.attr & 0x80) && colResult == 2) {
                    colResult = 3;
                }
                if(!(actorP2->param.attr & 0x8)) {
                    if(colResult == 2 && r1+r2 > 0.0001f) {
                        if(cbResult1) {
                            VECScale(&colDelta, &fixDir, r2/(r1+r2));
                            VECScale(&colXZDelta, &pointOfs, 0.001f+r2);
                            VECSubtract(&pointOfs, &fixDir, &fixDir);
                        } else {
                            VECScale(&colXZDelta, &pointOfs, 0.002f+(r1+r2));
                            VECSubtract(&pointOfs, &colDelta, &fixDir);
                        }
                        fixDir.y = 0;
                        if(VECSquareMag(&fixDir) > 0.001f) {
                            VECScale(&fixDir, &fixDir, -1);
                            if(fixDir.x < 0) {
                                if(newMove2.x > fixDir.x) {
                                    newMove2.x = fixDir.x;
                                }
                                
                            } else if(newMove2.x < fixDir.x) {
                                newMove2.x = fixDir.x;
                            }
                            if(fixDir.z < 0) {
                                if(newMove2.z > fixDir.z) {
                                    newMove2.z = fixDir.z;
                                }
                            } else {
                                if(newMove2.z < fixDir.z) {
                                    newMove2.z = fixDir.z;
                                }
                            }
                        }
                    } else if(colResult == 3 && h1+h2 > 0.0001f) {
                        if(cbResult1) {
                            pointOfs.x = pointOfs.z = 0;
                            VECScale(&colDelta, &fixDir, h2/(h1+h2));
                            
                            if(colDeltaNorm.y >= 0) {
                                pointOfs.y = 0.001f+h2;
                            } else {
                                pointOfs.y = -(0.001f+h2);
                            }
                            VECSubtract(&pointOfs, &fixDir, &fixDir);
                        } else {
                            pointOfs.x = pointOfs.z = 0;
                            if(colDeltaNorm.y >= 0) {
                                pointOfs.y = 0.002f+(h1+h2);
                            } else {
                                pointOfs.y = -(0.002f+(h1+h2));
                            }
                            VECSubtract(&pointOfs, &colDelta, &fixDir);
                        }
                        fixDir.x = fixDir.z = 0;
                        if(VECSquareMag(&fixDir) > 0.001f) {
                            VECScale(&fixDir, &fixDir, -1);
                            if(fixDir.y < 0) {
                                if(newMove2.y > fixDir.y) {
                                    newMove2.y = fixDir.y;
                                }
                            } else if(newMove2.y < fixDir.y) {
                                newMove2.y = fixDir.y;
                            }
                        }
                    }
                }
                fixDir.x = fixDir.y = fixDir.z  = 0;
                switch(colResult) {
                    case 0:
                    case 2:
                        dot = VECDotProduct(&colXZDelta, &newMove2);
                        if(dot > 0) {
                            VECScale(&colXZDelta, &fixDir, dot);
                        }
                        if(actorP1->param.attr & 0x20) {
                            actorP2->groundAttr |= 0x8000;
                        }
                        break;
                    
                    case 1:
                    case 3:
                        if((colDeltaNorm.y < 0 && col2.normPos.y < 0) || (colDeltaNorm.y > 0 && col2.normPos.y > 0)) {
                            fixDir.y = col2.normPos.y;
                        }
                        if(actorP1->param.attr & 0x20) {
                            actorP2->groundAttr |= 0x4000;
                        }
                        break;
                }
                VECSubtract(&newMove2, &fixDir, &newMove2);
                if(VECSquareMag(&newMove2) > 0.001f) {
                    VECNormalize(&newMove2, &fixDir);
                } else {
                    fixDir = newMove2;
                }
                VECScale(&fixDir, &fixDir, -0.002f);
                VECScale(&newMove2, &pointOfs, broadP->t);
                VECSubtract(&col2.point, &pointOfs, &actorP2->oldPos);
                VECSubtract(&actorP2->oldPos, &fixDir, &actorP2->oldPos);
                actorP2->moveDir = newMove2;
                actorP2->pos[posNo] = col2.point;
                actorP2->param.attr |= 0x40000000;
                actorP2->colT = broadP->t;
                actorP2->oldColT = 1;
                ClearColPoint(actorP2);
            }
        }
        
    }
    return ret;
}

static BOOL _ActorApplyColAttr(COLMAP_ACTOR *actorP, COLMAP_ATTR_PARAM *attrParam, int code)
{
    COLMAP_COLPOINT *point = &actorP->colPoint[actorP->colPointNum];
    int posIdx = (colWork.attr & 0x4) ? 1 : 0;
    BOOL result = (actorP->param.attr & 0x02000000) ? FALSE : TRUE;
    Mtx normMtx;
    HuVecF posOfs;
    HuVecF moveDir;
    HuVecF up;
    HuVecF ofs;
    HuVecF normal;
    float c;
    float s;
    float upDot;
    float dot;
    
    if(!result) {
        actorP->moveDir.x = 0;
        actorP->moveDir.y = 0;
        actorP->moveDir.z = 0;
        actorP->oldPos = actorP->pos[posIdx];
        if(VECSquareMag(&point->colOfs) > 0.0001f) {
            actorP->moveDir = point->colOfs;
            result = TRUE;
        }
    } else {
        moveDir = actorP->moveDir;
        up.x = 0;
        up.y = 1;
        up.z = 0;
        if(!(actorP->param.attr & 0x10000000)) {
            moveDir.y *= attrParam->yDeviate;
            upDot = VECDotProduct(&up, &point->normal);
            if(upDot > 0 && upDot > attrParam->maxDot) {
                moveDir.y = 0;
            }
        }
        switch(attrParam->type) {
            case 0:
                normal = point->normal;
                c = cos((float)M_PI);
                s = sin((float)M_PI);
                normMtx[0][0] = ((1-c)*(normal.x*normal.x))+c;
                normMtx[0][1] = ((1-c)*(normal.x*normal.y))-(normal.z*s);
                normMtx[0][2] = ((1-c)*(normal.x*normal.z))+(normal.y*s);
                normMtx[1][0] = ((1-c)*(normal.y*normal.x))+(normal.z*s);
                normMtx[1][1] = ((1-c)*(normal.y*normal.y))+c;
                normMtx[1][2] = ((1-c)*(normal.y*normal.z))-(normal.x*s);
                normMtx[2][0] = ((1-c)*(normal.z*normal.x))-(normal.y*s);
                normMtx[2][1] = ((1-c)*(normal.z*normal.y))+(normal.x*s);
                normMtx[2][2] = ((1-c)*(normal.z*normal.z))+c;
                normMtx[0][3] = normMtx[1][3] = normMtx[2][3] = 0;
                MTXMultVec(normMtx, &actorP->moveDir, &moveDir);
                moveDir.x = -moveDir.x;
                moveDir.y = -moveDir.y;
                moveDir.z = -moveDir.z;
                break;
            
            case 1:
                dot = VECDotProduct(&point->normal, &moveDir);
                if(dot < 0) {
                    VECScale(&point->normal, &ofs, dot);
                    VECSubtract(&moveDir, &ofs, &moveDir);
                }
                break;
        }
        actorP->paramAttr = attrParam->attr;
        VECScale(&moveDir, &actorP->moveDir, attrParam->speed);
        if(!(actorP->param.attr & 0x100)) {
            VECAdd(&actorP->moveDir, &point->colOfs, &actorP->moveDir);
        }
        VECScale(&actorP->moveDir, &posOfs, actorP->colT);
        VECSubtract(&actorP->pos[posIdx], &posOfs, &actorP->oldPos);
        VECScale(&point->normal, &posOfs, 0.002f);
        VECAdd(&actorP->oldPos, &posOfs, &actorP->oldPos);
    }
    return result;
}

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
                actorP->colT = actorP->oldColT;
                if((actorP->param.attr & 0x20000000) || _ColCorrection(actorP)) {
                    result = 1;
                }
                if(actorP->colPointNum >= 4) {
                    actorP->param.attr |= 0x02000000;
                } else {
                    actorP->colPointNum++;
                }
            }
            
            actorP->oldColT = 1;
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
        actorP->colT = 0;
        actorP->oldColT = 1;
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
            actorP->colT = 0;
            actorP->oldColT = 1;
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
            _ActorMeshSphereCol();
            colNum = _ActorColSphereBroad();
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