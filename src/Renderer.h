#ifndef _RENDERER_H_
#define _RENDERER_H_

struct Renderer {
    void Initialize();
    void Shutdown();
    void DrawIndex(unsigned int indicesCount);
    void DrawArray(unsigned int verticesCount);
};

#endif
