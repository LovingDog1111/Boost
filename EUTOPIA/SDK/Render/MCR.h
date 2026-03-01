#pragma once
#include "../SDK/Render/MinecraftUIRenderContext.h"
#include "../Utils/ColorUtil.h"

namespace MCR {

extern MinecraftUIRenderContext* renderCtx;
extern Font* mcFont;

void drawText2(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
              float textSize = 1.f);
float getTextWidth(const std::string& textStr, float textSize = 1.f);

extern ShaderColor* currentColor;



static void setColor(const UIColor& color);

}  