#include "Transform.h"
#include <cmath>

Transform combine(const Transform &a, const Transform &b) {
    Transform result;
    result.mScale = a.mScale * b.mScale;
    result.mRotation = b.mRotation * a.mRotation;
    result.mPosition = a.mRotation * (a.mScale * b.mPosition);
    result.mPosition = a.mPosition + result.mPosition;
    return result;
}

Transform inverse(const Transform &t) {
    Transform inv;
    inv.mRotation = inverse(t.mRotation);
    inv.mScale.x = fabsf(t.mScale.x) < VEC3_EPSILON ? 0.0f : 1.0f / t.mScale.x;
    inv.mScale.y = fabsf(t.mScale.y) < VEC3_EPSILON ? 0.0f : 1.0f / t.mScale.y;
    inv.mScale.z = fabsf(t.mScale.z) < VEC3_EPSILON ? 0.0f : 1.0f / t.mScale.z;
    vec3 invTrans = t.mPosition * -1.0f;
    inv.mPosition = inv.mRotation * (inv.mScale * invTrans);
    return inv;
}

Transform mix(const Transform &a, const Transform &b, float t) {
    quat bRot = b.mRotation;
    if(dot(a.mRotation, bRot) < 0.0f) {
        bRot = -bRot;  
    }
    return Transform(lerp(a.mPosition, b.mPosition, t),
                     nlerp(a.mRotation, bRot, t),
                     lerp(a.mScale, b.mScale, t));
}

mat4 transformToMat4(const Transform &t) {
    vec3 x = t.mRotation * vec3(1, 0, 0);
    vec3 y = t.mRotation * vec3(0, 1, 0);
    vec3 z = t.mRotation * vec3(0, 0, 1);
    x = x * t.mScale.x;
    y = y * t.mScale.y;
    z = z * t.mScale.z;
    vec3 p = t.mPosition;
    return mat4(
        x.x, x.y, x.z, 0,      
        y.x, y.y, y.z, 0,      
        z.x, z.y, z.z, 0,      
        p.x, p.y, p.z, 1
    );
}

Transform mat4ToTransform(const mat4 &m) {
    Transform result;
    result.mPosition = vec3(m.v[12], m.v[13], m.v[14]);
    result.mRotation = mat4ToQuat(m);
    mat4 rotScaleMat(
        m.v[0], m.v[1], m.v[2],  0,      
        m.v[4], m.v[5], m.v[6],  0,      
        m.v[8], m.v[9], m.v[10], 0,      
        0,      0,      0,       1    
    );
    mat4 invRotMat = quatToMat4(inverse(result.mRotation));
    mat4 scaleSkewMat = rotScaleMat * invRotMat;
    result.mScale = vec3(scaleSkewMat.v[0], scaleSkewMat.v[5], scaleSkewMat.v[10]);
    return result;
}

vec3 transformPoint(const Transform &a, const vec3 &b) {
    vec3 result;
    result = a.mRotation * (a.mScale * b);
    result = a.mPosition + result;
    return result;
}

vec3 transformVector(const Transform &a, const vec3 &b) {
    vec3 result;
    result = a.mRotation * (a.mScale * b);
    return result;
}

bool operator==(const Transform& a, const Transform& b) {
	return a.mPosition == b.mPosition &&
		a.mRotation == b.mRotation &&
		a.mScale == b.mScale;
}

bool operator!=(const Transform& a, const Transform& b) {
	return !(a == b);
}
