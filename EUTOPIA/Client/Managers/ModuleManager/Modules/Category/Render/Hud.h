#pragma once
#include "../../ModuleBase/Module.h"

class Hud : public Module {
public:
    Hud();
    void onD2DRender() override;

private:
    bool showPosition = true;
    bool showPotions = true;
    float textSize = 1.f;
    float spacing = 2.f;
    bool showDimension = true;
    UIColor textColor = UIColor(255, 255, 255, 255);
    UIColor potionColor = UIColor(200, 255, 200, 255);
    bool shouldGlow = true;

    std::string getEffectTimeLeftStr(MobEffectInstance* mEffectInstance);
    std::string getDimensionName(LocalPlayer* lp);
};