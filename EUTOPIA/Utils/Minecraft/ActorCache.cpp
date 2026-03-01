// performance

#include "ActorCache.h"

#include <chrono>

#include "..\Utils\ColorUtil.h"
#include "ActorUtils.h"

#define NOW                                                           \
    std::chrono::duration_cast<std::chrono::milliseconds>(            \
        std::chrono::high_resolution_clock::now().time_since_epoch()) \
        .count()

ActorCache* ActorCache::get() {
    if(!gActorCache)
        gActorCache = new ActorCache();
    return gActorCache;
}

std::vector<Actor*> ActorCache::getPlayers() {
    std::lock_guard<std::mutex> lock(mCacheMutex);

    uint64_t currentTime = NOW;

    if(mPlayersDirty || currentTime - mLastUpdateTime > 1) {
        mCachedPlayers = ActorUtils::getActorList(true);
        mPlayersDirty = false;
        mLastUpdateTime = currentTime;
    }

    return mCachedPlayers;
}

std::vector<Actor*> ActorCache::getAll() {
    std::lock_guard<std::mutex> lock(mCacheMutex);

    uint64_t currentTime = NOW;

    if(mAllDirty || currentTime - mLastUpdateTime > 1) {
        mCachedAll = ActorUtils::getActorList(false);
        mAllDirty = false;
        mLastUpdateTime = currentTime;
    }

    return mCachedAll;
}

void ActorCache::update() {
    std::lock_guard<std::mutex> lock(mCacheMutex);

    uint64_t currentTime = NOW;
    mCachedPlayers = ActorUtils::getActorList(true);
    mCachedAll = ActorUtils::getActorList(false);
    mPlayersDirty = false;
    mAllDirty = false;
    mLastUpdateTime = currentTime;
}

void ActorCache::invalidate() {
    std::lock_guard<std::mutex> lock(mCacheMutex);
    mPlayersDirty = true;
    mAllDirty = true;
}

void ActorCache::clear() {
    std::lock_guard<std::mutex> lock(mCacheMutex);
    mCachedPlayers.clear();
    mCachedAll.clear();
    mPlayersDirty = true;
    mAllDirty = true;
}