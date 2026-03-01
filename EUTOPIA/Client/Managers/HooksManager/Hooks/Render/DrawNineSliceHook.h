#pragma once
#include <DrawUtil.h>

#include "../../Client/Client.h"
#include "../FuncHook.h"
class DrawNiceSliceHook : public FuncHook {
   private:
    using func_t = __int64(__fastcall*)(MinecraftUIRenderContext*, mce::TexturePtr*, NinesliceInfo* );
    static inline func_t oFunc;

    static __int64 DrawNiceSliceCallback(MinecraftUIRenderContext* ctx, mce::TexturePtr* texture,
                                         NinesliceInfo* info) {
        if(!texture || !texture->mResourceLocation)
            return oFunc(ctx, texture, info);

        const char* texPath = texture->mResourceLocation->mFilePath.empty()
                                  ? nullptr
                                  : texture->mResourceLocation->mFilePath.c_str();
        /*
                if (strcmp(texturePtr->ptrToPath->filePath.getText(),
           "textures/ui/focus_border_white") == 0 ||
            strcmp(texturePtr->ptrToPath->filePath.getText(), "textures/ui/slider_border") == 0 ||
            strcmp(texturePtr->ptrToPath->filePath.getText(), "textures/ui/loading_bar") == 0)*/
        //if(texPath) {
        //    if(strcmp(texPath, "textures/ui/focus_border_white") == 0 ||
        //       strcmp(texPath, "textures/ui/slider_border") == 0 ||
        //       strcmp(texPath, "textures/ui/loading_bar") == 0) {
        //        // Skip rendering these textures
        //        return 0;
        //    }
        //}

        return oFunc(ctx, texture, info);
    }

   public:
    DrawNiceSliceHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&DrawNiceSliceCallback;
    }
};
