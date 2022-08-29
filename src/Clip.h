#ifndef _CLIP_H_
#define _CLIP_H_

#include <vector>
#include <string>
#include <cgltf.h>

#include "TransformTrack.h"
#include "Pose.h"

struct Clip {
    std::vector<TransformTrack> mTracks;
    std::string mName;
    float mStartTime;
    float mEndTime;
    bool mLooping;

	Clip();
	unsigned int GetIdAtIndex(unsigned int index);
	void SetIdAtIndex(unsigned int index, unsigned int id);
	float Sample(Pose& outPose, float inTime);
	TransformTrack& operator[](unsigned int joint);
	void RecalculateDuration();
	float GetDuration();
private:
    float AdjustTimeToFitRange(float inTime);

};

std::vector<Clip> LoadClips(cgltf_data *data);

#endif
