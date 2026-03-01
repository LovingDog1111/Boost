#pragma once
#include "../../ModuleBase/Module.h"

class Watermark : public Module {
private:
    bool enableGlow = true;
    bool showVersion = true;
    bool showDev = true;

    float textSize = 1.0f;
    float padding = 4.0f;
    float cornerOffsetX = 4.0f;
    float cornerOffsetY = 4.0f;
    float devTextSize = 0.5f;

    UIColor textColor = UIColor(255, 255, 255, 255);
    UIColor bgColor = UIColor(0, 0, 0, 150);
    UIColor devTextColor = UIColor(255, 100, 100, 255);

public:
    Watermark();
    void onD2DRender() override;
};