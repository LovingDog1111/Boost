#pragma once
#include "../FuncHook.h"
#include "../../../../../SDK/Render/MinecraftUIRenderContext.h"

class DrawTextHook : public FuncHook {
   protected:
    using func_t = __int64(__fastcall*)(MinecraftUIRenderContext*, Font*, float*, std::string*,
                                        mce::Color*, float, unsigned int, TextMeasureData*,
                                        CaretMeasureData*);
    static inline func_t oFunc;

    static __int64 callback(MinecraftUIRenderContext* _this, Font* font, float* pos,
                            std::string* text, mce::Color* color, float alpha,
                            unsigned int textAlignment, TextMeasureData* textMeasureData,
                            CaretMeasureData* caretMeasureData) {
        return oFunc(_this, font, pos, text, color, alpha, textAlignment, textMeasureData,
                     caretMeasureData);
    }

   public:
    DrawTextHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&callback;
    }
};