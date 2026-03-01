#pragma once
#include "../../ModuleBase/Module.h"
class ViewModel : public Module {
   public:
    glm::mat4x4 oldMatrix;
    Vec3<float> mhTrans = Vec3<float>(0.f, 0.f, 0.f);
    Vec3<float> mhScale = Vec3<float>(1.f, 1.f, 1.f);

    Vec3<float> mhRot = Vec3<float>(0.f, 0.f, 0.f);
    EnumSetting* swingAnimType = nullptr;
    EnumSetting* eatAnimType = nullptr;

   public:
    int swingAnimMode = 0;  // 0=Fancy, 1=Smooth, 2=Spin, 3=Wobble, 4=Exhibition
    int eatAnimMode = 0;    // 0=Fancy, 1=Circular, 2=Shake, 3=Pull, 4=Twist
    bool EatAnim = false;
    bool AttackAnim = false;
    bool Reset = false;
    float swingSpeed = 0.5f;
    float eatSpeed = 0.5f;

   private:
   public:
    ViewModel();
    virtual void onClientTick() override;
};