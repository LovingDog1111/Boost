#pragma once
#include "ComplexInventoryTranscation.h"
#include <Maths.h>
class ItemReleaseInventoryTransaction : public ComplexInventoryTransaction
{
public:
    enum class ActionType : int32_t {
        Release = 0,
        Use = 1,
    };

    ActionType mActionType{};
    int32_t mSlot{};
    NetworkItemStackDescriptor mItemInHand{};
   Vec3<float> mHeadPos{};

    ItemReleaseInventoryTransaction()
    {
        static uintptr_t vtable_addr = (uintptr_t)MemoryUtil::getVtableFromSig("48 8d 05 ? ? ? ? 48 89 43 ? c6 83 ? ? ? ? ? 89 ab ? ? ? ? c6 83 ? ? ? ? ? 89 ab ? ? ? ? 0f 57 c0 0f 11 83 ? ? ? ? 48 89 ab ? ? ? ? 48 c7 83 ? ? ? ? ? ? ? ? c6 83 ? ? ? ? ? 48 89 ab ? ? ? ? 89 ab");
        vtable = reinterpret_cast<uintptr_t**>(vtable_addr);
        type = Type::ItemReleaseTransaction;
        data = InventoryTransaction();
        data.mActions = std::unordered_map<InventorySource, std::vector<InventoryAction>>();
        data.mItems = std::vector<InventoryTransactionItemGroup>();
    }
};

static_assert(sizeof(ItemReleaseInventoryTransaction) == 224);