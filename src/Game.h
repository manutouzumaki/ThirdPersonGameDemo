#ifndef _GAME_H_
#define _GAME_H_

#include "Renderer.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Skeleton.h"
#include "Clip.h"
#include "Transform.h"
#include "Camera.h"

struct Game {
    Renderer mRenderer;
    Shader mShader;
    Shader mStaticShader;
    Mesh mMesh;

    Texture mTexture;
    Texture mGrassTexture;

    Mesh mTest;
    Pose mRestPose;
    Pose mBindPose;
    std::vector<mat4> mPosePalette;
    Skeleton mSkeleton;
    std::vector<Clip> mClips;
    float mPlayback;
    Pose mAnimatedPose;

    Camera mCamera;
    unsigned int mCurrentAnim;
    Transform mCloneTransform;
    vec3 mCloneDirection;
    vec3 mCloneRight;
    float mCloneRotation;
    float mCloneRotOffset;
    bool mCloneJumping;
    
    

    void Initialize();
    void Update(float dt);
    void Render();
    void Shutdown();
};

#endif
