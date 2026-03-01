#include "Module.h"
#include "../../../../../Libs/json.hpp"
#include "../../ModuleManager.h"
#include "../../../NotificationManager/NotificationManager.h"

using json = nlohmann::json;

Module::Module(std::string moduleName, std::string des, Category c, int k) {
	this->name = moduleName;
	this->description = des;
	this->category = c;
	this->keybind = k;

	registerSetting(new BoolSetting("Visible", "Visible on arraylist", &visible, true));
	registerSetting(new KeybindSetting("Keybind", "Keybind of module", &keybind, k));
	registerSetting(new EnumSetting("Toggle", "How module should be toggled", { "Press", "Hold" }, &toggleMode, 0));
}

Module::~Module() {
	for (auto& setting : settings) {
		delete setting;
		setting = nullptr;
	}
	settings.clear();
}

std::string Module::getModeText() {
	return "NULL";
}

bool Module::isEnabled() {
	return enabled;
}

bool Module::isVisible() {
	return visible;
}

bool Module::isHoldMode() {
	return toggleMode;
}

int Module::getKeybind() {
	return keybind;
}

void Module::setKeybind(int key) {
	this->keybind = key;
}

bool Module::runOnBackground() {
	return false;
}

void Module::setEnabled(bool enable) {
	if (this->enabled != enable) {
		this->enabled = enable;

		auto notificationsMod = ModuleManager::getModule<notifs>();
		if (notificationsMod && notificationsMod->isEnabled()) {
			std::string status = enable ? "Enabled" : "Disabled";
			std::string message = this->name + " was " + status;
			NotificationManager::addNotification(message, 2.f);
		}

		if (enable) {
			this->onEnable();
		}
		else {
			this->onDisable();
		}
	}
}
void Module::toggle() {
	setEnabled(!enabled);
}

void Module::onDisable() {
}

void Module::onEnable() {
}

void Module::onKeyUpdate(int key, bool isDown) {
	if (getKeybind() == key) {
		if (isHoldMode()) {
			setEnabled(isDown);
		}
		else {
			if (isDown) {
				toggle();
			}
		}
	}
}

void Module::onClientTick() {
}

void Module::onNormalTick(LocalPlayer* localPlayer) {
}

void Module::onLevelTick(Level* level) {
}

void Module::onUpdateRotation(LocalPlayer* localPlayer) {
}

void Module::onSendPacket(Packet* packet) {
}

void Module::onD2DRender() {
}

void Module::onMCRender(MinecraftUIRenderContext* renderCtx) {
}

void Module::onLevelRender() {
}

void Module::onAttack(Actor* actor, bool& cancel) {}
void Module::onBuild(const BlockPos& blockPos, const uint8_t face, bool& cancel) {}
void Module::onRecievePacket(Packet* packet, bool* cancel) {}

void Module::onLoadConfig(void* confVoid) {
	json* config = reinterpret_cast<json*>(confVoid);
	try {
		if (config->contains(getModuleName())) {
			json obj = config->at(getModuleName());
			if (obj.is_null()) return;

			if (obj.contains("enabled")) {
				enabled = obj["enabled"].get<bool>();
			}
			for (Setting* setting : this->settings) {
				if (obj.contains(setting->name)) {
					if (setting->type == SettingType::KEYBIND_S) {
						KeybindSetting* keySetting = (KeybindSetting*)setting;
						*keySetting->value = obj[keySetting->name].get<int>();
					}
					else if (setting->type == SettingType::BOOL_S) {
						BoolSetting* boolSetting = (BoolSetting*)setting;
						*boolSetting->value = obj[boolSetting->name].get<bool>();
					}
					else if (setting->type == SettingType::ENUM_S) {
						EnumSetting* enumSetting = (EnumSetting*)setting;
						*enumSetting->value = obj[enumSetting->name].get<int>();
					}
					else if (setting->type == SettingType::COLOR_S) {
						ColorSetting* colorSetting = (ColorSetting*)setting;
						json colorObj = obj[colorSetting->name];
						if (colorObj.contains("Red")) colorSetting->colorPtr->r = colorObj["Red"].get<int>();
						if (colorObj.contains("Green")) colorSetting->colorPtr->g = colorObj["Green"].get<int>();
						if (colorObj.contains("Blue")) colorSetting->colorPtr->b = colorObj["Blue"].get<int>();
						if (colorObj.contains("Alpha")) colorSetting->colorPtr->a = colorObj["Alpha"].get<int>();
						if (colorObj.contains("Synced")) colorSetting->colorSynced = colorObj["Synced"].get<bool>();
					}
					else if (setting->type == SettingType::STRING_S) {
						StringSetting* strSetting = (StringSetting*)setting;
						*strSetting->value = obj[strSetting->name].get<std::string>();
					}
					else if (setting->type == SettingType::SLIDER_S) {
						SliderSettingBase* sliderSetting = (SliderSettingBase*)setting;
						if (sliderSetting->valueType == ValueType::INT_T) {
							SliderSetting<int>* intSlider = (SliderSetting<int>*)sliderSetting;
							*intSlider->valuePtr = obj[intSlider->name].get<int>();
						}
						else if (sliderSetting->valueType == ValueType::FLOAT_T) {
							SliderSetting<float>* floatSlider = (SliderSetting<float>*)sliderSetting;
							*floatSlider->valuePtr = obj[floatSlider->name].get<float>();
						}
					}
					else if (setting->type == SettingType::VEC3_S) {
						Vec3SettingBase* vecSettingBase = (Vec3SettingBase*)setting;
						if (vecSettingBase->valueType == Vec3Type::INT_T) {
							Vec3Setting<int>* vecSetting = (Vec3Setting<int>*)vecSettingBase;
							json vecObj = obj[vecSetting->name];
							if (vecObj.contains("X")) vecSetting->valuePtr->x = vecObj["X"].get<int>();
							if (vecObj.contains("Y")) vecSetting->valuePtr->y = vecObj["Y"].get<int>();
							if (vecObj.contains("Z")) vecSetting->valuePtr->z = vecObj["Z"].get<int>();
						}
						else if (vecSettingBase->valueType == Vec3Type::FLOAT_T) {
							Vec3Setting<float>* vecSetting = (Vec3Setting<float>*)vecSettingBase;
							json vecObj = obj[vecSetting->name];
							if (vecObj.contains("X")) vecSetting->valuePtr->x = vecObj["X"].get<float>();
							if (vecObj.contains("Y")) vecSetting->valuePtr->y = vecObj["Y"].get<float>();
							if (vecObj.contains("Z")) vecSetting->valuePtr->z = vecObj["Z"].get<float>();
						}
					}
				}
			}
		}
	}
	catch (const std::exception& e) {
		GI::DisplayClientMessage("Failed to load config, error in Module %s! %s", this->getModuleName(), e.what());
	}
}

void Module::onSaveConfig(void* confVoid) {
	json* currentConfig = reinterpret_cast<json*>(confVoid);
	try {
		json obj;
		obj["enabled"] = enabled;

		for (Setting* setting : this->settings) {
			if (setting->type == SettingType::KEYBIND_S) {
				KeybindSetting* keySetting = (KeybindSetting*)setting;
				obj[keySetting->name] = *keySetting->value;
			}
			else if (setting->type == SettingType::BOOL_S) {
				BoolSetting* boolSetting = (BoolSetting*)setting;
				obj[boolSetting->name] = *boolSetting->value;
			}
			else if (setting->type == SettingType::ENUM_S) {
				EnumSetting* enumSetting = (EnumSetting*)setting;
				obj[enumSetting->name] = *enumSetting->value;
			}
			else if (setting->type == SettingType::STRING_S) {
				StringSetting* strSetting = (StringSetting*)setting;
				obj[strSetting->name] = *strSetting->value;
			}
			else if (setting->type == SettingType::COLOR_S) {
				ColorSetting* colorSetting = (ColorSetting*)setting;
				obj[colorSetting->name]["Red"] = colorSetting->colorPtr->r;
				obj[colorSetting->name]["Green"] = colorSetting->colorPtr->g;
				obj[colorSetting->name]["Blue"] = colorSetting->colorPtr->b;
				obj[colorSetting->name]["Alpha"] = colorSetting->colorPtr->a;
				obj[colorSetting->name]["Synced"] = colorSetting->colorSynced;
			}
			else if (setting->type == SettingType::SLIDER_S) {
				SliderSettingBase* sliderSetting = (SliderSettingBase*)setting;
				if (sliderSetting->valueType == ValueType::INT_T) {
					SliderSetting<int>* intSlider = (SliderSetting<int>*)sliderSetting;
					obj[intSlider->name] = *intSlider->valuePtr;
				}
				else if (sliderSetting->valueType == ValueType::FLOAT_T) {
					SliderSetting<float>* floatSlider = (SliderSetting<float>*)sliderSetting;
					obj[floatSlider->name] = *floatSlider->valuePtr;
				}
			}
			else if (setting->type == SettingType::VEC3_S) {
				Vec3SettingBase* vecSettingBase = (Vec3SettingBase*)setting;
				if (vecSettingBase->valueType == Vec3Type::INT_T) {
					Vec3Setting<int>* vecSetting = (Vec3Setting<int>*)vecSettingBase;
					obj[vecSetting->name]["X"] = vecSetting->valuePtr->x;
					obj[vecSetting->name]["Y"] = vecSetting->valuePtr->y;
					obj[vecSetting->name]["Z"] = vecSetting->valuePtr->z;
				}
				else if (vecSettingBase->valueType == Vec3Type::FLOAT_T) {
					Vec3Setting<float>* vecSetting = (Vec3Setting<float>*)vecSettingBase;
					obj[vecSetting->name]["X"] = vecSetting->valuePtr->x;
					obj[vecSetting->name]["Y"] = vecSetting->valuePtr->y;
					obj[vecSetting->name]["Z"] = vecSetting->valuePtr->z;
				}
			}
		}
		(*currentConfig)[getModuleName()] = obj;
	}
	catch (const std::exception& e) {
        GI::DisplayClientMessage("Failed to save config, error in Module %s! %s",
                                 this->getModuleName(), e.what());
	}
}