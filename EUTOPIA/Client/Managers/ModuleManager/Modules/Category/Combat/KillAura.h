#pragma once
#include <chrono>
#include <vector>

#include "../../ModuleBase/Module.h"

class KillAuraH : public Module {
   private:
  
    std::vector<Actor*> targetList;
    std::chrono::steady_clock::time_point lastAttack;
    std::chrono::steady_clock::time_point lastClearTime;
    size_t switchIndex = 0;
    bool shouldRotate = false;
    bool sentinel = false;
    bool flickRotating = false;
    Vec2<float> rotAngle;
    Vec2<float> rotAngle2;
    Vec2<float> rotAngle5;
    bool useAdjustedPosition = false;
    Vec3<float> adjustedPosition;
    int attackCounter = 0;
    int z = 0;
    int Odelay = 0;
    int oTick = 0;
    float range = 5.f;
    float wallRange = 0.f;
    float predictvalue = 0.f;
    float test = 0.f;
    float headspeed = 30.f;
    int interval = 1;
    int multiplier = 1;
    int rotMode = 1;
    bool randomize = false;
    int hitType = 0;
    int hitChance = 100;
    int autoWeaponMode = 0;
    bool targetVisualize = false;
    UIColor visualizeColor = UIColor(255, 0, 0, 255);
    bool includeMobs = false;
    bool enableStrafe = true;
    bool compareDist(Actor* a, Actor* b);
    void Attack(Actor* target);
    Actor* getNextTarget();
    bool canAttack();
    bool isInAttackRange(Actor* target);
    int getBestWeaponSlot(Actor* target);
    bool SpoofSwitch(int slot);
    void clearTargetListIfNeeded();
    bool sortByDist(Actor* a1, Actor* a2);

   public:
    KillAuraH();

    
    float getTargetRange() const {
        return range;
    }
    bool getAttackThroughWalls() const {
        return true;
    }
    float getWallRange() const {
        return wallRange;
    }
    float getRange() const {
        return range;
    }
    bool shouldDisableRegen() {
        return false;
    }

    std::string getModeText() override;
    virtual void onEnable() override;
    virtual void onDisable() override;
    virtual void onLevelRender() override;
    virtual void onNormalTick(LocalPlayer* localPlayer) override;
    virtual void onUpdateRotation(LocalPlayer* localPlayer) override;
    virtual void onSendPacket(Packet* packet) override;
};