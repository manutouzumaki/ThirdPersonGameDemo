#ifndef _MESH_H_
#define _MESH_H_

#include <cgltf.h>

struct Mesh {

    unsigned int mVao;
    unsigned int mVbo;
    unsigned int mEbo;

    unsigned int mVerticesCount;
    unsigned int mIndicesCount;

    Mesh() : mVao(0), mVbo(0), mEbo(0) { }
    void InitializeStatic(cgltf_data *data);
    void InitializeAnimated(cgltf_data *data);
    void InitializeCube();
    void InitializeSphere();
    void Shutdown();
    void Bind();
    void Unbind();
};

cgltf_data *LoadGLTFFile(const char *path);
void FreeGLTFFile(cgltf_data *data);

#endif
