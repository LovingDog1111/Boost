#pragma once
#include "../../ModuleBase/Module.h"

class CustomFont : public Module {
   public:
    EnumSetting* fontEnumSetting = nullptr;
    int fontMode = 1;
    int fontSize = 25;
    bool italic = false;
    bool shadow = false;
    bool compactMojanglesSpace = false;
    int textAntialiasMode = 0;  // 0: Default, 1: Clear, 2: None
    std::string getSelectedFont();

   public:
    CustomFont();

    bool isEnabled() override;
    bool isVisible() override;
};