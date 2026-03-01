#include "AutoPickaxe.h"

AutoPickaxe::AutoPickaxe()
    : Module("AutoPickaxe", "Forces the player to hold a pickaxe", Category::PLAYER) {}

void AutoPickaxe::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    auto* supplies = localPlayer->getsupplies();
    if(!supplies)
        return;

    Container* inventory = supplies->container;
    if(!inventory)
        return;

    std::vector<int> pickaxeSlots;

    for(int i = 0; i < 9; i++) {
        auto* itemStack = inventory->getItem(i);
        if(!itemStack || !itemStack->item)
            continue;

        std::string itemName = itemStack->item->mName;
        if(itemName.find("pickaxe") != std::string::npos)
            pickaxeSlots.push_back(i);
    }

    if(pickaxeSlots.empty())
        return;

    int selected = supplies->mSelectedSlot;
    if(selected < 0 || selected >= 9)
        selected = 0;

    for(int slot : pickaxeSlots) {
        if(slot >= 0 && slot < 9) {
            supplies->mSelectedSlot = slot;
            break;
        }
    }
}