#ifndef _GAME_H_
#define _GAME_H_

#include "Renderer.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Skeleton.h"
#include "Clip.h"

struct Game {
    Renderer mRenderer;
    Shader mShader;
    Mesh mMesh;

    Texture mTexture;

    Mesh mTest;
    Pose mRestPose;
    Pose mBindPose;
    std::vector<mat4> mPosePalette;
    Skeleton mSkeleton;
    std::vector<Clip> mClips;
    float mPlayback;
    Pose mAnimatedPose;
    

    void Initialize();
    void Update(float dt);
    void Render();
    void Shutdown();
};

#endif
