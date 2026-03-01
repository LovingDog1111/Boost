#include "ArrayList.h"
#include "../../../ModuleManager.h"

bool modee = false;

ArrayList::ArrayList() : Module("ArrayList", "Displays enabled modules in the top-right corner of the screen.", Category::RENDER) {
    registerSetting(new EnumSetting("Style", "Selects the visual style of the array list.", { "None", "Bar", "SemiBar", "Outline" }, &mode, 2));
    registerSetting(new BoolSetting("ShowMode", "Displays the module mode next to its name.", &modee, modee));
    registerSetting(new EnumSetting("TextStyle", "Controls how secondary text is displayed.", { "Normal", "Grayed Out" }, &toolTips, 0));
    registerSetting(new BoolSetting("Glow", "Adds a glow effect to the module list.", &shouldGlow, shouldGlow));
    registerSetting(new SliderSetting<float>("TextSize", "Adjusts the font size of the module list.", &size, size, 1, 2));
    registerSetting(new SliderSetting<float>("Opacity", "Adjusts the background opacity.", &opacity, opacity, 0.f, 1.f));
    registerSetting(new BoolSetting("AlignBottom", "Renders the array list aligned to the bottom instead of the top.", &bottom, bottom));
}

struct sortByWidth {
    float textSize;
    sortByWidth(float size) : textSize(size * 10.f) {}

    bool operator()(Module* lhs, Module* rhs) const {
        if (!lhs || !rhs)
            return lhs > rhs;

        const std::string& leftName = lhs->getModuleName();
        const std::string& rightName = rhs->getModuleName();

        float leftWidth = D2D::getTextWidth(leftName, textSize);
        float rightWidth = D2D::getTextWidth(rightName, textSize);

        if (modee) {
            std::string leftMode;
            std::string rightMode;

            if (lhs)
                leftMode = lhs->getModeText();
            if (rhs)
                rightMode = rhs->getModeText();

            if (!leftMode.empty() && leftMode != "NULL")
                leftWidth += D2D::getTextWidth(" " + leftMode, textSize);

            if (!rightMode.empty() && rightMode != "NULL")
                rightWidth += D2D::getTextWidth(" " + rightMode, textSize);
        }

        return leftWidth > rightWidth;
    }
};

void ArrayList::onD2DRender() {
    auto player = GI::getLocalPlayer();
    if (player == nullptr) return;
    if(!GI::canUseMoveKeys())
        return;

    float offset = 0.f;
    float textSize = (float)size;
    float textPadding = 1.f * (textSize / 25.f);
    Vec2<float> windowSize = D2D::getWindowSize();
    windowSize.x -= offset;

    auto& moduleList = ModuleManager::moduleList;
    std::sort(moduleList.begin(), moduleList.end(), sortByWidth(size));

    int index = 0;
    float yOffset = 0.f + offset;
    if (bottom) yOffset = windowSize.y;

    auto themeMod = ModuleManager::getModule<Colors>();

    float currentY = yOffset;

    for (Module* mod : moduleList) {
        if (!mod->isVisible()) continue;

        if (mod->isEnabled()) mod->arraylistAnim += (1 - mod->arraylistAnim) * 0.1f;
        else mod->arraylistAnim += (0.f - mod->arraylistAnim) * 0.09f;

        if (mod->arraylistAnim <= 0.001f) continue;

        int curIndex = -index * themeMod->getSeperation();
        UIColor Mcolor = themeMod->getColor(curIndex);
        UIColor color = UIColor(Mcolor.r, Mcolor.g, Mcolor.b, mod->arraylistAnim * 255.f);

        float textWidth = D2D::getTextWidth(mod->getModuleName(), textSize);
        float textHeight = D2D::getTextHeight(mod->getModuleName(), textSize);
        float toolTipWidth = mod->getModeText() == "NULL" ? 0.f : D2D::getTextWidth(" " + mod->getModeText(), textSize);

        float animHeight = textHeight * mod->arraylistAnim;

        float rectTop, rectBottom;
        if (!bottom) {
            rectTop = currentY;
            rectBottom = currentY + animHeight;
        }
        else {
            rectBottom = currentY;
            rectTop = currentY - animHeight;
        }

        Vec2<float> textPos = Vec2<float>(windowSize.x - (textWidth + toolTipWidth + textPadding * 2.f) * mod->arraylistAnim, rectTop);
        Vec2<float> textPoss = Vec2<float>(windowSize.x - (textWidth + textPadding * 2.f) * mod->arraylistAnim, rectTop);

        if (mode == 1 || mode == 2) textPos.x -= 2.f;

        Vec4<float> rect = Vec4<float>(textPos.x - textPadding * 2.f, rectTop, textPos.x + textWidth + toolTipWidth + textPadding * 2.f, rectBottom);
        Vec4<float> rectt = Vec4<float>(textPoss.x - textPadding * 2.f, rectTop, textPoss.x + textWidth + textPadding * 2.f, rectBottom);

        Vec4<float> clipRect = Vec4<float>(0, rectTop, windowSize.x, rectBottom);

        D2D::PushAxisAlignedClip(clipRect, false);

        Vec2<float> toolTipPos = Vec2<float>(textPos.x + textWidth, rectTop);
        std::string modName = std::string(mod->getModeText());

        if (modee) {
            D2D::fillRectangle(rect, UIColor(0, 0, 0, int((opacity * mod->arraylistAnim) * 255.f)));

            if (shouldGlow) D2D::drawGlowingText(textPos, mod->getModuleName(), color, textSize, 4.5f);
            else D2D::drawText(textPos, mod->getModuleName(), color, textSize);

            if (toolTips == 0 && modName != "NULL") {
                if (shouldGlow) D2D::drawGlowingText(toolTipPos, " " + modName, UIColor(255, 255, 255, int(255 * mod->arraylistAnim)), textSize, 4.5f);
                else D2D::drawText(toolTipPos, " " + modName, UIColor(255, 255, 255, int(255 * mod->arraylistAnim)), textSize);
            }

            if (toolTips == 1 && modName != "NULL") {
                if (shouldGlow) D2D::drawGlowingText(toolTipPos, " " + modName, UIColor(180, 180, 180, int(255 * mod->arraylistAnim)), textSize, 4.5f);
                else D2D::drawText(toolTipPos, " " + modName, UIColor(180, 180, 180, int(255 * mod->arraylistAnim)), textSize);
            }
        }
        else {
            D2D::fillRectangle(rectt, UIColor(0, 0, 0, int((opacity * mod->arraylistAnim) * 255.f)));

            if (shouldGlow) D2D::drawGlowingText(textPoss, mod->getModuleName(), color, textSize, 4.5f);
            else D2D::drawText(textPoss, mod->getModuleName(), color, textSize);
        }

        if (mode == 1) {
            Vec4<float> bar(rect.z - 2.f, rect.y, rect.z, rect.w);
            D2D::fillRectangle(bar, color);
        }

        if (mode == 2) {
            Vec4<float> bar(rect.z - 2.f, rect.y + 3.f, rect.z, rect.w - 3.f);
            D2D::fillRectangle(bar, color);
        }

        if (mode == 3) {
            D2D::drawRectangle(rect, color, 1.f);
        }

        D2D::PopAxisAlignedClip();

        if (!bottom) currentY += animHeight;
        else currentY -= animHeight;

        ++index;
    }
}