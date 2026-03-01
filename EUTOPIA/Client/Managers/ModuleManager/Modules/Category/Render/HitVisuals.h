#pragma once
#include <cmath>
#include <vector>

#include "..\..\ModuleBase\Module.h"

class Actor;


class HitVisuals : public Module {
   public:
    HitVisuals();
    void onD2DRender();

   private:
    Actor* target = nullptr;
    int lastHurtTime = 0;
    float angle = 0.f;
    std::vector<float> offsets;
    int sphereCount = 6;
};