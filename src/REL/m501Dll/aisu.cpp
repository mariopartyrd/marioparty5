#include "REL/m501/aisu.h"
#include "REL/m501/stage.h"
#include "REL/m501/particle.h"
#include "REL/m501/game_master.h"
#include "REL/mg_cpp/random.h"

#include "game/audio.h"
#include <stdio.h>

static CTickTimer lbl_1_bss_70(120);
static CTickTimer winnerMaxTime(210);
static CTickTimer lbl_1_bss_78(210);

CLinkList<m501::CAisu> g_aisuConeList;

CLinkList<m501::CAisu> g_aisuAllList;
CLinkList<m501::CAisu> g_aisuFallList;

CHandleMan *g_aisuHandleMan;

void m501::CAisu::Init(CHandleMan *handleMan)
{
    g_aisuHandleMan = handleMan;
    HU3DMODELID mdlId = Hu3DHookFuncCreate(DrawHook);
    Hu3DModelLayerSet(mdlId, 0);
    Hu3DModelCameraSet(mdlId, HU3D_CAM0);
}

m501::CAisu *m501::CAisu::GetPtr(CHandle handle)
{
    return (m501::CAisu *)g_aisuHandleMan->getPtr(handle);
}

void m501::CAisu::Close()
{
    while(!g_aisuConeList.IsEmpty()) {
        delete g_aisuConeList.Last();
    }
    while(!g_aisuAllList.IsEmpty()) {
        delete g_aisuAllList.Last();
    }
    while(!g_aisuFallList.IsEmpty()) {
        delete g_aisuFallList.Last();
    }
}

m501::CAisu::CAisu(int type, HuVecF *pos)
    : m_type(type),
    m_pos(*pos),
    m_joint()
{
    g_aisuAllList.InsertBack(this);
    setProc(proc_init);
    m_vel.x = 0;
    m_vel.y = 0;
    m_vel.z = 0;
    m_scale = 1;
    setObjName(0);
    m_handle = g_aisuHandleMan->createHandle(this);
    m_seNo = HuAudFXPlay(0x624);
}

m501::CAisu::~CAisu()
{
    g_aisuHandleMan->deleteHandle(m_handle);
}

static void execAisuList(CLinkList<m501::CAisu> &list, void (*func)(m501::CAisu *aisu));

void m501::CAisu::UpdateAll()
{
    execAisuList(g_aisuAllList, AisuUpdate);
    execAisuList(g_aisuFallList, AisuUpdate);
    execAisuList(g_aisuConeList, AisuUpdate);
}

void m501::CAisu::AisuUpdate(CAisu *aisu)
{
    aisu->callProc();
    if(!aisu->isTake()) {
        aisu->updateMatrix();
        
    }
}

void m501::CAisu::setObjName(int id)
{
    static char *objType[8] = {
        "str",
        "mav",
        "mint",
        "choco",
        "melon",
        "lemon",
        "ore",
        "bani",
    };
    static char *objSuffix[3] = {
        "_A",
        "_B",
        "_C"
    };
    sprintf(m_objName, "%s%s", objType[m_type], objSuffix[id]);
}

void m501::CAisu::proc_init()
{
    m_drawShadow = true;
    m_takeStart = false;
    setProc(proc_fall);
    m_shadowScale = 0;
}

void m501::CAisu::proc_fall()
{
    HuVecF vel = m_vel;
    m_vel.y -= 0.27240695529513886;
    VECAdd(&m_vel, &vel, &vel);
    VECScale(&vel, &vel, 0.5f);
    VECAdd(&m_pos, &vel, &m_pos);
    float t = 5060.02f/m_pos.y;
    float s = t*t;
    m_shadowScale = s;
    if(m_pos.y <= 5060.02f) {
        playSound(0x626);
        m_pos.y = 5060.02f;
        setObjName(1);
        setProc(proc_wait);
        m_drawShadow = false;
        removeLink();
        g_aisuFallList.InsertBack(this);
        m501::CStage::GetSingletonPtrRaw()->setAisuMotion();
        m501::CParticle::createFallParticle(&m_pos, m_type);
        g_aisuHandleMan->deleteHandle(m_handle);
        m_handle = g_aisuHandleMan->createHandle(this);
    }
}

void m501::CAisu::proc_wait()
{
    m_scale -= 1.0/240.0;
    if(m_scale < 0.05f) {
        m_scale = 0.05f;
        setProc(proc_sink);
    }
}

void m501::CAisu::playSound(int seId)
{
    HuAudFXStop(m_seNo);
    m_seNo = HuAudFXPlay(seId);
    float x = m_pos.x-1578.489f;
    float z = m_pos.z-414.244f;
    x = (x*cos(0.9296085834503174))-(z*sin(0.9296085834503174));
    x /= 454.20001220703125;
    int pan = (x*16)+MSM_PAN_CENTER;
    HuAudFXPanning(m_seNo, pan);
}

void m501::CAisu::proc_sink()
{
    m_pos.y -= 1.0/30.0;
    if(m_pos.y <= 5058.02f) {
        delete this;
        return;
    }
}

void m501::CAisu::proc_take()
{
    
}

static void drawAisuList(const CLinkList<m501::CAisu> &list, m501::CAisu::DrawData &drawData);

void m501::CAisu::DrawHook(HU3DMODEL *modelP, Mtx *mtx)
{
    m501::CAisu::DrawData drawData;
    Hu3DModelObjDrawInit();
    MTXCopy(*mtx, drawData.mtx);
    drawData.mdlId = m501::CGameMaster::GetAisuModel();
    drawAisuList(g_aisuAllList, drawData);
    drawAisuList(g_aisuFallList, drawData);
    drawAisuList(g_aisuConeList, drawData);
}

void m501::CAisu::DrawAisu(m501::CAisu::DrawData &drawData, m501::CAisu *aisu)
{
    aisu->Draw(drawData);
}

m501::CJoint *m501::CAisu::doTake(m501::CJoint *super, BOOL randomOfs)
{
    playSound(0x625);
    setObjName(2);
    setProc(proc_take);
    m_drawShadow = false;
    m_takeStart = true;
    m_joint.setSuper(super);
    m_joint.lockMatrix();
    Mtx *local = m_joint.getLocalMatrix();
    if(randomOfs) {
        (*local)[2][3] = 60.0f;
        (*local)[0][3] = CRandom::frandom(-10.0f, 10.0f);
        (*local)[1][3] = CRandom::frandom(-10.0f, 10.0f);
    } else {
        (*local)[2][3] = 0;
        (*local)[0][3] = 0;
        (*local)[1][3] = 0;
    }
    Mtx rot;
    MTXRotDeg(rot, 'X', -90);
    MTXConcat(*local, rot, *local);
    removeLink();
    g_aisuConeList.InsertBack(this);
    g_aisuHandleMan->deleteHandle(m_handle);
    m_handle = g_aisuHandleMan->createHandle(this);
    Mtx *jointMtx = m_joint.getMatrix();
    m_pos.x = 0;
    m_pos.y = 0;
    m_pos.z = 0;
    MTXMultVec(*jointMtx, &m_pos, &m_pos);
    m501::CParticle::createTakeParticle(&m_pos, m_type);
    return &m_joint;
}

bool m501::CAisu::checkSlowMove(HuVecF *pos)
{
    m501::CAisu *begin;
    m501::CAisu *end;
    for(begin=g_aisuFallList.Begin(), end=g_aisuFallList.End(); begin != end; begin=begin->next()) {
        double dx = pos->x-begin->m_pos.x;
        double dz = pos->z-begin->m_pos.z;
        double r2 = (dx*dx)+(dz*dz);
        if(r2 < 1600) {
            return true;
        }
    }
    return false;
}

m501::CAisu *m501::CAisu::getTakeAisu(HuVecF *pos, float r)
{
    m501::CAisu *begin;
    m501::CAisu *end;
    for(begin=g_aisuFallList.Begin(), end=g_aisuFallList.End(); begin != end; begin=begin->next()) {
        HuVecF delta;
        VECSubtract(pos, &begin->m_pos, &delta);
        float mag = VECMag(&delta);
        if(mag <= (r+40)) {
            return begin;
        }
    }
    return nullptr;
}

//Must be u8 return to prevent automatic conversion problems
u8 m501::CAisu::isTake()
{
    return m_takeStart;
}

void m501::CAisu::setPos(HuVecF *pos, float r)
{
    m_pos.x += (pos->x-m_pos.x)*r;
    m_pos.z += (pos->z-m_pos.z)*r;
}

static void drawAisuList(m501::CAisu *begin, m501::CAisu *end, m501::CAisu::DrawData &drawData);

static void drawAisuList(const CLinkList<m501::CAisu> &list, m501::CAisu::DrawData &drawData)
{
    drawAisuList(list.Begin(), list.End(), drawData);
}

static void drawAisuList(m501::CAisu *begin, m501::CAisu *end, m501::CAisu::DrawData &drawData)
{
    while(begin != end) {
        m501::CAisu::DrawAisu(drawData, begin);
        begin = begin->next();
    }
}

static void execAisuList(CLinkList<m501::CAisu> &list, void (*func)(m501::CAisu *aisu))
{
    if(list.IsEmpty()) {
        return;
    }
    CLinkList<m501::CAisu> temp;
    list.Copy(&temp);
    
    while(!temp.IsEmpty()) {
        m501::CAisu *aisu = temp.Begin();
        aisu->removeLink();
        list.InsertBack(aisu);
        func(aisu);
    }
}