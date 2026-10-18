#include "REL/mg_cpp/camera.h"
#include "game/pad.h"

void CCamera::rotate(float x, float y)
{
    Mtx rotRight, rotY, rot;
    HuVecF right, up, forward;
    getLookAt(right, up, forward);
    MTXRotAxisDeg(rotRight, &right, x);
    MTXRotDeg(rotY, 'Y', y);
    PSMTXConcat(rotRight, rotY, rot);
    MTXMultVecSR(rot, &forward, &forward);
    MTXMultVecSR(rot, &up, &m_up);
    VECSubtract(&m_pos, &forward, &m_target);
}

void CCamera::rotateUp(float y)
{
    Mtx rot;
    HuVecF dir;
    VECSubtract(&m_target, &m_pos, &dir);
    MTXRotAxisDeg(rot, &dir, y);
    MTXMultVec(rot, &m_up, &m_up);
    
}

void CCamera::getLookAt(HuVecF &right, HuVecF &up, HuVecF &forward)
{
    Mtx mtx;
    MTXLookAt(mtx, &m_pos, &m_up, &m_target);
    right.x = mtx[0][0];
    right.y = mtx[0][1];
    right.z = mtx[0][2];
    up.x = mtx[1][0];
    up.y = mtx[1][1];
    up.z = mtx[1][2];
    forward.x = mtx[2][0];
    forward.y = mtx[2][1];
    forward.z = mtx[2][2];
}

void CCamera::getLookAtMtxInverse(Mtx &m)
{
    MTXLookAt(m, &m_pos, &m_up, &m_target);
    MTXInverse(m, m);
}

void CCamera::applyOffsetLocal(HuVecF &ofs)
{
    Mtx m;
    HuVecF globalOfs;
    getLookAtMtxInverse(m);
    MTXMultVecSR(m, &ofs, &globalOfs);
    applyOffset(globalOfs);
}

void CCamera::applyOffset(HuVecF &ofs)
{
    VECAdd(&ofs, &m_target, &m_target);
    VECAdd(&ofs, &m_pos, &m_pos);
}

void CCamera::resetPos()
{
    m_up.x = 0;
    m_up.y = 1;
    m_up.z = 0;
    m_pos.x = 0;
    m_pos.y = 0;
    m_pos.z = 5000;
    m_target.x = 0;
    m_target.y = 0;
    m_target.z = 0;
}

void CCamera::setView(CCameraView &view)
{
    Mtx m;
    MTXQuat(m, &view.rot);
    m_up.x = m[0][1];
    m_up.y = m[1][1];
    m_up.z = m[2][1];
    m_pos.x = view.pos.x;
    m_pos.y = view.pos.y;
    m_pos.z = view.pos.z;
    m_target.x = view.pos.x-m[0][2];
    m_target.y = view.pos.y-m[1][2];
    m_target.z = view.pos.z-m[2][2];
    
}

void CCamera::getView(CCameraView &view)
{
    Mtx m;
    getLookAtMtxInverse(m);
    view.pos.x = m_pos.x;
    view.pos.y = m_pos.y;
    view.pos.z = m_pos.z;
    QUATMtx(&view.rot, m);
}

void CCamera::lerpView(CCameraView &to, CCameraView &from, float t)
{
    CCameraView temp;
    HuVecF a;
    HuVecF b;
    QUATSlerp(&to.rot, &from.rot, &temp.rot, t);
    VECScale(&to.pos, &a, 1-t);
    VECScale(&from.pos, &b, t);
    VECAdd(&a, &b, &temp.pos);
    setView(temp);
}

void CCamera::setPos(HuVecF &pos)
{
    HuVecF diff;
    VECSubtract(&pos, &m_pos, &diff);
    m_pos.x = pos.x;
    m_pos.y = pos.y;
    m_pos.z = pos.z;
    VECAdd(&m_target, &diff, &m_target);
    
}

void CCamera::lerpPos(HuVecF &to, HuVecF &from, float t)
{
    HuVecF a;
    HuVecF b;
    VECScale(&to, &a, 1-t);
    VECScale(&from, &b, t);
    VECAdd(&a, &b, &b);
    setPos(b);
}

void CCamera::writePos(u16 cameraBit)
{
    Hu3DCameraPosSetV(cameraBit, &m_pos, &m_up, &m_target);
}

void CCamera::readPos(u16 cameraBit)
{
    Hu3DCameraPosGet(cameraBit, &m_pos, &m_up, &m_target);
}

#define ASINT(x) (*((u32 *)&(x)))

void CCamera::printView()
{
    CCameraView view;
    getView(view);
    //TODO: Find a less hacky way to pass these to printf as int
    printf("{{0x%08x,0x%08x,0x%08x},{0x%08x,0x%08x,0x%08x,0x%08x}},\n",
        ASINT(view.pos.x), ASINT(view.pos.y), ASINT(view.pos.z),
        ASINT(view.rot.x), ASINT(view.rot.y), ASINT(view.rot.z), ASINT(view.rot.w));
}
#undef ASINT

float CCamera::GetStickX(int padNo)
{
    return HuPadStkX[padNo]*(1.0f/56.0f);
}

float CCamera::GetStickY(int padNo)
{
    return HuPadStkY[padNo]*(1.0f/56.0f);
}

float CCamera::GetSubStickX(int padNo)
{
    return HuPadSubStkX[padNo]*(1.0f/59.0f);
}

float CCamera::GetSubStickY(int padNo)
{
    return HuPadSubStkY[padNo]*(1.0f/59.0f);
}

void CCamera::debugView(int padNo)
{
    u32 btn = HuPadBtn[padNo];
    if(btn & PAD_BUTTON_Y) {
        m_up.x = 0;
        m_up.y = 1;
        m_up.z = 0;
    }
    float rx = -4*CCamera::GetStickX(padNo);
    float ry = -4*CCamera::GetStickY(padNo);
    rotate(ry, rx);
    bool fast = (u8)((btn & PAD_BUTTON_B) != 0);
    float speed = (fast) ? 70.0f : 20.0f;
    Mtx lookAtInv;
    getLookAtMtxInverse(lookAtInv);
    HuVecF ofs1;
    ofs1.x = (speed*((btn & PAD_BUTTON_RIGHT) != 0))-(speed*((btn & PAD_BUTTON_LEFT) != 0));
    ofs1.y = (speed*((btn & PAD_BUTTON_UP) != 0))-(speed*((btn & PAD_BUTTON_DOWN) != 0));
    ofs1.x += speed*GetSubStickX(padNo);
    ofs1.z = 0;
    MTXMultVecSR(lookAtInv, &ofs1, &ofs1);
    ofs1.y += speed*GetSubStickY(padNo);
    applyOffset(ofs1);
    HuVecF ofs2;
    ofs2.x = 0;
    ofs2.y = 0;
    ofs2.z = ((btn & PAD_BUTTON_X) != 0)-((btn & PAD_BUTTON_A) != 0);
    MTXMultVecSR(lookAtInv, &ofs2, &ofs2);
    ofs2.y = 0;
    float mag = VECMag(&ofs2);
    if(mag > 1e-10f) {
        VECScale(&ofs2, &ofs2, speed/mag);
        applyOffset(ofs2);
    }
    if(HuPadBtnDown[padNo] & PAD_BUTTON_B) {
        printView();
    }
    
}