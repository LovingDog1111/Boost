#include "CustomFont.h"

CustomFont::CustomFont() : Module("Fonts", "Font of Client", Category::CLIENT) {
    fontEnumSetting = (EnumSetting*)registerSetting(new EnumSetting("Font", "NULL",
                                                                    {"Arial",
                                                                     "Bahnschrift",
                                                                     "Calibri",
                                                                     "Cambria",
                                                                     "Candara",
                                                                     "Product Sans",
                                                                     "Verdana",
                                                                     "Noto Sans",
                                                                     "Comic Sans MS",
                                                                     "Courier New",
                                                                     "Georgia",
                                                                     "Impact",
                                                                     "Tahoma",
                                                                     "Times New Roman",
                                                                     "Trebuchet MS",
                                                                     "Segoe UI",
                                                                     "Lucida Console",
                                                                     "Garamond",
                                                                     "Papyrus",
                                                                     "Curlz MT",
                                                                     "Wingdings",
                                                                     "Constantia Bold",
                                                                     "Gabriola",
                                                                     "MingLiU_HKSCS-ExtB",
                                                                     "Mongolian Baiti",
                                                                     "Bahnschrift SemiBold",
                                                                     "Rockwell Condensed Negreta",
                                                                     "Century"},
                                                                    &fontMode, 7));

    registerSetting(new SliderSetting<int>("FontSize", "NULL", &fontSize, 25, 15, 40));
    registerSetting(new BoolSetting("Italic", "NULL", &italic, false));
    registerSetting(new BoolSetting("Shadow", "NULL", &shadow, false));
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
