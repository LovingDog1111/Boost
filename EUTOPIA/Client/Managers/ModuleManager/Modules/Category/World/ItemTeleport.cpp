#include "ItemTeleport.h"

ItemTeleport::ItemTeleport()
    : Module("ItemTeleport", "Teleport to items", Category::WORLD), range(100.f) {
    registerSetting(new SliderSetting<float>("Range", "Scan range", &range, 100.f, 1.f, 200.f));
}

void ItemTeleport::onEnable() {
    GI::DisplayClientMessage("[ItemTeleport] Enabled");
}

void ItemTeleport::onDisable() {
    GI::DisplayClientMessage("[ItemTeleport] Disabled");
}

void ItemTeleport::onNormalTick(LocalPlayer* player) {
    if(!player || !player->isAlive()) {
        return;
    }

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region) {
        return;
    }

    const float itemScanRange = range;
    Vec3<float> closestItemPos(0, 0, 0);
    float closestDist = FLT_MAX;
    Vec3<float> playerPrevPos = player->getPos();

    for(auto& entity : player->level->getRuntimeActorList()) {
        ActorTypeComponent* actorTypeComponent = entity->getActorTypeComponent();
        if(actorTypeComponent) {
            uint32_t entityId = static_cast<uint32_t>(actorTypeComponent->id);
            if(entityId == 64) {
                Vec3<float> itemPos = entity->getPos();
                float dist = calculateDistance(player->getPos(), itemPos);

                if(dist <= itemScanRange && dist < closestDist) {
                    closestDist = dist;
                    closestItemPos = itemPos;
                }
            }
        }
    }

    if(closestDist == FLT_MAX) {
        GI::DisplayClientMessage("[ItemTeleport] No items found in range");
        this->setEnabled(false);
        return;
    }

    GI::DisplayClientMessage("[ItemTeleport] Moving to closest item...");
    teleportToItem(player, closestItemPos,
                   playerPrevPos);
    this->setEnabled(false);
}

float ItemTeleport::calculateDistance(const Vec3<float>& p1, const Vec3<float>& p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2) +
                     std::pow(p2.z - p1.z, 2));
}

float ItemTeleport::calculateLength(const Vec3<float>& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

void ItemTeleport::teleportToItem(LocalPlayer* player, const Vec3<float>& itemPos,
                                  const Vec3<float>& prevPos) {
    Vec3<float> delta = itemPos.sub(player->getPos());
    float dist = calculateLength(delta);
    if(dist > 0) {
        Vec3<float> movement = delta * (1.5f / dist);
        player->lerpMotion(movement);

        AABB a = player->getAABB(true);
        a.lower = a.lower.add(movement);
        a.upper = a.upper.add(movement);
        player->setAABB(a);

        GI::DisplayClientMessage("[ItemTeleport] Teleported to item");
        teleportBack(player, prevPos);  
    }
}

void ItemTeleport::teleportBack(LocalPlayer* player, const Vec3<float>& prevPos) {
    Vec3<float> deltaBack = prevPos.sub(player->getPos());
    float distBack = calculateLength(deltaBack);
    if(distBack > 0) {
        Vec3<float> movementBack = deltaBack * (1.5f / distBack);
        player->lerpMotion(movementBack);

        AABB a = player->getAABB(true);
        a.lower = a.lower.add(movementBack);
        a.upper = a.upper.add(movementBack);
        player->setAABB(a);

        GI::DisplayClientMessage("[ItemTeleport] Teleported back to previous position");
    }
}
