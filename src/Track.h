#ifndef _TRACK_H_
#define _TRACK_H_

#include "Frame.h"
#include <vector>

struct TrackScalar {
    std::vector<FrameScalar> mFrames;
    Interpolation mInterpolation;
    TrackScalar();
    float GetStartTime();
    float GetEndTime();
    float Sample(float time, bool looping);
    FrameScalar& operator[](unsigned int index);
private:
    float SampleConstant(float time, bool looping);
    float SampleLinear(float time, bool looping);
    float SampleCubic(float time, bool looping);
    float Hermite(float time, const float &p1, const float &s1, const float &p2, const float &s2);
    int FrameIndex(float time, bool looping);
    float AdjustTimeToFitTrack(float time, float looping);
};

struct TrackVec3 {
    std::vector<FrameVec3> mFrames;
    Interpolation mInterpolation;
    TrackVec3();
    float GetStartTime();
    float GetEndTime();
    vec3 Sample(float time, bool looping);
    FrameVec3& operator[](unsigned int index);
private:
    vec3 SampleConstant(float time, bool looping);
    vec3 SampleLinear(float time, bool looping);
    vec3 SampleCubic(float time, bool looping);
    vec3 Hermite(float time, const vec3 &p1, const vec3 &s1, const vec3 &p2, const vec3 &s2);
    int FrameIndex(float time, bool looping);
    float AdjustTimeToFitTrack(float time, float looping);
};

struct TrackQuat {
    std::vector<FrameQuat> mFrames;
    Interpolation mInterpolation;
    TrackQuat();
    float GetStartTime();
    float GetEndTime();
    quat Sample(float time, bool looping);
    FrameQuat& operator[](unsigned int index);
private:
    quat SampleConstant(float time, bool looping);
    quat SampleLinear(float time, bool looping);
    quat SampleCubic(float time, bool looping);
    quat Hermite(float time, const quat &p1, const quat &s1, const quat &p2, const quat &s2);
    int FrameIndex(float time, bool looping);
    float AdjustTimeToFitTrack(float time, float looping);
};

#endif
