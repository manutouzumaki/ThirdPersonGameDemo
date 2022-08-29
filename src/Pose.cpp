#include "Pose.h"

Pose::Pose() { }

Pose::Pose(unsigned int numJoints) {
	Resize(numJoints);
}

Pose::Pose(const Pose& p) {
	*this = p;
}

Pose& Pose::operator=(const Pose& p) {
	if (&p == this) {
		return *this;
	}

	if (mParents.size() != p.mParents.size()) {
		mParents.resize(p.mParents.size());
	}
	if (mJoints.size() != p.mJoints.size()) {
		mJoints.resize(p.mJoints.size());
	}

	if (mParents.size() != 0) {
		memcpy(&mParents[0], &p.mParents[0],
			sizeof(int) * mParents.size());
	}
	if (mJoints.size() != 0) {
		memcpy(&mJoints[0], &p.mJoints[0],
			sizeof(Transform) * mJoints.size());
	}

	return *this;
}

void Pose::Resize(unsigned int size) {
	mParents.resize(size);
	mJoints.resize(size);
}

unsigned int Pose::Size() {
	return (unsigned int)mJoints.size();
}

Transform Pose::GetLocalTransform(unsigned int index) {
	return mJoints[index];
}

void Pose::SetLocalTransform(unsigned int index,
	const Transform& transform) {
	mJoints[index] = transform;
}

Transform Pose::GetGlobalTransform(unsigned int index) {
	Transform result = mJoints[index];
	for (int parent = mParents[index]; parent >= 0; parent = mParents[parent]) {
		result = combine(mJoints[parent], result);
	}
	return result;
}

Transform Pose::operator[](unsigned int index) {
	return GetGlobalTransform(index);
}

void Pose::GetMatrixPalette(std::vector<mat4>& out) {
	unsigned int size = Size();
	if (out.size() != size) {
		out.resize(size);
	}

	for (unsigned int i = 0; i < size; ++i) {
		Transform t = GetGlobalTransform(i);
		out[i] = transformToMat4(t);
	}
}

int Pose::GetParent(unsigned int index) {
	return mParents[index];
}

void Pose::SetParent(unsigned int index, int parent) {
	mParents[index] = parent;
}

bool Pose::operator==(const Pose& other) {
	if (mJoints.size() != other.mJoints.size()) {
		return false;
	}
	if (mParents.size() != other.mParents.size()) {
		return false;
	}
	unsigned int size = (unsigned int)mJoints.size();
	for (unsigned int i = 0; i < size; ++i) {
		Transform thisLocal = mJoints[i];
		Transform otherLocal = other.mJoints[i];

		int thisParent = mParents[i];
		int otherParent = other.mParents[i];

		if (thisParent != otherParent) {
			return false;
		}

		if (thisLocal != otherLocal) {
			return false;
		}
	}
	return true;
}

bool Pose::operator!=(const Pose& other) {
	return !(*this == other);
}

static Transform GetLocalTransform(const cgltf_node *node) {
        Transform result;
        if(node->has_matrix) {
            mat4 mat((float *)&node->matrix[0]);
            result = mat4ToTransform(mat);
        }
        if(node->has_translation) {
            result.mPosition = vec3(node->translation[0], node->translation[1], node->translation[2]);
        }
        if(node->has_rotation) {
            result.mRotation = quat(node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]);
        }
        if(node->has_scale) {
            result.mScale = vec3(node->scale[0], node->scale[1], node->scale[2]);
        }
        return result;
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


Pose LoadRestPose(cgltf_data *data) {
    unsigned int boneCount = (unsigned int)data->nodes_count;
    Pose result(boneCount);
    for(unsigned int i = 0; i < boneCount; ++i) {
        cgltf_node* node = &(data->nodes[i]);
        Transform transform = GetLocalTransform(data->nodes + i);
        result.SetLocalTransform(i, transform);
        int parent = GetNodeIndex(node->parent, data->nodes, boneCount);
        result.SetParent(i, parent);
    }
    return result;
}

Pose LoadBindPose(cgltf_data *data) {
    Pose restPose = LoadRestPose(data);
    unsigned int numBones = restPose.Size();
    std::vector<Transform> worldBindPose(numBones);
    for(unsigned int i = 0; i < numBones; ++i) {
        worldBindPose[i] = restPose.GetGlobalTransform(i); 
    }

    unsigned int numSkins = (unsigned int)data->skins_count;
    for(unsigned int i = 0; i < numSkins; ++i) {
        cgltf_skin* skin = &(data->skins[i]);
        std::vector<float> invBindAccessor;

        invBindAccessor.resize(skin->inverse_bind_matrices->count * 16);
        for(cgltf_size j = 0; j < skin->inverse_bind_matrices->count; ++j) {
            cgltf_accessor_read_float(skin->inverse_bind_matrices, j, &invBindAccessor[j * 16], 16);
        }
 
        unsigned int numJoints = (unsigned int)skin->joints_count;
        for(unsigned int j = 0; j < numJoints; ++j) {
            // Read the inverse bind matrix of the joint
            float *matrix = &(invBindAccessor[j * 16]);
            mat4 invBindMatrix = mat4(matrix);
            // invert, convert to transform
            mat4 bindMatrix = inverse(invBindMatrix);
            Transform bindTransform = mat4ToTransform(bindMatrix);
            // Set the transform in the world bind pose
            cgltf_node* jointNode = skin->joints[j];
            int jointIndex = GetNodeIndex(jointNode, data->nodes, numBones);
            worldBindPose[jointIndex] = bindTransform;
        }
    }

    // Convert the world bind pose to a regular bind pose
    Pose bindPose = restPose;
    for(unsigned int i = 0; i < numBones; ++i) {
        Transform current = worldBindPose[i];
        int p = bindPose.GetParent(i);
        if(p >= 0) {  // Bring into parent space
            Transform parent = worldBindPose[p];
            current = combine(inverse(parent), current);
        }
        bindPose.SetLocalTransform(i, current);
    } 
    return bindPose;
}
