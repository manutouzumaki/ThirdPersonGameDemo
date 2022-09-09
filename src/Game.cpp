#include "Game.h"

#include "Vec3.h"
#include "Mat4.h"
#include "Slotmap.h"
#include "Input.h"
#include "Defines.h"

#include <stdio.h>
#include <cmath>

#include <assert.h>
#include <glad/glad.h>




//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// TODO: Collision Detection System...
// -If it looks right it is right
// -Nothing is faster than not having to perform a task

// Barycentric Coords
void SolveBarycentric(vec3 a, vec3 b, vec3 c, vec3 p, float &u, float &v, float &w) {
    vec3 v0 = b - a;
    vec3 v1 = c - a;
    vec3 v2 = p - a;
    float d00 = dot(v0, v0);
    float d10 = dot(v1, v0);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    float denom = d00*d11 - d10*d10;
    v = (d20*d11 - d10*d21) / denom;
    w = (d00*d21 - d20*d10) / denom;
    u = 1.0f - v - w;
}

// Plane
struct Plane {
    vec3 n;  
    vec3 p;
};


// Polygones
// Convex Check
int IsConvexQuad(vec3 a, vec3 b, vec3 c, vec3 d) {
    // Quad is nonconvex if Dot(Cross(bd, ba), Cross(bd, bc)) >= 0
    vec3 bda = cross(d - b, a - b);
    vec3 bdc = cross(d - b, c - b);
    if(dot(bda, bdc) >= 0.0f) return 0;
    // Quad is now convex if Dot(Cross(ac, ad), Cross(ac, ab)) < 0
    vec3 acd = cross(c - a, d - a);
    vec3 acb = cross(c - a, b - a);
    return dot(acd, acb) < 0.0f;
}


//Axis Allined Bounding Boxes

struct AABB_min_max {
    vec3 min;
    vec3 max;
};

struct AABB_min_widths {
    vec3 min;
    float d[3];
};

struct AABB_center_radius {
    vec3 center;
    float r[3];
};
///the function can be effectively implemented by simply stripping
///the sign bit of the binary representation 


// Computing an encompassing bounding box for a rotated AABB using min-max representation
// Transform AABB a by the matrix m and translation t,
// find the maximun extents, and store result into AABB b
void UpdateAABB(AABB_min_max a, float m[3][3], float t[3], AABB_min_max &b) {
    // For all three axes
    for(int i = 0; i < 3; ++i) {
        // start by adding in the translation
        b.min.v[i] = b.max.v[i] = t[i];
        // Form extent by summing smaller and larger terms respectively
        for(int j = 0; j < 3; ++j) {
            float e = m[i][j] * a.min.v[j];
            float f = m[i][j] * a.max.v[j];
            if(e < f) {
                b.min.v[i] += e;
                b.max.v[i] += f;
            } else {
                b.min.v[i] += f;
                b.max.v[i] += e;
            }
        }
    }
}

// the code for the center-radius AABB representation becomes
// Transform AABB a by the matrix m and translation t,
// find the maximun extents, and store result into AABB b
void UpdateAABB(AABB_center_radius a, float m[3][3], float t[3], AABB_center_radius &b) {
    for(int i = 0; i < 3; ++i) {
        b.center.v[i] = t[i];
        b.r[i] = 0.0f;
        for(int j = 0; j < 3; ++j) {
            b.center.v[i] += m[i][j] * a.center.v[j];
            b.r[i] += fabsf(m[i][j]) * a.r[j];
        }
    }
}

// Spheres
struct Sphere {
    vec3 c; // sphere center
    float r; // sphere radius
};

int TestSphereSphere(Sphere a, Sphere b) {
    // Calculate the squared distance between centers
    vec3 d = a.c - b.c;
    float dist2 = dot(d, d);
    float radSum = a.r + b.r;
    return dist2 <= radSum * radSum;
}

// Computing the bounding Sphere
// Compute indices to the two most separeted points of the (up to) six points
// defining tha AAABB encompassing the point set. Return these as min and max
void MostSeparatedPointsOnAABB(int &min, int &max, vec3 pt[], int numPts) {
    int minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0;
    for(int i = 1; i < numPts; ++i) {
        if(pt[i].x < pt[minx].x) minx = i;
        if(pt[i].x > pt[maxx].x) maxx = i;
        if(pt[i].y < pt[miny].y) miny = i;
        if(pt[i].y > pt[maxy].y) maxy = i;
        if(pt[i].z < pt[minz].z) minz = i;
        if(pt[i].z > pt[maxz].z) maxz = i;
    }
    // Compute the squared distances for the three pairs of points
    float dist2x = dot(pt[maxx] - pt[minx], pt[maxx] - pt[minx]);
    float dist2y = dot(pt[maxy] - pt[miny], pt[maxy] - pt[miny]);
    float dist2z = dot(pt[maxz] - pt[minz], pt[maxz] - pt[minz]);
    min = minx;
    max = maxx;
    if(dist2y > dist2x && dist2y > dist2z) {
        min = miny;
        max = maxy;
    }
    if(dist2z > dist2x && dist2z > dist2y) {
        min = minz;
        max = maxz;
    }
}

void SphereFromDistantPoints(Sphere &s, vec3 pt[], int numPts) {
    // Find the most separated pair defining the encompassing AABB
    int min, max;
    MostSeparatedPointsOnAABB(min, max, pt, numPts);
    // Set up the sphere to just encompase these two points
    s.c = (pt[min] + pt[max]) * 0.5f;
    s.r = dot(pt[max] - s.c, pt[max] - s.c);
    s.r = sqrtf(s.r);
}

void SphereOfSphereAndPt(Sphere &s, vec3 &p) {
    // Compute squared distance between point and sphere center
    vec3 d = p - s.c;
    float dist2 = dot(d, d);
    // Only update s if point p is outside it
    if(dist2 > s.r * s.r) {
        float dist = sqrtf(dist2);
        float newRadius = (s.r + dist) * 0.5f;
        float k = (newRadius - s.r) / dist;
        s.r = newRadius;
        s.c = s.c + (d * k);
    }
}

// The full Code for computing the approximate bounding sphere becomes
void RitterSphere(Sphere &s, vec3 pt[], int numPts) {
    // Get sphere encompassing two approximately most distant points
    SphereFromDistantPoints(s, pt, numPts);
    
    // Grow sphere to include all points
    for(int i = 0; i < numPts; ++i) {
        SphereOfSphereAndPt(s, pt[i]);
    }
}

// Compute variance of a set of 1D values
float Variance(float x[], int n) {
    float u = 0.0f;
    for(int i = 0; i < n; ++i) {
        u += x[i];
    }
    u /= n;
    float s2 = 0.0f;
    for(int i = 0; i < n; ++i) {
        s2 += (x[i] - u) * (x[i] - u);
    }
    return s2 / n;
}


/*
| 00 01 02 | 
| 10 11 12 | 
| 20 21 22 | 
*/

void CovarianceMatrix(float cov[3][3], vec3 pt[], int numPts) {
    float oon = 1.0f / (float)numPts;
    vec3 c = vec3(0, 0, 0);
    float e00, e11, e22, e01, e02, e12;
    // Compute the center of mass (centroid) of the points 
    for(int i = 0; i < numPts; ++i) {
        c = c + pt[i];
    }
    c = c * oon;
    // Compute covariance elements;
    e00 = e11 = e22 = e01 = e02 = e12 = 0.0f;
    for(int i = 0; i < numPts; ++i) {
        // translate points so center of mass is at origin
        vec3 p = pt[i] - c;
        // Compute covariance of traslated points
        e00 += p.x * p.x;
        e01 += p.x * p.y;
        e02 += p.x * p.z;
        e11 += p.y * p.y;
        e12 += p.y * p.z;
        e22 += p.z * p.z;
    }
    // Fill the covariance matrix elements
    cov[0][0] = e00 * oon;
    cov[1][1] = e11 * oon;
    cov[2][2] = e22 * oon;
    cov[0][1] = cov[1][0] = e01 * oon;
    cov[0][2] = cov[2][0] = e02 * oon;
    cov[1][2] = cov[2][1] = e12 * oon;
}

struct OBB {
    vec3 c;      // OBB center point
    vec3 u[3];  // Local x-, y- and z-axes
    vec3 e;      // Positive halfwidth extents of OBB along each axis
};


// Given point p, return the point q on or in AABB b that is closest to p
void ClosestPtPointAABB(vec3 p, AABB_min_max b, vec3 &q) {
    // For each coordinate axis, if the point coordinate value is
    // outside box, clamp it to the box, else keep it as is
    for(int i = 0; i < 3; ++i) {
        float v = p.v[i];
        if(v < b.min.v[i]) v = b.min.v[i];
        if(v > b.max.v[i]) v = b.max.v[i];
        q.v[i] = v;
    }
}

// Computes the square distance between a point p and an AADD b
float SqDistPointAABB(vec3 p, AABB_min_max b) {
    float sqDist = 0.0f;
    for(int i = 0; i < 3; ++i) {
        float v = p.v[i];
        if(v < b.min.v[i]) sqDist += (b.min.v[i] - v) * (b.min.v[i] - v);
        if(v > b.max.v[i]) sqDist += (v - b.max.v[i]) * (v - b.max.v[i]);
    }
    return sqDist;
}

vec3 GetAABBNormalFromPoint(vec3 p, AABB_min_max b) {
    if(p.x == b.max.x) {
        return vec3(1, 0, 0);
    }
    else if(p.x == b.min.x) {
        return vec3(-1, 0, 0);
    }
    if(p.z == b.max.z) {
        return vec3(0, 0, 1);
    }
    else if(p.z == b.min.z) {
        return vec3(0, 0, -1);
    }
    if(p.y == b.max.y) {
        return vec3(0, 1, 0);
    }
    else if(p.y == b.min.y) {
        return vec3(0, -1, 0);
    }
    return vec3();
}

vec3 ClosestPtPointPlane(vec3 q, Plane plane)
{
    vec3 n = plane.n;
    vec3 p = plane.p;
    float t = dot(n, q - p) / dot(n , n);
    vec3 r = q - (n * t);
    return r;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////






void Game::Initialize() {
    // Initialize
    mRenderer.Initialize();
    mShader.Initialize("../src/shaders/Vertex.glsl", "../src/shaders/Fragment.glsl");
    mStaticShader.Initialize("../src/shaders/StaticVertex.glsl", "../src/shaders/StaticFragment.glsl");
    mCubemapShader.Initialize("../src/shaders/CubemapVertex.glsl", "../src/shaders/CubemapFragment.glsl");
    
    mRedTexutre.Initialize("../assets/red.png");
    mGreenTexutre.Initialize("../assets/green.png");
    mGrassTexture.Initialize("../assets/grass/TexturesCom_Ground_Grass01_2x2_512_translucency.png");
    const char *faces[] = {
        "../assets/red/bkg1_right1.png",
        "../assets/red/bkg1_left2.png",
        "../assets/red/bkg1_top3.png",
        "../assets/red/bkg1_bottom4.png",
        "../assets/red/bkg1_front5.png",
        "../assets/red/bkg1_back6.png"
    };
    mCubemap.InitializeCubemap(faces);
    
    
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
    mat4 projection = perspective(60.0f, 1920.0f/1080.0f, 0.01f, 100.0f);
    mShader.UpdateMat4("projection", projection);
    mCamera.Initialize(vec3(0, 6, -10), vec3(0, 3, 0));
    
    //mCloneTransform.mPosition = vec3(0, 4.5f, 0);
    mCloneTransform.mPosition = vec3(0, 2.5f, -8);
    mCloneTransform.mScale = vec3(1.0f, 1.0f, 1.0f);
    mCloneTransform.mRotation = angleAxis(TO_RAD(0.0f), vec3(0, 1, 0)); 
    mShader.UpdateMat4("model", transformToMat4(mCloneTransform));
    mShader.UpdateVec3("light", vec3(-2, 8, -4));     
    mTexture.Bind(&mShader, "tex0", 0);

    mShader.UpdateMat4Array("invBindPose", (int)mSkeleton.mInvBindPose.size(), &mSkeleton.mInvBindPose[0]);
    
    mStaticShader.UpdateMat4("projection", projection);
    mStaticShader.UpdateVec3("light", vec3(-2, 8, -4));     
    
    mCubemapShader.UpdateMat4("projection", projection);

    mCamera.UpdateFollowCamera(&mCloneTransform);
    mCamera.UpdateCameraInShader(&mShader);
    mCamera.UpdateCameraInShader(&mStaticShader);

    mCurrentAnim = 1;
    mCloneDirection = vec3(0, 0, 1);
    mCloneRight = vec3(1, 0, 0);
    mCloneRotation = TO_RAD(-90.0f);
    mCloneRotOffset = 0.0f;
    mCloneGravity = vec3(0, -9.8f*3.0f, 0);
    mCloneVelocity = vec3(0, 0, 0); 

    mCubemapShader.UpdateMat4("model", mat4());
}


void Game::Update(float dt) {
    mCamera.UpdateFollowCamera(&mCloneTransform);
    mCamera.UpdateCameraInShader(&mShader);
    mCamera.UpdateCameraInShader(&mStaticShader);
    mCamera.UpdateCameraInShader(&mCubemapShader);


    mPlayback = mClips[mCurrentAnim].Sample(mAnimatedPose, mPlayback + dt); 
    
    mAnimatedPose.GetMatrixPalette(mPosePalette);
    mShader.UpdateMat4Array("pose", (int)mPosePalette.size(), &mPosePalette[0]);
    
    if(mCloneIsJumping) { 
        mCloneVelocity = mCloneVelocity + mCloneGravity * dt;
    }
    else {
        mCloneVelocity = vec3(0, 0, 0);
    }

    // TODO improve this a lot....
    float speed = 6.0f;
    if(MouseGetButtonDown(MOUSE_BUTTON_RIGHT)) {
        mCloneRotOffset = 0.0f;
        mCloneDirection = normalized(vec3(mCamera.mFront.x, 0.0f, mCamera.mFront.z));
        mCloneRight = normalized(cross(vec3(0, 1, 0), mCloneDirection));
        mCloneRotation = mCamera.mYaw;
    
        if(MouseGetButtonDown(MOUSE_BUTTON_LEFT) && mCloneJumping == false) {
            mCurrentAnim = 3;
            mCloneTransform.mPosition = mCloneTransform.mPosition + (mCloneDirection * speed) * dt;
        }
    }
    if(KeyboardGetKeyDown(KEYBOARD_KEY_W) && mCloneJumping == false) {
        mCloneRotOffset = 0.0f;
        mCurrentAnim = 3;
        mCloneTransform.mPosition = mCloneTransform.mPosition + (mCloneDirection * speed) * dt;
    }
    if(KeyboardGetKeyDown(KEYBOARD_KEY_S) && mCloneJumping == false) {
        mCurrentAnim = 3;
        mCloneRotOffset = 180.0f;
        mCloneTransform.mPosition = mCloneTransform.mPosition - (mCloneDirection * speed) * dt;
    }
    if(KeyboardGetKeyDown(KEYBOARD_KEY_A) && mCloneJumping == false) { 
        mCurrentAnim = 3;
        mCloneRotOffset = -90.0f;
        mCloneTransform.mPosition = mCloneTransform.mPosition + (mCloneRight * speed) * dt;
    }
    if(KeyboardGetKeyDown(KEYBOARD_KEY_D) && mCloneJumping == false) {
        mCurrentAnim = 3;
        mCloneRotOffset = 90.0f;
        mCloneTransform.mPosition = mCloneTransform.mPosition - (mCloneRight * speed) * dt; 
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
#if 0
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
#else
    if(KeyboardGetKeyJustDown(KEYBOARD_KEY_SPACE) && mCloneIsJumping == false) {
        mCloneVelocity.y += 1000.0f * dt; 
        mCloneIsJumping = true;
    }
#endif
    mCloneTransform.mPosition = mCloneTransform.mPosition + mCloneVelocity * dt;

    // TODO: try to create a first implementation of axis align collision detection and resolution
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create tha AABB for the cube to be tested
    bool dirtyFlag = false;
    
    AABB_min_max greenCubeAABB;
    greenCubeAABB.min.x = -2.0f;
    greenCubeAABB.min.y = 0.0f;
    greenCubeAABB.min.z = -2.0f;
    greenCubeAABB.max.x = 2.0f;
    greenCubeAABB.max.y = 4.0f;
    greenCubeAABB.max.z = 2.0f;
    float distanceFromBox = SqDistPointAABB(mCloneTransform.mPosition, greenCubeAABB);
    if(distanceFromBox > 0.0f) {
        ClosestPtPointAABB(mCloneTransform.mPosition, greenCubeAABB, mCollisionPoint);
        if(mCloneTransform.mPosition.y > mCollisionPoint.y) {
            mCloneIsJumping = true;
        }
    }
    if(distanceFromBox <= 0.0f){
        vec3 collisionNormal = GetAABBNormalFromPoint(mCollisionPoint, greenCubeAABB);
        Plane collisionPlane;
        collisionPlane.n = normalized(collisionNormal);
        collisionPlane.p = mCollisionPoint;
        mCloneTransform.mPosition = ClosestPtPointPlane(mCloneTransform.mPosition, collisionPlane);
        if(collisionNormal.y) {
            mCloneIsJumping = false;
            mCloneVelocity.y = 0.0f; 
            dirtyFlag = true;
        }
    }

    greenCubeAABB.min.x = 5.0f;
    greenCubeAABB.min.y = 0.0f;
    greenCubeAABB.min.z = 5.0f;
    greenCubeAABB.max.x = 15.0f;
    greenCubeAABB.max.y = 6.0f;
    greenCubeAABB.max.z = 15.0f;
    distanceFromBox = SqDistPointAABB(mCloneTransform.mPosition, greenCubeAABB);
    if(distanceFromBox > 0.0f) {
        ClosestPtPointAABB(mCloneTransform.mPosition, greenCubeAABB, mOtherCollisionPoint);
        if(mCloneTransform.mPosition.y > mOtherCollisionPoint.y) {
            mCloneIsJumping = true;
        }
    }
    if(distanceFromBox <= 0.0f){
        vec3 collisionNormal = GetAABBNormalFromPoint(mOtherCollisionPoint, greenCubeAABB);
        Plane collisionPlane;
        collisionPlane.n = normalized(collisionNormal);
        collisionPlane.p = mOtherCollisionPoint;
        mCloneTransform.mPosition = ClosestPtPointPlane(mCloneTransform.mPosition, collisionPlane);
        if(collisionNormal.y) {
            mCloneIsJumping = false;
            mCloneVelocity.y = 0.0f; 
            dirtyFlag = true;
        }
    }


    // TODO: add colision with the floor, gravity and a jump
    // floor collision test and resolution
    AABB_min_max floorAABB;
    floorAABB.min.x = -500.0f;
    floorAABB.min.y = -4.0f;
    floorAABB.min.z = -500.0f;
    floorAABB.max.x = 500.0f;
    floorAABB.max.y = 0.5f;
    floorAABB.max.z = 500.0f;
    float distanceFromFloor = SqDistPointAABB(mCloneTransform.mPosition, floorAABB);
    if(distanceFromFloor > 0.0f) {
        ClosestPtPointAABB(mCloneTransform.mPosition, floorAABB, mFloorCollisionPont);
        if(mCloneTransform.mPosition.y > mFloorCollisionPont.y) {
            if(!dirtyFlag) mCloneIsJumping = true;
        }

    }
    if(distanceFromFloor <= 0.0f){
        vec3 collisionNormal = GetAABBNormalFromPoint(mFloorCollisionPont, floorAABB);
        Plane collisionPlane;
        collisionPlane.n = normalized(collisionNormal);
        collisionPlane.p = mFloorCollisionPont;
        mCloneTransform.mPosition = ClosestPtPointPlane(mCloneTransform.mPosition, collisionPlane);
        if(collisionNormal.y) {
            mCloneIsJumping = false;
            mCloneVelocity.y = 0.0f; 
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////


    mCloneTransform.mRotation = angleAxis(-(mCloneRotation + TO_RAD(90.0f + mCloneRotOffset)), vec3(0, 1, 0));
    mShader.UpdateMat4("model", transformToMat4(mCloneTransform));

    
    static float cubemapTimer = 0.0f;
    Transform cubemapTransform;
    cubemapTransform.mRotation = angleAxis(cubemapTimer, vec3(0, 1, 0));
    mCubemapShader.UpdateMat4("model", transformToMat4(cubemapTransform));
    cubemapTimer += dt * 0.02f;
}

void Game::Render() {

#if 1
    glDisable(GL_DEPTH_TEST);
    mCubemap.Bind(&mCubemapShader, "cubemap", 0);
    mMesh.Bind();
    mCubemapShader.Bind();
    mRenderer.DrawArray(mMesh.mVerticesCount);
    glEnable(GL_DEPTH_TEST);
#endif


    mTexture.Bind(&mShader, "tex0", 0);
    mTest.Bind();
    mShader.Bind();
    mRenderer.DrawIndex(mTest.mIndicesCount);
    
    Transform floorModel;
    floorModel.mPosition = vec3(0, 0, 0);
    floorModel.mScale = vec3(1000.0f, 1.0f, 1000.0f);
    floorModel.mRotation = angleAxis(TO_RAD(0.0f), vec3(0, 1, 0));
    mStaticShader.UpdateMat4("model", transformToMat4(floorModel));

    mGrassTexture.Bind(&mStaticShader, "tex0", 0);
    mMesh.Bind();
    mStaticShader.Bind();
    mRenderer.DrawArray(mMesh.mVerticesCount);



    // TODO: test AABB closet point

    floorModel.mPosition = vec3(0, 2, 0);
    floorModel.mScale = vec3(4.0f, 4.0f, 4.0f);
    floorModel.mRotation = angleAxis(TO_RAD(0.0f), vec3(0, 1, 0));
    mStaticShader.UpdateMat4("model", transformToMat4(floorModel));
    mRedTexutre.Bind(&mStaticShader, "tex0", 0);
    mMesh.Bind();
    mStaticShader.Bind();
    mRenderer.DrawArray(mMesh.mVerticesCount);

    floorModel.mPosition = vec3(10, 3, 10);
    floorModel.mScale = vec3(10, 6, 10);
    floorModel.mRotation = angleAxis(TO_RAD(0.0f), vec3(0, 1, 0));
    mStaticShader.UpdateMat4("model", transformToMat4(floorModel));
    mRedTexutre.Bind(&mStaticShader, "tex0", 0);
    mMesh.Bind();
    mStaticShader.Bind();
    mRenderer.DrawArray(mMesh.mVerticesCount);

    floorModel.mPosition = mCollisionPoint;
    floorModel.mScale = vec3(0.2f, 0.2f, 0.2f);
    floorModel.mRotation = angleAxis(TO_RAD(0.0f), vec3(0, 1, 0));
    mStaticShader.UpdateMat4("model", transformToMat4(floorModel));
    mGreenTexutre.Bind(&mStaticShader, "tex0", 0);
    mMesh.Bind();
    mStaticShader.Bind();
    mRenderer.DrawArray(mMesh.mVerticesCount);

    floorModel.mPosition = mFloorCollisionPont;
    floorModel.mScale = vec3(0.2f, 0.2f, 0.2f);
    floorModel.mRotation = angleAxis(TO_RAD(0.0f), vec3(0, 1, 0));
    mStaticShader.UpdateMat4("model", transformToMat4(floorModel));
    mGreenTexutre.Bind(&mStaticShader, "tex0", 0);
    mMesh.Bind();
    mStaticShader.Bind();
    mRenderer.DrawArray(mMesh.mVerticesCount);

    floorModel.mPosition = mOtherCollisionPoint;
    floorModel.mScale = vec3(0.2f, 0.2f, 0.2f);
    floorModel.mRotation = angleAxis(TO_RAD(0.0f), vec3(0, 1, 0));
    mStaticShader.UpdateMat4("model", transformToMat4(floorModel));
    mGreenTexutre.Bind(&mStaticShader, "tex0", 0);
    mMesh.Bind();
    mStaticShader.Bind();
    mRenderer.DrawArray(mMesh.mVerticesCount);

}

void Game::Shutdown() {
    
    printf("ShutingDown\n");

    mTest.Unbind();
    mShader.Unbind();
    
    mMesh.Shutdown();
    mCubemap.Shutdown();
    mTexture.Shutdown();
    mRedTexutre.Shutdown();
    mGreenTexutre.Shutdown();
    mGrassTexture.Shutdown();
    mTest.Shutdown();
    mStaticShader.Shutdown();
    mCubemapShader.Shutdown();
    mShader.Shutdown();
    mRenderer.Shutdown();
}
