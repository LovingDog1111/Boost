#pragma once


#include "../../ModuleBase/Module.h"

class AntiHit : public Module {
   private:
   
    float longReach = 15.0f;
    float orbitRadius = 5.0f;
    float orbitSpeed = 0.04f;


    float curX = 0.0f;
    float curZ = 0.0f;
    float baseY = 0.0f;
    float orbitAngle = 0.0f;


    void sendPos(Vec3<float> p);
    //Actor* findBestTarget(LocalPlayer* lp);

   public:
    AntiHit();

    static bool isTeleporting;


    virtual void onEnable() override;
    virtual void onLevelRender() override;
    virtual void onDisable() override {};

 
};