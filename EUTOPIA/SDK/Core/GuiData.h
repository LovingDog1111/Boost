#pragma once
#include <Maths.h>
#include <Logger.h>
#include<MemoryUtil.h>
#include "../MCTextFormat.h"
class Unkownn
{
public:
	char sb[0x120];
};


class GuiData {
private:
	char pad_0x0[0x40]; 
public:
	Vec2<float> windowSizeReal; 
	Vec2<float> windowSizeReal2; 
	Vec2<float> windowSizeScaled; 
	double guiScale1; 
	float guiScale2; 

	void displayMessage(const std::string& msg)//std::optional<std::string> const& sender
	{
		MCOptional mcSender = MCOptional::nullopt();
		static auto address  = MemoryUtil::findSignature("40 55 53 56 57 41 56 48 8D AC 24 A0 FE FF FF 48 81 EC 60 02 00 00 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 50 01 00 00 41");
		MemoryUtil::callFastcall<void>(address, this, msg, mcSender, false);
	}
};