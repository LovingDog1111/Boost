#pragma once
#include "../../ModuleBase/Module.h"
// made by daddy smking //
class TestTpmine : public Module {
   private:
    Vec3<float> goal;
    Vec3<int> oreBlockPos;
    float closestDistance = FLT_MAX;
    bool works = false;
    float prevTimer = 20.0f;
    int range = 30;
    int timerValue = 100;
    int clusterSize = 1;
    bool antiGravel = false;
    bool antiBorder = false;
    bool antiLava = true;
    bool midClickTP = false;
    bool dim = false;
    bool iron = false;
    bool coal = false;
    bool gold = false;
    bool emerald = false;
    bool ancient = false;
    bool redstone = false;
    bool quartz = false;
    bool lapis = false;
    bool spawners = false;
    bool cuOre = false;
    int cuID = 1;
    float tpSpeed = 4.5f;

    struct IntList {
        int data[64];
        int n = 0;
        void push(int v);
        bool contains(int v) const;
    };

    void findOres();
    bool checkClusterValidity(BlockSource* region, int x, int y, int z, int blockId);
    int findConnectedVeinSize(BlockSource* region, int startX, int startY, int startZ, int blockId);
    void centerPlayerOnBlock(LocalPlayer* player);
    bool centerPlayer = true;

   public:
    TestTpmine();
    void onEnable() override;
    void onNormalTick(LocalPlayer* player) override;
    void onDisable() override;
};