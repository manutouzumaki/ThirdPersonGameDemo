#include "Camera.h"
#include "Shader.h"
#include "Input.h"
#include "Defines.h"
#include <stdio.h>
#include <math.h>

void Camera::Initialize(vec3 position, vec3 target) {
    mPosition = position;
    mFront = normalized(target - position);
    mWorldUp = vec3(0, 1, 0);
    mRight = normalized(cross(mWorldUp, mFront));
    mUp = normalized(cross(mFront, mRight));
    mDistance = len(target - position);
    mFirstClickX = 0;
    mFirstClickY = 0;
    mPitch = 0;
    mYaw = TO_RAD(-90.0f);
}

void Camera::UpdateFollowCamera(Transform *target) {
    if(MouseGetButtonDown(MOUSE_BUTTON_LEFT) || MouseGetButtonDown(MOUSE_BUTTON_RIGHT)) {
        float offsetX = (float)(MouseGetCursorX() - MouseGetLastCursorX());
        float offsetY = (float)(MouseGetCursorY() - MouseGetLastCursorY());

        mYaw += offsetX * 0.01f;
        mPitch += offsetY * 0.01f;

        if(mPitch > TO_RAD(89.0f)) {
            mPitch = TO_RAD(89.0f);
        }
        if(mPitch < TO_RAD(-89.0f)) {
            mPitch = TO_RAD(-89.0f);
        }
        
        vec3 posRelTarget = target->mPosition - mPosition;
        posRelTarget.x = cosf(mYaw) * cosf(mPitch);
        posRelTarget.y = sinf(mPitch);
        posRelTarget.z = sinf(mYaw) * cosf(mPitch);

        mPosition = target->mPosition + posRelTarget;
        mFront = normalized(target->mPosition - mPosition);
        mRight = normalized(cross(mWorldUp, mFront));
        mUp = normalized(cross(mFront, mRight));
    }
    
    mDistance -= (float)MouseGetWheel();
    if(mDistance <= 1.0f) {
        mDistance = 1.0f;
    }
    if(mDistance >= 20.0f) {
        mDistance = 20.0f;
    }

    mPosition = (target->mPosition + vec3(0, 3, 0)) - (mFront * mDistance);
#if 1
    if(mPosition.y <= 0.5f) {
        mPosition.y = 0.5f;
    }
#endif
}

void Camera::UpdateCameraInShader(Shader *shader) {
    mat4 view = lookAt(mPosition, mPosition + (mFront * mDistance), mWorldUp);
    shader->UpdateMat4("view", view);
}
