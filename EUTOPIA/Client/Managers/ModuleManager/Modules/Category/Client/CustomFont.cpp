#include "CustomFont.h"

CustomFont::CustomFont() : Module("Fonts", "Font of Client", Category::CLIENT) {
    fontEnumSetting = (EnumSetting*)registerSetting(new EnumSetting(
        "Font", "NULL",
        {"Arial", "Bahnschrift", "Mojangles", "Noto Sans", "Product Sans", "Verdana"}, &fontMode,
        1));
    registerSetting(new SliderSetting<int>("FontSize", "NULL", &fontSize, 22, 10, 30));
    registerSetting(new BoolSetting("Italic", "NULL", &italic, false));
    registerSetting(new BoolSetting("Shadow", "NULL", &shadow, false));
    registerSetting(new EnumSetting("TextAA", "Text antialiasing mode",
                                    {"Default", "Clear", "None"}, &textAntialiasMode, 0));
    registerSetting(new BoolSetting("SpaceFix", "Reduce space width for Mojangles font",
                                    &compactMojanglesSpace, false));
}

bool CustomFont::isEnabled() {
    return true;
}

bool CustomFont::isVisible() {
    return false;
}

std::string CustomFont::getSelectedFont() {
    return fontEnumSetting->enumList[fontMode];
}