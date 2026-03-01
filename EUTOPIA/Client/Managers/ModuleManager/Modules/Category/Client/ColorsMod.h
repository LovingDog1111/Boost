#pragma once

#include "../../ModuleBase/Module.h"
#include "../../Utils/ColorUtil.h"
#include "../../Utils/TimerUtil.h"

class Colors : public Module {
   public:
    UIColor mainColor = UIColor(50, 205, 50, 255);
    UIColor midColor = UIColor(0, 255, 255, 255);

    int modeValue = 0;
    int colorTheme = 1;
    float brightness = 1.f;
    float saturation = 1.f;
    float seconds = 3.f;
    float effectSpeed = 1.0f;
    float effectIntensity = 1.0f;
    float hueValue = 200.0f;
    int separation = 125;
    float speed = 1.0f;
    Colors();

    int getSeparationValue() const {
        return this->separation;
    }

    UIColor getColor(int index = 0) const;
    UIColor getThemeMainColor(int index) const;

    bool isEnabled() override;
    void setEnabled(bool enabled) override;
    bool isVisible() override;
    virtual void onNormalTick(LocalPlayer* localPlayer) override;
    enum ColorMode {
        SOLID = 0,
        SPECTRUM = 1,
        DEEP_SEA = 2,
        GALAXY = 3,
        LOLLIPOP = 4,
        PRISM = 5,
        SUNSET = 6,
        PULSE = 7,
        RASTA = 8
    };
};

extern Colors* g_Colors;