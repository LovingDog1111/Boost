#pragma once
#include "ComplexInventoryTranscation.h"
#include <Maths.h>
class ItemUseInventoryTransaction : public ComplexInventoryTransaction
{
public:
    enum class ActionType : int32_t {
        Place = 0,
        Use = 1,
        Destroy = 2,
    };

    enum class PredictedResult : int32_t
    {
        Failure,
        Success
    };

    enum class TriggerType : int8_t
    {
        Unknown,
        PlayerInput,
        SimulationTick,
    };

    ActionType mActionType{};
    TriggerType mTriggerType = TriggerType::PlayerInput;
   Vec3<int> mBlockPos{};
    int32_t mTargetBlockRuntimeId{};
    uint32_t mFace{};
    uint32_t mSlot{};
    NetworkItemStackDescriptor mItemInHand{};
   Vec3<float> mPlayerPos{};
   Vec3<float> mClickPos{};
    PredictedResult mPredictedResult = PredictedResult::Success;

    ItemUseInventoryTransaction()
    {
        static uintptr_t vtable_addr  = (uintptr_t)MemoryUtil::getVtableFromSig("48 8D ? ? ? ? ? 48 89 ? 8B 46 ? 89 47 ? 0F B6 ? ? 88 47 ? 8B 56");
        vtable = reinterpret_cast<uintptr_t**>(vtable_addr);
        type = Type::ItemUseTransaction;
        data = InventoryTransaction();
        data.mActions = std::unordered_map<InventorySource, std::vector<InventoryAction>>();
        data.mItems = std::vector<InventoryTransactionItemGroup>();
    }

};