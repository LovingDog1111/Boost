#pragma once
#include <unordered_set>

#include "../../ModuleBase/Module.h"

class WaterTP : public Module {
   public:
    WaterTP();

    void onEnable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onDisable() override;

   private:
    Vec3<float> goal;                         
    float closestDistance;                      
    bool tpdone = false;                        
    bool works = false;                        
    int range;                                  
    std::unordered_set<BlockPos> visitedWater;  
};
