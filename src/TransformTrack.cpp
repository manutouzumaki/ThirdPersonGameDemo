#include "TransformTrack.h"

TransformTrack::TransformTrack() {
    mId = 0;
}

float TransformTrack::GetStartTime() {
    float result = 0.0f;
    bool isSet = false; 

    if(mPosition.mFrames.size() > 1) {
        result = mPosition.GetStartTime();
        isSet = true;
    }
    if(mRotation.mFrames.size() > 1) {
        float rotationStart = mRotation.GetStartTime();
        if(rotationStart < result || isSet == false) {
            result = rotationStart;
            isSet = true;
        }
    }
    if(mScale.mFrames.size() > 1) {
        float scaleStart = mScale.GetStartTime();
        if(scaleStart < result || isSet == false) {
            result = scaleStart;
            isSet = true;
        }
    }
    return result;
}


float TransformTrack::GetEndTime() {
    float result = 0.0f;
    bool isSet = false; 
    
    if(mPosition.mFrames.size() > 1) {
        result = mPosition.GetEndTime();
        isSet = true;
    }
    if(mRotation.mFrames.size() > 1) {
        float rotationEnd = mRotation.GetEndTime();
        if(rotationEnd > result || isSet == false) {
            result = rotationEnd;
            isSet = true;
        }
    }
    if(mScale.mFrames.size() > 1) {
        float scaleEnd = mScale.GetEndTime();
        if(scaleEnd > result || isSet == false) {
            result = scaleEnd;
            isSet = true;
        }
    }
    return result;
}

bool TransformTrack::IsValid() {
    return mPosition.mFrames.size() > 1 || mRotation.mFrames.size() > 1 || mScale.mFrames.size() > 1;
}

Transform TransformTrack::Sample(const Transform &ref, float time, bool looping) {
    Transform result = ref; // Assign default values
    if(mPosition.mFrames.size() > 1) {
        result.mPosition = mPosition.Sample(time, looping);
    }
    if(mRotation.mFrames.size() > 1) {
        result.mRotation = mRotation.Sample(time, looping);
    }
    if(mScale.mFrames.size() > 1) {
        result.mScale = mScale.Sample(time, looping);
    }
    return result;
}
