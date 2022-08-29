#include "Clip.h"

Clip::Clip() {
    mName = "No Name";
    mStartTime = 0.0f;
    mEndTime = 0.0f;
    mLooping = false;
}

unsigned int Clip::GetIdAtIndex(unsigned int index) {
    return mTracks[index].mId;
}

void Clip::SetIdAtIndex(unsigned int index, unsigned int id) {
    mTracks[index].mId = id;
}

float Clip::Sample(Pose& outPose, float inTime) {
    if(GetDuration() == 0.0f) {
        return 0.0f;
    }
    inTime = AdjustTimeToFitRange(inTime);

    unsigned int trackSize = (unsigned int)mTracks.size();
    for(unsigned int i = 0; i < trackSize; ++i) {
        unsigned int joint = mTracks[i].mId;
        Transform local = outPose.GetLocalTransform(joint);
        Transform animated = mTracks[i].Sample(local, inTime, mLooping);
        outPose.SetLocalTransform(joint, animated);
    }
    return inTime;
}

TransformTrack& Clip::operator[](unsigned int joint) {
    unsigned int size = (unsigned int)mTracks.size();
    for(unsigned int i = 0; i < size; ++i) {
        if(mTracks[i].mId == joint) {
            return mTracks[i];
        }
    }
    mTracks.push_back(TransformTrack());
    mTracks[mTracks.size() - 1].mId = joint;
    return mTracks[mTracks.size() - 1];
}

void Clip::RecalculateDuration() {
    mStartTime = 0.0f;
    mEndTime = 0.0f;
    bool startSet = false;
    bool endSet = false;
    unsigned int trackSize = (unsigned int)mTracks.size();
    for(unsigned int i = 0; i < trackSize; ++i) {
        if(mTracks[i].IsValid()) {
            float trackStartTime = mTracks[i].GetStartTime();
            float trackEndTime = mTracks[i].GetEndTime();
            if(trackStartTime < mStartTime || startSet == false) {
                mStartTime = trackStartTime;
                startSet = true;
            }
            if(trackEndTime > mEndTime || endSet == false) {
                mEndTime = trackEndTime;
                endSet = true;
            }
        }
    }
}

float Clip::GetDuration() {
    return mEndTime - mStartTime;
}

float Clip::AdjustTimeToFitRange(float inTime) {
    if(mLooping) {
        float duration = mEndTime - mStartTime;
        if(duration <= 0.0f) {
            return 0.0f;
        }
        inTime = fmodf(inTime - mStartTime, mEndTime - mStartTime);
        if(inTime < 0.0f) {
            inTime += mEndTime - mStartTime;
        }
        inTime += mStartTime;
    }
    else {
        if(inTime < mStartTime) {
           inTime = mStartTime;
        }
        if(inTime > mEndTime) {
           inTime = mEndTime;
        }
    }
    return inTime;
}

static void GetScalarValues(std::vector<float>& out, unsigned int compCount, const cgltf_accessor& inAccessor) {
    out.resize(inAccessor.count * compCount);
    for(cgltf_size i = 0; i < inAccessor.count; ++i) {
        cgltf_accessor_read_float(&inAccessor, i, &out[i * compCount], compCount);
    }
}

static int GetNodeIndex(cgltf_node *target, cgltf_node *nodes, unsigned int nodeCount) {
    if(target == 0) {
        return -1;
    }
    for(unsigned int i = 0; i < nodeCount; ++i) {
        if(target == &nodes[i]) {
            return (int)i;
        }
    }
    return -1;
}

#if 0
static void TrackScalarFromChannel(TrackScalar& inOutTrack, cgltf_animation_channel *inChannel) {
    cgltf_animation_sampler *sampler = inChannel->sampler;
    Interpolation interpolation = INTERPOLATION_CONSTANT;
    if(sampler->interpolation == cgltf_interpolation_type_linear) {
        interpolation = INTERPOLATION_LINEAR;
    }
    else if(sampler->interpolation == cgltf_interpolation_type_cubic_spline) {
        interpolation = INTERPOLATION_CUBIC;
    }
    bool isSamplerCubic = interpolation == INTERPOLATION_CUBIC;
    inOutTrack.mInterpolation = interpolation;

    std::vector<float> timelineFloats;
    GetScalarValues(timelineFloats, 1, *sampler->input);
    std::vector<float> valueFloats;
    GetScalarValues(valueFloats, 1, *sampler->output);

    unsigned int numFrames = (unsigned int)sampler->input->count;
    unsigned int numberOfValuesPerFrame = (unsigned int)(valueFloats.size() / timelineFloats.size());
    inOutTrack.mFrames.resize(numFrames);
    for(unsigned int i = 0; i < numFrames; ++i) {
        int baseIndex = i * numberOfValuesPerFrame;
        FrameScalar& frame = inOutTrack[i];
        int offset = 0;

        frame.mTime = timelineFloats[i];
        frame.mIn = isSamplerCubic ? valueFloats[baseIndex + offset++] : 0.0f;
        frame.mValue = valueFloats[baseIndex + offset++];
        frame.mOut = isSamplerCubic ? valueFloats[baseIndex + offset++] : 0.0f;
    }
}
#endif

static void TrackVec3FromChannel(TrackVec3& inOutTrack, cgltf_animation_channel *inChannel) {
    cgltf_animation_sampler *sampler = inChannel->sampler;
    Interpolation interpolation = INTERPOLATION_CONSTANT;
    if(sampler->interpolation == cgltf_interpolation_type_linear) {
        interpolation = INTERPOLATION_LINEAR;
    }
    else if(sampler->interpolation == cgltf_interpolation_type_cubic_spline) {
        interpolation = INTERPOLATION_CUBIC;
    }
    bool isSamplerCubic = interpolation == INTERPOLATION_CUBIC;
    inOutTrack.mInterpolation = interpolation;

    std::vector<float> timelineFloats;
    GetScalarValues(timelineFloats, 1, *sampler->input);
    std::vector<float> valueFloats;
    GetScalarValues(valueFloats, 3, *sampler->output);

    unsigned int numFrames = (unsigned int)sampler->input->count;
    unsigned int numberOfValuesPerFrame = (unsigned int)(valueFloats.size() / timelineFloats.size());
    inOutTrack.mFrames.resize(numFrames);
    for(unsigned int i = 0; i < numFrames; ++i) {
        int baseIndex = i * numberOfValuesPerFrame;
        FrameVec3& frame = inOutTrack[i];
        int offset = 0;

        frame.mTime = timelineFloats[i];
        for(int component = 0; component < 3; ++component) {
            frame.mIn.v[component] = isSamplerCubic ? valueFloats[baseIndex + offset++] : 0.0f;
        }
        for(int component = 0; component < 3; ++component) {
            frame.mValue.v[component] = valueFloats[baseIndex + offset++];
        }
        for(int component = 0; component < 3; ++component) {
            frame.mOut.v[component] = isSamplerCubic ? valueFloats[baseIndex + offset++] : 0.0f;
        }
    }
}

static void TrackQuatFromChannel(TrackQuat& inOutTrack, cgltf_animation_channel *inChannel) {
    cgltf_animation_sampler *sampler = inChannel->sampler;
    Interpolation interpolation = INTERPOLATION_CONSTANT;
    if(sampler->interpolation == cgltf_interpolation_type_linear) {
        interpolation = INTERPOLATION_LINEAR;
    }
    else if(sampler->interpolation == cgltf_interpolation_type_cubic_spline) {
        interpolation = INTERPOLATION_CUBIC;
    }
    bool isSamplerCubic = interpolation == INTERPOLATION_CUBIC;
    inOutTrack.mInterpolation = interpolation;

    std::vector<float> timelineFloats;
    GetScalarValues(timelineFloats, 1, *sampler->input);
    std::vector<float> valueFloats;
    GetScalarValues(valueFloats, 4, *sampler->output);

    unsigned int numFrames = (unsigned int)sampler->input->count;
    unsigned int numberOfValuesPerFrame = (unsigned int)(valueFloats.size() / timelineFloats.size());
    inOutTrack.mFrames.resize(numFrames);
    for(unsigned int i = 0; i < numFrames; ++i) {
        int baseIndex = i * numberOfValuesPerFrame;
        FrameQuat& frame = inOutTrack[i];
        int offset = 0;

        frame.mTime = timelineFloats[i];
        for(int component = 0; component < 4; ++component) {
            frame.mIn.v[component] = isSamplerCubic ? valueFloats[baseIndex + offset++] : 0.0f;
        }
        for(int component = 0; component < 4; ++component) {
            frame.mValue.v[component] = valueFloats[baseIndex + offset++];
        }
        for(int component = 0; component < 4; ++component) {
            frame.mOut.v[component] = isSamplerCubic ? valueFloats[baseIndex + offset++] : 0.0f;
        }
    }
}


std::vector<Clip> LoadClips(cgltf_data *data) {
    unsigned int numClips = (unsigned int)data->animations_count;
    unsigned int numNodes = (unsigned int)data->nodes_count;

    std::vector<Clip> result;
    result.resize(numClips);

    for(unsigned int i = 0; i < numClips; ++i) {
        result[i].mName = data->animations[i].name;
        unsigned int numChannels = (unsigned int)data->animations[i].channels_count;
        for(unsigned int j = 0; j < numChannels; ++j) {
            cgltf_animation_channel *channel = &data->animations[i].channels[j];
            cgltf_node* target = channel->target_node;
            int nodeId = GetNodeIndex(target, data->nodes, numNodes);
			if (channel->target_path == cgltf_animation_path_type_translation) {
				TrackVec3& track = result[i][nodeId].mPosition;
				TrackVec3FromChannel(track, channel);
			}
			else if (channel->target_path == cgltf_animation_path_type_scale) {
				TrackVec3& track = result[i][nodeId].mScale;
				TrackVec3FromChannel(track, channel);
			}
			else if (channel->target_path == cgltf_animation_path_type_rotation) {
				TrackQuat& track = result[i][nodeId].mRotation;
				TrackQuatFromChannel(track, channel);
			}
        }
        result[i].RecalculateDuration();
    }
    return result;
}
