#pragma once
#include "Setting.h"
#include <vector>

class EnumSetting : public Setting {
public:
	std::vector<std::string> enumList;
	int* value;

	EnumSetting(std::string settingName, std::string des, std::vector<std::string> cEnumList, int* ptr, int defaultValue, std::optional<std::function<bool(void)>> _dependOn = std::nullopt) {
		this->name = settingName;
		this->description = des;
		this->enumList = cEnumList;
		this->value = ptr;
		*this->value = defaultValue;
		this->dependOn = _dependOn;
		this->type = SettingType::ENUM_S;
	}
};