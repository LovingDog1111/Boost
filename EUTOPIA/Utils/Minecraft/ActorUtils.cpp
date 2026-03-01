// made by dani

#include "ActorUtils.h"

#include "..\Client\Managers\ModuleManager\ModuleManager.h"
#include <..\SDK\World\Actor\Actor.h>
#include <..\SDK\World\Actor\Components\ActorOwnerComponent.h>
#include <..\SDK\World\Actor\Components\ActorTypeComponent.h>
#include <..\SDK\World\Actor\Components\RuntimeIDComponent.h>
#include "..\SDK\World\Inventory\PlayerInventory.h"
#include "..\SDK\NetWork\MinecraftPacket.h"
#include "..\SDK\NetWork\Packets\InventoryTranscationPacket.h"
#include "..\SDK\World\Level\Level.h"
#include "..\Libs\entt\entt\entity\registry.hpp"
static AntiBot *antibot = nullptr;
#include "..\SDK\World\Actor\Components\RuntimeIDComponent.h"

#include <unordered_set>

static std::unordered_set<decltype(RuntimeIDComponent::mRuntimeID)> g_cachedPlayers;



bool ActorUtils::isBot(Actor *actor) {
    if(!actor)
        return true;

    auto localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return false;

    
    if(actor == localPlayer)
        return false;

   
    if(!actor->isPlayer())
        return true;


    std::string name = actor->getNameTag();
    if(name.empty())
        return true;


    float health = actor->getHealth();
    if(health <= 0.f)
        return true;


  
    if(!actor->getRenderPositionComponent())
        return true;
    if(!actor->getStateVectorComponent())
        return true;
    if(!actor->getAABBShapeComponent())
        return true;

  
    if(!actor->isAlive())
        return true;


    


    if(!actor->getRuntimeIDComponent() || actor->getRuntimeIDComponent()->mRuntimeID <= 0)
        return true;

   

    return false;
}


std::vector<Actor *> ActorUtils::getActorList(bool playerOnly) {
    auto player = GI::getLocalPlayer();
    if(!player)
        return {};

    std::vector<Actor *> actors;

    try {
        auto context = player->getEntityContext();
        if(context) {
            for(auto &&[entId, moduleOwner, type, ridc, aabb] :
                context->registry.registry
                    .view<ActorOwnerComponent, ActorTypeComponent, RuntimeIDComponent,
                          AABBShapeComponent>()
                    .each()) {
                if(!context->registry.registry.valid(entId))
                    continue;

                Actor *actor = moduleOwner.mActor;
                if(!actor)
                    continue;

                if(playerOnly && type.id != ActorType::Player)
                    continue;

                actors.push_back(actor);
            }
        } else {
            auto tactors = player->level->getRuntimeActorList();
            for(auto *actor : tactors) {
                if(!actor)
                    continue;

                if(playerOnly && !actor->isPlayer())
                    continue;

                actors.push_back(actor);
            }
        }
    } catch(...) {
        return {};
    }

    return actors;
}


std::vector<Actor *> ActorUtils::getActorsOfType(ActorType type) {
    auto player = GI::getLocalPlayer();
    if(!player)
        return {};

    std::vector<Actor *> actors;

    try {
        auto context = player->getEntityContext();
        if(context) {
            for(auto &&[_, moduleOwner, typeComponent] :
                context->registry.registry.view<ActorOwnerComponent, ActorTypeComponent>().each()) {
                if(!context->registry.registry.valid(_))
                    continue;

                if(!moduleOwner.mActor)
                    continue;

                if(typeComponent.id == type)
                    actors.push_back(moduleOwner.mActor);
            }
        }
    } catch(...) {
        return {};
    }

    return actors;
}




Actor *ActorUtils::getActorFromUniqueId(const int64_t uniqueId) {
    auto player = GI::getLocalPlayer();
    if(!player)
        return nullptr;

    auto context = player->getEntityContext();
    if(!context)
        return nullptr;

    for(auto &&[_, moduleOwner, ridc, uidc] :
        context->registry.registry
            .view<ActorOwnerComponent, RuntimeIDComponent, ActorUniqueIDComponent>()
            .each()) {
        if(uidc.mUniqueID == uniqueId && moduleOwner.mActor)
            return moduleOwner.mActor;
    }

    return nullptr;
}


Actor *ActorUtils::getActorFromRuntimeID(int64_t runtimeId) {
    auto player = GI::getLocalPlayer();
    if(!player)
        return nullptr;

    auto context = player->getEntityContext();
    if(!context)
        return nullptr;

    for(auto &&[_, moduleOwner, ridc] :
        context->registry.registry.view<ActorOwnerComponent, RuntimeIDComponent>().each()) {
        if(ridc.mRuntimeID == runtimeId && moduleOwner.mActor)
            return moduleOwner.mActor;
    }

    return nullptr;
}
