#pragma once
#pragma once
#include "../../ModuleBase/Module.h"

class SetColor : public Module {
public:
	bool fade = true;
	UIColor ArmorColor = (255, 0, 0, 175);
    UIColor ItemColor = (0, 255, 0, 175);
    float brightness = 1.0f;  // Default brightness
	SetColor();
};