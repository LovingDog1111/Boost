#pragma once
#include <memory>
#include <unordered_map>
#include "InventorySource.h"
#include "InventoryAction.h"
class InventoryTransactionItemGroup {
public:
    InventoryTransactionItemGroup() = delete;

    int                                mItemId;   // this+0x0
    int                                mItemAux;  // this+0x4
    std::unique_ptr<class CompoundTag> mTag;      // this+0x8
    int                                mCount;    // this+0x10
    bool                               mOverflow; // this+0x14
};


class InventoryTransaction {
public:
    std::unordered_map<InventorySource, std::vector<InventoryAction>> mActions; // 0x0
    std::vector<InventoryTransactionItemGroup>                        mItems;   // 0x40

    void addAction(InventoryAction const& action) {
        static void* func = reinterpret_cast<void*>(MemoryUtil::getFuncFromCall(MemoryUtil::findSignature("e8 ? ? ? ? 48 81 c3 ? ? ? ? 48 3b de 75 ? bb")));
        using AddAction = void(__fastcall*)(InventoryTransaction*, InventoryAction const&, bool);
        reinterpret_cast<AddAction>(func)(this, action, false);
    }
};