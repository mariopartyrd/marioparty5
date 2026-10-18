#ifndef MGCPP_CAMERA_H
#define MGCPP_CAMERA_H

#include "game/hu3d.h"

struct CCameraView {
    HuVecF pos;
    Quaternion rot;
};

class CCamera {
public:
    void rotate(float x, float y);
    void rotateUp(float y);
    void getLookAt(HuVecF &right, HuVecF &up, HuVecF &forward);
    void getLookAtMtxInverse(Mtx &m);
    void applyOffsetLocal(HuVecF &ofs);
    void applyOffset(HuVecF &ofs);
    void resetPos();
    void setView(CCameraView &view);
    void getView(CCameraView &view);
    void lerpView(CCameraView &to, CCameraView &from, float t);
    void setPos(HuVecF &pos);
    void lerpPos(HuVecF &to, HuVecF &from, float t);
    void writePos(u16 cameraBit);
    void readPos(u16 cameraBit);
    void printView();
    void debugView(int padNo);
    
public:
    static float GetStickX(int padNo);
    static float GetStickY(int padNo);
    static float GetSubStickX(int padNo);
    static float GetSubStickY(int padNo);
private:
    HuVecF m_pos;
    HuVecF m_up;
    HuVecF m_target;
};

#endif