#pragma once
#include "../../Utils/Maths.h"
#include <MemoryUtil.h>


class RootUIControl
{
public:

	CLASS_MEMBER(std::string, layerName, 0x20);

	std::string GetName()
	{
		const char* ptr = reinterpret_cast<const char*>(this);

		if (ptr == nullptr || reinterpret_cast<uintptr_t>(ptr) >=MemoryUtil::getGameAddress())
			return std::string("UnknownLayer");

		std::string result;
		while (*ptr != '\0')
		{
			result += *ptr;
			ptr++;
		}
		return result;
	}
};

class VisualTree
{
public:
	CLASS_MEMBER(RootUIControl*, root, 0x8);
};
class ScreenView {
public:
	float timeCounter1; // 0x0
	float deltaTime; // 0x4
	double timeCounter2; // 0x8
	Vec2<float> windowSize; // 0x10
	char              pad_0x0018[0x20];   // this+0x0018
	CLASS_MEMBER(VisualTree*, tree, 0x48);
};
