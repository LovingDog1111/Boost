#pragma once

#include "../../ModuleBase/Module.h"

class DeviceSpoofer : public Module {
   public:
    DeviceSpoofer();

    void onEnable() override;
    void onDisable() override;
};
