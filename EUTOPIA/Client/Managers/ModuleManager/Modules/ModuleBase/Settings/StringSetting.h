#pragma once
#include <string>

#include "Setting.h"

class StringSetting : public Setting {
   public:
    std::string* valuePtr;  
    std::string defaultValue;

    StringSetting(const std::string& settingName, const std::string& des, std::string* vPtr,
                  const std::string& defVal) {
        this->name = settingName;
        this->description = des;
        this->valuePtr = vPtr;
        this->defaultValue = defVal;
        *this->valuePtr = defVal;

        this->type = SettingType::STRING_S;
    }
};
