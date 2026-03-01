#pragma once
#include <chrono>
#include <sstream>
#include <string>
#include <vector>

#include "../../ModuleBase/Module.h"

struct PlayerInfo2 {
    std::string name;
    Vec3<float> pos;
};

class PlayerScanner : public Module {
   public:
    PlayerScanner();
    void onEnable() override;
    void onNormalTick(LocalPlayer* player) override;

   private:
    std::vector<PlayerInfo2> results;
    Vec3<float> scanTarget;
    bool scanning = false;
    bool waiting = false;
    std::chrono::steady_clock::time_point waitStart;
    float stepSize = 300.f;
    float waitSeconds = 5.f;
    float speed = 10.f;
    float arrivalThreshold = 1.f;
    float mapMin = 1.f;
    float mapMax = 1199.f;
    void resetAABB(LocalPlayer* player);
};
