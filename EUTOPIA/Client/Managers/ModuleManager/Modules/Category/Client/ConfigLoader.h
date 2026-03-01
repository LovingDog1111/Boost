#pragma once
#include <string>

#include "../../ModuleBase/Module.h"


extern int selectedConfig;

class ConfigLoader : public Module {
   public:
    ConfigLoader();
    void onEnable() override;
};
