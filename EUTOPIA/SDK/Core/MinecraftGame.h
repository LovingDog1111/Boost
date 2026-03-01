#pragma once
#include <MemoryUtil.h>
#include "../Render/Font.h"
class MinecraftGame {
public:
    CLASS_MEMBER(std::shared_ptr<FontRepos>, fontRepository, 0xE70);
	CLASS_MEMBER(bool,mouseGrabbed,0x1B8);
    CLASS_MEMBER(Font*, mcFont, 0xE78);

	void playUI(std::string name, float volume, float pitch)
	{
		if (this == nullptr)
			return;
		 
		MemoryUtil::callVirtualFunc<void, std::string, float, float>(
			200, this, name, volume, pitch);
	}

	float* minecraftTimer;

};
