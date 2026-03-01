#pragma once
#include <string>
#include <unordered_set>
#include <vector>

#include "../../ModuleBase/Module.h"

class ModWarning : public Module {
   private:
    static bool kickSettingEnabled;
    static std::unordered_set<std::string> warnedMods;
    static bool autoKickEnabled;

   public:
    ModWarning();

    void onNormalTick(LocalPlayer* localPlayer);

    std::vector<std::string> getCurrentPlayerList();

    void toggleKickSetting();

    void kickLocalPlayer();
};
