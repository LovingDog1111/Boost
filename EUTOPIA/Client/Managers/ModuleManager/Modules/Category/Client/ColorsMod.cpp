#include "ColorsMod.h"

#include <algorithm>
#include <string>

#include "../../Utils/TimerUtil.h"

Colors* g_Colors = new Colors();

Colors::Colors() : Module("Colors", "Change the color of the client.", Category::CLIENT) {
    registerSetting(new EnumSetting(
        "Mode", "Color mode",
        {"Solid", "Spectrum", "DeepSea", "Galaxy", "Lollipop", "Prism", "Sunset", "Pulse", "Rasta"},
        &modeValue, 9));

    registerSetting(new SliderSetting<float>("Brightness", "Controls color brightness", &brightness,
                                             1.f, 0.f, 1.f));
    registerSetting(new SliderSetting<float>("Saturation", "Controls color saturation", &saturation,
                                             1.f, 0.f, 1.f));
    registerSetting(
        new SliderSetting<float>("Speed", "Control color change speed", &speed, 4.f, 0.1f, 5.f));
    registerSetting(
        new SliderSetting<int>("Separation", "Changes separation", &separation, 125, 0, 255));

    registerSetting(new ColorSetting("Primary", "Used in Solid mode", &mainColor, mainColor, true));
    registerSetting(
        new ColorSetting("Secondary", "Used in gradient modes", &midColor, midColor, true));
}

bool Colors::isEnabled() {
    return true;
}

void Colors::setEnabled(bool enabled) {}

bool Colors::isVisible() {
    return false;
}

UIColor Colors::getColor(int index) const {
    switch(modeValue) {
        case Colors::SOLID:
            return mainColor;
        case Colors::SPECTRUM:
            return ColorUtil::Rainbow(speed, saturation, brightness, index * separation);
        case Colors::DEEP_SEA:
            return ColorUtil::getCustomColors(speed, index * separation,
                                              {{0.0f, UIColor(0, 100, 200)},
                                               {0.5f, UIColor(0, 200, 255)},
                                               {1.0f, UIColor(0, 150, 200)}});
        case Colors::GALAXY:
            return ColorUtil::getCustomColors(speed, index * separation,
                                              {{0.0f, UIColor(75, 0, 130)},
                                               {0.3f, UIColor(148, 0, 211)},
                                               {0.6f, UIColor(0, 0, 128)},
                                               {1.0f, UIColor(75, 0, 130)}});
        case Colors::LOLLIPOP:
            return ColorUtil::getCustomColors(speed, index * separation,
                                              {{0.0f, UIColor(255, 105, 180)},
                                               {0.2f, UIColor(255, 20, 147)},
                                               {0.4f, UIColor(255, 0, 255)},
                                               {0.6f, UIColor(138, 43, 226)},
                                               {0.8f, UIColor(75, 0, 130)},
                                               {1.0f, UIColor(255, 105, 180)}});
        case Colors::PRISM:
            return ColorUtil::Rainbow(speed, saturation, brightness, index * separation);
        case Colors::SUNSET:
            return ColorUtil::getCustomColors(speed, index * separation,
                                              {{0.0f, UIColor(255, 69, 0)},
                                               {0.3f, UIColor(255, 140, 0)},
                                               {0.6f, UIColor(255, 215, 0)},
                                               {1.0f, UIColor(255, 69, 0)}});
        case Colors::PULSE:
            return ColorUtil::getCustomColors(
                speed, index * separation,
                {{0.0f, mainColor}, {0.5f, midColor}, {1.0f, mainColor}});
        case Colors::RASTA:
            return ColorUtil::getCustomColors(speed, index * separation,
                                              {{0.0f, UIColor(255, 0, 0)},
                                               {0.33f, UIColor(255, 255, 0)},
                                               {0.66f, UIColor(0, 255, 0)},
                                               {1.0f, UIColor(255, 0, 0)}});
        default:
            return mainColor;
    }
}

void Colors::onNormalTick(LocalPlayer* localPlayer) {}
