#pragma once
#include <cstdint>
#include "InventoryTranscation.h"
class ComplexInventoryTransaction {
public:

    virtual ~ComplexInventoryTransaction() = default;

    // ComplexInventoryTransaction inner types define
    enum class Type : int {
        NormalTransaction = 0x0,
        InventoryMismatch = 0x1,
        ItemUseTransaction = 0x2,
        ItemUseOnEntityTransaction = 0x3,
        ItemReleaseTransaction = 0x4,
    };

    uintptr_t** vtable; // this+0x0
    Type                 type;
    InventoryTransaction data;

    ComplexInventoryTransaction()
    {
        static uintptr_t vtable_addr = (uintptr_t)MemoryUtil::getVtableFromSig(
            "48 8d 05 ? ? ? ? 48 8b f9 48 89 01 8b da 48 83 c1 ? e8 ? ? ? ? f6 c3 ? 74 ? ba ? ? ? "
            "? 48 8b cf e8 ? ? ? ? 48 8b 5c 24 ? 48 8b c7 48 83 c4 ? 5f c3 cc cc cc cc cc cc cc cc "
            "cc cc cc cc cc cc 48 89 5c 24 ? 48 89 74 24 ? 57 41 56");
        vtable = reinterpret_cast<uintptr_t**>(vtable_addr);
        type = Type::NormalTransaction;
        data = InventoryTransaction();
        data.mActions = std::unordered_map<InventorySource, std::vector<InventoryAction>>();
        data.mItems = std::vector<InventoryTransactionItemGroup>();
    };

    Type getTransacType() {
        return type;
    }
};