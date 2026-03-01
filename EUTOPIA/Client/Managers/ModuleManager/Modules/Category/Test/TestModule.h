#pragma once
#include "../../ModuleBase/Module.h"

class TestModule : public Module {

public:
    Vec3<float> smoothPos = Vec3<float>(0, 0, 0);
    float smoothPitch = 0.0f;
    float smoothYaw = 0.0f;
    bool isInitialized = false;


	EnumSetting* Enum1 = nullptr;
	EnumSetting* Enum2= nullptr;
	EnumSetting* Enum3 = nullptr;
	int EnumMode1 = 0;
	int EnumMode2 = 0;
	int EnumMode3 = 0;
	int MyInt1 = 1;
	int MyInt2 = 5;
	int MyInt3 = 10;
	int MyInt4 = 40;
	bool myBool1 = false;
	bool myBool2 = false;
	bool myBool3 = false;
	bool myBool4 = false;
	float myFloat1 = 0.5f;
	float myFloat2 = 1.0f;
	float myFloat3 = 1.5f;
	float myFloat4 = 2.0f;
	UIColor myColor1 = UIColor(255, 0, 0, 255);
	UIColor myColor2 = UIColor(0, 255, 0, 255);
	UIColor myColor3 = UIColor(0, 0, 255, 255);
	UIColor myColor4 = UIColor(255, 255, 0, 255);
	std::string getSelectedFont();
    int timerValue = 40;

   public:

	TestModule();
	~TestModule();
	bool isVisible() override;


	 void onKeyUpdate(int key, bool isDown)override;
	 void onClientTick()override;
	 void onNormalTick(LocalPlayer* localPlayer)override;
	 void onLevelTick(Level* level)override;
	 void onUpdateRotation(LocalPlayer* localPlayer)override;
	 void onSendPacket(Packet* packet)override;
	 void onD2DRender()override;
	 void onLevelRender()override;
     void onMCRender(MinecraftUIRenderContext* mcr) override;
	 void onEnable() override;
	 void onDisable() override;
	 void onReceivePacket(Packet* packet, bool* cancel) override;
     void onRenderActorBefore(Actor* entity, Vec3<float>* cameraTargetPos,
                              Vec3<float>* pos) override;
     void onRenderActorAfter(Actor* entity) override;
};
