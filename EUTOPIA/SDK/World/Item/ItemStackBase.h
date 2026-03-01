#pragma once
#include <cstdint>
#include <chrono>
#include "../../Core/WeakPtr.h"
#include "../../CompoundTag.h"
#include "../../../Utils/MemoryUtil.h"
class BlockLegacy;
class ItemStackBase {
  public:
      uintptr_t** mVfTable;
      WeakPtr<class Item> mItem;
     
      CompoundTag* mCompoundTag; 
      const class Block* mBlock;
      short mAuxValue;
      unsigned char mCount;
      bool valid;
      char sb[0x60];
      //bool mShowPickUp;
      //bool mWasPickedUp;
      //std::chrono::steady_clock::time_point mPickupTime;
      //std::vector<BlockLegacy const*>     mCanPlaceOn;
      //uint64_t                                  mCanPlaceOnHash;
      //std::vector<BlockLegacy const*>     mCanDestroy;
      //uint64_t                                  mCanDestroyHash;
      //int                                  mBlockingTick;
  
      Item* getItem() {
          if (this && mItem) {
              return mItem.get();
  
          }
      else return nullptr;
      }

          short getDamageValue() {
          return reinterpret_cast<short (*)(ItemStackBase*)>(
              MemoryUtil::findSignature("40 53 48 83 EC ? 48 8B 51 ? 33 DB 48 85 D2"))(this);
      }


      bool isBlock() {
    static uintptr_t funcAddr = 0;
    if (funcAddr == 0) {
        uintptr_t callAddr = MemoryUtil::findSignature("E8 ? ? ? ? 84 C0 74 ? 48 8B CE E8 ? ? ? ? ? ? ? 48 85 C9");
        funcAddr = MemoryUtil::getFuncFromCall(callAddr);
    }


    using FuncType = bool(__fastcall*)(void*);
    auto func = reinterpret_cast<FuncType>(funcAddr);
    return func(this);
}
  };