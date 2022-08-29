#ifndef _POSE_H_
#define _POSE_H_

#include <vector>
#include <cgltf.h>
#include "Transform.h"

struct Pose {
    std::vector<Transform> mJoints;
    std::vector<int> mParents;
    
	Pose();
	Pose(const Pose& p);
	Pose& operator=(const Pose& p);
	Pose(unsigned int numJoints);
	void Resize(unsigned int size);
	unsigned int Size();
	Transform GetLocalTransform(unsigned int index);
	void SetLocalTransform(unsigned int index, const Transform& transform);
	Transform GetGlobalTransform(unsigned int index);
	Transform operator[](unsigned int index);
	void GetMatrixPalette(std::vector<mat4>& out);
	int GetParent(unsigned int index);
	void SetParent(unsigned int index, int parent);

	bool operator==(const Pose& other);
	bool operator!=(const Pose& other);
};

Pose LoadRestPose(cgltf_data *data);
Pose LoadBindPose(cgltf_data *data);

#endif
