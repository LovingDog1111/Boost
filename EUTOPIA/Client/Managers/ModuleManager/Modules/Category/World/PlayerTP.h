#pragma once
#include "../../ModuleBase/Module.h"

class PlayerTP : public Module {
   private:
    Actor* target = nullptr;

    float speed = 1.5f;
    float threshold = 1.0f;

    void resetAABB(LocalPlayer* p);
    void setDefaultTimer();
    void setTimerFast();

   public:
    PlayerTP();

    inline void setTarget(Actor* t) {
        target = t;
    }
    inline Actor* getTarget() const {
        return target;
    }

    virtual void onEnable() override;
    virtual void onNormalTick(LocalPlayer* player) override;
    virtual void onDisable() override;
};
