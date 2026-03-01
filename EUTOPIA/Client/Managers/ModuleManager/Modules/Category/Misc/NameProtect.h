#pragma once
#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"


class NameProtect : public Module {
   public:
    NameProtect();
    void onEnable() override;
    void onNormalTick(LocalPlayer* player) override;
    void onDisable() override;
    void onSendPacket(Packet* packet) override;

   private:
    std::string oldNametag = "";
    std::string newName = "Boost";
};