#include "REL/mg_cpp/model.h"
#include "game/data.h"
#include "game/charman.h"
#include "REL/mg_cpp/assert.h"


CModel::CModel(void *data)
{
    m_mdlId = Hu3DModelCreate(data);
}

CModel::CModel(unsigned int fileId)
{
    m_mdlId = Hu3DModelCreateData(fileId);
}

CModel::CModel(HU3DMODELHOOK hookFunc)
{
    m_mdlId = Hu3DHookFuncCreate(hookFunc);
}

CModel::CModel(int charNo, int lod)
{
    int model = 0;
    switch(lod) {
        case 0:
            model = CHAR_MODEL0;
            break;
        
        case 1:
            model = CHAR_MODEL1;
            break;
        
        case 2:
            model = CHAR_MODEL2;
            break;
        
        case 3:
            model = CHAR_MODEL3;
            break;
    }
    m_mdlId = CharModelCreate(charNo, model);
}

CModel::CModel(const CModel &other)
{
    m_mdlId = Hu3DModelLink(other.getMdlId());
}

void CModel::setAttr(u32 attr)
{
    Hu3DModelAttrSet(m_mdlId, attr);
}

void CModel::resetAttr(u32 attr)
{
    Hu3DModelAttrReset(m_mdlId, attr);
}

void CModel::setObjXForm(OMOBJ *obj)
{
    HU3DMODELID mdlId = m_mdlId;
    Hu3DModelPosSet(mdlId, obj->trans.x, obj->trans.y, obj->trans.z);
    Hu3DModelRotSet(mdlId, obj->rot.x, obj->rot.y, obj->rot.z);
    Hu3DModelScaleSet(mdlId, obj->scale.x, obj->scale.y, obj->scale.z);
}

void CModel::setPos(const HuVecF &pos)
{
    Hu3DModelPosSet(m_mdlId, pos.x, pos.y, pos.z);
}

void CModel::setRot(const HuVecF &rot)
{
    Hu3DModelRotSet(m_mdlId, rot.x, rot.y, rot.z);
}

void CModel::setScale(const HuVecF &scale)
{
    Hu3DModelScaleSet(m_mdlId, scale.x, scale.y, scale.z);
}

void CModel::setScale(float scale)
{
    Hu3DModelScaleSet(m_mdlId, scale, scale, scale);
}

void CModel::setTPLvl(float tpLvl)
{
    Hu3DModelTPLvlSet(m_mdlId, tpLvl);
}

void CModel::setLayer(s16 layer)
{
    Hu3DModelLayerSet(m_mdlId, layer);
}

void CModel::setCamera(u16 bit)
{
    Hu3DModelCameraSet(m_mdlId, bit);
}

void CModel::setHookData(void *ptr)
{
    Hu3DData[m_mdlId].hookData = ptr;
}

Mtx &CModel::getMatrix(void)
{
    return Hu3DData[m_mdlId].mtx;
}

CModel::~CModel()
{
    Hu3DModelKill(m_mdlId);
}

HSFOBJECT *CModel::getObjPtr(const char *name)
{
    HSFOBJECT *p = Hu3DModelObjPtrGet(m_mdlId, name);
    ASSERT_LINE(p != NULL, 138);
    return p;
}

void CModel::setShadow()
{
    Hu3DModelShadowSet(m_mdlId);
}

void CModel::setShadowMap()
{
    Hu3DModelShadowMapSet(m_mdlId);
}

void CModel::setShadowMapObj(const char *objName)
{
    Hu3DModelShadowMapObjSet(m_mdlId, objName);
}

void CModel::resetShadowMapObj()
{
    Hu3DModelShadowMapObjReset(m_mdlId);
}

void CModel::resetShadowMapObj(const char *objName)
{
    HSFOBJECT *obj = getObjPtr(objName);
    HSFCONSTDATA *constData = (HSFCONSTDATA *)obj->constData;
    constData->attr &= ~HU3D_CONST_SHADOW_MAP;
}

void CModel::dispOffObj(const char *objName)
{
    HSFOBJECT *obj = getObjPtr(objName);
    obj->flags |= HSF_MATERIAL_DISPOFF;
}

void CModel::dispOnObj(const char *objName)
{
    HSFOBJECT *obj = getObjPtr(objName);
    obj->flags &= ~HSF_MATERIAL_DISPOFF;
}

void CModel::dispOffObjChild(const char *objName)
{
    HSFOBJECT *obj = getObjPtr(objName);
    obj->flags |= HSF_MATERIAL_DISPOFF;
    HSFTRANSFORM *xform = &obj->mesh.base;
    xform->scale.x = 0;
    xform->scale.y = 0;
    xform->scale.z = 0;
}

void CModel::dispOnObjChild(const char *objName)
{
    HSFOBJECT *obj = getObjPtr(objName);
    obj->flags &= ~HSF_MATERIAL_DISPOFF;
    HSFTRANSFORM *xform = &obj->mesh.base;
    xform->scale.x = 1;
    xform->scale.y = 1;
    xform->scale.z = 1;
}

void CModel::setMotTime(int time)
{
    Hu3DMotionTimeSet(getMdlId(), time);
}