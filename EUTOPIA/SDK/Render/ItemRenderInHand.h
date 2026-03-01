#pragma once
#include "../World/Item/ItemStack.h"
class ItemRenderInhand {
   public:
    ItemStack* mItemStack;
    ItemStack* mOffhandStack;
    int mSlot;
};