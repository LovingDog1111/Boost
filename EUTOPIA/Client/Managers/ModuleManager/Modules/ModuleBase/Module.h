#pragma once
#include <string>
#include <vector>
#include "../../../../../SDK/GlobalInstance.h"

#include "../../../../../Utils/RenderUtil.h"
#include "../../../../../Utils/TimerUtil.h"
#include "../../../../../Libs/json.hpp"

#include "Settings/Setting.h"
#include "Settings/BoolSetting.h"
#include "Settings/ColorSetting.h"
#include "Settings/EnumSetting.h"
#include "Settings/KeybindSetting.h"
#include "Settings/SliderSetting.h"
#include "../../../../../Utils/StringObfuscator.h"
#include "../../../../../SDK/Render/MinecraftUIRenderContext.h"
#include "../../../../../SDK/World/Inventory/ContainerScreenController.h"

enum class Category {
	COMBAT = 0,
	MOVEMENT = 1,
	RENDER = 2,
	PLAYER = 3,
	WORLD = 4,
	MISC = 5,
	CLIENT = 6
};

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
	std::string requiredGroup = Obf::STR_DEFAULT();
	// Permission hierarchy helper
	static int groupLevel(const std::string& g) {
		if (g == Obf::STR_DEFAULT()) return 0;
		if (g == Obf::STR_PREMIUM()) return 1;
		if (g == Obf::STR_DEV()) return 2;
		return 0;
	}
public:
	static inline std::string currentGroup = "default";
	static void setCurrentGroup(const std::string& g) { currentGroup = g; }
	static const std::string& getCurrentGroup() { return currentGroup; }
	void setRequiredGroup(const std::string& g) { requiredGroup = g; }
	const std::string& getRequiredGroup() const { return requiredGroup; }
protected:
	bool hasPermission() const { return groupLevel(currentGroup) >= groupLevel(requiredGroup); }
	inline Setting* registerSetting(Setting* setting) {
		this->settings.push_back(setting);
		return setting;
	}
public:
	//Arraylist stuff
	float arraylistAnim = 0.f;

	//ClickGUI stuff
	float selectedAnim = 0.f;
	float enabledAnim = 0.f;
	bool extended = false;
  
	// GUI填充动画进度
	float fillAnim = 0.f;
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
    int* modulePagePtr = nullptr;
   

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
    virtual void onSendPacketWithCancel(Packet* packet, bool* cancel);
	virtual void onReceivePacket(Packet* packet,bool *cancel);
	virtual void onD2DRender();
	virtual void onMCRender(MinecraftUIRenderContext* renderCtx);
    virtual void onLevelRender();
    virtual void onRenderActorBefore(Actor*actor ,Vec3<float>*camera,Vec3<float>*pos);
    virtual void onRenderActorAfter(Actor* actor);
    virtual void onChestScreen(ContainerScreenController* csc);
	virtual void onLoadConfig(void* conf);
	virtual void onSaveConfig(void* conf);
    virtual void onRenderNameTag(Actor* actor, Vec3<float>* pos, bool unknownFlag, float delta,
                                 mce::Color* color);
};