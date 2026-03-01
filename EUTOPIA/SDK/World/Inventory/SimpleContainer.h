#pragma once
#include "../Item/ItemStack.h"
#include "../../../Utils/MemoryUtil.h"
class Container{
  public:
	ItemStack* getItem(int slot) {
		return MemoryUtil::callVirtualFunc<ItemStack*, int>(7, this, slot);
	}
	void setItem(int slot, ItemStack const& stack) {
		MemoryUtil::callVirtualFunc<void, int, ItemStack const&>(12, this, slot, stack);
	}
    void removeItem(int slot, int count) {
        MemoryUtil::callVirtualFunc<void, int, int>(14, this, slot, count);
    }


};
class SimpleContainer : public Container {
};
