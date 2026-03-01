#pragma once
#include <string>
#include <unordered_set>

#include "../../ModuleBase/Module.h"

class TrapTPA : public Module {
   public:
    TrapTPA();

    void onNormalTick(LocalPlayer* localPlayer) override;

   private:
    float healthThreshold;
    std::unordered_set<std::string> sentTargets;

    void sendTPARequest(const std::string& playerName);
};
