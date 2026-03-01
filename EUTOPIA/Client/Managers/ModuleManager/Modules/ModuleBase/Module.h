#pragma once
#include <string>
#include <vector>
#include "../../../../../SDK/GlobalInstance.h"
#include "../../../../../Renderer/D2D.h"
#include "../../../../../Utils/DrawUtil.h"
#include "../../../../../Utils/TimerUtil.h"
#include "../../../../../Utils/Minecraft/PlayerUtil.h"
#include "../../../../../Utils/Minecraft/TargetUtil.h"
#include "../../../../../Utils/Minecraft/WorldUtil.h"
#include "../../../../../Libs/json.hpp"

#include "Settings/BoolSetting.h"
#include "Settings/ColorSetting.h"
#include "Settings/EnumSetting.h"
#include "Settings/KeybindSetting.h"
#include "Settings/SliderSetting.h"
#include "Settings/StringSetting.h"
#include "Settings/Vec3Setting.h"

enum class Category {
	COMBAT = 0,
	MOVEMENT = 1,
	RENDER = 2,
	PLAYER = 3,
	CLIENT = 4, MISC = 5, WORLD = 6 };

class Module {
private:
	std::string name;
	std::string description;
	Category category;
	bool enabled = false;
	bool visible = true;
	int keybind = 0x0;
	int toggleMode = 0;

	std::vector<Setting*> settings;
public:
	//Arraylist stuff
	float arraylistAnim = 0.f;

	//ClickGUI stuff
	float selectedAnim = 0.f;
	float enabledAnimWidth = 0.f;
	UIColor currentTextColor;
	bool extended = false;
	float currentHeight = 0.f;
protected:
	inline Setting* registerSetting(Setting* setting) {
		this->settings.push_back(setting);
		return setting;
	}
public:
	Module(std::string moduleName, std::string des, Category c, int k = 0x0);
	~Module();

	inline std::string getModuleName() {
		return this->name;
	}

	inline std::string getDescription() {
		return this->description;
	}

	inline Category getCategory() { 
		return this->category;
	}

	inline std::vector<Setting*>& getSettingList() {
		return this->settings;
	}
public:
	virtual std::string getModeText();
	virtual bool isEnabled();
	virtual bool isVisible();
	virtual bool isHoldMode();
	virtual int getKeybind();
	virtual void setKeybind(int key);
	virtual bool runOnBackground();
	virtual void setEnabled(bool enable);
	virtual void toggle();
	virtual void onDisable();
	virtual void onEnable();
	virtual void onKeyUpdate(int key, bool isDown);
	virtual void onClientTick();
	virtual void onNormalTick(LocalPlayer* localPlayer);
	virtual void onLevelTick(Level* level);
	virtual void onUpdateRotation(LocalPlayer* localPlayer);
	virtual void onSendPacket(Packet* packet);
	virtual void onD2DRender();
	virtual void onMCRender(MinecraftUIRenderContext* renderCtx);
	virtual void onLevelRender();
	virtual void onLoadConfig(void* conf);
	virtual void onSaveConfig(void* conf);
	virtual void onAttack(Actor* actor, bool& cancel);
	virtual void onBuild(const BlockPos& blockPos, const uint8_t face, bool& cancel);
	virtual void onRecievePacket(Packet* packet, bool* cancel);
};
