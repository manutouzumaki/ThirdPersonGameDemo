#ifndef _TEXTURE_H_
#define _TEXTURE_H_

struct Shader;

struct Texture {
    unsigned int mHandle;
    int mWidth;
    int mHeight;
    int mChannels;

    void Initialize(const char *path);
    void InitializeCubemap(const char **faces);
    void Shutdown();
    void Bind(Shader *shader, const char *varName, unsigned int textureIndex);
    void Unbind(unsigned textureIndex);
};

#endif
