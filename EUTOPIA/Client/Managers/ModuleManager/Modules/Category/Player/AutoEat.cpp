#include "AutoEat.h"

#include <../Client/Managers/HooksManager/Hooks/Input/KeyMouseHook.h>
#include <../Utils/Minecraft/InvUtil.h>

#include <unordered_set>

static int minFoodCount = 1;
static int delayTicks = 10;
static int tickCounter = 0;
static float hungerThreshold = 18.0f;
static int lastSlot = -1;
static bool eating = false;

static const std::unordered_set<uint16_t> foodWhitelist = {
    278, 284, 289, 263, 262, 257, 287, 283, 276, 270, 269, 268, 292, 566, 277,
    266, 279, 280, 267, 567, 293, 272, 273, 288, 275, 285, 294, 290, 264, 265};

AutoEat::AutoEat() : Module("AutoEat", "Automatically eats if hungry", Category::PLAYER) {
    registerSetting(
        new SliderSetting<int>("Min Count", "Minimum food count", &minFoodCount, 1, 1, 64));
    registerSetting(new SliderSetting<int>("Delay", "Delay (ticks)", &delayTicks, 10, 0, 100));
    registerSetting(
        new SliderSetting<float>("Hunger", "Eat below hunger", &hungerThreshold, 18.f, 0.f, 20.f));
}

constexpr char MOUSE_RIGHT = 1;

void holdEat(short mouseX, short mouseY) {
    KeyMouseHook::simulate(MOUSE_RIGHT, true, mouseX, mouseY);
}

void releaseEat(short mouseX, short mouseY) {
    KeyMouseHook::simulate(MOUSE_RIGHT, false, mouseX, mouseY);
}

void AutoEat::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer || localPlayer->getHealth() <= 0)
        return;

    auto hungerAttr = localPlayer->getAttribute(AttributeHashes::HUNGER);
    if(!hungerAttr || hungerAttr->mCurrentValue >= hungerThreshold) {
        if(eating && lastSlot != -1 && InvUtil::getSelectedSlot() != lastSlot) {
            InvUtil::switchTo(lastSlot);
        }
        releaseEat(RenderUtil::mpos.x, RenderUtil::mpos.y);
        eating = false;
        lastSlot = -1;
        return;
    }

    tickCounter++;
    if(tickCounter < delayTicks)
        return;
    tickCounter = 0;

    int foodSlot = -1;
    for(int i = 0; i < 9; i++) {
        ItemStack* item = InvUtil::getItem(i);
        if(!InvUtil::isVaildItem(item) || item->mCount < minFoodCount)
            continue;
        auto baseItem = item->getItem();
        if(!baseItem)
            continue;
        if(foodWhitelist.count(baseItem->ItemId)) {
            foodSlot = i;
            break;
        }
    }
    if(foodSlot == -1)
        return;

    if(!eating) {
        lastSlot = InvUtil::getSelectedSlot();
        InvUtil::switchTo(foodSlot);
        eating = true;
    }

    holdEat(RenderUtil::mpos.x, RenderUtil::mpos.y);
}

void AutoEat::onEnable() {}
void AutoEat::onDisable() {
    if(eating) {
        releaseEat(RenderUtil::mpos.x, RenderUtil::mpos.y);
        if(lastSlot != -1)
            InvUtil::switchTo(lastSlot);
        eating = false;
        lastSlot = -1;
    }
}
