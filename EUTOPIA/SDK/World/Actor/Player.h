#pragma once
#include "Mob.h"
class Player : public Mob {


   public:

	float getSpeed() {
		return MemoryUtil::callVirtualFunc<float>(142, this);
	}


	void setSpeed(float speed) {
		MemoryUtil::callVirtualFunc<void, float>(143, this, speed);

	}

	int getItemUseDuration() {
		return MemoryUtil::callVirtualFunc<int>(149, this);
	}
};