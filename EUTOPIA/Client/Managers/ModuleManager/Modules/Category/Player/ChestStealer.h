#pragma once

#include "../../../../../../SDK/GlobalInstance.h"
#include "../../../../../../SDK/NetWork/Packets/InventoryTranscationPacket.h"
#include "../../../../../../SDK/World/Inventory/ContainerID.h"
#include "../../../../../../SDK/World/Inventory/SimpleContainer.h"
#include "../../../../../../SDK/World/Inventory/Transcation/ComplexInventoryTranscation.h"
#include "../../../../../../SDK/World/Inventory/Transcation/InventoryAction.h"
#include "../../../../../../SDK/World/Item/ItemStack.h"
#include "../../../../../../Utils/TimerUtil.h"
#include "../../ModuleBase/Module.h"

class ChestStealer : public Module {
   public:
    ChestStealer();
    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* player) override;
    void onReceivePacket(Packet* pkt, bool* cancel) override;
    void onChestScreen(ContainerScreenController* csc) override;

   private:

    void takeItem(int chestSlot, ItemStack* item);
    uint64_t getDelayMs() const;
    bool shouldDelay();


    int mDelayValue = 0; 
    bool mRandomizeDelay = false;
    int mRandomMin = 50;  
    int mRandomMax = 120;  


    bool mChestOpen = false;
    uint64_t mLastItemTaken = 0;
    uint64_t mChestOpenedAt = 0;
};