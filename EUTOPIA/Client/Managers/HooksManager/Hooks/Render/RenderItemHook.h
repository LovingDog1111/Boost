#pragma once
#include "../FuncHook.h"
#include "../../../../../SDK/Render/BaseActorRenderContext.h"
#include "../../../../../SDK/Core/ClientInstance.h"
#include "../../../../../SDK/GlobalInstance.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class RenderItemHook : public FuncHook {
private:
    using func_t = void(__thiscall*)(void*, void*, Actor*, ItemStack*, bool, bool, bool, bool);
    static inline func_t oFunc;

    static void RenderItemCallBack(void* a1, void* a2, Actor* entity, ItemStack* itemStack, bool a5,
                                   bool a6, bool a7, bool a8) {

     
            oFunc(a1, a2, entity, itemStack, a5, a6, a7, a8);
    }

public:
    RenderItemHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&RenderItemCallBack;
    }
};