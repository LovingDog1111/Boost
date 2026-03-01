#pragma once

#include <random>


#include "../../ModuleBase/Module.h"


class MathUtils {
   public:
    static int random(int min, int max) {
        if(min > max)
            std::swap(min, max);
        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }
};



class AutoClicker : public Module {
   private:
    int Mode = 0;
    int delay = 0;
    int Odelay = 0;
    bool weapons = true;
    bool breakBlocks = true;
    bool rightclick = false;
    bool hold = false;
    int mCurrentCPS = 0;
    float mCPS = 10.f;
    float mRandomCPSMin = 8.f;
    float mRandomCPSMax = 12.f;
    bool mRandomizeCPS = true;
    bool mAllowBlockBreaking = true;
    bool mWeaponsOnly = true;
    float minCPS = 8;
    float maxCPS = 12;

   public:
    AutoClicker();
    void randomizeCPS() {
        mCurrentCPS = getCPS();
    }
    int getCPS() {
        if(mRandomizeCPS) {
            return MathUtils::random(static_cast<int>(mRandomCPSMin),
                                     static_cast<int>(mRandomCPSMax));
        }
        return static_cast<int>(mCPS);
    }
    void onLevelRender() override;
    void onEnable() override;
};
