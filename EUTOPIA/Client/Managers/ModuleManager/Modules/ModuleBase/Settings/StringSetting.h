#pragma once
#include "Setting.h"

class StringSetting : public Setting {
public:
	std::string* value;
	std::string defaultValue;
	bool isCapturing = false;

	StringSetting(std::string settingName,
		std::string des,
		std::string* ptr,
		std::string defaultVal,
		std::optional<std::function<bool(void)>> _dependOn = std::nullopt) {

		this->name = settingName;
		this->description = des;
		this->value = ptr;
		this->defaultValue = defaultVal;
		*this->value = defaultVal;
		this->dependOn = _dependOn;
		this->type = SettingType::STRING_S;
	}
};