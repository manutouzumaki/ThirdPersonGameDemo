#ifndef _VEC2_H_
#define _VEC2_H_

#define VEC2_EPSILON 0.000001f

struct vec2 {
    union {
        struct {
            float x;
            float y;
        };
        float v[2];
    };

    inline vec2() : x(0.0f), y(0.0f) { }
    inline vec2(float _x, float _y) : x(_x), y(_y) { }
    inline vec2(float *fv) : x(fv[0]), y(fv[1]) { }
};

vec2 operator+(const vec2 &l, const vec2 &r);
vec2 operator-(const vec2 &l, const vec2 &r);
vec2 operator*(const vec2 &l, float f);
vec2 operator*(const vec2 &l, const vec2 &r);
float dot(const vec2 &l, const vec2 &r);
float lenSq(const vec2 &v);
float len(const vec2 &v);
void normalize(vec2 &v);
vec2 normalized(const vec2 &v);
float angle(const vec2 &l, const vec2 &r);
vec2 project(const vec2 &a, const vec2 &b);
vec2 reject(const vec2 &a, const vec2 &b);
vec2 reflect(const vec2 &a, const vec2 &b);
vec2 lerp(const vec2 &s, const vec2 &e, float t);
vec2 slerp(const vec2 &s, const vec2 &e, float t);
vec2 nlerp(const vec2 &s, const vec2 &e, float t);
bool operator==(const vec2 &l, const vec2 &r);
bool operator!=(const vec2 &l, const vec2 &r);

#endif
