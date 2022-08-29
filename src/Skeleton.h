#ifndef _SKELETON_H_
#define _SKELETON_H_

#include "Pose.h"

struct Skeleton {
    Pose mRestPose;
    Pose mBindPose;
    std::vector<mat4> mInvBindPose;
    void SetPoses(const Pose& rest, const Pose& bind);
};

#endif
