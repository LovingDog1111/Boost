#pragma once
#include "../../ModuleBase/Module.h"
#include "../../ModuleBase/Settings/BoolSetting.h"

class LocalPlayer;

class Scaffold : public Module {
   public:
    Scaffold();
    void onNormalTick(LocalPlayer* localPlayer) override;
    BlockPos getPlacePos(LocalPlayer* player);
    bool canPlaceBlock(const BlockPos& pos);
    int selectBlockSlot(LocalPlayer* player);
    void updateHUDAnimation(float deltaTime);

    BoolSetting* towerModeSetting = nullptr;
    BoolSetting* silentRotationSetting = nullptr;
    BoolSetting* airPlaceSetting = nullptr;

    bool towerMode = false;
    bool silentRotation = false;
    bool airPlace = false;

    float mStartY = 0.f;
    int mLastSlot = -1;
    BlockPos mLastBlock;
    int mLastFace = 0;
    bool mIsTowering = false;
    float mAnimBlockCount = 0.f;
};