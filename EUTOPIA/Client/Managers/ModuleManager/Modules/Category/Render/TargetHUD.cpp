#include "TargetHUD.h"

#include <Minecraft/TargetUtil.h>

#include <algorithm>

#include "../../../../../../SDK/Render/Matrix.h"
#include "../../../ModuleManager.h"
#include "../Client/ColorsMod.h"

float hudSize;
float hudOpacity;
bool showSelf;
int underlineMode;
bool dynamicSize;
bool showHealthBar;
bool showAbsorption;

bool smoothHurt;

static std::unordered_map<Actor*, float> colorFactors;

TargetHUD::TargetHUD() : Module("TargetHUD", "Highlights players and entities", Category::RENDER) {
    registerSetting(new SliderSetting<float>("Size", "", &hudSize, 0.5f, 0.f, 1.f));
    registerSetting(new SliderSetting<float>("Opacity", "", &hudOpacity, 1.f, 0.f, 1.f));
    registerSetting(new BoolSetting("Self", "Render yourself", &showSelf, true));
    registerSetting(
        new EnumSetting("Underline", "Style", {"None", "Single", "Box"}, &underlineMode, 0));
    registerSetting(new BoolSetting("Dynamic Size", "Resize with distance", &dynamicSize, true));
    registerSetting(new BoolSetting("HealthBar", "Show health bar", &showHealthBar, true));
    registerSetting(new BoolSetting("Absorption", "Show absorption bar", &showAbsorption, true));
    registerSetting(new BoolSetting("Smooth Hurt", "", &smoothHurt, true));
}

static bool envaledchar(char c) {
    return !(c >= 0 && *reinterpret_cast<unsigned char*>(&c) < 128);
}

std::string sanitizeHUD(const std::string& text) {
    std::string out;
    bool wasValid = true;
    for(char c : text) {
        bool isValid = !envaledchar(c);
        if(wasValid) {
            if(!isValid)
                wasValid = false;
            else
                out += c;
        } else
            wasValid = isValid;
    }
    return out;
}

void TargetHUD::onD2DRender() {
    LocalPlayer* lp = GI::getLocalPlayer();
    if(!lp)
        return;

    Colors* colorsModule = ModuleManager::getModule<Colors>();
    if(!colorsModule)
        return;

    UIColor tagColor = colorsModule->getColor();
    tagColor.a = static_cast<int>(255 * hudOpacity);

    for(Actor* actor : ActorUtils::getActorList(true)) {
        if(!TargetUtil::isTargetValid(actor, false, 99.f) && !(actor == lp && showSelf))
            continue;

        Vec2<float> screenPos;
        if(!Matrix::WorldToScreen(actor->getEyePos().add2(0.f, 0.75f, 0.f), screenPos))
            continue;

        std::string name = sanitizeHUD(actor->getNameTag());
        float distance = lp->getPos().dist(actor->getPos());
        float scale = hudSize;
        if(dynamicSize) {
            if(distance <= 1.f)
                scale = hudSize * 3.f;
            else if(distance >= 4.f)
                scale = hudSize;
            else
                scale = hudSize * (3.f - ((distance - 1.f) * (2.f / 3.f)));
        }

        std::string distanceStr;
        char buffer[16];
        sprintf_s(buffer, "%.1fm", distance);
        distanceStr = buffer;

        std::string hpStr = std::to_string((int)actor->getHealth());

        float textWidth = D2D::getTextWidth(name, scale);
        float textHeight = D2D::getTextHeight(name, scale);
        float padding = 1.f * scale;
        Vec2<float> textPos(screenPos.x - textWidth / 2.f, screenPos.y - textHeight / 2.f);
        Vec4<float> bgRect(textPos.x - padding * 3.f, textPos.y - padding,
                           textPos.x + textWidth + padding * 3.f, textPos.y + textHeight + padding);

        float& colorFactor = colorFactors[actor];
        colorFactor += (actor->getHurtTime() > 8) ? 0.08f : -0.04f;
        colorFactor = std::clamp(colorFactor, 0.f, 1.f);

        UIColor rectColor =
            smoothHurt ? UIColor(int(255 * colorFactor), 0, 0, int(255 * hudOpacity))
                       : (actor->getHurtTime() > 8 ? UIColor(255, 0, 0, int(255 * hudOpacity))
                                                   : UIColor(0, 0, 0, int(255 * hudOpacity)));

        
      
        


        D2D::fillRectangle(bgRect, rectColor);
        D2D::drawText(textPos, name + " | " + distanceStr + " | " + hpStr,
                             UIColor(255, 255, 255, 255), scale, true);

        if(showHealthBar) {
            float healthRatio = std::clamp(actor->getHealth() / actor->getMaxHealth(), 0.f, 1.f);
            Vec4<float> healthBg(bgRect.x, bgRect.y - 2.f, bgRect.z, bgRect.y);
            Vec4<float> healthFg(bgRect.x, bgRect.y - 2.f,
                                 bgRect.x + (bgRect.z - bgRect.x) * healthRatio, bgRect.y);
            D2D::fillRectangle(healthBg, UIColor(40, 40, 40, int(255 * hudOpacity)));
            int red = std::clamp(int((1.0f - healthRatio) * 320), 0, 255);
            int green = std::clamp(int(healthRatio * 320), 0, 255);
            D2D::fillRectangle(healthFg, UIColor(red, green, 60, int(255 * hudOpacity)));
        }

        if(underlineMode == 1)
            D2D::fillRectangle(
                Vec4<float>(bgRect.x, bgRect.w - 1.f * scale, bgRect.z, bgRect.w), tagColor);
        if(underlineMode == 2)
            D2D::drawRectangle(bgRect, tagColor, 1.f);
    }
}


