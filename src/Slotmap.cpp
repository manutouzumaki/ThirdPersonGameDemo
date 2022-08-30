#include "Slotmap.h"
#include <memory.h>
#include <assert.h>

static const unsigned int MAX_UNSIGNED_INT = 0xFFFFFFFF;

void TransformSlotmap::Initialize() {
    mCount = 0;
    mGeneration = 0;
    mFreeList = 0;
    for(unsigned int i = 0; i < 100; ++i) {
        SlotmapKey *indice = mIndices + i;
        indice->mId = i + 1;
        indice->mGen = MAX_UNSIGNED_INT;
    }
    memset(mData, 100, sizeof(TransformComponent));
    memset(mErase, 100, sizeof(unsigned int));
}

SlotmapKey TransformSlotmap::AddComponent(TransformComponent component) {
    assert(mCount < 100);
    assert(mGeneration < MAX_UNSIGNED_INT);
    assert(mFreeList < 100);
    unsigned int index = mFreeList;
    SlotmapKey *key = mIndices + index;
    mFreeList = key->mId;
    key->mId = mCount;
    key->mGen = mGeneration;
    mData[key->mId] = component;
    mErase[key->mId] = index;
    SlotmapKey outKey = {index, mGeneration};
    mGeneration++;
    mCount++;
    return outKey;
}

void TransformSlotmap::RemoveComponent(SlotmapKey key) {
    SlotmapKey internalKey = mIndices[key.mId];
    if(internalKey.mGen != key.mGen) {
        assert(!"try to delete a entity already gone");
    }
    
    mIndices[key.mId].mId = mFreeList;
    mIndices[key.mId].mGen = MAX_UNSIGNED_INT;
    mFreeList = key.mId;

    // TODO: now we need to delete the actual data
    unsigned int dataIndex = internalKey.mId;
    
    if(dataIndex != mCount - 1) {
        // delete the element
        mData[dataIndex] = mData[mCount - 1];
        mErase[dataIndex] = mErase[mCount - 1]; 
        // update the modify element
        mIndices[mErase[dataIndex]].mId = dataIndex;
    }
    --mCount;
}

TransformComponent TransformSlotmap::GetComponent(SlotmapKey key) {
    SlotmapKey indice = mIndices[key.mId];
    if(indice.mGen != key.mGen) {
        assert(!"ERROR wrong element!!!");
    }
    return mData[indice.mId];
}
