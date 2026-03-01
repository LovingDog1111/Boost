#pragma once
#include "../../ModuleBase/Module.h"

class GlintColor : public Module {
   private:
   public:
    GlintColor();

    UIColor glintColor = UIColor(255, 255, 255, 255);

    Vec3<float>* glintColorPtr = nullptr;
    bool isGlint = false;
    void onDisable() override;
};