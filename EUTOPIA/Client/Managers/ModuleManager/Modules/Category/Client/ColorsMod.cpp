#include "ColorsMod.h"

Colors::Colors() : Module("Colors", "Change client theme color", Category::CLIENT) {
    registerSetting(new EnumSetting("Mode",
                                    "Color mode\nSingle: Uses 1 color only\nWave: Transitions "
                                    "between 2 seperate colors\nRGB: Red Green Blue transition",
                                    {"Single", "Wave", "RGB"}, &colorType, 0));
    registerSetting(new ColorSetting("Color", "Client color", &mainColor, mainColor, false, false,
                                     [&]() -> bool { return (colorType == 0); }));
    registerSetting(new ColorSetting("Primary", "Client color", &primaryColor, primaryColor, false,
                                     false, [&]() -> bool { return (colorType == 1); }));
    registerSetting(new ColorSetting("Secondary", "Client color", &secondColor, secondColor, false,
                                     false, [&]() -> bool { return (colorType == 1); }));

    registerSetting(new SliderSetting<float>("Seconds", "RGB Interval in seconds", &seconds,
                                             seconds, 1.f, 10.f,
                                             [&]() -> bool { return (colorType == 2); }));
    registerSetting(new SliderSetting<float>("Saturation", "RGB Saturation", &saturation,
                                             saturation, 0.f, 1.f,
                                             [&]() -> bool { return (colorType == 2); }));
    registerSetting(new SliderSetting<float>("Brightness", "RGB Brightness", &brightness,
                                             brightness, 0.f, 1.f,
                                             [&]() -> bool { return (colorType == 2); }));
    registerSetting(new SliderSetting<int>("Seperation", "RGB Seperation", &seperation, seperation,
                                           1, 100, [&]() -> bool { return (colorType == 2); }));
}