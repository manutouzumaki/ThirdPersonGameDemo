#ifndef _FRAME_H_
#define _FRAME_H_

#include "Vec3.h"
#include "Quat.h"

struct FrameScalar {
    float mValue;
    float mIn;
    float mOut;
    float mTime;
};

struct FrameVec3 {
    vec3 mValue;
    vec3 mIn;
    vec3 mOut;
    float mTime;

};

struct FrameQuat {
    quat mValue;
    quat mIn;
    quat mOut;
    float mTime;

};

enum Interpolation {
    INTERPOLATION_CONSTANT,
    INTERPOLATION_LINEAR,
    INTERPOLATION_CUBIC
};

#endif
