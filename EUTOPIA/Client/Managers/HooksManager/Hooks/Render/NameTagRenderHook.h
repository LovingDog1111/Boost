#pragma once
#include "../FuncHook.h"

class NameTagRenderHook : public FuncHook {
   private:
    using func_t = void(__thiscall*)(void* _this, void* a2, void* a3, void* a4, Actor* actor,
                                     void* a6, Vec3<float>* pos, bool unknown, float deltaThing,
                                     mce::Color* color);
    static inline func_t oFunc = nullptr;



    static void NameTagRenderCallback(void* _this, void* a2, void* a3, void* a4, Actor* actor,
                                      void* a6, Vec3<float>* pos, bool unknown, float deltaThing,
                                      mce::Color* color) {
        ModuleManager::onRenderNameTag(actor, pos, unknown, deltaThing, color);


        auto nameTags = ModuleManager::getModule<NameTags>();

     
        if(nameTags && nameTags->isEnabled() && GI::getLocalPlayer() != nullptr) {
            if(color) {
                color->a = 0;
                color->r = 0;
                color->g = 0;
                color->b = 0;
            }

            if(pos) {
                pos->x = FLT_MAX;
                pos->y = FLT_MAX;
                pos->z = FLT_MAX;
            }
        }

        auto targetHUD = ModuleManager::getModule<TargetHUD>();
        if(targetHUD && targetHUD->isEnabled() && GI::getLocalPlayer() != nullptr) {
            if(color) {
                color->a = 0;
                color->r = 0;
                color->g = 0;
                color->b = 0;
            }

            if(pos) {
                pos->x = FLT_MAX;
                pos->y = FLT_MAX;
                pos->z = FLT_MAX;
            }
        }


        oFunc(_this, a2, a3, a4, actor, a6, pos, unknown, deltaThing, color);
    }

   public:
    NameTagRenderHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&NameTagRenderCallback;
    }
};
