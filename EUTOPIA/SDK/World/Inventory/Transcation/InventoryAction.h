#pragma once
#include "InventorySource.h"
#include "../../Item/NetworItemStackDescriptor.h"
  #include "../../Item/ItemStack.h" 

class InventoryAction
{
public:
    InventorySource            mSource;           
    unsigned int               Slot;        
    NetworkItemStackDescriptor mFromItemDescriptor;
    NetworkItemStackDescriptor mToItemDescriptor;  
    ItemStack                  mFromItem;    
  
    ItemStack                  mToItem;       

    InventoryAction(int slot, ItemStack* sourceItem, ItemStack* targetItem) {
        Slot = slot;
        if (sourceItem) mFromItem = *sourceItem;
        if (targetItem) mToItem = *targetItem;
        mSource = InventorySource();
        mSource.mType = InventorySourceType::ContainerInventory;
        if (sourceItem) mFromItemDescriptor = NetworkItemStackDescriptor(*sourceItem);
        if (targetItem) mToItemDescriptor = NetworkItemStackDescriptor(*targetItem);
    }
};