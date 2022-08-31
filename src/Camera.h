#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Vec3.h"
#include "Transform.h"
#include "Shader.h"

struct Camera {
    vec3 mPosition;
    vec3 mFront;
    vec3 mUp;
    vec3 mRight;
    vec3 mWorldUp;
    float mDistance;
    float mPitch;
    float mYaw; 

    int mFirstClickX;
    int mFirstClickY;


    void Initialize(vec3 position, vec3 target);

    void UpdateFollowCamera(Transform *target);
    void UpdateCameraInShader(Shader *shader);
};

#endif
