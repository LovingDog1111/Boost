#pragma once
#include "..\SDK\World\Item\ItemStack.h"
#include "..\SDK\World\Item\Item.h"
#include "..\SDK\GlobalInstance.h"
#include <unordered_map>

class ItemUtils {
   public:
    static int getItemValue(ItemStack* item);
    static int getBestItem(SItemType type, bool hotbarOnly = false);
};
