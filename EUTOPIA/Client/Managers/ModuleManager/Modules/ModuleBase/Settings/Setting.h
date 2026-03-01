#pragma once
#include <string>
#include <optional>
#include <functional>

class Module;

enum class SettingType {
	UNKNOW_S,
	BOOL_S,
	KEYBIND_S,
	ENUM_S,
	COLOR_S,
	SLIDER_S,
	VEC3_S,
	STRING_S
};

class Setting {
public:
	std::string name;
	std::string description;
	std::optional<std::function<bool(void)>> dependOn = std::nullopt;
	SettingType type = SettingType::UNKNOW_S;
public:
	// ClickGUI stuff
	float selectedAnim = 0.f;
};
