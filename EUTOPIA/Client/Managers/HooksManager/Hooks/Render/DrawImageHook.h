#pragma once
#include "../../Client/Client.h"
#include "../FuncHook.h"
#include "../SDK/Render/MCR.h"

class DrawImageHook : public FuncHook {
   private:
    using func_t = __int64(__fastcall*)(MinecraftUIRenderContext*,  mce::TexturePtr*, Vec2<float>&,
                                        Vec2<float>&, Vec2<float>&, Vec2<float>&);
    static inline func_t oFunc;

    static __int64 DrawImageCallback(MinecraftUIRenderContext* ctx, mce::TexturePtr* texture,
                                     Vec2<float>& pos, Vec2<float>& size, Vec2<float>& uvPos,
                                     Vec2<float>& uvSize) {
        if(!texture || !texture->mClientTexture)
            return oFunc(ctx, texture, pos, size, uvPos, uvSize);

        const char* texPath = texture->mResourceLocation->mFilePath.empty()
                                  ? nullptr
                                  : texture->mResourceLocation->mFilePath.c_str();

        if(texPath && strcmp(texPath, "textures/ui/title") == 0) {
            if(texPath) {
                if(!MCR::renderCtx)
                    return 0;
                if(texPath && strcmp(texPath, "textures/ui/title") == 0) {
                    if(!MCR::renderCtx)
                        return 0;

                    static const std::string text = "Boostv4";
                    static Colors* colorsMod = ModuleManager::getModule<Colors>();
                    float scale = 6.0f;

                    float textWidth = RenderUtil::getTextWidth(text, scale);

                    float textX = pos.x + (size.x / 2.f) - (textWidth / 2.f);
                    float textY = pos.y;

                    Vec2<float> textPos = {textX, textY};

                    RenderUtil::drawText(textPos, text, colorsMod->getColor().toMCColor(), scale);
                    MCR::renderCtx->flushText(1.0f);

                    return 0;
                }
            }
        }
        return oFunc(ctx, texture, pos, size, uvPos, uvSize);
    }

   public:
    DrawImageHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&DrawImageCallback;
    }
};
