#include "AutoPickaxe.h"

AutoPickaxe::AutoPickaxe()
    : Module("Auto Pickaxe", "Forces the player to hold a pickaxe", Category::PLAYER) {}

void AutoPickaxe::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    Container* inventory = localPlayer->getsupplies()->container;
    if(!inventory)
        return;

    static int lastSlot = localPlayer->getsupplies()->mSelectedSlot;
    std::vector<int> swordSlots;

    for(int i = 0; i < 9; i++) {
        auto itemStack = inventory->getItem(i);
        if(itemStack && itemStack->item) {
            std::string itemName = itemStack->item->mName;

            if(itemName.find("pickaxe") != std::string::npos) {
                swordSlots.push_back(i);
            }
        }
    }

    static int currentIndex = 0;
    if(!swordSlots.empty()) {
        for(int j = 0; j < 5; ++j) {
            localPlayer->getsupplies()->mSelectedSlot = swordSlots[currentIndex];
            currentIndex = (currentIndex + 1) % swordSlots.size();
        }
    }
}