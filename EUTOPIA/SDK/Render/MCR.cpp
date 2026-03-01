#include "MCR.h"

ShaderColor* MCR::currentColor = nullptr;

MinecraftUIRenderContext* MCR::renderCtx = nullptr;
Font* MCR::mcFont = nullptr;


void MCR::drawText2(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
                    float textSize) {
    if(!renderCtx || !mcFont)
        return;

    mce::Color mcColor = color.toMCColor();

    RectangleArea rect = {};
    rect._x0 = textPos.x;
    rect._x1 = textPos.x;
    rect._y0 = textPos.y;
    rect._y1 = textPos.y;

    TextMeasureData textMeasureData = {};
    textMeasureData.fontSize = textSize;
    textMeasureData.linePadding = 0.f;
    textMeasureData.renderShadow = true;
    textMeasureData.showColorSymbol = false;
    textMeasureData.hideHyphen = false;

    static CaretMeasureData caretMeasureData = {};
    caretMeasureData.position = -1.f;
    caretMeasureData.shouldRender = false;

    std::string temp = textStr;
    renderCtx->drawText(mcFont, &rect, &temp, &mcColor, mcColor.a, 0, &textMeasureData,
                        &caretMeasureData);
}

void MCR::setColor(const UIColor& color) {
    currentColor->color = color.toMCColor();
    currentColor->dirty = true;
}

float MCR::getTextWidth(const std::string& textStr, float textSize) {
    if(!renderCtx || !mcFont)
        return 0.f;
    std::string temp = textStr;
    return renderCtx->getLineLength(mcFont, &temp, textSize, false);
}
