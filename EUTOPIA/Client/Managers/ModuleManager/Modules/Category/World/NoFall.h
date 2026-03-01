#pragma once
#include "../../ModuleBase/Module.h"

class NoFall : public Module {
   private:
    int Mode = 0;
    bool altToggle = false;
    float lastY = 0.f;

   public:
    NoFall();
    void onSendPacket(Packet* packet) override;
    void onEnable() override;
    void onDisable() override;
};
