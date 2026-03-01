#define NOMINMAX
#include "WaterTP.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

WaterTP::WaterTP() : Module("WaterTP", "Teleport to water", Category::WORLD) {
    registerSetting(new SliderSetting<int>("Range", "Scan range", &range, 30, 1, 100));
}

void WaterTP::onEnable() {
}

void WaterTP::onNormalTick(LocalPlayer* localPlayer) {

}

void WaterTP::onDisable() {

}
