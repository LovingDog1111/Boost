#include <cstdlib>  // For rand()
#include <ctime>    // For time()

#include "Timer2.h"

float timerMax = 20.f;
float timerMin = 20.f;
float timerSpeed = 1.0f;
float stutterFrequency = 1.0f;
float stutterValue = 5.0f;

Timer2::Timer2() : Module("Timerv2", "lol", Category::MISC) {
    registerSetting(
        new SliderSetting<float>("Min", "Min ticks per second", &timerMin, 24.f, 20.f, 50.f));
    registerSetting(
        new SliderSetting<float>("Max", "Max ticks per second", &timerMax, 24.f, 20.f, 50.f));
    registerSetting(new SliderSetting<float>("Step", "tickshift", &timerSpeed, 1.f, 0.f, 5.f));
    registerSetting(new SliderSetting<float>("Stutter frequence",
                                             "how often stuttering occurs (0 = none)",
                                             &stutterFrequency, 1.f, 0.f, 5.f));
    registerSetting(new SliderSetting<float>(
        "Stutter Value", "how much timer drops when stuttering", &stutterValue, 5.f, 0.f, 20.f));
    srand(static_cast<unsigned>(time(nullptr)));
}

void Timer2::onDisable() {
    Minecraft* mc = Game.getClientInstance()->minecraft;
    if(mc != nullptr) {
        GI::getClientInstance()->minecraftSim->setSimTimer(20.f);

    }
}

void Timer2::onClientTick() {
    static bool increasing = true;
    static float currentTimer = timerMin;
    Minecraft* mc = Game.getClientInstance()->minecraft;
    if(mc != nullptr) {
        if(stutterFrequency > 0) {
            int stutterChance = static_cast<int>(100 / stutterFrequency);
            if(rand() % stutterChance == 0) {
                currentTimer = std::max(timerMin, currentTimer - stutterValue);
                increasing = true;
            }
        }
        float step = timerSpeed;
        if(increasing) {
            currentTimer += step;
            if(currentTimer >= timerMax) {
                currentTimer = timerMax;
                increasing = false;
            }
        } else {
            currentTimer -= step;
            if(currentTimer <= timerMin) {
                currentTimer = timerMin;
                increasing = true;
            }
        }

        GI::getClientInstance()->minecraftSim->setSimTimer(currentTimer);
    }
}