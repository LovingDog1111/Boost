#include"PlayerInventory.h"
#include"../../GlobalInstance.h"


void PlayerInventory::swapSlots(int from, int to)
{
    ItemStack* item1 = container->getItem(from);
    ItemStack* item2 = container->getItem(to);

    auto action1 = InventoryAction(from, item1, item2);
    auto action2 = InventoryAction(to, item2, item1);

    action1.mSource.mType = InventorySourceType::ContainerInventory;
    action2.mSource.mType = InventorySourceType::ContainerInventory;
    action1.mSource.mContainerId = static_cast<int>(ContainerID::Inventory);
    action2.mSource.mContainerId = static_cast<int>(ContainerID::Inventory);

    auto pkt = MinecraftPacket::createPacket<InventoryTransactionPacket>();

    auto cit = std::make_unique<ComplexInventoryTransaction>();

    cit->data.addAction(action1);
    cit->data.addAction(action2);

    pkt->mTransaction = std::move(cit);
    GI::getPacketSender()->sendToServer(pkt.get());
    //GI::getClientInstance()->getPacketSender()->sendToServer(pkt);
}

void PlayerInventory::startUsingItem(int slot) {
    auto player = GI::getLocalPlayer();

    auto pkt = MinecraftPacket::createPacket<InventoryTransactionPacket>();
    auto cit = std::make_unique<ItemUseInventoryTransaction>();
    cit->mActionType = ItemUseInventoryTransaction::ActionType::Use;
    cit->mSlot = slot;
    cit->mItemInHand = *player->supplies->container->getItem(slot);


    pkt->mTransaction = std::move(cit);

   GI::getPacketSender()->sendToServer(pkt.get());
}

void PlayerInventory::releaseUsingItem(int slot)
{
    auto player = GI::getLocalPlayer();

    int oldSlot = player->supplies->mSelectedSlot;
    player->supplies->mSelectedSlot = slot;
    player->gamemode->releaseUsingItem();
    player->supplies->mSelectedSlot = oldSlot;
}

void PlayerInventory::equipArmor(int slot) {
    auto player = Game.getLocalPlayer();
    if(!player)
        return;

    auto supplies = player->getsupplies();
    if(!supplies)
        return;

    auto container = supplies->getcontainer();
    if(!container)
        return;

    ItemStack* itemStack = container->getItem(slot);
    if(!itemStack || !itemStack->mItem || !itemStack->isValid())
        return;

    Item* item = itemStack->getItem();
    if(!item)
        return;

    int armorSlot = item->getArmorSlot();
    if(armorSlot < 0 || armorSlot > 3)
        return;

    ItemStack* armorStack = player->getArmorContainer()->getItem(armorSlot);

    InventoryAction action(slot, itemStack, armorStack);
    action.mSource.mType = InventorySourceType::ContainerInventory;
    action.mSource.mContainerId = static_cast<int>(ContainerID::Inventory);

    auto pkt = MinecraftPacket::createPacket<InventoryTransactionPacket>();
    auto cit = std::make_unique<ComplexInventoryTransaction>();
    if(!cit)
        return;

    cit->data.addAction(action);

    if(armorStack && armorStack->mItem && armorStack->isValid()) {
        InventoryAction action2(armorSlot, armorStack, itemStack);
        action2.mSource.mType = InventorySourceType::ContainerInventory;
        action2.mSource.mContainerId = static_cast<int>(ContainerID::Armor);
        cit->data.addAction(action2);
    }

    pkt->mTransaction = std::move(cit);
    Game.getPacketSender()->sendToServer(pkt.get());
}
