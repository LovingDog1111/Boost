#pragma once
#include "../../Utils/MemoryUtil.h"

class BaseActorRenderContext;
class ItemStack;

class ItemRenderer {
   public:
    void renderGuiItemNew(BaseActorRenderContext* baseActorRenderContext, ItemStack* itemStack,
                          int mode, float x, float y, float opacity, float scale, float a9,
                          bool ench) {
        using oFunc_t = void(__fastcall*)(
            ItemRenderer*, BaseActorRenderContext * baseActorRenderContext, ItemStack * itemStack,
            int, float x, float y, bool ench, float opacity, float a9, float scale);
        static auto oFunc = reinterpret_cast<oFunc_t>(MemoryUtil::findSignature(
            "40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 44 89 4D ? 49 8B F8"));
        oFunc(this, baseActorRenderContext, itemStack, mode, x, y, ench, opacity, 0, scale);
    }
};

