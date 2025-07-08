#include <math.h>
#include <humath.h>

typedef struct MathBBox_s {
    HuVec2F min;
    HuVec2F max;
} MathBBox;

typedef struct MathCircle_s {
    HuVec2F center;
    float radius;
} MathCircle;

typedef struct MathSphere_s {
    HuVecF center;
    float radius;
} MathSphere;

typedef struct MathPolygon_s {
    int pointNum;
    HuVec2F *point;
} MathPolygon;

static float MathDot2DGet(HuVec2F *a, HuVec2F *b)
{
    return (a->x*b->x)+(a->y*b->y);
}

static float MathCross2DGet(HuVec2F *a, HuVec2F *b)
{
    return (a->x*b->y)-(a->y*b->x);
}

static float MathVecMag2DGet(HuVec2F *a)
{
    return sqrt(HuSquare(a->x)+HuSquare(a->y));
}

static void MathVecNorm2D(HuVec2F *a, HuVec2F *b)
{
    float scale = 1/MathVecMag2DGet(b);
    a->x = b->x*scale;
    a->y = b->y*scale;
}

static BOOL MathCirclePointCheck(HuVec2F *point, MathCircle *circle)
{
    float dx = point->x-circle->center.x;
    float dy = point->y-circle->center.y;
    float r = circle->radius;
    return ((dx*dx)+(dy*dy) > (r*r)) ? FALSE : TRUE;
}

static BOOL MathCircleCircleCheck(MathCircle *a, MathCircle *b)
{
    float dx = a->center.x-b->center.x;
    float dy = a->center.y-b->center.y;
    float r = a->radius+b->radius;
    return ((dx*dx)+(dy*dy) < (r*r)) ? TRUE : FALSE;
}

static BOOL MathBBoxCheck(MathBBox *a, MathBBox *b)
{
    if(a->min.x > b->max.x) {
        return FALSE;
    }
    if(a->max.x < b->min.x) {
        return FALSE;
    }
    if(a->min.y > b->max.y) {
        return FALSE;
    }
    if(a->max.y < b->min.y) {
        return FALSE;
    }
    return TRUE;
}

static BOOL MathPointShapeCheck(HuVec2F *point, MathPolygon *poly)
{
    int outNum;
    int pointNum;
    int dir;
    int i;
    
    outNum = 0;
    dir = 0;
    pointNum = poly->pointNum-1;
    for(i=0; i<pointNum; i++) {
        HuVec2F *a = &poly->point[i];
        HuVec2F *b = &poly->point[i+1];
        dir = ((point->x-a->x)*(point->y-b->y))-((point->y-a->y)*(point->x-b->x));
        if(dir < 0) {
            outNum++;
        }
    }
    return (outNum == 0 || outNum == pointNum) ? TRUE : FALSE;
}

static BOOL MathBBoxCircleCheck(MathCircle *circle, MathBBox *bbox)
{
    static HuVec2F size;
    static HuVec2F centerOfs;
    static HuVec2F dir;
    static HuVec2F dirNorm;
    static HuVec2F offset;
    static HuVec2F result;
    static float cross;
    static float radius;
    
    if(MathCirclePointCheck(&bbox->min, circle)) {
        return TRUE;
    }
    if(MathCirclePointCheck(&bbox->max, circle)) {
        return TRUE;
    }
    size.x = bbox->max.x-bbox->min.x;
    size.y = bbox->max.y-bbox->min.y;
    centerOfs.x = circle->center.x-bbox->min.x;
    centerOfs.y = circle->center.y-bbox->min.y;
    cross = MathCross2DGet(&size, &centerOfs);
    if(cross == 0) {
        result = circle->center;
    } else {
        dir.x = size.y*cross;
        dir.y = -(size.x*cross);
        MathVecNorm2D(&dirNorm, &dir);
        offset.x = bbox->min.x-circle->center.x;
        offset.y = bbox->min.y-circle->center.y;
        radius = MathDot2DGet(&dirNorm, &offset);
        if(radius > circle->radius) {
            return FALSE;
        }
        result.x = circle->center.x+(dirNorm.x*radius);
        result.y = circle->center.y+(dirNorm.y*radius);
    }
    if(bbox->min.x < result.x && result.x < bbox->max.x) {
        return TRUE;
    }
    if(bbox->max.x < result.x && result.x < bbox->min.x) {
        return TRUE;
    }
    if(bbox->min.y < result.y && result.y < bbox->max.y) {
        return TRUE;
    }
    if(bbox->max.y < result.y && result.y < bbox->min.y) {
        return TRUE;
    }
    return FALSE;
}

static BOOL MathCircleShapeCheck(MathCircle *circle, MathPolygon *poly)
{
    int i;
    int max;
    if(MathPointShapeCheck(&circle->center, poly)) {
        return TRUE;
    }
    max = poly->pointNum-1;
    for(i=0; i<max; i++) {
        MathBBox bbox;
        bbox.min.x = poly->point[i].x;
        bbox.min.y = poly->point[i].y;
        bbox.max.x = poly->point[i+1].x;
        bbox.max.x = poly->point[i+1].y;
        if(MathBBoxCircleCheck(circle, &bbox)) {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL MathCircleRectCheck(MathBBox *bbox, MathCircle *circle)
{
    static HuVec2F point[5];
    static MathPolygon poly;
    
    point[0].x = bbox->min.x;
    point[0].y = bbox->min.y;
    point[1].x = bbox->max.x;
    point[1].y = bbox->min.y;
    point[2].x = bbox->max.x;
    point[2].y = bbox->max.y;
    point[3].x = bbox->min.x;
    point[3].y = bbox->max.y;
    point[4].x = bbox->min.x;
    point[4].y = bbox->min.y;
    poly.pointNum = 5;
    poly.point = point;
    return MathCircleShapeCheck(circle, &poly);
}

static BOOL MathSphereCheck(MathSphere *a, MathSphere *b)
{
    float dx = a->center.x-b->center.x;
    float dy = a->center.y-b->center.y;
    float dz = a->center.z-b->center.z;
    float r = a->radius+b->radius;
    return ((dx*dx)+(dy*dy)+(dz*dz) < (r*r)) ? TRUE : FALSE;
}

static BOOL MathCircleCollide(MathCircle *a, MathCircle *b)
{
    static float dx;
    static float dy;
    static float r;
    static float mag;
    dx = a->center.x-b->center.x;
    dy = a->center.y-b->center.y;
    r = a->radius+b->radius;
    mag = sqrtf((dx*dx)+(dy*dy));
    if(mag < r) {
        if(0.0f == mag) {
            dx = 0;
            dy = -1;
        } else {
            dx /= mag;
            dy /= mag;
        }
        a->center.x = b->center.x+(dx*r);
        a->center.y = b->center.y+(dy*r);
        return TRUE;
    }
    return FALSE;
}

static BOOL MathCircleRectCollide(MathCircle *circle, MathBBox *bbox)
{
    static HuVec2F size;
    static HuVec2F centerOfs;
    static HuVec2F dir;
    static HuVec2F dirNorm;
    static HuVec2F offset;
    static HuVec2F result;
    static float cross;
    static float radius;
    static MathCircle outCircle;
    
    size.x = bbox->max.x-bbox->min.x;
    size.y = bbox->max.y-bbox->min.y;
    centerOfs.x = circle->center.x-bbox->min.x;
    centerOfs.y = circle->center.y-bbox->min.y;
    cross = MathCross2DGet(&size, &centerOfs);
    if(cross == 0) {
        result = circle->center;
    } else {
        dir.x = size.y*cross;
        dir.y = -(size.x*cross);
        MathVecNorm2D(&dirNorm, &dir);
        offset.x = bbox->min.x-circle->center.x;
        offset.y = bbox->min.y-circle->center.y;
        radius = MathDot2DGet(&dirNorm, &offset);
        if(radius > circle->radius) {
            return FALSE;
        }
        result.x = circle->center.x+(dirNorm.x*radius);
        result.y = circle->center.y+(dirNorm.y*radius);
    }
    outCircle.center.x = result.x;
    outCircle.center.y = result.y;
    outCircle.radius = 1;
    return MathCircleCollide(circle, &outCircle);
    
}