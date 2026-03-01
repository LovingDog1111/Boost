#pragma once

#include "../../ModuleBase/Module.h"

class NameTags : public Module {
   private:
    float opacity = 1.0f;
    bool showSelf = true;
    int underlineMode = 0;
    bool dynamicSize = true;
    float tagSize = 0.5f;
    bool showHealthBar = true;
    bool showAbsorption = true;
    int nametagstyle = 0; 
    UIColor friendColor = UIColor(0, 255, 255, 255);
    void updateBps();

   public:
    NameTags();
    virtual void onD2DRender() override;
    virtual void onMCRender(MinecraftUIRenderContext* renderCtx) override;
    virtual void onRenderNameTag(Actor* actor, Vec3<float>* pos, bool unknownFlag, float delta,
                                 mce::Color* color) override;
};
