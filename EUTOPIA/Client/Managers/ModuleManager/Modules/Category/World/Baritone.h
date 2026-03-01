#pragma once
#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"
#include "../Player/PacketMine.h"

class Baritone : public Module {
   private:
    int range = 80;
    int tickDelay = 2;
    int delayCounter = 0;
    bool breaking = false;
    bool spoofSwitch = false;
    void drawTracerToBlock(const Vec3<float>& blockPos);
    bool lapis = false;
    bool autoSprint = true;
    bool scanning = false;
    int scanDelay = 0;



   public:
    Baritone();
    bool hasTarget = false;
    bool pausedBeforeTeleport = false;
    float hungerr = 20.f;
    Vec3<float> targetPos;
    void onEnable() override;
    bool mineDiamond = true;
    bool mineIron = false;
    bool mineGold = false;
    bool mineCoal = false;
    bool mineRedstone = false;
    bool mineEmerald = false;
    bool mineAncient = false;
    bool mineQuartz = false;
    bool mineLapis = false;
    float pckminespeed = 1.f;
    float bcontinueReset = 1.f;
    float bmineRange = 6.f;
    int bmineType = 0;
    int bmultiTask = 0;
    bool bswitchBack = false;
    bool bdoubleMine = false;
    int bpacketMode = 0;
    bool tptoitems;

    void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onLevelRender() override;
};
