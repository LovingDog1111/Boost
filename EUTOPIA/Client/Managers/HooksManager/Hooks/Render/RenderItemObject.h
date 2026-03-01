#pragma once
#pragma once
#include "../../../../../SDK/Render/BaseActorRenderContext.h"
#include "../FuncHook.h"
#include "../../../../../SDK/Render/ItemRenderInHand.h"

class RenderItemObjectHook : public FuncHook {
   private:
    using func_t = void(__thiscall*)(void*, void*, void*, void*,void*);
    static inline func_t oFunc;

    static void RenderItemObjectCallback(ItemRenderInhand* _this, void* renderContext, void* entity,
                                          void* renderMetadata, void* itemFalg) {
        oFunc(_this, renderContext, entity, renderMetadata, itemFalg);
    }

   public:
    RenderItemObjectHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&RenderItemObjectCallback;
    }
};