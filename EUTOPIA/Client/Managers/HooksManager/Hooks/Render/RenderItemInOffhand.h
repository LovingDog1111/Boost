#pragma once
#pragma once
#include "../../../../../SDK/Render/BaseActorRenderContext.h"
#include "../../../../../SDK/Render/RenderItemInHandDescription.h"
#include "../FuncHook.h"
#include "../../../../../SDK/Render/ItemRenderInHand.h"

class RenderItemInOffhandHook : public FuncHook {
   private:
    using func_t = void*(__thiscall*)(void*, void*, Actor*, void*);
    static inline func_t oFunc;

    static void* RenderItemInOffhandCallback(ItemRenderInhand* _this, void* renderContext,
                                             Actor* entity,
                                             void* itemFalg) {
        return oFunc(_this, renderContext, entity, itemFalg);

    }

   public:
    RenderItemInOffhandHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&RenderItemInOffhandCallback;
    }
};