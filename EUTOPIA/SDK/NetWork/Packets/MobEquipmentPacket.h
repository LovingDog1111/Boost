//
// Created by vastrakai on 7/5/2024.
//

#pragma once

#include "../../World/Item/NetworItemStackDescriptor.h"
#include "Packet.h"

class MobEquipmentPacket : public ::Packet {
   public:
    enum class ContainerID : signed char {
        None = -1,
        Inventory = 0,
        First = 1,
        Last = 100,
        Offhand = 119,
        Armor = 120,
        SelectionSlots = 122,
        PlayerUIOnly = 124
    };
    static const PacketID ID = PacketID::MobEquipment;

    CLASS_MEMBER(__int64, mRuntimeId, 0x30);
    CLASS_MEMBER(NetworkItemStackDescriptor, mItem, 0x38);
    CLASS_MEMBER(int, mSlot, 0x98);
    CLASS_MEMBER(int, mSelectedSlot, 0x9C);
    CLASS_MEMBER(uint8_t, mContainerId, 0xA0);
    CLASS_MEMBER(uint8_t, mSlotByte, 0xA1);
    CLASS_MEMBER(uint8_t, mSelectedSlotByte, 0xA2);
    CLASS_MEMBER(uint8_t, mContainerIdByte, 0xA3);
};