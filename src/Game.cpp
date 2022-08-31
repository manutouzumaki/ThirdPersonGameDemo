#include "Game.h"

#include "Vec3.h"
#include "Mat4.h"
#include "Slotmap.h"
#include "Input.h"
#include "Defines.h"

#include <stdio.h>
#include <cmath>

#include <assert.h>

void Game::Initialize() {
    // Initialize
    mRenderer.Initialize();
    mShader.Initialize("../src/shaders/Vertex.glsl", "../src/shaders/Fragment.glsl");
    mStaticShader.Initialize("../src/shaders/StaticVertex.glsl", "../src/shaders/StaticFragment.glsl");
    
    mGrassTexture.Initialize("../assets/grass/TexturesCom_Ground_Grass01_2x2_512_albedo.png");
    mMesh.InitializeCube();


    // TODO: load animated model
    mTexture.Initialize("../assets/clone/textures/Stormtroopermat_baseColor.png");
    cgltf_data *CloneModel = LoadGLTFFile("../assets/clone2/clone.gltf"); 
    //mTexture.Initialize("../assets/Woman.png");
    //cgltf_data *CloneModel = LoadGLTFFile("../assets/Woman.gltf");
    mTest.InitializeAnimated(CloneModel);
    mRestPose = LoadRestPose(CloneModel);
    mBindPose = LoadBindPose(CloneModel);
    mSkeleton.SetPoses(mRestPose, mBindPose);
    mClips = LoadClips(CloneModel);
    mPlayback = 0.0f;
    mAnimatedPose = mRestPose;
    
    FreeGLTFFile(CloneModel);
    
    // Set Uniforms
    mat4 projection = perspective(60.0f, 800.0f/600.0f, 0.01f, 1000.0f);
    mShader.UpdateMat4("projection", projection);
    mCamera.Initialize(vec3(0, 6, -10), vec3(0, 3, 0));
    
    mCloneTransform.mPosition = vec3(0, 0.5f, 0);
    mCloneTransform.mScale = vec3(1.0f, 1.0f, 1.0f);
    mCloneTransform.mRotation = angleAxis(TO_RAD(0.0f), vec3(0, 1, 0)); 
    mShader.UpdateMat4("model", transformToMat4(mCloneTransform));
    mShader.UpdateVec3("light", vec3(0, 8, -1));     
    mTexture.Bind(&mShader, "tex0", 0);

    mShader.UpdateMat4Array("invBindPose", (int)mSkeleton.mInvBindPose.size(), &mSkeleton.mInvBindPose[0]);
    
    Transform floorModel;
    floorModel.mPosition = vec3(0, 0, 0);
    floorModel.mScale = vec3(50.0f, 1.0f, 50.0f);
    floorModel.mRotation = angleAxis(TO_RAD(0.0f), vec3(0, 1, 0));

    mStaticShader.UpdateMat4("projection", projection);
    mStaticShader.UpdateMat4("model", transformToMat4(floorModel));
    mStaticShader.UpdateVec3("light", vec3(0, 8, -1));     

    mCamera.UpdateFollowCamera(&mCloneTransform);
    mCamera.UpdateCameraInShader(&mShader);
    mCamera.UpdateCameraInShader(&mStaticShader);

    mCurrentAnim = 1;
    mCloneDirection = vec3(0, 0, 1);
    mCloneRight = vec3(1, 0, 0);
    mCloneRotation = TO_RAD(-90.0f);
    mCloneRotOffset = 0.0f;
    
}

void Game::Update(float dt) {
    mCamera.UpdateFollowCamera(&mCloneTransform);
    mCamera.UpdateCameraInShader(&mShader);
    mCamera.UpdateCameraInShader(&mStaticShader);


    mPlayback = mClips[mCurrentAnim].Sample(mAnimatedPose, mPlayback + dt); 
    
    mAnimatedPose.GetMatrixPalette(mPosePalette);
    mShader.UpdateMat4Array("pose", (int)mPosePalette.size(), &mPosePalette[0]);


    // TODO improve this a lot....
    if(MouseGetButtonDown(MOUSE_BUTTON_RIGHT)) {
        mCloneRotOffset = 0.0f;
        mCloneDirection = normalized(vec3(mCamera.mFront.x, 0.0f, mCamera.mFront.z));
        mCloneRight = normalized(cross(vec3(0, 1, 0), mCloneDirection));
        mCloneRotation = mCamera.mYaw;
    
        if(MouseGetButtonDown(MOUSE_BUTTON_LEFT) && mCloneJumping == false) {
            mCurrentAnim = 3;
            mCloneTransform.mPosition = mCloneTransform.mPosition + (mCloneDirection * 4.0f) * dt;
        }
    }
    if(KeyboardGetKeyDown(KEYBOARD_KEY_W) && mCloneJumping == false) {
        mCloneRotOffset = 0.0f;
        mCurrentAnim = 3;
        mCloneTransform.mPosition = mCloneTransform.mPosition + (mCloneDirection * 4.0f) * dt;
    }
    if(KeyboardGetKeyDown(KEYBOARD_KEY_S) && mCloneJumping == false) {
        mCurrentAnim = 3;
        mCloneRotOffset = 180.0f;
        mCloneTransform.mPosition = mCloneTransform.mPosition - (mCloneDirection * 4.0f) * dt;
    }
    if(KeyboardGetKeyDown(KEYBOARD_KEY_A) && mCloneJumping == false) { 
        mCurrentAnim = 3;
        mCloneRotOffset = -90.0f;
        mCloneTransform.mPosition = mCloneTransform.mPosition + (mCloneRight * 4.0f) * dt;
    }
    if(KeyboardGetKeyDown(KEYBOARD_KEY_D) && mCloneJumping == false) {
        mCurrentAnim = 3;
        mCloneRotOffset = 90.0f;
        mCloneTransform.mPosition = mCloneTransform.mPosition - (mCloneRight * 4.0f) * dt; 
    }
    if(KeyboardGetKeyUp(KEYBOARD_KEY_W) &&
       KeyboardGetKeyUp(KEYBOARD_KEY_S) &&
       KeyboardGetKeyUp(KEYBOARD_KEY_A) &&
       KeyboardGetKeyUp(KEYBOARD_KEY_D) &&
       mCloneJumping == false) {
        mCurrentAnim = 1;
    }
    if(MouseGetButtonDown(MOUSE_BUTTON_RIGHT) && MouseGetButtonDown(MOUSE_BUTTON_LEFT)) {
        mCurrentAnim = 3;
    }

    static float timer = 0.0f;
    if(KeyboardGetKeyJustDown(KEYBOARD_KEY_SPACE) && timer == 0.0f) {
        mCloneJumping = true;
        mPlayback = 0.0f;
    }
    if(mCloneJumping) {
        mCurrentAnim = 2;
        timer += dt; 
    }

    if(timer > 2.4f) {
        mCloneJumping = false;
        timer = 0.0f;
    }

    mCloneTransform.mRotation = angleAxis(-(mCloneRotation + TO_RAD(90.0f + mCloneRotOffset)), vec3(0, 1, 0));
    mShader.UpdateMat4("model", transformToMat4(mCloneTransform));



}

void Game::Render() {
    mTexture.Bind(&mShader, "tex0", 0);
    mTest.Bind();
    mShader.Bind();
    mRenderer.DrawIndex(mTest.mIndicesCount);
    
    mGrassTexture.Bind(&mStaticShader, "tex0", 0);
    mMesh.Bind();
    mStaticShader.Bind();
    mRenderer.DrawArray(mMesh.mVerticesCount);
}

void Game::Shutdown() {
    
    printf("ShutingDown\n");

    mTest.Unbind();
    mShader.Unbind();
    
    mMesh.Shutdown();
    mTexture.Shutdown();
    mGrassTexture.Shutdown();
    mTest.Shutdown();
    mStaticShader.Shutdown();
    mShader.Shutdown();
    mRenderer.Shutdown();
}
