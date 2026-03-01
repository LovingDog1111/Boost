#pragma once
#include <vector>

#include "../../ModuleBase/Module.h"

class PortalFinder : public Module {
   private:
    int range = 200;
    bool tracerSetting = true;
    std::vector<Vec3<float>> foundPortals;

   public:
    PortalFinder();
    void onEnable() override;
    void onLevelRender() override;
};
