#include "AutoSwitch.h"

#include <chrono>

#include "../../Utils/Minecraft/InvUtil.h"

AutoSwitch::AutoSwitch() : Module("AutoSwitch", "Automatically switches slots", Category::COMBAT) {
    registerSetting(new SliderSetting<int>("Slot1", "First hotbar slot", &slot1, 0, 0, 8));
    registerSetting(new SliderSetting<int>("Slot2", "Second hotbar slot", &slot2, 1, 0, 8));
    registerSetting(
        new SliderSetting<float>("Delay", "Delay between switches", &delay, 0.5f, 0.05f, 5.0f));
}

void AutoSwitch::onNormalTick(LocalPlayer *player) {
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    float elapsed = std::chrono::duration<float>(now - lastTime).count();
    if(elapsed < delay)
        return;

    lastTime = now;

    toggle = !toggle;
    int targetSlot = toggle ? slot1 : slot2;
    InvUtil::switchTo(targetSlot);
}
