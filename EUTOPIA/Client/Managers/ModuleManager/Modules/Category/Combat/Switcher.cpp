#include "Switcher.h"
#include <Minecraft/InvUtil.h>

Switcher::Switcher() : Module("Switcher", "Switch to swords when attacking", Category::COMBAT) {}

static bool isSwordSafe(ItemStack* stack) {
    if(!stack)
        return false;
    if(!stack->item)
        return false;
    auto& name = stack->item->mName;
    if(name.empty())
        return false;
    return name.find("sword") != std::string::npos;
}

static int getBestSword(LocalPlayer* p) {
    auto supplies = p->getsupplies();
    if(!supplies)
        return -1;
    auto container = supplies->container;
    if(!container)
        return -1;

    int best = -1;
    int val = -1;

    for(int i = 0; i < 9; i++) {
        ItemStack* s = container->getItem(i);
        if(!isSwordSafe(s))
            continue;

        std::string name = s->item->mName;
        int v = 0;
        if(name.find("netherite") != std::string::npos)
            v = 5;
        else if(name.find("diamond") != std::string::npos)
            v = 4;
        else if(name.find("iron") != std::string::npos)
            v = 3;
        else if(name.find("stone") != std::string::npos)
            v = 2;
        else if(name.find("wood") != std::string::npos)
            v = 1;

        if(v > val) {
            val = v;
            best = i;
        }
    }

    return best;
}

void Switcher::onNormalTick(LocalPlayer* lp) {
    if(!lp)
        return;

    auto supplies = lp->getsupplies();
    if(!supplies)
        return;
    auto container = supplies->container;
    if(!container)
        return;

    int best = getBestSword(lp);
    if(best == -1)
        return;

    int current = supplies->mSelectedSlot;
    ItemStack* cur = container->getItem(current);

    if(isSwordSafe(cur))
        return;

    supplies->mSelectedSlot = best;
    InvUtil::sendMobEquipment(best);
}
