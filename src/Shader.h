#ifndef _SHADER_H_
#define _SHADER_H_

#include "Vec4.h"
#include "Vec3.h"
#include "Mat4.h"

struct FileResult {
    void *data;
    size_t size;
};

struct Shader {
    unsigned int mProgram;
    
    void Initialize(const char *vertexPath, const char *fragmentPath);
    void Shutdown();
    void Bind();
    void Unbind();

    void UpdateVec3(const char* varName, vec3 vector);
    void UpdateVec4(const char* varName, vec4 vector);
    void UpdateMat4(const char* varName, mat4 matrix);
    void UpdateInt(const char* varName, int value);
    void UpdateIntArray(const char* varName, int size, int* array);
    void UpdateMat4Array(const char* varName, int size, mat4* array);
};

#endif
