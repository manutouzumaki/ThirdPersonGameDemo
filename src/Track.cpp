#include "Track.h"

////////////////////////////////////////////////////////////////////////
// TRACK HELPERS ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

namespace TrackHelpers {
    inline float Interpolate(float a, float b, float t) {
        return a + (b - a) * t;
    }

    inline vec3 Interpolate(const vec3& a, const vec3& b, float t) {
        return lerp(a, b, t);
    }

    inline quat Interpolate(const quat& a, const quat& b, float t) {
        quat result = mix(a, b, t);
        if(dot(a, b) < 0) { // Neighborhood
            result = mix(a, -b, t);
        }
        return normalized(result); // nlerp not slerp
    }
    
    inline quat AdjustHermiteResult(quat q) {
        return normalized(q);
    }
    
    inline void Neighborhood(const quat& a, quat& b) {
        if(dot(a, b) < 0) {
            b = -b;
        }
    }
};

////////////////////////////////////////////////////////////////////////
// SCALAR TRACK ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

TrackScalar::TrackScalar() {
    mInterpolation = INTERPOLATION_LINEAR;
}

float TrackScalar::GetStartTime() {
    return mFrames[0].mTime;
}

float TrackScalar::GetEndTime() {
    return mFrames[mFrames.size() - 1].mTime;
}

float TrackScalar::Sample(float time, bool looping) {
    if(mInterpolation == INTERPOLATION_CONSTANT) {
        return SampleConstant(time, looping);
    }
    else if(mInterpolation == INTERPOLATION_LINEAR) {
        return SampleLinear(time, looping);
    }
    return SampleCubic(time, looping);
}

FrameScalar& TrackScalar::operator[](unsigned int index) {
    return mFrames[index];
}

float TrackScalar::SampleConstant(float time, bool looping) {
    int frame = FrameIndex(time, looping);
    if(frame < 0 || frame >= (int)mFrames.size()) {
        return float();
    }
    return mFrames[frame].mValue;
}

float TrackScalar::SampleLinear(float time, bool looping) {
    int frame = FrameIndex(time, looping);
    if(frame < 0 || frame >= (int)mFrames.size() - 1) {
        return float();
    }
    int nextFrame = frame + 1;
    float trackTime = AdjustTimeToFitTrack(time, looping);
    float thisTime = mFrames[frame].mTime;
    float frameDelta = mFrames[nextFrame].mTime - thisTime;

    if(frameDelta <= 0.0f) {
        return float();
    }

    float t = (trackTime - thisTime) / frameDelta;
    float start = mFrames[frame].mValue;
    float end = mFrames[nextFrame].mValue;

    return TrackHelpers::Interpolate(start, end, t);
}

float TrackScalar::SampleCubic(float time, bool looping) {
    int frame = FrameIndex(time, looping);
    if(frame < 0 || frame >= (int)mFrames.size() - 1) {
        return float();
    }
    int nextFrame = frame + 1;
    float trackTime = AdjustTimeToFitTrack(time, looping);
    float thisTime = mFrames[frame].mTime;
    float frameDelta = mFrames[nextFrame].mTime - thisTime;

    if(frameDelta <= 0.0f) {
        return float();
    }

    float t = (trackTime - thisTime) / frameDelta;
    float p1 = mFrames[frame].mValue;
    float s1 = mFrames[frame].mOut * frameDelta;
    float p2 = mFrames[nextFrame].mValue;
    float s2 = mFrames[nextFrame].mIn * frameDelta;

    return Hermite(t, p1, s1, p2, s2);
}

float TrackScalar::Hermite(float t, const float &p1, const float &s1, const float &p2, const float &s2) {
    float tt = t * t;
    float ttt = tt * t;
    
    float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
	float h2 = -2.0f * ttt + 3.0f * tt;
	float h3 = ttt - 2.0f * tt + t;
	float h4 = ttt - tt;

	return p1 * h1 + p2 * h2 + s1 * h3 + s2 * h4;
}

int TrackScalar::FrameIndex(float time, bool looping) {
    unsigned int size = (unsigned int)mFrames.size();
    if(size <= 1) {
        return -1;
    }
    if(looping) {
        float startTime = mFrames[0].mTime; 
        float endTime = mFrames[size - 1].mTime; 
        time = fmodf(time - startTime, endTime - startTime);
        if(time < 0.0f) {
            time += endTime - startTime;
        }
        time += startTime;
    }
    else {
        if(time <= mFrames[0].mTime) {
            return 0;
        }
        if(time >= mFrames[size - 2].mTime) {
            return (int)size - 2;
        }
    }
    for(int i = (int)size - 1; i >= 0; --i) {
        if(time >= mFrames[i].mTime) {
            return i;
        } 
    }
    return -1;
}

float TrackScalar::AdjustTimeToFitTrack(float time, float looping) {
    unsigned int size = (unsigned int)mFrames.size();
    if(size <= 1) {
        return 0.0f;
    }
    float startTime = mFrames[0].mTime;
    float endTime = mFrames[size - 1].mTime;
    float duration = endTime - startTime;
    if(duration <= 0.0f) {
        return 0.0f;
    }
    if(looping) {
        time = fmodf(time - startTime, endTime - startTime);
        if(time < 0.0f) {
            time += endTime - startTime;
        }
        time = time + startTime;
    }
    else {
        if(time < mFrames[0].mTime) {
            time = startTime;
        }
        if(time >= mFrames[size - 1].mTime) {
            time = endTime;
        }
    }
    return time;
}

////////////////////////////////////////////////////////////////////////
// VEC3 TRACK //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

TrackVec3::TrackVec3() {
    mInterpolation = INTERPOLATION_LINEAR;
}

float TrackVec3::GetStartTime() {
    return mFrames[0].mTime;
}

float TrackVec3::GetEndTime() {
    return mFrames[mFrames.size() - 1].mTime;
}

vec3 TrackVec3::Sample(float time, bool looping) {
    if(mInterpolation == INTERPOLATION_CONSTANT) {
        return SampleConstant(time, looping);
    }
    else if(mInterpolation == INTERPOLATION_LINEAR) {
        return SampleLinear(time, looping);
    }
    return SampleCubic(time, looping);
}
FrameVec3& TrackVec3::operator[](unsigned int index) {
    return mFrames[index];
}
vec3 TrackVec3::SampleConstant(float time, bool looping) {
    int frame = FrameIndex(time, looping);
    if(frame < 0 || frame >= (int)mFrames.size() - 1) {
        return vec3();
    }
    return mFrames[frame].mValue;
}
vec3 TrackVec3::SampleLinear(float time, bool looping) {
    int frame = FrameIndex(time, looping);
    if(frame < 0 || frame >= (int)mFrames.size() - 1) {
        return vec3();
    }
    int nextFrame = frame + 1;
    float trackTime = AdjustTimeToFitTrack(time, looping);
    float thisTime = mFrames[frame].mTime;
    float frameDelta = mFrames[nextFrame].mTime - thisTime;
    
    if(frameDelta <= 0.0f) {
        return vec3();
    }

    float t = (trackTime - thisTime) / frameDelta;
    vec3 start = mFrames[frame].mValue;
    vec3 end = mFrames[nextFrame].mValue;

    return TrackHelpers::Interpolate(start, end, t);
}
vec3 TrackVec3::SampleCubic(float time, bool looping) {
    int frame = FrameIndex(time, looping);
    if(frame < 0 || frame >= (int)mFrames.size() - 1) {
        return vec3();
    }
    int nextFrame = frame + 1;
    float trackTime = AdjustTimeToFitTrack(time, looping);
    float thisTime = mFrames[frame].mTime;
    float frameDelta = mFrames[nextFrame].mTime - thisTime;

    if(frameDelta <= 0.0f) {
        return vec3();
    }

    float t = (trackTime - thisTime) / frameDelta;
    vec3 p1 = mFrames[frame].mValue;
    vec3 s1 = mFrames[frame].mOut * frameDelta;
    vec3 p2 = mFrames[nextFrame].mValue;
    vec3 s2 = mFrames[nextFrame].mIn * frameDelta;

    return Hermite(t, p1, s1, p2, s2);
}
vec3 TrackVec3::Hermite(float t, const vec3 &p1, const vec3 &s1, const vec3 &p2, const vec3 &s2) {
    float tt = t * t;
    float ttt = tt * t;
    
    float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
	float h2 = -2.0f * ttt + 3.0f * tt;
	float h3 = ttt - 2.0f * tt + t;
	float h4 = ttt - tt;

	return p1 * h1 + p2 * h2 + s1 * h3 + s2 * h4;
}

int TrackVec3::FrameIndex(float time, bool looping) {
    unsigned int size = (unsigned int)mFrames.size();
    if(size <= 1) {
        return -1;
    }
    if(looping) {
        float startTime = mFrames[0].mTime;
        float endTime = mFrames[size - 1].mTime; 
        time = fmodf(time - startTime, endTime - startTime);
        if(time < 0.0f) {
            time += endTime - startTime;
        }
        time += startTime;
    }
    else {
        if(time < mFrames[0].mTime) {
            return 0;
        }
        if(time > mFrames[size - 2].mTime) {
            return size - 2;
        }
    }
    for(int i = (int)size - 1; i >= 0; --i) {
        if(time >= mFrames[i].mTime) {
            return i;
        }
    }
    return -1;
}

float TrackVec3::AdjustTimeToFitTrack(float time, float looping) {
    unsigned int size = (unsigned int)mFrames.size();
    if(size <= 1) {
        return -1;
    }
    float startTime = mFrames[0].mTime;
    float endTime = mFrames[size - 1].mTime;
    float duration = endTime - startTime;
    if(duration <= 0.0f) {
        return 0.0f;
    }
    if(looping) {
        time = fmodf(time - startTime, endTime - startTime);
        if(time < 0.0f) {
            time += endTime - startTime;
        }
        time += startTime;

    }
    else {
        if(time < mFrames[0].mTime) {
            time = startTime;
        }
        if(time >= mFrames[size - 1].mTime) {
            time = endTime; 
        }
    }
    return time;
}


////////////////////////////////////////////////////////////////////////
// QUAT TRACK //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

TrackQuat::TrackQuat() {
    mInterpolation = INTERPOLATION_LINEAR;
}

float TrackQuat::GetStartTime() {
    return mFrames[0].mTime;
}
float TrackQuat::GetEndTime() {
    return mFrames[mFrames.size() - 1].mTime;
}

quat TrackQuat::Sample(float time, bool looping) {
    if(mInterpolation == INTERPOLATION_CONSTANT) {
        return SampleConstant(time, looping);
    }
    else if(mInterpolation == INTERPOLATION_LINEAR) {
        return SampleLinear(time, looping);
    }
    return SampleCubic(time, looping);
}

FrameQuat& TrackQuat::operator[](unsigned int index) {
    return mFrames[index];
}

quat TrackQuat::SampleConstant(float time, bool looping) {
    int frame = FrameIndex(time, looping);
    if(frame < 0 || frame >= (int)mFrames.size() - 1) {
        return quat();
    }
    return mFrames[frame].mValue;
}

quat TrackQuat::SampleLinear(float time, bool looping) {
    int frame = FrameIndex(time, looping);
    if(frame < 0 || frame >= (int)mFrames.size() - 1) {
        return quat();
    }

    int nextFrame = frame + 1;
    float trackTime = AdjustTimeToFitTrack(time, looping);
    float thisTime = mFrames[frame].mTime;
    float frameDelta = mFrames[nextFrame].mTime - thisTime;

    if(frameDelta <= 0.0f) {
        return quat();
    }

    float t = (trackTime - thisTime) / frameDelta;
    quat start = mFrames[frame].mValue;
    quat end = mFrames[nextFrame].mValue;

    return TrackHelpers::Interpolate(start, end, t);


}

quat TrackQuat::SampleCubic(float time, bool looping) {
    int frame = FrameIndex(time, looping);
    if(frame < 0 || frame >= (int)mFrames.size() - 1) {
        return quat();
    }

    int nextFrame = frame + 1;
    float trackTime = AdjustTimeToFitTrack(time, looping);
    float thisTime = mFrames[frame].mTime;
    float frameDelta = mFrames[nextFrame].mTime - thisTime;

    if(frameDelta <= 0.0f) {
        return quat();
    }

    float t = (trackTime - thisTime) / frameDelta;

    quat p1 = normalized(mFrames[frame].mValue);
    quat s1 = mFrames[frame].mOut * frameDelta;
    quat p2 = normalized(mFrames[nextFrame].mValue);
    quat s2 = mFrames[nextFrame].mIn * frameDelta;

    return Hermite(t, p1, s1, p2, s2);
}

quat TrackQuat::Hermite(float t, const quat &p1, const quat &s1, const quat &_p2, const quat &s2) {
    float tt = t * t;
    float ttt = tt * t;

    quat p2 = _p2;
    TrackHelpers::Neighborhood(p1, p2);
    
    float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
	float h2 = -2.0f * ttt + 3.0f * tt;
	float h3 = ttt - 2.0f * tt + t;
	float h4 = ttt - tt;

	quat result = p1 * h1 + p2 * h2 + s1 * h3 + s2 * h4;
    return TrackHelpers::AdjustHermiteResult(result);
}

int TrackQuat::FrameIndex(float time, bool looping) {
    unsigned int size = (unsigned int)mFrames.size();
    if(size <= 1) {
        return -1;
    }
    if(looping) {
        float startTime = mFrames[0].mTime;
        float endTime = mFrames[size - 1].mTime; 
        time = fmodf(time - startTime, endTime - startTime);
        if(time < 0.0f) {
            time += endTime - startTime;
        }
        time += startTime;
    }
    else {
        if(time < mFrames[0].mTime) {
            return 0;
        }
        if(time > mFrames[size - 2].mTime) {
            return size - 2;
        }
    }
    for(int i = (int)size - 1; i >= 0; --i) {
        if(time >= mFrames[i].mTime) {
            return i;
        }
    }
    return -1;
}
float TrackQuat::AdjustTimeToFitTrack(float time, float looping) {
    unsigned int size = (unsigned int)mFrames.size();
    if(size <= 1) {
        return -1;
    }
    float startTime = mFrames[0].mTime;
    float endTime = mFrames[size - 1].mTime;
    float duration = endTime - startTime;
    if(duration <= 0.0f) {
        return 0.0f;
    }
    if(looping) {
        time = fmodf(time - startTime, endTime - startTime);
        if(time < 0.0f) {
            time += endTime - startTime;
        }
        time += startTime;

    }
    else {
        if(time < mFrames[0].mTime) {
            time = startTime;
        }
        if(time >= mFrames[size - 1].mTime) {
            time = endTime; 
        }
    }
    return time;
}
