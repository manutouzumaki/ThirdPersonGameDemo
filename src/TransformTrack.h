#ifndef _TRANSFORMTRACK_H_
#define _TRANSFORMTRACK_H_

#include "Track.h"
#include "Transform.h"

struct TransformTrack {
    unsigned int mId;
    TrackVec3 mPosition;
    TrackQuat mRotation;
    TrackVec3 mScale;

    TransformTrack();
    float GetStartTime();
    float GetEndTime();
    bool IsValid();
    Transform Sample(const Transform &ref, float time, bool looping);

};

#endif
