#pragma once


#include <complex.h>

#include "..\SDK\World\Actor\ActorType.h"
#include "..\SDK\NetWork\Packets\InventoryTranscationPacket.h"
#include <vector>

class ActorUtils {
   public:
    static std::vector<class Actor*> getActorList(bool playerOnly);
    static std::vector<Actor*> getActorsOfType(ActorType type);
    template <typename T = Actor>
    static std::vector<T*> getActorsTyped(ActorType type) {
        const std::vector<Actor*> actors = getActorsOfType(type);
        std::vector<T*> result;
        for(auto actor : actors) {
            result.push_back(static_cast<T*>(actor));
        }
        return result;
    }
    //static bool isBot(Actor* actor);
    //static std::shared_ptr<InventoryTransactionPacket> createAttackTransaction(Actor* actor,
                                                                             //  int slot = -1);
    static Actor* getActorFromUniqueId(int64_t uniqueId);
    static Actor* getActorFromRuntimeID(int64_t runtimeId);
    static bool isBot(Actor* actor);
};
