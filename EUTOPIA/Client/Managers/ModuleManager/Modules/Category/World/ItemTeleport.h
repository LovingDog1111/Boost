#pragma once
#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"
#include "../Player/PacketMine.h"

class ItemTeleport : public Module {
   public:
    ItemTeleport();
    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* player) override;

   private:
    float range;  
    float calculateDistance(const Vec3<float>& p1, const Vec3<float>& p2);
    void teleportToItem(LocalPlayer* player, const Vec3<float>& itemPos,
                        const Vec3<float>& prevPos);  
    float calculateLength(const Vec3<float>& v);
    void teleportBack(LocalPlayer* player, const Vec3<float>& prevPos);
};
