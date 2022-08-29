#include "Mesh.h"
#include <glad/glad.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cgltf.h>
#include <memory>
#include <vector>

#include "Vec3.h"
#include "Vec2.h"
#include "Vec4.h"
#include "Transform.h"

#define ArrayCount(array) (sizeof(array)/sizeof((array)[0]))

struct StaticVertex {
    vec3 mPosition;
    vec3 mNormal;
    vec2 mTexcoord;
};

struct AnimVertex {
    vec3 mPosition;
    vec3 mNormal;
    vec2 mTexcoord;
    vec4 mWeights;
    ivec4 mJoints;
};

cgltf_data *LoadGLTFFile(const char *path) {
    cgltf_options options;
    memset(&options, 0, sizeof(cgltf_options));
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);
    if(result != cgltf_result_success) {
        std::cout << "Could not load: " << path << "\n";
        return 0;
    }
    result =   cgltf_load_buffers(&options, data, path);
    if(result != cgltf_result_success) {
        cgltf_free(data);
        std::cout << "Could not load: " << path << "\n";
        return 0;
    }
    result = cgltf_validate(data);
    if(result != cgltf_result_success) {
        cgltf_free(data);
        std::cout << "Invalid file: " << path << "\n";
        return 0;
    }
    return data;
}

void FreeGLTFFile(cgltf_data *data) {
    if(data == 0) {
        std::cout << "WARNING: Can't free null data\n";
    }
    else {
        cgltf_free(data);
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

void Mesh::InitializeStatic(cgltf_data *data) {
    std::vector<StaticVertex> vertices;
    vertices.resize(data->accessors[0].count);

    std::vector<unsigned int> indices;

    cgltf_node *nodes = data->nodes;
    unsigned int nodeCount = (unsigned int)data->nodes_count;
    for(unsigned int index = 0; index < nodeCount; ++index) {
        cgltf_node *node = nodes + index;
        if(node->mesh == 0) {
            continue;
        }
    
        cgltf_primitive *primitives = node->mesh->primitives;
        unsigned int primitiveCount = (unsigned int)node->mesh->primitives_count; 
        for(unsigned int j = 0; j < primitiveCount; ++j) {
            cgltf_primitive *primitive = primitives + j;
            
            // TODO: load the vertex
            cgltf_attribute *attributes = primitive->attributes;
            unsigned int attributeCount = (unsigned int)primitive->attributes_count;
            for(unsigned int k = 0; k < attributeCount; ++k) {
                cgltf_attribute *attribute = attributes + k; 
                cgltf_accessor *accessor = attribute->data;
                if(attribute->type == cgltf_attribute_type_position) {
                    for(cgltf_size i = 0; i < accessor->count; ++i) {
                        vec3 position;
                        cgltf_accessor_read_float(accessor, i, position.v, 3);
                        vertices[i].mPosition = position;
                    }
                }
                if(attribute->type == cgltf_attribute_type_normal) {
                    for(cgltf_size i = 0; i < accessor->count; ++i) {
                        vec3 normal;
                        cgltf_accessor_read_float(accessor, i, normal.v, 3);
                        if(lenSq(normal) < VEC3_EPSILON) {
                            normal = vec3(0, 1, 0);
                        }
                        vertices[i].mNormal = normalized(normal);

                    }
                }
                if(attribute->type == cgltf_attribute_type_texcoord) {
                    for(cgltf_size i = 0; i < accessor->count; ++i) {
                        vec2 texcoord;
                        cgltf_accessor_read_float(accessor, i, texcoord.v, 2);
                        vertices[i].mTexcoord = texcoord;
                    }
                }
            }

            // TODO: load indices
            if(primitive->indices != 0) {
                unsigned int indexCount = (unsigned int)primitive->indices->count;
                indices.resize(indexCount);
                for(unsigned int k = 0; k < indexCount; ++k) {
                    indices[k] = (unsigned int)cgltf_accessor_read_index(primitive->indices, k);
                }

            }
        }
    }

    mVerticesCount = (int)vertices.size();
    mIndicesCount = (int)indices.size();

    glGenVertexArrays(1, &mVao);
    glGenBuffers(1, &mVbo);
    glGenBuffers(1, &mEbo);
    
    glBindVertexArray(mVao);

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(StaticVertex) * mVerticesCount, &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mIndicesCount, &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(StaticVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(StaticVertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(StaticVertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Mesh::InitializeAnimated(cgltf_data *data) { 
    std::vector<AnimVertex> vertices;
    vertices.resize(data->accessors[0].count);

    std::vector<unsigned int> indices;

    cgltf_node *nodes = data->nodes;
    unsigned int nodeCount = (unsigned int)data->nodes_count;
    for(unsigned int index = 0; index < nodeCount; ++index) {
        cgltf_node *node = nodes + index;
        if(node->mesh == 0 || node->skin == 0) {
            continue;
        }
        cgltf_skin *skin = node->skin; 
        cgltf_primitive *primitives = node->mesh->primitives;
        unsigned int primitiveCount = (unsigned int)node->mesh->primitives_count; 
        for(unsigned int j = 0; j < primitiveCount; ++j) {
            cgltf_primitive *primitive = primitives + j;
            
            // TODO: load the vertex
            cgltf_attribute *attributes = primitive->attributes;
            unsigned int attributeCount = (unsigned int)primitive->attributes_count;
            for(unsigned int k = 0; k < attributeCount; ++k) {
                cgltf_attribute *attribute = attributes + k; 
                cgltf_accessor *accessor = attribute->data;
                if(attribute->type == cgltf_attribute_type_position) {
                    for(cgltf_size i = 0; i < accessor->count; ++i) {
                        vec3 position;
                        cgltf_accessor_read_float(accessor, i, position.v, 3);
                        vertices[i].mPosition = position;
                    }
                }
                if(attribute->type == cgltf_attribute_type_normal) {
                    for(cgltf_size i = 0; i < accessor->count; ++i) {
                        vec3 normal;
                        cgltf_accessor_read_float(accessor, i, normal.v, 3);
                        if(lenSq(normal) < VEC3_EPSILON) {
                            normal = vec3(0, 1, 0);
                        }
                        vertices[i].mNormal = normalized(normal);

                    }
                }
                if(attribute->type == cgltf_attribute_type_texcoord) {
                    for(cgltf_size i = 0; i < accessor->count; ++i) {
                        vec2 texcoord;
                        cgltf_accessor_read_float(accessor, i, texcoord.v, 2);
                        vertices[i].mTexcoord = texcoord;
                    }
                }
                if(attribute->type == cgltf_attribute_type_joints) {
                    for(cgltf_size i = 0; i < accessor->count; ++i) {
                        vec4 tmpJoints;
                        cgltf_accessor_read_float(accessor, i, tmpJoints.v, 4);
                        ivec4 joints(
                            (int)(tmpJoints.x + 0.5f),        
                            (int)(tmpJoints.y + 0.5f),        
                            (int)(tmpJoints.z + 0.5f),        
                            (int)(tmpJoints.w + 0.5f)
                        );
                        joints.x = std::max(0, GetNodeIndex(skin->joints[joints.x], nodes, nodeCount));
                        joints.y = std::max(0, GetNodeIndex(skin->joints[joints.y], nodes, nodeCount));
                        joints.z = std::max(0, GetNodeIndex(skin->joints[joints.z], nodes, nodeCount));
                        joints.w = std::max(0, GetNodeIndex(skin->joints[joints.w], nodes, nodeCount)); 
                        vertices[i].mJoints = joints;
                    }
                }
                if(attribute->type == cgltf_attribute_type_weights) {
                    for(cgltf_size i = 0; i < accessor->count; ++i) {
                        vec4 weights;
                        cgltf_accessor_read_float(accessor, i, weights.v, 4);
                        vertices[i].mWeights = weights;
                    }
                }
            }

            // TODO: load indices
            if(primitive->indices != 0) {
                unsigned int indexCount = (unsigned int)primitive->indices->count;
                indices.resize(indexCount);
                for(unsigned int k = 0; k < indexCount; ++k) {
                    indices[k] = (unsigned int)cgltf_accessor_read_index(primitive->indices, k);
                }

            }
        }
    }

    mVerticesCount = (int)vertices.size();
    mIndicesCount = (int)indices.size();

    glGenVertexArrays(1, &mVao);
    glGenBuffers(1, &mVbo);
    glGenBuffers(1, &mEbo);
    
    glBindVertexArray(mVao);

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(AnimVertex) * mVerticesCount, &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mIndicesCount, &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void *)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(4, 4, GL_INT, sizeof(AnimVertex), (void*)(12 * sizeof(float)));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::InitializeCube() {
    StaticVertex vertices[] = {
        {{-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
                                                            
        {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f},  {0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f},  {0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f},  {0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f},  {0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
                                                            
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
                                                            
        {{0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{0.5f,  0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
                                                            
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
                                                            
        {{-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}}
    };
    
    mVerticesCount = ArrayCount(vertices);
    mIndicesCount = 0;

    glGenVertexArrays(1, &mVao);
    glGenBuffers(1, &mVbo);

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(StaticVertex) * mVerticesCount, vertices, GL_STATIC_DRAW);

    glBindVertexArray(mVao);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(StaticVertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(StaticVertex), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(StaticVertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}

void Mesh::InitializeSphere() {

}

void Mesh::Shutdown() {
    if(glIsVertexArray(mVao)) {
        glDeleteVertexArrays(1, &mVao);
    }
    if(glIsBuffer(mVbo)) {
        glDeleteBuffers(1, &mVbo);
    }
    if(glIsBuffer(mEbo)) {
        glDeleteBuffers(1, &mEbo);
    }
}

void Mesh::Bind() {
    glBindVertexArray(mVao);
}

void Mesh::Unbind() {
    glBindVertexArray(0);
}

