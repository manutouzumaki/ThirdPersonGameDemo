#include "Game.h"

#include "Vec3.h"
#include "Mat4.h"
#include "Transform.h"

#include <stdio.h>
#include <cmath>
//#include <glad/glad.h>

#define TO_RAD(value) ((value)*(3.14159265359f/180.0f))

void Game::Initialize() {
    // Initialize
    mRenderer.Initialize();
    mShader.Initialize("../src/shaders/Vertex.glsl", "../src/shaders/Fragment.glsl");
    mMesh.InitializeCube();


    // TODO: load animated model
    mTexture.Initialize("../assets/dancing_stormtrooper/textures/Stormtroopermat_baseColor.png");
    cgltf_data *CloneModel = LoadGLTFFile("../assets/dancing_stormtrooper/scene.gltf"); 
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
    mat4 view = lookAt(vec3(0, 0, 5), vec3(0, 0, 0), vec3(0, 1, 0));
    mShader.UpdateMat4("view", view);
    Transform model;
    model.mPosition = vec3(0, -2, 0);
    model.mScale = vec3(1.0f, 1.0f, 1.0f);
    model.mRotation = angleAxis(TO_RAD(0.0f), vec3(0, 1, 0)); 
    mShader.UpdateMat4("model", transformToMat4(model));
    mShader.UpdateVec3("light", vec3(0.5f, 1, 1));     

    mShader.UpdateMat4Array("invBindPose", 120, &mSkeleton.mInvBindPose[0]);

    mShader.Bind();
    mTexture.Bind(&mShader, "tex0", 0);
    mTest.Bind();
}

void Game::Update(float dt) {
    mPlayback = mClips[0].Sample(mAnimatedPose, mPlayback + dt); 
    mAnimatedPose.GetMatrixPalette(mPosePalette);
    mShader.UpdateMat4Array("pose", 120, &mPosePalette[0]);
}

void Game::Render() {
    // Draw Mesh

    //mRenderer.DrawArray(mMesh.mVerticesCount);
    mRenderer.DrawIndex(mTest.mIndicesCount);


}

void Game::Shutdown() {
    
    printf("ShutingDown\n");

    mTest.Unbind();
    mTexture.Unbind(0);
    mShader.Unbind();
    
    mMesh.Shutdown();
    mTexture.Shutdown();
    mTest.Shutdown();
    mShader.Shutdown();
    mRenderer.Shutdown();
}
