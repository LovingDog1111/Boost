#pragma once
#include "../../Utils/MemoryUtil.h"
#include "GameSession.h"
class Simulation {
  public:
      float mTimer = 20.f;
      char pad_04[0x4];
      float mDeltaTime = 0.f;
      float mTimerMultiplier = 1.f;
      char pad_08[0x8];
      float mTime = 1.f;
  };

class MinecraftSim {
public:
	Simulation* getGameSimulation() {
		return hat::member_at<Simulation*>(this, 0XE8);
	}
    Simulation* getRenderSimulation() {
		return hat::member_at<Simulation*>(this, 0xF0);
	}

  void setSimTimer(float timer) {
		getGameSimulation()->mTimer = timer;
    getRenderSimulation()->mTimer = timer;
	}

  void setSimSpeed(float speed) {
		getGameSimulation()->mTimerMultiplier = speed;
    getRenderSimulation()->mTimerMultiplier = speed;
	}

  float getSimTimer() {
    return getGameSimulation()->mTimer;
  }

  float getSimSpeed() {
    return getGameSimulation()->mTimerMultiplier;
  }

  

  GameSession* getGameSession() {
      return hat::member_at<GameSession*>(this, 0xD0);
  }
  
};
