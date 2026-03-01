#pragma once


#include <memory>
#include <mutex>
#include <vector>

#include "..\SDK\World\Actor\Actor.h"

class ActorCache {
   private:
    std::vector<Actor*> mCachedPlayers;
    std::vector<Actor*> mCachedAll;
    uint64_t mLastUpdateTime = 0;
    std::mutex mCacheMutex;

    bool mPlayersDirty = true;
    bool mAllDirty = true;

   public:
    static ActorCache* get();

    std::vector<Actor*> getPlayers();

    std::vector<Actor*> getAll();

    void update();

    void invalidate();

    void clear();
};

inline ActorCache* gActorCache = nullptr;