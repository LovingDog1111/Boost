#pragma once

#include "MobTags.h"

#include <algorithm>

#include "../../../../../../SDK/Render/Matrix.h"

bool showparrot;
bool showenderman;

MobTags::MobTags() : Module("MobTags", "Render nametags for mobs", Category::RENDER) {
    registerSetting(new SliderSetting<float>("Size", "", &tagSize, 0.5f, 0.f, 1.f));
    registerSetting(new SliderSetting<float>("Opacity", "", &opacity, 1.f, 0.f, 1.f));
    registerSetting(new BoolSetting("Dynamic Size", "", &dynamicSize, true));
    registerSetting(new BoolSetting("HealthBar", "", &showHealthBar, true));
    registerSetting(new BoolSetting("Absorption", "", &showAbsorption, true));
    registerSetting(new BoolSetting("Zombie", "Show zombie", &showZombie, true));
    registerSetting(new BoolSetting("Skeleton", "Show skeleton", &showSkeleton, true));
    registerSetting(new BoolSetting("Creeper", "Show creeper", &showCreeper, true));
    registerSetting(new BoolSetting("Spider", "Show spider", &showSpider, true));
    registerSetting(new BoolSetting("Wolf", "Show wolf", &showWolf, true));
    registerSetting(new BoolSetting("Cow", "Show cow", &showCow, true));
    registerSetting(new BoolSetting("Chicken", "Show chicken", &showChicken, true));
    registerSetting(new BoolSetting("Pig", "Show pig", &showPig, true));
    registerSetting(new BoolSetting("Parrot", "Show parrot", &showparrot, true));
    registerSetting(new BoolSetting("Enderman", "Show enderman", &showenderman, true));
}

static std::string getMobName(ActorType type) {
    switch(type) {
        case ActorType::Zombie:
            return "Zombie";
        case ActorType::Skeleton:
            return "Skeleton";
        case ActorType::Creeper:
            return "Creeper";
        case ActorType::Spider:
            return "Spider";
        case ActorType::Wolf:
            return "Wolf";
        case ActorType::Cow:
            return "Cow";
        case ActorType::Chicken:
            return "Chicken";
        case ActorType::Pig:
            return "Pig";
        case ActorType::Parrot:
            return "Parrot";
        case ActorType::EnderMan:
            return "Enderman";
        default:
            return "";
    }
}

void MobTags::onD2DRender() {
    LocalPlayer* lp = GI::getLocalPlayer();
    if(!lp)
        return;

    for(Actor* actor : ActorUtils::getActorList(false)) {
        ActorType type = ActorType::Undefined;
        if(auto typeComp = actor->getActorTypeComponent())
            type = static_cast<ActorType>(typeComp->id);

        if((type == Utils::getMob("Zombie") && !showZombie) ||
           (type == Utils::getMob("Skeleton") && !showSkeleton) ||
           (type == Utils::getMob("Creeper") && !showCreeper) ||
           (type == Utils::getMob("Spider") && !showSpider) ||
           (type == Utils::getMob("Wolf") && !showWolf) ||
           (type == Utils::getMob("Cow") && !showCow) ||
           (type == Utils::getMob("Chicken") && !showChicken) ||
           (type == Utils::getMob("Pig") && !showPig) ||
           (type == Utils::getMob("Parrot") && !showparrot) ||
           (type == Utils::getMob("Enderman") && !showenderman)) {
            continue;
        }

        Vec2<float> screenPos;
        if(!Matrix::WorldToScreen(actor->getEyePos().add2(0.f, 0.75f, 0.f), screenPos))
            continue;

        std::string name = getMobName(type);
        if(name.empty())
            continue;  
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

        float textWidth = D2D::getTextWidth(name, scale);
        float textHeight = D2D::getTextHeight(name, scale);
        Vec2<float> textPos(screenPos.x - textWidth / 2.f, screenPos.y - textHeight / 2.f);
        Vec4<float> bgRect(textPos.x - 2.f * scale, textPos.y - 1.f * scale,
                           textPos.x + textWidth + 2.f * scale,
                           textPos.y + textHeight + 1.f * scale);

        D2D::fillRectangle(bgRect, UIColor(12, 27, 46, static_cast<int>(255 * opacity)));

        if(showHealthBar) {
            float health = actor->getHealth();
            float maxHealth = actor->getMaxHealth();
            float healthRatio = std::min(health / maxHealth, 1.0f);
            float barHeight = 2.f * scale;
            float barWidth = bgRect.z - bgRect.x;
            Vec4<float> healthBg(bgRect.x, bgRect.y - barHeight - 2.f, bgRect.z, bgRect.y - 2.f);
            Vec4<float> healthFg(bgRect.x, bgRect.y - barHeight - 2.f,
                                 bgRect.x + barWidth * healthRatio, bgRect.y - 2.f);

            D2D::fillRectangle(healthBg,
                                      UIColor(40, 40, 40, static_cast<int>(255 * opacity)));

            int red = std::clamp(static_cast<int>((1.0f - healthRatio) * 320), 0, 255);
            int green = std::clamp(static_cast<int>(healthRatio * 320), 0, 255);
            D2D::fillRectangle(healthFg,
                                      UIColor(red, green, 60, static_cast<int>(255 * opacity)));

            if(showAbsorption) {
                float absorption = actor->getAbsorption();
                if(absorption > 0.f) {
                    float absorptionPercent = std::clamp(absorption / maxHealth, 0.0f, 1.0f);
                    float absorptionWidth = barWidth * absorptionPercent;
                    Vec4<float> absorptionRect(bgRect.x + 1.f, bgRect.y - barHeight * 1.3f,
                                               bgRect.x + absorptionWidth - 1.f,
                                               bgRect.y - barHeight - 2.f);
                    D2D::fillRectangle(absorptionRect, UIColor(255, 230, 80, 220));
                }
            }
        }

        D2D::drawText(textPos, name, UIColor(255, 255, 255, 255), scale, true);
    }
}
