#pragma once


#include "../../ModuleBase/Module.h"

class MobTags : public Module {
   public:
    MobTags();
    void onD2DRender() override;

   private:
    float tagSize = 0.5f;
    float opacity = 1.f;
    bool dynamicSize = true;
    bool showHealthBar = true;
    bool showAbsorption = true;

    bool showZombie = true;
    bool showSkeleton = true;
    bool showCreeper = true;
    bool showSpider = true;
    bool showWolf = true;
    bool showCow = true;
    bool showChicken = true;
    bool showPig = true;
};
