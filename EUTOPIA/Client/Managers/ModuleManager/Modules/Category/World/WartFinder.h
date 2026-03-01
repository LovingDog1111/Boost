#pragma once
#include <chrono>

#include "../../ModuleBase/Module.h"

class WartFinder : public Module {
   private:
    int range = 300;
    int chunkSize = 300;
    bool netherWartSetting = true;
    bool progressSetting = true;

    Vec3<float> playerPos;
    int currentMinX, currentMaxX, currentMinY, currentMaxY, currentMinZ, currentMaxZ;
    int scannedX, scannedY, scannedZ;
    bool scanning = false;
    bool foundAny = false;
    std::chrono::steady_clock::time_point lastScanTime;

   public:
    WartFinder();

    void onEnable() override;
    void onNormalTick(LocalPlayer* player) override;
};
