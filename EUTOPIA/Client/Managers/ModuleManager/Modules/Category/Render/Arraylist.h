#pragma once
#include "../../ModuleBase/Module.h"

class ArrayList : public Module {
private:
	int mode = 0;
	float spacing = 0.f;
	float scale = 1.f;
	float opacity = 0.2f;
	float size = 1.f;
	int toolTips = 0;
	bool shouldGlow = true;
public:
	bool bottom = false; 

	virtual void onD2DRender();
	ArrayList();
};
