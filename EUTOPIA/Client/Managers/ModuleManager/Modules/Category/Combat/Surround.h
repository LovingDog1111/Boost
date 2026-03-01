#pragma once
#include "../../ModuleBase/Module.h"

class Surround : public Module {
   private:
    int mode = 0;
    int placeDelay = 1;
    int placePerTick = 1;
    bool packetPlace = false;
    bool center = true;
    bool dynamic = true;
    bool airPlace = false;
    int switchMode = 0;
    bool nukkitAABB = false;
    bool crystalCheck = false;
    bool crystalBlocker = false;
    bool disableOnComplete = false;
    bool disableOnJump = true;
    bool eatStop = false;
    bool button = true;
    bool echest = false;
    bool render = true;
    bool rotate = false;
    UIColor color = UIColor(125, 0, 255);
    int alpha = 40;
    int lineAlpha = 175;
    bool fade = false;

    std::vector<BlockPos> placeList;
    std::unordered_map<BlockPos, float> fadeBlockList;
    bool shouldSurround = false;
    int oldSlot = -1;
    bool wasSurrounding = false;

    inline AABB getBlockAABB(const BlockPos& blockPos) {
        AABB result;
        result.lower = blockPos.CastTo<float>();
        result.upper = result.lower.add(Vec3<float>(1.f, 1.f, 1.f));
        return result;
    }

   public:
    Surround();

    void onDisable() override;
    void onEnable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onLevelRender() override;
    void onSendPacket(Packet* packet, bool& cancel);
};