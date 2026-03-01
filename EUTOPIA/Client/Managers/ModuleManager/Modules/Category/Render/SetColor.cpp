#include "SetColor.h"

SetColor::SetColor() : Module("SetColor", "Make Hurt Color More Beautiful", Category::RENDER) {
    registerSetting(new ColorSetting("Color", "NULL", &ArmorColor, ArmorColor));
	registerSetting(new ColorSetting("Item Color", "NULL", &ItemColor, ItemColor));
	registerSetting(new SliderSetting<float>("Brightness", "NULL", &brightness, brightness,0.f,255.f));
	registerSetting(new BoolSetting("Fade", "NULL", &fade, fade));
}