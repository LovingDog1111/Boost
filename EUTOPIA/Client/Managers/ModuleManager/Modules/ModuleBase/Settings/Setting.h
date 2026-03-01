#pragma once
#include <string>

enum class SettingType { UNKNOW_S, BOOL_S, KEYBIND_S, ENUM_S, COLOR_S, SLIDER_S, PAGE_S, STRING_S };

class Setting {
   public:
    std::string name;
    int settingPage = 0;
    std::string description;
    SettingType type = SettingType::UNKNOW_S;

   public:
    // ClickGUI stuff
    float selectedAnim = 0.f;
};
