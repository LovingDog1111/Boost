#pragma once
#include <vector>
#include "Setting.h"
#include "SliderSetting.h"
#include "../../../../../../Utils/ColorUtil.h"

class ColorSetting : public Setting {
public:
	UIColor* colorPtr;
	std::vector<SliderSetting<uint8_t>*> colorSliders;
	bool colorSynced = false;
	bool showSynced = true;
	bool extended = false;
	float syncSelectedAnim = 0.f;
	float currentHeight = 0.f;
	ColorSetting(std::string settingName, std::string des, UIColor* ptr, UIColor defaultValue, bool alpha = true, bool _showSynced = true, std::optional<std::function<bool(void)>> _dependOn = std::nullopt) {
		this->name = settingName;
		this->description = des;
		this->colorPtr = ptr;
		this->showSynced = _showSynced;

		colorSliders.push_back(new SliderSetting<uint8_t>("Red", "NULL", &colorPtr->r, defaultValue.r, 0, 255));
		colorSliders.push_back(new SliderSetting<uint8_t>("Green", "NULL", &colorPtr->g, defaultValue.g, 0, 255));
		colorSliders.push_back(new SliderSetting<uint8_t>("Blue", "NULL", &colorPtr->b, defaultValue.b, 0, 255));

		if (alpha) colorSliders.push_back(new SliderSetting<uint8_t>("Alpha", "NULL", &colorPtr->a, defaultValue.a, 0, 255));
		this->dependOn = _dependOn;
		this->type = SettingType::COLOR_S;
	}

	~ColorSetting() {
		for (auto& slider : colorSliders) {
			delete slider;
			slider = nullptr;
		}
		colorSliders.clear();
	}
};
