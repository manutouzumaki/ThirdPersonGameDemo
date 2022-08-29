#include "Vec2.h"
#include <cmath>

vec2 operator+(const vec2 &l, const vec2 &r) {
    return vec2(l.x + r.x, l.y + r.y);
}

vec2 operator-(const vec2 &l, const vec2 &r) {
    return vec2(l.x - r.x, l.y - r.y);
}

vec2 operator*(const vec2 &l, float f) {
    return vec2(l.x * f, l.y * f);
}

vec2 operator*(const vec2 &l, const vec2 &r) {
    return vec2(l.x * r.x, l.y * r.y);
}

float dot(const vec2 &l, const vec2 &r) {
    return l.x * r.x + l.y * r.y;
}

float lenSq(const vec2 &v) {
    return v.x * v.x + v.y * v.y;
}

float len(const vec2 &v) {
    float lenSq = v.x * v.x + v.y * v.y;
    if(lenSq < VEC2_EPSILON) {
        return 0.0f;
    }
    return sqrtf(lenSq);
}

void normalize(vec2 &v) {
    float lenSq = v.x * v.x + v.y * v.y;
    if(lenSq < VEC2_EPSILON) { return; }
    float invLen = 1.0f / sqrtf(lenSq);
    v.x *= invLen;
    v.y *= invLen;
}

vec2 normalized(const vec2 &v) {
    float lenSq = v.x * v.x + v.y * v.y;
    if(lenSq < VEC2_EPSILON) { return v; }
    float invLen = 1.0f / sqrtf(lenSq);
    return vec2(v.x * invLen, v.y * invLen);
}

float angle(const vec2 &l, const vec2 &r) {
    float lenSqL = l.x * l.x + l.y * l.y;
    float lenSqR = r.x * r.x + r.y * r.y;
    if(lenSqL < VEC2_EPSILON || lenSqR < VEC2_EPSILON) {
        return 0.0f;
    }
    float dot = l.x * r.x + l.y * r.y;
    float len = sqrtf(lenSqL) * sqrtf(lenSqR);
    return acosf(dot / len);
}

vec2 project(const vec2 &a, const vec2 &b) {
    float magB = len(b);
    if(magB < VEC2_EPSILON) {
        return vec2();
    }
    float scale = dot(a, b) / magB;
    return b * scale;
}

vec2 reject(const vec2 &a, const vec2 &b) {
    vec2 projection = project(a, b);
    return a - projection;
}

vec2 reflect(const vec2 &a, const vec2 &b) {
    float magB = len(b);
    if(magB < VEC2_EPSILON) {
        return vec2();
    }
    float scale = dot(a, b) / magB;
    vec2 proj2 = b * (scale * 2.0f);
    return a - proj2;
}

vec2 lerp(const vec2 &s, const vec2 &e, float t) {
    return vec2(
        s.x + (e.x - s.x) * t,        
        s.y + (e.y - s.y) * t        
    );
}

vec2 slerp(const vec2 &s, const vec2 &e, float t) {
    if(t < 0.01f) {
        return lerp(s, e, t);
    }
    vec2 from = normalized(s);
    vec2 to = normalized(e);
    float theta = angle(from, to);
    float sin_theta = sinf(theta);
    float a = sinf((1.0f - t) * theta) / sin_theta;
    float b = sinf(t * theta) / sin_theta;
    return from * a + to * b;
}

vec2 nlerp(const vec2 &s, const vec2 &e, float t) {
    vec2 linear(
        s.x + (e.x - s.x) * t,
        s.y + (e.y - s.y) * t
    );
    return normalized(linear);
}

bool operator==(const vec2 &l, const vec2 &r) {
    vec2 diff(l - r);
    return lenSq(diff) < VEC2_EPSILON;
}

bool operator!=(const vec2 &l, const vec2 &r) {
    return !(l == r);
}
