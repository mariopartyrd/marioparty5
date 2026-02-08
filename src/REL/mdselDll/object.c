#include "REL/mdselDll.h"
#include "stdarg.h"
#include "string.h"

static float stdCurveSample[MDMATH_CURVE_STD_MAX][MDMATH_CURVE_SAMPLE_MAX] = {
    { 0.0f, 0.1f, 0.4f, 0.2f, 1.0f },
    { 0.0f, 0.5f, 1.0f },
};

static int stdCurveSampleNum[MDMATH_CURVE_STD_MAX];
static float stdCurveTime[MDMATH_CURVE_STD_MAX][MDMATH_CURVE_SAMPLE_MAX];
static float stdCurveSlope[MDMATH_CURVE_STD_MAX][MDMATH_CURVE_SAMPLE_MAX];

void MdObjDebugFontCreate(OMOBJ *obj)
{
    MDDEBUGFONT_WORK *debugFont = obj->data;
    MdDebugFontCreate(debugFont);
    obj->objFunc = MdObjDebugFontMain;
}

void MdObjDebugFontMain(OMOBJ *obj)
{
    MDDEBUGFONT_WORK *debugFont = obj->data;
    static char *ctrlStr[MD_DEBUGFONT_CTRL_MAX] = {
        "nop",
        "camera",
        "light",
        "model"
    };
    MdDebugFontUpdate(debugFont);
    
    if(HuPadBtnDown[0] & PAD_BUTTON_LEFT) {
        if(--debugFont->ctrl == -1) {
            debugFont->ctrl = MD_DEBUGFONT_CTRL_MAX-1;
        }
    }
    if(HuPadBtnDown[0] & PAD_BUTTON_RIGHT) {
        debugFont->ctrl = ++debugFont->ctrl%MD_DEBUGFONT_CTRL_MAX;
    }
    MdDebugFontPrint(debugFont, "<- %s ->", ctrlStr[debugFont->ctrl]);
    MdDebugFontPrint(debugFont, "X : FONT COLOR");
    MdDebugFontPrint(debugFont, "Y : FONT ONOFF");
}

void MdObjPadMain(OMOBJ *obj)
{
    MDPAD_WORK *pad = obj->data;
    int i;
    for(i=0; i<4; i++) {
        pad->DStkDown[i] = HuPadDStk[i] & ~pad->DStk[i];
        pad->DStk[i] = HuPadDStk[i];
    }
}

void MdObjCameraCreate(OMOBJ *obj)
{
    MDCAMERA_WORK *camera = obj->data;
    MdCameraCreate(camera);
    camera->createF = TRUE;
    obj->objFunc = MdObjCameraMain;
}

void MdObjCameraMain(OMOBJ *obj)
{
    MDCAMERA_WORK *camera = obj->data;
    MdCameraUpdate(camera);
}

void MdObjLightCreate(OMOBJ *obj)
{
    MDLIGHT_WORK *light = obj->data;
    MdLightCreate(light);
    light->createF = TRUE;
    obj->objFunc = MdObjLightMain;
}

void MdObjLightMain(OMOBJ *obj)
{
    MDLIGHT_WORK *light = obj->data;
    MdLightUpdate(light);
}

void MdDebugFontCreate(MDDEBUGFONT_WORK *debugFont)
{
    debugFont->createF = TRUE;
    debugFont->ctrl = MD_DEBUGFONT_CTRL_CAMERA;
    debugFont->printF = FALSE;
    debugFont->color = FONT_COLOR_YELLOW;
    debugFont->posX = 16;
    debugFont->posY = 40;
    debugFont->ofsY = 216;
    debugFont->rowOfsY = 0;
    debugFont->rowH = 15;
    debugFont->size = 1.5f;
    debugFont->maxLen = 0;
}

void MdDebugFontUpdate(MDDEBUGFONT_WORK *debugFont)
{
    debugFont->maxLen = 0;
    debugFont->rowOfsY = debugFont->ofsY;
}

void MdDebugFontPrint(MDDEBUGFONT_WORK *debugFont, const char *str, ...)
{
    size_t len = 0;
    char outBuf[160];
    va_list arg;
    if(debugFont->printF) {
        
        
        va_start(arg, str);
        vsprintf(outBuf, str, arg);
        len = strlen(outBuf);
        if(debugFont->maxLen < len) {
            debugFont->maxLen = len;
        }
        fontcolor = debugFont->color;
        print8(debugFont->posX, debugFont->posY+debugFont->rowOfsY, debugFont->size, "%s", outBuf);
        va_end(arg);
        debugFont->rowOfsY += debugFont->rowH;
    }
}

void MdCameraCreate(MDCAMERA_WORK *cameraWork)
{
    MDCAMERA_TYPE_INFO type[MDCAMERA_MAX] = {
        { HU3D_CAM0, MDCAMERA_FOCUS_TARGET },
        { HU3D_CAM1, MDCAMERA_FOCUS_TARGET },
    };
    MDCAMERA_RECT rect[MDCAMERA_MAX] = {
        { 0, 0, 640, 480 },
        { 0, 0, 640, 480 },
    };
    MDCAMERA_PARAM param[MDCAMERA_MAX] = {
        {
            { 90, 14, 1620 },
            { 0, -120, 0 },
        },
        {
            { 90, 0, 2500 },
            { 0, 0, 0 },
        },
    };
    float aspect[MDCAMERA_MAX];
    
    MDCAMERA *camera;
    int i;
    
    cameraWork->selCamera = 0;
    for(i=0; i<MDCAMERA_MAX; i++) {
        camera = &cameraWork->camera[i];
        camera->bit = type[i].bit;
        camera->focusType = type[i].focusType;
        Hu3DCameraCreate(camera->bit);
        camera->pos.x = 0;
        camera->pos.y = 0;
        camera->pos.z = 0;
        camera->target.x = 0;
        camera->target.y = 0;
        camera->target.z = 0;
        camera->hvr.x = param[i].hvr.x;
        camera->hvr.y = param[i].hvr.y;
        camera->hvr.z = param[i].hvr.z;
        camera->unk50 = 0;
        camera->center.x = param[i].center.x;
        camera->center.y = param[i].center.y;
        camera->center.z = param[i].center.z;
        camera->fovy = 30;
        aspect[i] = ((rect[i].w*HU_DISP_WIDTH)/640)/((rect[i].h*480)/480);
        Hu3DCameraPerspectiveSet(camera->bit, camera->fovy, 20, 20000, aspect[i]);
        Hu3DCameraViewportSet(camera->bit, rect[i].x, rect[i].y, rect[i].w, rect[i].h, 0, 1);
        Hu3DCameraScissorSet(camera->bit, rect[i].x, rect[i].y, rect[i].w, rect[i].h);
    }
    MdCameraUpdate(cameraWork);
}

void MdCameraDebug(MDCAMERA_WORK *cameraWork)
{
    MDCAMERA *camera;
    if(HuPadBtnDown[0] & PAD_TRIGGER_R) {
        cameraWork->selCamera = ++cameraWork->selCamera%MDCAMERA_MAX;
    }
    camera = &cameraWork->camera[cameraWork->selCamera];
    if(HuPadBtnDown[0] & PAD_BUTTON_DOWN) {
        camera->focusType = ++camera->focusType%MDCAMERA_FOCUS_MAX;
    }
    if(HuPadTrigL[0] <= 75) {
        if(HuPadSubStkX[0] <= -29) {
            camera->hvr.x += 1;
        } else if(HuPadSubStkX[0] >= 29) {
            camera->hvr.x -= 1;
        }
        if(HuPadSubStkY[0] <= -29) {
            camera->hvr.y -= 1;
        } else if(HuPadSubStkY[0] >= 29) {
            camera->hvr.y += 1;
        }
    } else if(HuPadBtn[0] & PAD_TRIGGER_L) {
        if(HuPadStkY[0] <= -36) {
            camera->center.y -= 50;
        } else if(HuPadStkY[0] >= 36) {
            camera->center.y += 50;
        }
        if(HuPadSubStkY[0] <= -29) {
            camera->hvr.z += 50;
        } else if(HuPadSubStkY[0] >= 29) {
            camera->hvr.z -= 50;
        }
    } else {
        if(HuPadStkY[0] <= -36) {
            camera->center.y -= 10;
        } else if(HuPadStkY[0] >= 36) {
            camera->center.y += 10;
        }
        if(HuPadSubStkY[0] <= -29) {
            camera->hvr.z += 10;
        } else if(HuPadSubStkY[0] >= 29) {
            camera->hvr.z -= 10;
        }
    }
}

void MdCameraUpdate(MDCAMERA_WORK *cameraWork)
{
    int i;
    static HuVecF up = { 0, 1, 0 };
    static HuVecF forward = { 0, 0, 1 };
    
    for(i=0; i<MDCAMERA_MAX; i++) {
        MDCAMERA *camera = &cameraWork->camera[i];
        if(camera->focusType == MDCAMERA_FOCUS_TARGET) {
            camera->target.x = camera->center.x;
            camera->target.y = camera->center.y;
            camera->target.z = camera->center.z;
            camera->pos.x = camera->target.x+(camera->hvr.z*HuCos(camera->hvr.y)*HuCos(camera->hvr.x));
            camera->pos.y = camera->target.y+(camera->hvr.z*HuSin(camera->hvr.y));
            camera->pos.z = camera->target.z+(camera->hvr.z*HuCos(camera->hvr.y)*HuSin(camera->hvr.x));
        } else if(camera->focusType == MDCAMERA_FOCUS_POS) {
            camera->pos.x = camera->center.x;
            camera->pos.y = camera->center.y;
            camera->pos.z = camera->center.z;
            camera->target.x = camera->pos.x+(camera->hvr.z*HuCos(camera->hvr.y)*HuCos(camera->hvr.x));
            camera->target.y = camera->pos.y+(camera->hvr.z*HuSin(camera->hvr.y));
            camera->target.z = camera->pos.z+(camera->hvr.z*HuCos(camera->hvr.y)*HuSin(camera->hvr.x));
        }
        VECSubtract(&camera->target, &camera->pos, &camera->dir);
        VECNormalize(&camera->dir, &camera->dir);
        VECCrossProduct(&up, &camera->dir, &camera->cross);
        VECNormalize(&camera->cross, &camera->cross);
        VECCrossProduct(&camera->dir, &camera->cross, &camera->up);
        VECNormalize(&camera->up, &camera->up);
        Hu3DCameraPosSet(camera->bit,
            camera->pos.x, camera->pos.y, camera->pos.z,
            camera->up.x, camera->up.y, camera->up.z,
            camera->target.x, camera->target.y, camera->target.z);
    }
}

void MdCameraPrint(MDCAMERA_WORK *cameraWork)
{
    MDCAMERA *camera = &cameraWork->camera[cameraWork->selCamera];
    MdDebugFontPrint(cameraWork->debugFont, "camera(HU3D_CAM%d -> TYPE%d))", cameraWork->selCamera, camera->focusType);
    MdDebugFontPrint(cameraWork->debugFont, "   hvr(%.1f / %.1f / %.1f / %.1f)", camera->hvr.x, camera->hvr.y, camera->hvr.z);
    MdDebugFontPrint(cameraWork->debugFont, "   pos(%.1f / %.1f / %.1f)", camera->pos.x, camera->pos.y, camera->pos.z);
    MdDebugFontPrint(cameraWork->debugFont, "target(%.1f / %.1f / %.1f)", camera->target.x, camera->target.y, camera->target.z);
    MdDebugFontPrint(cameraWork->debugFont, "  fovy(%.1f)", camera->fovy);
}

void MdCameraPosSet(MDCAMERA *camera, float h, float v, float r, float cx, float cy, float cz)
{
    camera->hvr.x = h;
    camera->hvr.y = v;
    camera->hvr.z = r;
    camera->center.x = cx;
    camera->center.y = cy;
    camera->center.z = cz;
}

void MdLightCreate(MDLIGHT_WORK *lightWork)
{
    MDLIGHT *light;
    Hu3DLighInit();
    lightWork->lightSel = 0;
    light = &lightWork->light[0];
    light->type = MDLIGHT_TYPE_G_DIRECT;
    light->hvr.x = 45;
    light->hvr.y = -45;
    light->hvr.z = 1000;
    light->center.x = 0;
    light->center.y = 0;
    light->center.z = 0;
    light->rgb.r = 255;
    light->rgb.g = 224;
    light->rgb.b = 144;
    light->rgb.a = 255;
    light->pos.x = 0;
    light->pos.y = 0;
    light->pos.z = 0;
    light->target.x = 0;
    light->target.y = 0;
    light->target.z = 0;
    light->lightDir.x = 0;
    light->lightDir.y = 0;
    light->lightDir.z = 1;
    light->lightPos = light->pos;
    VECSubtract(&light->target, &light->pos, &light->lightDir);
    light->gLightId = Hu3DGLightCreateV(&light->lightPos, &light->lightDir, &light->rgb);
    MdLightUpdateData(light);
    Hu3DGLightStaticSet(light->gLightId, TRUE);
    Hu3DGLightInfinitytSet(light->gLightId);
}

void MdLightUpdate(MDLIGHT_WORK *lightWork)
{
    int i;
    for(i=0; i<MDLIGHT_MAX; i++) {
        MDLIGHT *light = &lightWork->light[i];
        MdLightUpdateData(light);
    }
}

void MdLightUpdateData(MDLIGHT *light)
{
    if(light->type == MDLIGHT_TYPE_NOP) {
        return;
    }
    if(light->type == MDLIGHT_TYPE_G_DIRECT || light->type == MDLIGHT_TYPE_L_DIRECT) {
        light->target.x = light->center.x;
        light->target.y = light->center.y;
        light->target.z = light->center.z;
        light->pos.x = light->target.x+(light->hvr.z*HuCos(light->hvr.y)*HuCos(light->hvr.x));
        light->pos.y = light->target.y+(light->hvr.z*HuSin(light->hvr.y));
        light->pos.z = light->target.z+(light->hvr.z*HuCos(light->hvr.y)*HuSin(light->hvr.x));
        light->lightPos = light->pos;
        VECSubtract(&light->target, &light->pos, &light->lightDir);
        VECNormalize(&light->lightDir, &light->lightDir);
        if(light->type == MDLIGHT_TYPE_G_DIRECT) {
            Hu3DGLightPosSetV(light->gLightId, &light->lightPos, &light->lightDir);
            Hu3DGLightColorSet(light->gLightId, light->rgb.r, light->rgb.g, light->rgb.b, light->rgb.a);
        } else if(light->type == MDLIGHT_TYPE_L_DIRECT) {
            Hu3DLLightPosSetV(light->lLightId, light->gLightId, &light->lightPos, &light->lightDir);
            Hu3DLLightColorSet(light->lLightId, light->gLightId, light->rgb.r, light->rgb.g, light->rgb.b, light->rgb.a);
        }
    } else if(light->type == MDLIGHT_TYPE_G_POINT || light->type == MDLIGHT_TYPE_L_POINT) {
        light->target.x = light->center.x;
        light->target.y = light->center.y;
        light->target.z = light->center.z;
        light->pos.x = light->target.x+(light->hvr.z*HuCos(light->hvr.y)*HuCos(light->hvr.x));
        light->pos.y = light->target.y+(light->hvr.z*HuSin(light->hvr.y));
        light->pos.z = light->target.z+(light->hvr.z*HuCos(light->hvr.y)*HuSin(light->hvr.x));
        light->lightPos = light->target;
        VECSubtract(&light->pos, &light->target, &light->lightDir);
        VECNormalize(&light->lightDir, &light->lightDir);
        if(light->type == MDLIGHT_TYPE_G_POINT) {
            Hu3DGLightPosSetV(light->gLightId, &light->lightPos, &light->lightDir);
            Hu3DGLightColorSet(light->gLightId, light->rgb.r, light->rgb.g, light->rgb.b, light->rgb.a);
        } else if(light->type == MDLIGHT_TYPE_L_POINT) {
            Hu3DLLightPosSetV(light->lLightId, light->gLightId, &light->lightPos, &light->lightDir);
            Hu3DLLightColorSet(light->lLightId, light->gLightId, light->rgb.r, light->rgb.g, light->rgb.b, light->rgb.a);
            Hu3DLLightPointSet(light->lLightId, light->gLightId, light->hvr.z, 0.1f, GX_DA_GENTLE);
        }
    } else if(light->type == MDLIGHT_TYPE_G_SPOT || light->type == MDLIGHT_TYPE_L_SPOT) {
        light->lightPos = light->pos;
        VECSubtract(&light->target, &light->pos, &light->lightDir);
        VECNormalize(&light->lightDir, &light->lightDir);
        if(light->type == MDLIGHT_TYPE_G_SPOT) {
            Hu3DGLightPosSetV(light->gLightId, &light->lightPos, &light->lightDir);
            Hu3DGLightColorSet(light->gLightId, light->rgb.r, light->rgb.g, light->rgb.b, light->rgb.a);
        } else if(light->type == MDLIGHT_TYPE_L_SPOT) {
            Hu3DLLightPosSetV(light->lLightId, light->gLightId, &light->lightPos, &light->lightDir);
            Hu3DLLightColorSet(light->lLightId, light->gLightId, light->rgb.r, light->rgb.g, light->rgb.b, light->rgb.a);
        }
    }
}

void MdLightDebug(MDLIGHT_WORK *lightWork)
{
    MDLIGHT *light;
    if(HuPadBtnDown[0] & PAD_TRIGGER_R) {
        lightWork->lightSel = ++lightWork->lightSel%MDLIGHT_MAX;
    }
    light = &lightWork->light[lightWork->lightSel];
    if(light->type == MDLIGHT_TYPE_G_DIRECT || light->type == MDLIGHT_TYPE_L_DIRECT) {
        if(HuPadTrigL[0] <= 75) {
            if(HuPadSubStkX[0] <= -29) {
                light->hvr.x += 1;
            } else if(HuPadSubStkX[0] >= 29) {
                light->hvr.x -= 1;
            }
            if(HuPadSubStkY[0] <= -29) {
                light->hvr.y -= 1;
            } else if(HuPadSubStkY[0] >= 29) {
                light->hvr.y += 1;
            }
        } else {
            if(HuPadSubStkY[0] <= -29) {
                light->hvr.z -= 5;
            } else if(HuPadSubStkY[0] >= 29) {
                light->hvr.z += 5;
            }
        }
    } else if(light->type == MDLIGHT_TYPE_G_POINT || light->type == MDLIGHT_TYPE_L_POINT) {
        if(HuPadTrigL[0] <= 75) {
            if(HuPadSubStkX[0] <= -29) {
                light->center.x += 10;
            } else if(HuPadSubStkX[0] >= 29) {
                light->center.x -= 10;
            }
            if(HuPadSubStkY[0] <= -29) {
                light->center.z -= 10;
            } else if(HuPadSubStkY[0] >= 29) {
                light->center.z += 10;
            }
        } else {
            if(HuPadSubStkY[0] <= -29) {
                light->center.y -= 5;
            } else if(HuPadSubStkY[0] >= 29) {
                light->center.y += 5;
            }
            if(HuPadSubStkX[0] <= -29) {
                light->hvr.z -= 5;
            } else if(HuPadSubStkX[0] >= 29) {
                light->hvr.z += 5;
            }
        }
    } else if(light->type == MDLIGHT_TYPE_G_SPOT || light->type == MDLIGHT_TYPE_L_SPOT) {
        if(HuPadTrigL[0] <= 75) {
            if(HuPadSubStkX[0] <= -29) {
                light->pos.x -= 5;
            } else if(HuPadSubStkX[0] >= 29) {
                light->pos.x += 5;
            }
            if(HuPadSubStkY[0] <= -29) {
                light->pos.z -= 5;
            } else if(HuPadSubStkY[0] >= 29) {
                light->pos.z += 5;
            }
            if(HuPadStkY[0] <= -36) {
                light->pos.y -= 5;
            } else if(HuPadStkY[0] >= 36) {
                light->pos.y += 5;
            }
        } else {
            if(HuPadSubStkX[0] <= -29) {
                light->target.x -= 5;
            } else if(HuPadSubStkX[0] >= 29) {
                light->target.x += 5;
            }
            if(HuPadSubStkY[0] <= -29) {
                light->target.z -= 5;
            } else if(HuPadSubStkY[0] >= 29) {
                light->target.z += 5;
            }
            if(HuPadStkY[0] <= -36) {
                light->target.y -= 5;
            } else if(HuPadStkY[0] >= 36) {
                light->target.y += 5;
            }
        }
    }
    
    
}

void MdLightPrint(MDLIGHT_WORK *lightWork)
{
    MDLIGHT *light = &lightWork->light[lightWork->lightSel];
    static char *lightType[MDLIGHT_TYPE_MAX] = {
        "NOP",
        "G_DIRECT",
        "L_DIRECT",
        "G_POINT",
        "L_POINT",
        "G_SPOT",
        "L_SPOT"
    };
    MdDebugFontPrint(lightWork->debugFont, "light(%d)->%s)", lightWork->lightSel, lightType[light->type]);
    MdDebugFontPrint(lightWork->debugFont, "   hvr(%.1f / %.1f / %.1f)", light->hvr.x, light->hvr.y, light->hvr.z);
    MdDebugFontPrint(lightWork->debugFont, "   pos(%.1f / %.1f / %.1f)", light->pos.x, light->pos.y, light->pos.z);
    MdDebugFontPrint(lightWork->debugFont, "target(%.1f / %.1f / %.1f)", light->target.x, light->target.y, light->target.z);
    MdDebugFontPrint(lightWork->debugFont, "rgb(%d / %d / %d)", light->rgb.r, light->rgb.g, light->rgb.b);
}

Mtx *MdMathMatrixDir(HuVecF *start, HuVecF *end)
{
    static Mtx out;
    static HuVecF up = { 0, 1, 0 };
    HuVecF cross;
    HuVecF cross2;
    HuVecF dir;
    HuVecF a;
    HuVecF b;
    
    b = *end;
    a = *start;
    VECSubtract(&b, &a, &dir);
    VECNormalize(&dir, &dir);
    VECCrossProduct(&up, &dir, &cross);
    VECNormalize(&cross, &cross);
    VECCrossProduct(&dir, &cross, &cross2);
    VECNormalize(&cross2, &cross2);
    out[0][1] = cross2.x;
    out[1][1] = cross2.y;
    out[2][1] = cross2.z;
    out[0][0] = cross.x;
    out[1][0] = cross.y;
    out[2][0] = cross.z;
    out[0][2] = dir.x;
    out[1][2] = dir.y;
    out[2][2] = dir.z;
    return &out;
}

void MdMathMatrixRot(Mtx *matrix, HuVecF *rot)
{
    Mtx rotX, rotY, rotZ;
    MTXRotDeg(rotX, 'X', rot->x);
    MTXRotDeg(rotY, 'Y', rot->y);
    MTXRotDeg(rotZ, 'Z', rot->z);
    MTXConcat(rotZ, rotX, *matrix);
    MTXConcat(*matrix, rotY, *matrix);
}

void MdMathClamp(float *val, float min, float max)
{
    if(*val <= min) {
        *val = min;
    }
    if(*val >= max) {
        *val = max;
    }
}

void MdMathAngleClamp(float *angle)
{
    if(*angle > 180.0f) {
        *angle = -180.0f+(*angle-180.0f);
    } else if(*angle < -180.0f) {
        *angle = 180.0f+(*angle+180.0f);
    }
}

void MdMathOscillate(int type, float *in, float *out, float t)
{
    *in += 1/t;
    if(*in <= 0) {
        *in = 0;
    }
    if(*in >= 1) {
        *in = 1;
    }
    switch(type) {
        case MDMATH_OSC_0:
            *out = *in;
            break;
        
        case MDMATH_OSC_90:
            *out = HuSin((*in)*90);
            break;
        
        case MDMATH_OSC_90_REV:
            *out = 1-HuSin(90-((*in)*90));
            break;
        
        case MDMATH_OSC_180:
            *out = HuSin((*in)*180);
            break;
        
        case MDMATH_OSC_360:
            *out = HuSin((*in)*360);
            break;
        
        case MDMATH_OSC_540:
            *out = HuSin((*in)*540);
            break;
        
        case MDMATH_OSC_720:
            *out = HuSin((*in)*720);
            break;
        
    }
}

static float StdCurveGet(int type, float t);
static void CurveInit(int sampleNum, float *sampleT, float *sample, float *sampleSlope);
static float CurveGet(int sampleNum, float t, float *sampleT, float *sample, float *sampleSlope);
static void StdCurveInit(int idx);

void MdMathStdCurveGet(int type, float *in, float *out, float t)
{
    *in += 1/t;
    if(*in <= 0) {
        *in = 0;
    }
    if(*in >= 1) {
        *in = 1;
    }
    *out = StdCurveGet(type, *in);
}

void MdMathLerp(float *out, float a, float b, float t)
{
    *out = a+(t*(b-a));
}

void MdMathLerpScale(HuVecF *out, float a, float b, float t)
{
    float result = a+(t*(b-a));
    out->x = result;
    out->y = result;
    out->z = result;
}

void MdMathLerpColor(u8 *out, float a, float b, float t)
{
    *out = a+(t*(b-a));
}

void MdMathLerpOfs(float *out, float a, float b, float t, float ofs)
{
    *out = a+((b-a)*(t+ofs));
}

void MdMathLerpScaleOfs(HuVecF *out, float a, float b, float t, float ofs)
{
    float result = a+((b-a)*(t+ofs));
    out->x = result;
    out->y = result;
    out->z = result;
}

void MdMathCurveInit(int sampleNum, float *sampleT, float *sample, float *sampleSlope)
{
    int i;
    sampleT[0] = 0;
    (sampleT-1)[sampleNum] = 1;
    for(i=1; i<sampleNum-1; i++) {
        sampleT[i] = (1.0f/(sampleNum-1))*i;
    }
    
    CurveInit(sampleNum, sampleT, sample, sampleSlope);
}

void MdMathCurveGet(float *out, int sampleNum, float *sampleT, float *sample, float *sampleSlope, float t)
{
    *out = CurveGet(sampleNum, t, sampleT, sample, sampleSlope);
}

void MdMathStdCurveInit(void)
{
    int i;
    int j;
    for(i=0; i<MDMATH_CURVE_STD_MAX; i++) {
        for(j=0; j<MDMATH_CURVE_SAMPLE_MAX; j++) {
            if(1.0f == stdCurveSample[i][j]) {
                stdCurveSampleNum[i] = j+1;
                break;
            }
        }
    }
    for(i=0; i<MDMATH_CURVE_STD_MAX; i++) {
        stdCurveTime[i][0] = 0;
        stdCurveTime[i][stdCurveSampleNum[i]-1] = 1;
        for(j=1; j<stdCurveSampleNum[i]-1; j++) {
            stdCurveTime[i][j] = (1.0f/(stdCurveSampleNum[i]-1))*j;
        }
    }
    for(i=0; i<MDMATH_CURVE_STD_MAX; i++) {
        StdCurveInit(i);
    }
}

static void CurveInit(int sampleNum, float *sampleT, float *sample, float *sampleSlope)
{
    int i;
    float step;
    static float dt[MDMATH_CURVE_SAMPLE_MAX+1];
    static float dt2[MDMATH_CURVE_SAMPLE_MAX+1];
    static float slope[MDMATH_CURVE_SAMPLE_MAX+1];
    sampleNum--;
    for(i=0; i<sampleNum; i++) {
        dt[i] = sampleT[i+1]-sampleT[i];
        slope[i] = (sample[i+1]-sample[i])/dt[i];
    }
    slope[sampleNum] = slope[0];
    for(i=1; i<sampleNum; i++) {
        dt2[i] = 2*(sampleT[i+1]-sampleT[i-1]);
    }
    dt2[sampleNum] = 2*(dt[0]+dt[sampleNum-1]);
    for(i=1; i<=sampleNum; i++) {
        sampleSlope[i] = slope[i]-slope[i-1];
    }
    slope[1] = dt[0];
    slope[sampleNum-1] = dt[sampleNum-1];
    slope[sampleNum] = dt2[sampleNum];
    for(i=2; i<sampleNum-1; i++) {
        slope[i] = 0;
    }
    for(i=1; i<sampleNum; i++) {
        step = dt[i]/dt2[i];
        sampleSlope[i+1] = sampleSlope[i+1]-(step*sampleSlope[i]);
        dt2[i+1] = dt2[i+1]-(step*dt[i]);
        slope[i+1] = slope[i+1]-(step*slope[i]);
        
    }
    slope[0] = slope[sampleNum];
    sampleSlope[0] = sampleSlope[sampleNum];
    for(i=sampleNum-2; i>=0; i--) {
        step = dt[i]/dt2[i+1];
        sampleSlope[i] = sampleSlope[i]-(step*sampleSlope[i+1]);
        slope[i] = slope[i]-(step*slope[i+1]);
    }
    step = sampleSlope[0]/slope[0];
    sampleSlope[0] = step;
    sampleSlope[sampleNum] = step;
    for(i=1; i<sampleNum; i++) {
        sampleSlope[i] = (sampleSlope[i]-(step*slope[i]))/dt2[i];
    }
}

static float CurveGet(int sampleNum, float t, float *sampleT, float *sample, float *sampleSlope)
{
    float dt;
    float sampleDt;
    float len;
    
    int i;
    int num;
    int mid;
    
    sampleNum--;
    len = sampleT[sampleNum]-sampleT[0];
    while(t > sampleT[sampleNum]) {
        t -= len;
    }
    while(t < sampleT[0]) {
        t += len;
    }
    for(i=0, num=sampleNum; i<num;) {
        mid = (i+num)/2;
        if(sampleT[mid] < t) {
            i = mid+1;
        } else {
            num = mid;
        }
    }
    if(i > 0) {
        i--;
    }
    sampleDt = sampleT[i+1]-sampleT[i];
    dt = t-sampleT[i];
    return sample[i]+(dt*((dt*((3*sampleSlope[i])+((dt*(sampleSlope[i+1]-sampleSlope[i]))/sampleDt)))+(((sample[i+1]-sample[i])/sampleDt)-(sampleDt*((2*sampleSlope[i])+sampleSlope[i+1])))));
}

static void StdCurveInit(int idx)
{
    int i;
    float *point;
    int pointNum;
    float *time;
    float *samplePoint;
    float step;
    
    static float dt[MDMATH_CURVE_SAMPLE_MAX+1];
    static float dt2[MDMATH_CURVE_SAMPLE_MAX+1];
    static float slope[MDMATH_CURVE_SAMPLE_MAX+1];
    
    pointNum = stdCurveSampleNum[idx]-1;
    time = &stdCurveTime[idx][0];
    samplePoint = &stdCurveSample[idx][0];
    point = &stdCurveSlope[idx][0];
    for(i=0; i<pointNum; i++) {
        dt[i] = time[i+1]-time[i];
        slope[i] = (samplePoint[i+1]-samplePoint[i])/dt[i];
    }
    slope[pointNum] = slope[0];
    for(i=1; i<pointNum; i++) {
        dt2[i] = 2*(time[i+1]-time[i-1]);
    }
    dt2[pointNum] = 2*(dt[0]+dt[pointNum-1]);
    for(i=1; i<=pointNum; i++) {
        point[i] = slope[i]-slope[i-1];
    }
    slope[1] = dt[0];
    slope[pointNum-1] = dt[pointNum-1];
    slope[pointNum] = dt2[pointNum];
    for(i=2; i<pointNum-1; i++) {
        slope[i] = 0;
    }
    for(i=1; i<pointNum; i++) {
        step = dt[i]/dt2[i];
        point[i+1] = point[i+1]-(step*point[i]);
        dt2[i+1] = dt2[i+1]-(step*dt[i]);
        slope[i+1] = slope[i+1]-(step*slope[i]);
        
    }
    slope[0] = slope[pointNum];
    point[0] = point[pointNum];
    for(i=pointNum-2; i>=0; i--) {
        step = dt[i]/dt2[i+1];
        point[i] = point[i]-(step*point[i+1]);
        slope[i] = slope[i]-(step*slope[i+1]);
    }
    step = point[0]/slope[0];
    point[0] = step;
    point[pointNum] = step;
    for(i=1; i<pointNum; i++) {
        point[i] = (point[i]-(step*slope[i]))/dt2[i];
    }
}

static float StdCurveGet(int type, float t)
{
    float dt;
    float sampleDt;
    float len;
    
    float *sampleT;
    float *sampleSlope;
    float *sample;
    int i;
    int num;
    int mid;
    
    int sampleNum;
    sampleNum = stdCurveSampleNum[type]-1;
    sampleT = &stdCurveTime[type][0];
    sample = &stdCurveSample[type][0];
    sampleSlope = &stdCurveSlope[type][0];
    len = sampleT[sampleNum]-sampleT[0];
    while(t > sampleT[sampleNum]) {
        t -= len;
    }
    while(t < sampleT[0]) {
        t += len;
    }
    for(i=0, num=sampleNum; i<num;) {
        mid = (i+num)/2;
        if(sampleT[mid] < t) {
            i = mid+1;
        } else {
            num = mid;
        }
    }
    if(i > 0) {
        i--;
    }
    sampleDt = sampleT[i+1]-sampleT[i];
    dt = t-sampleT[i];
    return sample[i]+(dt*((dt*((3*sampleSlope[i])+((dt*(sampleSlope[i+1]-sampleSlope[i]))/sampleDt)))+(((sample[i+1]-sample[i])/sampleDt)-(sampleDt*((2*sampleSlope[i])+sampleSlope[i+1])))));
}

#define MDMESS_WIN_MODE_PAUSE 0
#define MDMESS_WIN_MODE_OPEN 1
#define MDMESS_WIN_MODE_CLOSE 2
#define MDMESS_WIN_MODE_WAIT 3
#define MDMESS_WIN_MODE_WAIT_CHOICE 4
#define MDMESS_WIN_MODE_CHOICE 5


void MdMessCreate(MDMESS *mess)
{
    MDMESS_WIN *win;
    int i;
    HuWinInit(0);
    for(win=&mess->win[0], i=0; i<MDMESS_WIN_MAX; i++, win++) {
        win->mode = MDMESS_WIN_MODE_PAUSE;
        win->winId = HUWIN_NONE;
    }
}

void MdMessMain(MDMESS *mess, int winNo)
{
    MDMESS_WIN *win = &mess->win[winNo];
    s16 stat;
    
    switch(win->mode) {
        case MDMESS_WIN_MODE_PAUSE:
            break;

        case MDMESS_WIN_MODE_OPEN:
            HuWinExOpen(win->winId);
            win->mode = MDMESS_WIN_MODE_PAUSE;
            break;
        
        case MDMESS_WIN_MODE_CLOSE:
            HuWinExClose(win->winId);
            HuWinExKill(win->winId);
            win->winId = HUWIN_NONE;
            win->mode = MDMESS_WIN_MODE_PAUSE;
            break;
       
        case MDMESS_WIN_MODE_WAIT:
        case MDMESS_WIN_MODE_WAIT_CHOICE:
            stat = HuWinStatGet(win->winId);
            if(stat == HUWIN_STAT_NONE) {
                if(win->mode == MDMESS_WIN_MODE_WAIT) {
                    win->mode = MDMESS_WIN_MODE_PAUSE;
                } else if(win->mode == MDMESS_WIN_MODE_WAIT_CHOICE) {
                    win->mode = MDMESS_WIN_MODE_CHOICE;
                }
            }
            break;
        
        case MDMESS_WIN_MODE_CHOICE:
            win->choice = HuWinChoiceGet(win->winId, -1);
            win->mode = MDMESS_WIN_MODE_PAUSE;
            break;
    }
}

void MdMessKill(MDMESS *mess)
{
    HuWinAllKill();
}

void MdMessWinOpen(MDMESS *mess, int winNo, s16 type, s16 frame, s16 drawNo, s16 pri, float posX, float posY, s16 w, s16 h, float bgTPLvl)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId == HUWIN_NONE) {
        win->type = type;
        if(win->type == MDMESS_WIN_SPEAKER) {
            win->winId = HuWinExCreateFrame(posX, posY, w, h, HUWIN_SPEAKER_NONE, frame);
        } else if(win->type == MDMESS_WIN_NOSPEAKER) {
            win->winId = HuWinCreate(posX, posY, w, h, frame);
        }
        HuWinPosSet(win->winId, posX, posY);
        if(frame == 0) {
            HuWinMesPalSet(win->winId, 7, 255, 255, 255);
        } else if(frame == 1) {
            HuWinMesPalSet(win->winId, 7, 0, 0, 0);
        }
        HuWinBGTPLvlSet(win->winId, bgTPLvl);
        HuWinDrawNoSet(win->winId, drawNo);
        HuWinPriSet(win->winId, pri);
        if(win->type == MDMESS_WIN_NOSPEAKER) {
            HuWinDispOn(win->winId);
        }
        if(win->type == MDMESS_WIN_SPEAKER) {
            win->mode = MDMESS_WIN_MODE_OPEN;
        } else if(win->type == MDMESS_WIN_NOSPEAKER) {
            win->mode = MDMESS_WIN_MODE_PAUSE;
        }
    }
}

void MdMessWinClose(MDMESS *mess, int winNo)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId != HUWIN_NONE) {
        HuWinHomeClear(win->winId);
        if(win->type == MDMESS_WIN_SPEAKER) {
            win->mode = MDMESS_WIN_MODE_CLOSE;
        } else if(win->type == MDMESS_WIN_NOSPEAKER) {
            HuWinDispOff(win->winId);
            HuWinKill(win->winId);
            win->winId = HUWIN_NONE;
            win->mode = MDMESS_WIN_MODE_PAUSE;
        }
    }
}

void MdMessWinKill(MDMESS *mess, int winNo)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId != HUWIN_NONE) {
        HuWinHomeClear(win->winId);
        if(win->type == MDMESS_WIN_SPEAKER) {
            HuWinExKill(win->winId);
        } else if(win->type == MDMESS_WIN_NOSPEAKER) {
            HuWinDispOff(win->winId);
            HuWinKill(win->winId);
        }
        win->winId = HUWIN_NONE;
        win->mode = MDMESS_WIN_MODE_PAUSE;
    }
}

void MdMessWinPosSet(MDMESS *mess, int winNo, float posX, float posY)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId != HUWIN_NONE) {
        HuWinPosSet(win->winId, posX, posY);
    }
}

void MdMessWinSpeedSet(MDMESS *mess, int winNo, s16 speed)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId != HUWIN_NONE) {
        HuWinMesSpeedSet(win->winId, speed);
    }
}

void MdMessWinPad1Set(MDMESS *mess, int winNo)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId != HUWIN_NONE) {
        HUWIN *w = &winData[win->winId];
        w->padMask = 1 << 0;
    }
}

void MdMessWinHomeClear(MDMESS *mess, int winNo)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId != HUWIN_NONE) {
        HuWinHomeClear(win->winId);
    }
}

void MdMessWinMesSet(MDMESS *mess, int winNo, BOOL choice, ...)
{
    int i;
    int num = 0;
    MDMESS_WIN *win = &mess->win[winNo];
    va_list list;
    static u32 mesTbl[HUWIN_INSERTMES_MAX];
    static s16 mesNoTbl[HUWIN_INSERTMES_MAX] = {
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7
    };
    
    if(win->winId != HUWIN_NONE) {
        HuWinHomeClear(win->winId);
        va_start(list, choice);
        for(i=0; i<HUWIN_INSERTMES_MAX; i++) {
            mesTbl[i] = va_arg(list, u32);
            if(mesTbl[i] == MDMESS_LIST_END) {
                break;
            }
            num++;
        }
        if(num > 1) {
            for(i=1; i<num; i++) {
                HuWinInsertMesSet(win->winId, mesTbl[i], mesNoTbl[i-1]);
            }
        }
        HuWinMesSet(win->winId, mesTbl[0]);
        if(choice == FALSE) {
            win->mode = MDMESS_WIN_MODE_WAIT;
        } else if(choice == TRUE) {
            win->mode = MDMESS_WIN_MODE_WAIT_CHOICE;
        }
        va_end(list);
    }
}

s16 MdMessWinChoiceNowGet(MDMESS *mess, int winNo)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId != HUWIN_NONE) {
        HUWIN *w = &winData[win->winId];
        win->choice = w->choice;
    }
}

void MdMessWinChoiceSet(MDMESS *mess, int winNo, s16 choice)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId != HUWIN_NONE) {
        HUWIN *w = &winData[win->winId];
        w->choice = win->choice = choice;
    }
}

int MdMessWinChoiceGet(MDMESS *mess, int winNo)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId == HUWIN_NONE) {
        return;
    }
    return win->choice;
}

s16 MdMessWinStatGet(MDMESS *mess, int winNo)
{
    MDMESS_WIN *win = &mess->win[winNo];
    s16 stat;
    if(win->winId == HUWIN_NONE) {
        return;
    }
    stat = HuWinStatGet(win->winId);
    return stat;
}

BOOL MdMessWinCheck(MDMESS *mess, int winNo)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->mode == MDMESS_WIN_MODE_PAUSE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void MdMessWinAttrSet(MDMESS *mess, int winNo, u32 attr)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId == HUWIN_NONE) {
        return;
    }
    HuWinAttrSet(win->winId, attr);
}

void MdMessWinAttrReset(MDMESS *mess, int winNo, u32 attr)
{
    MDMESS_WIN *win = &mess->win[winNo];
    if(win->winId == HUWIN_NONE) {
        return;
    }
    HuWinAttrReset(win->winId, attr);
}