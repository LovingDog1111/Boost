#pragma once
#include "ComplexInventoryTranscation.h"
#include "../../../../Utils/Maths.h"
class ItemUseOnActorInventoryTransaction : public ComplexInventoryTransaction
{
public:
    enum class ActionType : int32_t {
        Interact = 0,
        Attack = 1,
        ItemInteract = 2,
    };

    uint64_t mActorId{};
    ActionType mActionType{};
    int32_t mSlot{};
    NetworkItemStackDescriptor mItemInHand{};
    Vec3<float> mPlayerPos{};
    Vec3<float> mClickPos{};

    ItemUseOnActorInventoryTransaction()
    {
        static uintptr_t vtable_addr = (uintptr_t)MemoryUtil::getVtableFromSig("48 8d 05 ? ? ? ? 48 89 03 48 89 6b ? 89 6b ? c7 43");
        vtable = reinterpret_cast<uintptr_t**>(vtable_addr);
        type = Type::ItemUseOnEntityTransaction;
        data = InventoryTransaction();
        data.mActions = std::unordered_map<InventorySource, std::vector<InventoryAction>>();
        data.mItems = std::vector<InventoryTransactionItemGroup>();
    }
};

