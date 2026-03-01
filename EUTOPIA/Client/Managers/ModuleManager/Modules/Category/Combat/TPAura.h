#pragma once
#include <chrono>
#include <vector>

#include "../../ModuleBase/Module.h"

class TPAura : public Module {
   private:
    std::vector<Actor*> targetList;
    std::chrono::steady_clock::time_point lastAttack;
    std::int64_t lastAttackTime = 0;
    std::int64_t lastTeleportTime = 0;
    size_t switchIndex = 0;
    bool shouldRotate = false;
    Vec2<float> rotAngle;

   
    float range = 5.f;
    float wallRange = 0.f;
    int mode = 0; 
    bool throughWalls = false;
    bool includeMobs = false;
    int minCPS = 5;
    int maxCPS = 10;
    int rotMode = 0;
    int delay = 0;            
    int attackDelay = 0;      
    float tpDistance = 1.f;   
    float tpMoveSpeed = 1.f;
    int tpMode = 0;           
    float tpYOffset = 0.f;    
    bool upAndDown = false;  
    float upDownSpeed = 0.1f;
    float upDownRange = 1.f;
    float tpRandomness = 0.0f; 

    float upDownOffset = 0.f;
    bool upDownGoingUp = true;

    void attack(Actor* target);
    Actor* getNextTarget();
    bool canAttack();

   public:
    TPAura();
    virtual void onEnable() override;
    virtual void onDisable() override;
    virtual void onNormalTick(LocalPlayer* localPlayer) override;
    virtual void onUpdateRotation(LocalPlayer* localPlayer) override;

};
