#pragma once
#include "../../ModuleBase/Module.h"

class Clip : public Module {
private:
    AABB originalAABB;
    bool hasOriginalAABB = false;
    
public:
    float offset = 0.19f;
    bool showHitbox = true;
    
    Clip();
    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onLevelRender() override;
    
private:
    bool isBlockSolid(BlockPos pos);
    Vec3<float> getPlayerDirection(LocalPlayer* player);
};