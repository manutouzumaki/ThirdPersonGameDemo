#include "Skeleton.h"

void Skeleton::SetPoses(const Pose& rest, const Pose& bind) {
    mRestPose = rest;
    mBindPose = bind;

    // TODO: this can be faster...
    unsigned int size = mBindPose.Size();
    mInvBindPose.resize(size);
    for(unsigned int i = 0; i < size; ++i) {
        Transform world = mBindPose.GetGlobalTransform(i);
        mInvBindPose[i] = inverse(transformToMat4(world));
    }
}
