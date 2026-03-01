#pragma once

#include <string>

#include "../../SDK/World/Inventory/PlayerInventory.h"
#include "../../SDK/World/Item/ItemStack.h"

namespace InvUtil {

PlayerInventory* getPlayerInventory();
Container* getInventory();
void switchSlot(int32_t slot);
bool isVaildItem(ItemStack* item); 
int getItemId(ItemStack* stack);
std::string getItemName(ItemStack* stack);
int getHeldItemId();
std::string getHeldItemName();
uint8_t getSelectedSlot();
ItemStack* getItem(int32_t slot);
ItemStack* getItemStack(int slot);
ItemStack* getBadItemStack(int slot); 
void switchTo(int hotbarSlot);
void sendMobEquipment(uint8_t slot);
int32_t findItemSlotInHotbar(uint16_t itemId);
int32_t findItemSlotInInventory(uint16_t itemId);
void moveItem(uint8_t from, uint8_t to);
float caculateItemDurability(ItemStack* item);

}  
