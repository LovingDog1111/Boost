#include "NameTag.h"

#include <Minecraft/TargetUtil.h>

#include <algorithm>

#include "../../../../../../SDK/Render/Matrix.h"
#include "../../../ModuleManager.h"
#include "../Client/ColorsMod.h"

int styletag = 0;
#include <map>
#include <mutex>
#include <unordered_map>

std::mutex bpsMutex;
std::unordered_map<Actor*, float> bpsMap;
std::unordered_map<Actor*, std::map<int64_t, float>> bpsHistory;
std::unordered_map<Actor*, float> avgBpsMap;



NameTags::NameTags() : Module("NameTags", "Better nametags.", Category::RENDER) {
    registerSetting(new SliderSetting<float>("Size", "NULL", &tagSize, 0.5f, 0.f, 1.f));
    registerSetting(new SliderSetting<float>("Opacity", "NULL", &opacity, 1.f, 0.f, 1.f));
    registerSetting(new BoolSetting("Self", "Render yourself", &showSelf, true));
    registerSetting(
        new EnumSetting("Mode", "Underline style", {"None", "Single", "Box"}, &underlineMode, 0));
    registerSetting(new BoolSetting("Dynamic Size", "Resize with distance", &dynamicSize, true));
    registerSetting(new BoolSetting("HealthBar", "Show health bar", &showHealthBar, true));
    registerSetting(new BoolSetting("Absorption", "Show absorption bar", &showAbsorption, true));
    registerSetting(
        new EnumSetting("Style", "Nametag background style", {"Default", "Rounded"}, &styletag, 0));
}

static bool envaledchar(char c) {
    return !(c >= 0 && *reinterpret_cast<unsigned char*>(&c) < 128);
}

std::string sanitizex(const std::string& text) {
    std::string out;
    bool wasValid = true;
    for(char c : text) {
        bool isValid = !envaledchar(c);
        if(wasValid) {
            if(!isValid) {
                wasValid = false;
            } else {
                out += c;
            }
        } else {
            wasValid = isValid;
        }
    }
    return out;
}

void NameTags::onRenderNameTag(Actor* actor, Vec3<float>* pos, bool unknownFlag, float delta,
                               mce::Color* color) {
    if(!actor || !color)
        return;

    color->r = 0;
    color->g = 0;
    color->b = 0;
    color->a = 0;
}

void NameTags::onD2DRender() {
    LocalPlayer* lp = GI::getLocalPlayer();
    if(!lp)
        return;

    Colors* colorsModule = ModuleManager::getModule<Colors>();
    if(!colorsModule)
        return;

    UIColor tagColor = colorsModule->getColor();
    tagColor.a = static_cast<int>(255 * opacity);

    for(Actor* actor : ActorUtils::getActorList(true)) {
        if(!TargetUtil::isTargetValid(actor, false, 150.f) && !(actor == lp && showSelf))
            continue;

        if(!actor->isPlayer())
            continue;
        

        Vec2<float> screenPos;
        if(!Matrix::WorldToScreen(actor->getEyePos().add2(0.f, 0.75f, 0.f), screenPos))
            continue;

        std::string name = sanitizex(actor->getNameTag());

        float distance = lp->getPos().dist(actor->getPos());
        float scale = tagSize;
        if(dynamicSize) {
            if(distance <= 1.f)
                scale = tagSize * 3.f;
            else if(distance >= 4.f)
                scale = tagSize;
            else
                scale = tagSize * (3.f - ((distance - 1.f) * (2.0f / 3.f)));
        }

        float textWidth = RenderUtil::getTextWidth(name, scale);
        float textHeight = RenderUtil::getTextHeight(name, scale);
        float padding = 1.f * scale;
        Vec2<float> textPos(screenPos.x - textWidth / 2.f, screenPos.y - textHeight / 2.f);
        Vec4<float> bgRect(textPos.x - padding * 3.0f, textPos.y - padding,
                           textPos.x + textWidth + padding * 3.0f,
                           textPos.y + textHeight + padding);

        float health = actor->getHealth();
        float maxHealth = actor->getMaxHealth();
        float absorption = actor->getAbsorption();
        float healthRatio = std::min(health / maxHealth, 1.0f);
        float barHeight = 2.f * scale;
        float barWidth = bgRect.z - bgRect.x;
        Vec4<float> healthBg(bgRect.x, bgRect.y - barHeight - 2.f, bgRect.z, bgRect.y - 2.f);
        Vec4<float> healthFg(bgRect.x, bgRect.y - barHeight - 2.f,
                             bgRect.x + barWidth * healthRatio, bgRect.y - 2.f);

        if(styletag == 0)
            RenderUtil::fillRectangle(bgRect, UIColor(12, 27, 46, static_cast<int>(255 * opacity)));
        else
            RenderUtil::fillRoundedRectangle(
                bgRect, UIColor(0, 0, 0, static_cast<int>(255 * opacity)), 10.f);

        if(showHealthBar) {
            RenderUtil::fillRectangle(healthBg,
                                      UIColor(40, 40, 40, static_cast<int>(255 * opacity)));
            int red = std::clamp(static_cast<int>((1.0f - healthRatio) * 320), 0, 255);
            int green = std::clamp(static_cast<int>(healthRatio * 320), 0, 255);
            RenderUtil::fillRectangle(healthFg,
                                      UIColor(red, green, 60, static_cast<int>(255 * opacity)));

            if(showAbsorption && absorption > 0.f) {
                float absorptionPercent = std::clamp(absorption / maxHealth, 0.0f, 1.0f);
                float absorptionWidth = barWidth * absorptionPercent;
                Vec4<float> absorptionRect(bgRect.x + 1.f, bgRect.y - barHeight * 1.3f,
                                           bgRect.x + absorptionWidth - 1.f,
                                           bgRect.y - barHeight - 2.f);

                UIColor absorptionColor(255, 230, 80, 220);
                RenderUtil::fillRoundedRectangle(absorptionRect, absorptionColor, barHeight / 4.0f);
                UIColor borderColor(255, 255, 120, 200);
                RenderUtil::drawRoundedRectangle(absorptionRect, borderColor, barHeight / 4.0f,
                                                 0.5f);
            }

            if(scale > 0.6f) {
                std::string healthText = std::to_string((int)health);
                if(absorption > 0)
                    healthText += "+" + std::to_string((int)absorption);
                float textScale = 0.3f * scale;
                float valueW = RenderUtil::getTextWidth(healthText, textScale);
                float valueH = RenderUtil::getTextHeight(healthText, textScale);
                Vec2<float> healthTextPos(bgRect.z + 4.f * scale,
                                          healthBg.y + (barHeight - valueH) / 2.0f);
                RenderUtil::drawText(healthTextPos, healthText, UIColor(255, 255, 255, 255),
                                     textScale);
            }
        }

        if(underlineMode == 1)
            RenderUtil::fillRectangle(
                Vec4<float>(bgRect.x, bgRect.w - 1.f * scale, bgRect.z, bgRect.w), tagColor);
        if(underlineMode == 2)
            RenderUtil::drawRectangle(bgRect, tagColor, 1.0f);

        RenderUtil::drawText(textPos, name, UIColor(255, 255, 255, 255), scale, true);

        Vec2<float> armorPos(screenPos.x - 10.f, screenPos.y - 20.f);
        for(int i = 0; i < 4; ++i) {
            ItemStack* armorStack = actor->getArmor(i);
            if(armorStack && armorStack->getItem()) {
                armorPos.x += 18.f;
            }
        }
    }
}

void NameTags::onMCRender(MinecraftUIRenderContext* renderCtx) {
}
