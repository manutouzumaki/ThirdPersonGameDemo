#ifndef _VEC4_H_
#define _VEC4_H_

struct vec4 {
    union {
        struct {
            float x;
            float y;
            float z;
            float w;
        };
        float v[4];
    };

    inline vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
    inline vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) { }
    inline vec4(float *fv) : x(fv[0]), y(fv[1]), z(fv[2]), w(fv[3]) { }
};

struct ivec4 {
    union {
        struct {
            int x;
            int y;
            int z;
            int w;
        };
        int v[4];
    };

    inline ivec4() : x(0), y(0), z(0), w(0) { }
    inline ivec4(int _x, int _y, int _z, int _w) : x(_x), y(_y), z(_z), w(_w) { }
    inline ivec4(int *fv) : x(fv[0]), y(fv[1]), z(fv[2]), w(fv[3]) { }
};

#endif
