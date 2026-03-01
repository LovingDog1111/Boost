#include "Watermark.h"
#include "../../../ModuleManager.h"
#include "../../../../../Client.h"

Watermark::Watermark() : Module("Watermark", "Displays a configurable watermark in the top-left corner.", Category::RENDER) {
    registerSetting(new BoolSetting("EnableGlow", "Add glow effect", &enableGlow, enableGlow));
    registerSetting(new BoolSetting("ShowVersion", "Show client version", &showVersion, showVersion));
    registerSetting(new BoolSetting("ShowDev", "Show dev label", &showDev, showDev));

    registerSetting(new SliderSetting<float>("TextSize", "Font size", &textSize, textSize, 0.5f, 3.f));
    registerSetting(new SliderSetting<float>("Padding", "Padding around text", &padding, padding, 0.f, 20.f));
    registerSetting(new SliderSetting<float>("CornerOffsetX", "Horizontal offset", &cornerOffsetX, cornerOffsetX, 0.f, 50.f));
    registerSetting(new SliderSetting<float>("CornerOffsetY", "Vertical offset", &cornerOffsetY, cornerOffsetY, 0.f, 50.f));
    registerSetting(new SliderSetting<float>("DevTextSize", "Dev text size", &devTextSize, devTextSize, 0.3f, 1.5f));

    registerSetting(new ColorSetting("TextColor", "Text color", &textColor, textColor));
    registerSetting(new ColorSetting("BackgroundColor", "Background color", &bgColor, bgColor));
    registerSetting(new ColorSetting("DevTextColor", "Dev text color", &devTextColor, devTextColor));
}

void Watermark::onD2DRender() {
    auto player = GI::getLocalPlayer();
    if (!player) return;

    std::string textToDraw = "Boost";
    if(showVersion) textToDraw += "+";

    float width = D2D::getTextWidth(textToDraw, textSize);
    float height = D2D::getTextHeight(textToDraw, textSize);

    Vec2<float> position(cornerOffsetX, cornerOffsetY);
    Vec4<float> bgRect(position.x - padding, position.y - padding,
        position.x + width + padding, position.y + height + padding);

    if (enableGlow) {
        D2D::drawGlowingText(position, textToDraw, textColor, textSize, 6.f);
    }
    else {
        D2D::drawText(position, textToDraw, textColor, textSize);
    }

    if (showDev) {
        std::string devText = "Recode";
        float devWidth = D2D::getTextWidth(devText, devTextSize);
        Vec2<float> devPosition(position.x + width + 4.f, position.y - (devTextSize * 3.f));

        if (enableGlow) {
            D2D::drawGlowingText(devPosition, devText, devTextColor, devTextSize, 3.f);
        }
        else {
            D2D::drawText(devPosition, devText, devTextColor, devTextSize);
        }
    }
}