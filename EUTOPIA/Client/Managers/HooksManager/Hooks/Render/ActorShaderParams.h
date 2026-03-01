#pragma once
#pragma once
#include "../../../../../SDK/Render/BaseActorRenderContext.h"
#include "../FuncHook.h"

class ActorShaderParamsHook : public FuncHook {
   private:
    using func_t = void(__thiscall*)(ScreenContext*, BaseActorRenderContext*, Actor*, mce::Color*, mce::Color*,mce::Color*,mce::Color*,void*,void*,void*,void*,void**,void*,float,void*);
    static inline func_t oFunc;

    static void ActorShaderParamsCallBack(ScreenContext *screenContext,
                                          BaseActorRenderContext *entityContext, Actor *entity,
                                          mce::Color *overlay, mce::Color *changeColor,
                                          mce::Color *changeColor2, mce::Color *glintColor,
                                          void *uvOffset1, void *uvOffset2, void *uvRot1,
                                          void *uvRot2, void **glintUVScale, void *uvAnim,
                                          float brightness, void *lightEmission) {
        static SetColor *setColor = ModuleManager::getModule<SetColor>();
        auto player = GI::getLocalPlayer();
        if(!player) {
            oFunc(screenContext, entityContext, entity, overlay, changeColor, changeColor2,
                  glintColor, uvOffset1, uvOffset2, uvRot1, uvRot2, glintUVScale, uvAnim,
                  brightness, lightEmission);
            return;
        }

        if(setColor && setColor->isEnabled() && player) {
            mce::Color overlayColor = setColor->ArmorColor.toMCColor();
            mce::Color changeColor = setColor->ItemColor.toMCColor();
            float Cbrightness = setColor->brightness;

            oFunc(screenContext, entityContext, entity, &overlayColor, &changeColor, changeColor2,
                  glintColor, uvOffset1, uvOffset2, uvRot1, uvRot2, glintUVScale, uvAnim,
                  Cbrightness, lightEmission);
            return;
        }
        oFunc(screenContext, entityContext, entity, overlay, changeColor, changeColor2, glintColor,
              uvOffset1, uvOffset2, uvRot1, uvRot2, glintUVScale, uvAnim, brightness,
              lightEmission);
    }


   public:
    ActorShaderParamsHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&ActorShaderParamsCallBack;
    }
};