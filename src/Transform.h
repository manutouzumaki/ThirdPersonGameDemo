#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "Vec3.h"
#include "Quat.h"
#include "Mat4.h"

struct Transform {
    vec3 mPosition;
    quat mRotation;
    vec3 mScale;

    Transform(const vec3 &p, const quat &r, const vec3 &s) :
        mPosition(p), mRotation(r), mScale(s) { }
    Transform() :
        mPosition(vec3(0, 0, 0)),
        mRotation(quat(0, 0, 0, 1)),
        mScale(vec3(1, 1, 1))
    { }
};

Transform combine(const Transform &a, const Transform &b);
Transform inverse(const Transform &t);
Transform mix(const Transform &a, const Transform &b, float t);
mat4 transformToMat4(const Transform &t);
Transform mat4ToTransform(const mat4 &m);
vec3 transformPoint(const Transform &a, const vec3 &b);
vec3 transformVector(const Transform &a, const vec3 &b);
bool operator==(const Transform& a, const Transform& b);
bool operator!=(const Transform& a, const Transform& b);
#endif
