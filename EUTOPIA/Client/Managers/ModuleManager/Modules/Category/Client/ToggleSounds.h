#pragma once
#include <chrono>
#include <deque>
#include <string>
#include <unordered_map>

#include "../../ModuleBase/Module.h"
#include "Audio.h"

class ToggleSounds : public Module {
   private:
    Audio audioPlayer;
    int soundMode = 0;
    std::unordered_map<std::string, bool> moduleStates;

   public:
    ToggleSounds();
    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    std::string getSoundPath(bool enabled);
    void testAudioPlayback();
};