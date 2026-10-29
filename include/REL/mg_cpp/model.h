#ifndef MGCPP_MODEL_H
#define MGCPP_MODEL_H

#include "game/hu3d.h"
#include "game/object.h"

class CModel {
public:
    CModel(void *data);
    CModel(unsigned int fileId);
    CModel(HU3DMODELHOOK hookFunc);
    CModel(int charNo, int lod);
    CModel(const CModel &other);
    ~CModel();
    void setAttr(u32 attr);
    void resetAttr(u32 attr);
    void setObjXForm(OMOBJ *obj);
    void setPos(const HuVecF &pos);
    void setRot(const HuVecF &rot);
    void setScale(const HuVecF &scale);
    void setScale(float scale);
    void setTPLvl(float tpLvl);
    void setLayer(s16 layer);
    void setCamera(u16 bit);
    void setHookData(void *ptr);
    Mtx &getMatrix(void);
    HSFOBJECT *getObjPtr(const char *name);
    void setShadow();
    void setShadowMap();
    void setShadowMapObj(const char *objName);
    void resetShadowMapObj();
    void resetShadowMapObj(const char *objName);
    void dispOffObj(const char *objName);
    void dispOnObj(const char *objName);
    void dispOffObjChild(const char *objName);
    void dispOnObjChild(const char *objName);
    void setMotTime(int time);
    
    operator HU3DMODELID() const
    {
        return m_mdlId;
    }
    
public:
    HU3DMODELID getMdlId() const
    {
        return m_mdlId;
    }
private:
    HU3DMODELID m_mdlId;
};

#endif