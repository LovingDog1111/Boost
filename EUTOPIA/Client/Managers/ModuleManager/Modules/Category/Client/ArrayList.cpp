#include "Arraylist.h"

#include "../../../../../../Utils/TimerUtil.h"
#include "../../../ModuleManager.h"

Arraylist::Arraylist() : Module("Arraylist", "Display module enabled", Category::CLIENT) {
    registerSetting(
        new EnumSetting("Mode", "Render mode", {"Mojangles", "Custom"}, &rendermode, 0));
    registerSetting(new BoolSetting("Bottom", "Display arraylist at the bottom", &bottom, true));
    registerSetting(new BoolSetting("Show Modes", "Show Modes", &showModes, true));
    registerSetting(new BoolSetting("Outline", "NULL", &outline, false));
    registerSetting(new EnumSetting("ModeColor", "NULL", {"White", "Gray"}, &modeColorEnum, 0));

    registerSetting(new SliderSetting<int>("Opacity", "NULL", &opacity, 130, 0, 255));
    registerSetting(new SliderSetting<int>("Spacing", "NULL", &spacing, -2, -3, 3));
    registerSetting(new SliderSetting<int>("Offset", "NULL", &offset, 10, 0, 30));
    registerSetting(new SliderSetting<float>("Size", "NULL", &size, 1.f, 0.5f, 1.5f));
}

bool Arraylist::sortByLength(Module* lhs, Module* rhs) {
    if(!lhs || !rhs)
        return false;
    float textSize = 1.f;

    float textWidth1 = RenderUtil::getTextWidth(lhs->getModuleName(), textSize);
    float textWidth2 = RenderUtil::getTextWidth(rhs->getModuleName(), textSize);

    static Arraylist* arralistMod = ModuleManager::getModule<Arraylist>();
    if(arralistMod->showModes) {
        if(lhs->getModeText() != "NULL")
            textWidth1 += RenderUtil::getTextWidth(" " + lhs->getModeText(), textSize);

        if(rhs->getModeText() != "NULL")
            textWidth2 += RenderUtil::getTextWidth(" " + rhs->getModeText(), textSize);
    }

    return (textWidth1 > textWidth2);
}

void Arraylist::refresharraylist() {
    auto& moduleList = ModuleManager::moduleList;
    moduleList.erase(std::remove(moduleList.begin(), moduleList.end(), nullptr), moduleList.end());

    std::sort(moduleList.begin(), moduleList.end(), [this](Module* lhs, Module* rhs) {
        if(!lhs || !rhs)
            return false;

        float textSize = 1.f;
        float width1 = RenderUtil::getTextWidth(lhs->getModuleName(), textSize);
        float width2 = RenderUtil::getTextWidth(rhs->getModuleName(), textSize);

        if(showModes) {
            if(lhs->getModeText() != "NULL")
                width1 += RenderUtil::getTextWidth(" " + lhs->getModeText(), textSize);
            if(rhs->getModeText() != "NULL")
                width2 += RenderUtil::getTextWidth(" " + rhs->getModeText(), textSize);
        }

        return width1 > width2;
    });
}

void Arraylist::onD2DRender() {
    if(rendermode != 1) {
        return;
    }
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    if(localPlayer == nullptr)
        return;
    color = ModuleManager::getModule<Colors>()->getColor();
    Vec2<float> windowSize =
        Game.getClientInstance()->guiData->windowSizeReal;  // D2D::getWindowSize();
    Vec2<float> windowSizeScaled = Game.getClientInstance()->guiData->windowSizeScaled;

    float textSize = size;
    float textPaddingX = std::max(1.f, ((float)spacing + 3.f)) * textSize;
    float textPaddingY = (float)spacing * textSize;
    float textHeight = RenderUtil::getTextHeight("", textSize);

    float lineWidth = 1.5f * textSize;

    Vec2<float> Pos = Vec2<float>(windowSize.x - (float)offset, (float)offset);
    if(bottom)
        Pos.y = windowSize.y - (textHeight + textPaddingY * 2.f) - (float)offset;

    UIColor mainColor = UIColor(color.r, color.g, color.b);  // UIColor(115, 135, 255);
    UIColor modeColor = UIColor(255, 255, 255);
    if(modeColorEnum == 1)
        modeColor = UIColor(175, 175, 175);

    refresharraylist();
    int index = 0;
    Vec4<float> lastRect;
    std::vector<Module*>& moduleList = ModuleManager::moduleList;

    for(auto& mod : moduleList) {
        if(!mod)
            continue;
        std::string moduleName = mod->getModuleName();
        std::string modeText = mod->getModeText();
        bool shouldRender = (mod->isEnabled() && mod->isVisible());

        if(!showModes)
            modeText = "NULL";

        mod->arraylistAnim = Math::lerp(mod->arraylistAnim, shouldRender ? 1.05f : -0.1f,
                                        RenderUtil::deltaTime * 15.f);

        if(mod->arraylistAnim > 1.f)
            mod->arraylistAnim = 1.f;

        if(mod->arraylistAnim < 0.f)
            mod->arraylistAnim = 0.f;

        if(mod->arraylistAnim > 0.f) {
            float fullTextWidth = RenderUtil::getTextWidth(moduleName, textSize);
            if(modeText != "NULL")
                fullTextWidth += RenderUtil::getTextWidth(" " + modeText, textSize);

            Vec4<float> rectPos =
                Vec4<float>(Pos.x - textPaddingX - fullTextWidth - textPaddingX, Pos.y, Pos.x,
                            Pos.y + textPaddingY + textHeight + textPaddingY);

            float animRect = (windowSize.x + 25.f - rectPos.x) * (1.f - mod->arraylistAnim);
            rectPos.x += animRect;
            rectPos.z += animRect;

            Vec2<float> textPos = Vec2<float>(rectPos.x + textPaddingX, rectPos.y + textPaddingY);

            RenderUtil::fillRectangle(rectPos, UIColor(0, 0, 0, opacity));
            if(outline) {
                RenderUtil::fillRectangle(
                    Vec4<float>(rectPos.x - lineWidth, rectPos.y, rectPos.x, rectPos.w), mainColor);
                RenderUtil::fillRectangle(
                    Vec4<float>(rectPos.z, rectPos.y, rectPos.z + lineWidth, rectPos.w), mainColor);
                if(index == 0) {
                    if(bottom)
                        RenderUtil::fillRectangle(
                            Vec4<float>(rectPos.x - lineWidth, rectPos.w, rectPos.z + lineWidth,
                                        rectPos.w + lineWidth),
                            mainColor);
                    else
                        RenderUtil::fillRectangle(
                            Vec4<float>(rectPos.x - lineWidth, rectPos.y - lineWidth,
                                        rectPos.z + lineWidth, rectPos.y),
                            mainColor);
                } else {
                    if(bottom)
                        RenderUtil::fillRectangle(
                            Vec4<float>(lastRect.x - lineWidth, rectPos.w - lineWidth, rectPos.x,
                                        rectPos.w),
                            mainColor);
                    else
                        RenderUtil::fillRectangle(Vec4<float>(lastRect.x - lineWidth, rectPos.y,
                                                              rectPos.x, rectPos.y + lineWidth),
                                                  mainColor);
                }
            }
            RenderUtil::drawText(textPos, moduleName, mainColor, textSize, true);
            if(modeText != "NULL") {
                textPos.x += RenderUtil::getTextWidth(moduleName + " ", textSize);
                RenderUtil::drawText(textPos, modeText, modeColor, textSize, true);
            }
            lastRect = rectPos;
        }

        float yAnim = mod->arraylistAnim * 1.25f;
        if(yAnim > 1.f)
            yAnim = 1.f;

        Pos.y += (textHeight + textPaddingY * 2.f) * yAnim * (bottom ? -1.f : 1.f);

        if(shouldRender)
            index++;
    }
    if(outline) {
        if(bottom)
            RenderUtil::fillRectangle(Vec4<float>(lastRect.x - lineWidth, lastRect.y - lineWidth,
                                                  lastRect.z + lineWidth, lastRect.y),
                                      mainColor);
        else
            RenderUtil::fillRectangle(Vec4<float>(lastRect.x - lineWidth, lastRect.w,
                                                  lastRect.z + lineWidth, lastRect.w + lineWidth),
                                      mainColor);
    }
}

void Arraylist::onMCRender(MinecraftUIRenderContext* ctx) {
    if(rendermode != 0)
        return;

    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    Vec2<float> screen = GI::getClientInstance()->guiData->windowSizeScaled;
    Colors* colorsModule = ModuleManager::getModule<Colors>();
    UIColor mainColor = colorsModule->getColor();
    UIColor modeColor = modeColorEnum == 1 ? UIColor(175, 175, 175) : UIColor(255, 255, 255);

    refresharraylist();
    float offsetY = offset;
    if(bottom)
        offsetY = screen.y - offset;

    std::vector<Module*>& moduleList = ModuleManager::moduleList;
    int index = 0;

    for(auto& mod : moduleList) {
        if(!mod)
            continue;
        std::string moduleName = mod->getModuleName();
        std::string modeText = showModes ? mod->getModeText() : "NULL";
        bool shouldRender = mod->isEnabled() && mod->isVisible();

        mod->arraylistAnim =
            Math::lerp(mod->arraylistAnim, shouldRender ? 1.f : 0.f, RenderUtil::deltaTime * 15.f);
        mod->arraylistAnim = std::clamp(mod->arraylistAnim, 0.f, 1.f);
        if(mod->arraylistAnim <= 0.f)
            continue;

        float textSize = size * 0.85f;
        float fullTextWidth = DrawUtil::getTextWidth(moduleName, textSize);
        if(modeText != "NULL")
            fullTextWidth += DrawUtil::getTextWidth(" " + modeText, textSize);

        float x = screen.x - offset - fullTextWidth;
        float y = bottom
                      ? offsetY - (DrawUtil::getTextHeight(textSize) + spacing) * mod->arraylistAnim
                      : offsetY;
        x += (screen.x + 25.f - x) * (1.f - mod->arraylistAnim);

        DrawUtil::drawText({x, y}, moduleName, mainColor, textSize);
        if(modeText != "NULL") {
            float textOffsetX = DrawUtil::getTextWidth(moduleName + " ", textSize);
            DrawUtil::drawText({x + textOffsetX, y}, modeText, modeColor, textSize);
        }

        float yAnim = (DrawUtil::getTextHeight(textSize) + spacing) * mod->arraylistAnim * 1.25f;
        offsetY += bottom ? -yAnim : yAnim;

        if(shouldRender)
            index++;
    }
}