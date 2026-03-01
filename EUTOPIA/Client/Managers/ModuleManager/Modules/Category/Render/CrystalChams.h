#pragma once
#pragma once
#include "../../../../../../Utils/RenderUtil.h"
#include "../../ModuleBase/Module.h"

class CrystalChams : public Module {
   private:
    // Single color & speed controlling all layers
    UIColor color = UIColor(255, 0, 0, 100);
    float speed = 1.0f;
    // Vertical oscillation parameters
    float upDownSpeed = 1.0f;   // Speed of the up-down movement
    float heightOffset = 0.5f;  // Maximum vertical offset

    // Other parameters
    float range = 16.0f;
    float scale = 0.5f;
    bool fill = true;

    int lineAlpha = 255;

   public:
    CrystalChams();

    void onLevelRender() override;
};