#pragma once
#include "../../ModuleBase/Module.h"

class Arraylist : public Module {
   public:
    bool bottom = true;
    void refresharraylist();
   private:

    int rendermode;
    bool showModes = true;
    UIColor color;
    int modeColorEnum = 0;
    bool outline = false;
    int offset = 15;
    int opacity = 125;
    int spacing = -1;
    float size = 1.f;
    bool sortByLength(Module* lhs, Module* rhs);
    std::vector<Module*> cachedList;

   public:
    Arraylist();

    void onD2DRender() override;
    void onMCRender(MinecraftUIRenderContext* renderCtx) override;
};