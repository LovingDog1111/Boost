#include "DeathPos.h"

#include <Minecraft/TargetUtil.h>

#include "../../../../../../SDK/Render/Matrix.h"
#include "../../../ModuleManager.h"
#include "../Client/ColorsMod.h"

DeathPos::DeathPos() : Module("DeathPos", "Displays coordinates of your deaths", Category::RENDER) {
    registerSetting(
        new SliderSetting<float>("Size", "Scale of coordinate text", &coordSize, 1.0f, 0.5f, 3.0f));
    registerSetting(new BoolSetting("Reset", "Clear saved death positions", &resetFlag, false));
    registerSetting(
        new BoolSetting("Sync Color", "Sync death coords color with ColorsMod", &syncColor, false));
    registerSetting(
        new BoolSetting("LB Mode", "Display deathpos on low health (LB mode)", &lbMode, false));
}

void DeathPos::resetDeaths() {
    deathPositions.clear();
}

void DeathPos::onEnable() {
    resetDeaths();
}

void DeathPos::onDisable() {
    resetDeaths();
}

void DeathPos::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    bool shouldAddDeathPos = false;

    if(localPlayer->isDead()) {
        shouldAddDeathPos = true;
    } else if(lbMode) {
        float currentHealth = localPlayer->getHealth();

        if(currentHealth <= 1.0f) {
            shouldAddDeathPos = true;
        }
    }

    if(shouldAddDeathPos) {
        if(deathPositions.empty() || deathPositions.back().pos != localPlayer->getPos()) {
            int dim = localPlayer->getDimensionTypeComponent()->type;
            deathPositions.push_back({localPlayer->getPos(), dim});
        }
    }
}

void DeathPos::onD2DRender() {
    if(resetFlag) {
        resetDeaths();
        resetFlag = false;
    }

    Colors* colorsModule = ModuleManager::getModule<Colors>();
    UIColor syncedColor = colorsModule ? colorsModule->getColor() : UIColor(255, 255, 255, 255);

    for(const DeathPosition& death : deathPositions) {
        char coordText[150];
        UIColor textColor;

        if(syncColor) {
            textColor = syncedColor;
        } else {
            if(death.dimensionId == 0) {
                textColor = UIColor(0, 255, 0, 255);
            } else if(death.dimensionId == -1) {
                textColor = UIColor(255, 0, 0, 255);
            } else if(death.dimensionId == 1) {
                textColor = UIColor(128, 0, 255, 255);
            } else {
                textColor = UIColor(200, 200, 200, 255);
            }
        }

        if(death.dimensionId == 0) {
            sprintf_s(coordText, "Death Overworld: %.1f, %.1f, %.1f", death.pos.x, death.pos.y,
                      death.pos.z);
        } else if(death.dimensionId == -1) {
            sprintf_s(coordText, "Death Nether: %.1f, %.1f, %.1f", death.pos.x / 8.f, death.pos.y,
                      death.pos.z / 8.f);
        } else if(death.dimensionId == 1) {
            sprintf_s(coordText, "Death End: %.1f, %.1f, %.1f", death.pos.x, death.pos.y,
                      death.pos.z);
        } else {
            sprintf_s(coordText, "Death Dim %d: %.1f, %.1f, %.1f", death.dimensionId, death.pos.x,
                      death.pos.y, death.pos.z);
        }

        Vec3<float> offsetPos(death.pos.x, death.pos.y + 2.f, death.pos.z);
        Vec2<float> screenPos;

        if(Matrix::WorldToScreen(offsetPos, screenPos)) {
            float width = RenderUtil::getTextWidth(coordText, coordSize);
            float height = RenderUtil::getTextHeight(coordText, coordSize);

            Vec4<float> bg(screenPos.x - width / 2.f - 4.f, screenPos.y - height / 2.f - 2.f,
                           screenPos.x + width / 2.f + 4.f, screenPos.y + height / 2.f + 2.f);

            RenderUtil::fillRectangle(bg, UIColor(0, 0, 0, 120));
            RenderUtil::drawText(Vec2<float>(bg.x + 4.f, bg.y + 2.f), std::string(coordText),
                                 textColor, coordSize, true);
        }
    }
}