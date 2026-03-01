#pragma once
#include "../../ModuleBase/Module.h"

class Velocity : public Module {
public:
	float horizontal = 0.f;
	float vertical = 0.f;

	Velocity();
    void onReceivePacket(Packet* packet, bool* cancel) override;
	std::string getModeText() override;
};