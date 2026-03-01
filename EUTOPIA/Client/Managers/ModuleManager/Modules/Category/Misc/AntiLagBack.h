#pragma once

#include "../../ModuleBase/Module.h"

class AntiLagBack : public Module {
   public:
    AntiLagBack();

    float currentX;
    float currentY;
    float currentZ;

    virtual void onEnable() override;
    virtual void onLevelRender() override;
};