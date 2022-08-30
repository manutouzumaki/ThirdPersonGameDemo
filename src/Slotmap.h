#ifndef _SLOTMAP_H_
#define _SLOTMAP_H_

// TODO: test for slotmap data structure
#include "Vec3.h"

struct TransformComponent {
    const char *name;
    vec3 mPosition;
};

struct SlotmapKey {
    unsigned int mId;
    unsigned int mGen; 
};

// try for 100 components
struct TransformSlotmap {
    unsigned int mCount;
    unsigned int mGeneration;
    unsigned int mFreeList;

    SlotmapKey mIndices[100];
    TransformComponent mData[100];
    unsigned int mErase[100];
    
    void Initialize();
    SlotmapKey AddComponent(TransformComponent component);
    void RemoveComponent(SlotmapKey key);
    TransformComponent GetComponent(SlotmapKey key);
};


#endif
