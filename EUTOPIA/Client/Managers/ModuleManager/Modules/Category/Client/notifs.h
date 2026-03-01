#pragma once
#include <string>
#include <unordered_map>

#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"
#include "../../../../NotificationManager/NotificationManager.h"

class notifs : public Module {
   public:
    notifs();
    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer *player) override;

   private:
    std::unordered_map<std::string, bool> moduleStates;
};
