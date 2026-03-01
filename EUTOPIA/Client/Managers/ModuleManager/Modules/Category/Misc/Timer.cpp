#include "Timer.h"
#include "../../../../../../SDK/GlobalInstance.h"

Timer::Timer() : Module("Timer", "Modify TPS (ticks per second) of game", Category::MISC) {
    registerSetting(new SliderSetting<int>("TPS", "ticks per second", &timerValue, 20, 1, 200));
}

void Timer::onEnable() {
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            prevTimer = mc->getSimTimer();
        }
    }
}

void Timer::onDisable() {
    ClientInstance* ci = GI::getClientInstance();
    if(ci != nullptr) {
        MinecraftSim* mc = ci->minecraftSim;
        if(mc != nullptr) {
            mc->setSimTimer(prevTimer);
            mc->setSimSpeed(prevTimer / 20.0f);
        }
    }
}

void Timer::onNormalTick(LocalPlayer* localPlayer) {
    ClientInstance* ci = GI::getClientInstance();
    if(ci != nullptr) {
        MinecraftSim* mc = ci->minecraftSim;
        if(mc != nullptr) {
            mc->setSimTimer(static_cast<float>(timerValue));
            float speed = static_cast<float>(timerValue) / 20.0f;
            mc->setSimSpeed(speed);
        }
    }
}