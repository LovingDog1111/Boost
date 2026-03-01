#pragma once
#include "../../../../../SDK/Render/BaseActorRenderContext.h"
#include "../../../../../SDK/Render/RenderItemInHandDescription.h"
#include "../FuncHook.h"

class RenderItemInHandHook : public FuncHook {
   private:
    using func_t = void*(__thiscall*)(RenderItemInHandDescription*, void*, void*, void*, void*,
                                      void*, bool, void*, unsigned __int16, void*);
    static inline func_t oFunc;

    static void* ItemInHandRenderer_renderCallback(RenderItemInHandDescription* _this,
                                                   void* renderObject, void* itemFlags,
                                                   void* material, void* glintTexture,
                                                   void* worldMatrix, bool isDrawingUI,
                                                   void* globalConstantBuffers,
                                                   unsigned __int16 viewId, void* renderMetadata) {
        auto result = oFunc(_this, renderObject, itemFlags, material, glintTexture, worldMatrix,
                            isDrawingUI, globalConstantBuffers, viewId, renderMetadata);

        static GlintColor* glintColorModule = ModuleManager::getModule<GlintColor>();

        if(!glintColorModule || !glintColorModule->isEnabled())
            return result;

        float r = glintColorModule->glintColor.r / 255.0f;
        float g = glintColorModule->glintColor.g / 255.0f;
        float b = glintColorModule->glintColor.b / 255.0f;

        float a = 1.0f;

        _this->mGlintColor = Vec3<float>(r, g, b);
        _this->mGlintAlpha = a;

        return result;
    }

   public:
    RenderItemInHandHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&ItemInHandRenderer_renderCallback;
    }
};