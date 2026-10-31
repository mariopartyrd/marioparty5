#ifndef M501_AISU_H
#define M501_AISU_H

#include "REL/mg_cpp/linknode.h"
#include "REL/mg_cpp/handle.h"
#include "REL/m501/joint.h"

#include "humath.h"
#include "game/hu3d.h"
#include "game/audio.h"

namespace m501 {

class CAisu : public CLinkNode<CAisu> {
    
private:
    typedef void (CAisu::*AisuProc)();
    
public:
    struct DrawData {
        HU3DMODELID mdlId;
        Mtx mtx;
    };

public:
    CAisu(int type, HuVecF *pos);
    ~CAisu();
    void callProc()
    {
        (*this.*m_proc)();
    }
    static bool checkSlowMove(HuVecF *pos);
    static m501::CAisu *getTakeAisu(HuVecF *pos, float radius);
    u8 isTake();
    void setPos(HuVecF *pos, float r);
    
    m501::CJoint *doTake(m501::CJoint *joint, BOOL randomOfs);
    
    void updateMatrix()
    {
        m_joint.lockMatrix();
        Mtx *m = m_joint.getLocalMatrix();
        MTXTrans(*m, m_pos.x, m_pos.y, m_pos.z);
        float scaleY = m_scale;
        (*m)[1][1] = scaleY;
        float scaleXZ = (scaleY+(2*(1-scaleY)));
        (*m)[0][0] = scaleXZ;
        (*m)[2][2] = scaleXZ;
    }
    
    void Draw(m501::CAisu::DrawData &drawData)
    {
        Mtx m;
        if(m_drawShadow) {
            MTXTrans(m, m_pos.x, 5068.02f, m_pos.z);
            m[0][0] = m_shadowScale*15;
            m[2][2] = m_shadowScale*15;
            MTXConcat(drawData.mtx, m, m);
            Hu3DModelObjDraw(drawData.mdlId, "kage", m);
        }
        Mtx *jointMtx = m_joint.getMatrix();
        MTXConcat(drawData.mtx, *jointMtx, m);
        Hu3DModelObjDraw(drawData.mdlId, m_objName, m);
    }
    
    static void DrawAisu(m501::CAisu::DrawData &drawData, m501::CAisu *aisu);
    

    
public:
    static void Init(CHandleMan *handleMan);
    static CAisu *GetPtr(CHandle handle);
    static void Close();
    static void UpdateAll();
    static void AisuUpdate(CAisu *aisu);
    static void DrawHook(HU3DMODEL *modelP, Mtx *mtx);

    
private:
    void setObjName(int id);
    void proc_init();
    void proc_fall();
    void proc_wait();
    void playSound(int seId);
    void proc_sink();
    void proc_take();
    void setProc(AisuProc proc) {
        m_proc = proc;
    }
    
private:
    AisuProc m_proc; //0x8
    int m_type; //0x14
    //0x18
    bool m_drawShadow : 1;
    bool m_takeStart : 1;
    float m_scale; //0x1C
    float m_shadowScale; //0x20
public:
    HuVecF m_pos; //0x24
private:
    HuVecF m_vel; //0x30
    char m_objName[16]; //0x3C
    int m_seNo; //0x4C
    m501::CJoint m_joint; //0x50
    CHandle m_handle; //0xC4
};

};

#endif