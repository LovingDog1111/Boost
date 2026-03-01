#include "KillCounter.h"

#include <cmath>

KillCounter::KillCounter() : Module("KillCounter", "Counts kills", Category::MISC) {
    registerSetting(
        new EnumSetting("Mode", "Render mode", {"Mojangles", "Custom"}, &rendermode, 0));
}

void KillCounter::onEnable() {
    killCount = 0;
    target = nullptr;
    lastDeadTarget = nullptr;
}

void KillCounter::updateTarget() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;


    if(target) {
        if(!target->getEntityContext() || !target->isPlayer()) {
            target = nullptr;
        } else {
            bool alive = false;
            try {
                alive = target->isAlive();
            } catch(...) {
                target = nullptr;
            }

            if(!alive && lastDeadTarget != target) {
                killCount++;
                lastDeadTarget = target;
                target = nullptr;
            }
        }
    }


    if(target)
        return;

    std::vector<Actor*> actors = ActorUtils::getActorList(true);
    Actor* closest = nullptr;
    float closestDist = FLT_MAX;
    Vec3<float> playerPos = localPlayer->getPos();

    for(Actor* actor : actors) {
        if(!actor || actor == localPlayer)
            continue;
        if(!actor->isPlayer())
            continue;

        std::string name;
        try {
            name = actor->getNameTag();
        } catch(...) {
            continue;
        }
        if(name.empty())
            continue;

        if(!actor->getEntityContext())
            continue;

        int hurtTime = 0;
        try {
            hurtTime = actor->getHurtTime();
        } catch(...) {
            continue;
        }


        if(hurtTime <= 0)
            continue;

        float dist = 0.f;
        try {
            Vec3<float> pos = actor->getPos();
            float dx = pos.x - playerPos.x;
            float dy = pos.y - playerPos.y;
            float dz = pos.z - playerPos.z;
            dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        } catch(...) {
            continue;
        }

        if(dist > 10.f)
            continue;

        if(dist < closestDist) {
            closest = actor;
            closestDist = dist;
        }
    }

    if(closest)
        target = closest;
}



void KillCounter::onMCRender(MinecraftUIRenderContext* ctx) {
    if(rendermode != 0)
        return;

    updateTarget();

    Vec2<float> screen = GI::getClientInstance()->guiData->windowSizeScaled;
    Colors* colorsModule = ModuleManager::getModule<Colors>();
    UIColor color = colorsModule->getColor();

    std::string text = "Kills: " + std::to_string(killCount);
    float textWidth = DrawUtil::getTextWidth(text, 1.f);
    float x = (screen.x / 2.f) - (textWidth / 2.f);

    float y = 10.f;
    PacketCounter* packetMod = ModuleManager::getModule<PacketCounter>();
    PlayerCounter* playerMod = ModuleManager::getModule<PlayerCounter>();

    if(packetMod && packetMod->isEnabled())
        y += 14.f;
    if(playerMod && playerMod->isEnabled())
        y += 14.f;

    DrawUtil::drawText({x, y}, text, color, 1.f);
}

void KillCounter::onD2DRender() {
    if(rendermode != 1 || !isEnabled())
        return;

    updateTarget();

    Colors* colorsModule = ModuleManager::getModule<Colors>();
    if(!colorsModule)
        return;

    UIColor accent = colorsModule->getColor();
    UIColor textColor(255, 255, 255, 255);

    Vec2<float> screen = GI::getClientInstance()->guiData->windowSizeReal;
    std::string text = "Kills: " + std::to_string(killCount);

    float scale = 1.f;
    float padding = 6.f;
    float lineHeight = RenderUtil::getTextHeight(text, scale);

    float panelWidth = RenderUtil::getTextWidth(text, scale) + padding * 2.f;
    float panelHeight = lineHeight + padding * 2.f;

    float x = (screen.x - panelWidth) * 0.5f;
    float y = screen.y * 0.03f;

    PacketCounter* packetMod = ModuleManager::getModule<PacketCounter>();
    PlayerCounter* playerMod = ModuleManager::getModule<PlayerCounter>();

    if(packetMod && packetMod->isEnabled())
        y += panelHeight + 2.f;
    if(playerMod && playerMod->isEnabled())
        y += panelHeight + 2.f;

    Vec4<float> bg(x, y, x + panelWidth, y + panelHeight);
    RenderUtil::fillRoundedRectangle(bg, UIColor(0, 0, 0, 170), 8.f);
    RenderUtil::drawRoundedRectangle(bg, accent, 8.f, 1.f);

    RenderUtil::drawText({x + padding, y + padding}, text, textColor, scale);
}

void KillCounter::onDisable() {
    killCount = 0;
    target = nullptr;
    lastDeadTarget = nullptr;
}
