#pragma once
#include <chrono>

#include "../../ModuleBase/Module.h"

class UserInfo : public Module {
   public:
    UserInfo();
    ~UserInfo();

    bool showTime = true;
    int opacity = 130;
    int offset = 10;
    bool showUsername = true;
    bool showServer = true;
    bool showPing = true;
    bool showPlaytime = true;
    void onD2DRender() override;

   private:
    std::chrono::steady_clock::time_point lastTick;
    long long playtimeSeconds = 0;
    bool wasOnServer = false;

    UIColor textColor = UIColor(255, 255, 255);
};