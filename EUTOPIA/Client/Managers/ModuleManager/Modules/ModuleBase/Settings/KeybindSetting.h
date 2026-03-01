#pragma once
#include "Setting.h"

class KeybindSetting : public Setting {
public:
	int* value;

	KeybindSetting(std::string settingName, std::string des, int* ptr, int defaultValue, std::optional<std::function<bool(void)>> _dependOn = std::nullopt) {
		this->name = settingName;
		this->description = des;
		this->value = ptr;
		*this->value = defaultValue;
		this->dependOn = _dependOn;
		this->type = SettingType::KEYBIND_S;
	}
};