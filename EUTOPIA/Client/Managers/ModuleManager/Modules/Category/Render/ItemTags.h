#pragma once
#include <string>

#include "../../ModuleBase/Module.h"


class ItemTags : public Module {
   public:
    float tagSize = 0.5f;
    float opacity = 1.f;
    bool dynamicSize = true;

    ItemTags();
    void onD2DRender() override;
    void onNormalTick(LocalPlayer *player) override;

};
