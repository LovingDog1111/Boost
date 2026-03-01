#include "PlayerCounter.h"

#include "../../Utils/DrawUtil.h"
#include "../Client/Managers/ModuleManager/ModuleManager.h"
#include "../Client/Managers/ModuleManager/Modules/Category/Client/ColorsMod.h"


PlayerCounter::PlayerCounter() : Module("PlayerCounter", "Counts online players", Category::MISC) {
    registerSetting(
        new EnumSetting("Mode", "Render mode", {"Mojangles", "Custom"}, &rendermode, 0));

}

void PlayerCounter::onMCRender(MinecraftUIRenderContext* ctx) {
    if(rendermode != 0)
        return;

    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    Vec2<float> screen = GI::getClientInstance()->guiData->windowSizeScaled;
    Colors* colorsModule = ModuleManager::getModule<Colors>();
    UIColor color = colorsModule->getColor();

    playerCount = localPlayer->level->getPlayerList()->size();

    std::string text = "Players: " + std::to_string(playerCount);
    float textWidth = DrawUtil::getTextWidth(text, 1.f);

    float x = (screen.x / 2.f) - (textWidth / 2.f);

    PacketCounter* packetMod = ModuleManager::getModule<PacketCounter>();
    float y = 10.f;
    if(packetMod && packetMod->isEnabled())
        y += 14.f;

    DrawUtil::drawText({x, y}, text, color, 1.f);
}

void PlayerCounter::onD2DRender() {
    if(rendermode != 1 || !isEnabled())
        return;

    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    Colors* colorsModule = ModuleManager::getModule<Colors>();
    if(!colorsModule)
        return;

    UIColor accent = colorsModule->getColor();
    UIColor textColor(255, 255, 255, 255);

    Vec2<float> screen = GI::getClientInstance()->guiData->windowSizeReal;

    playerCount = localPlayer->level->getPlayerList()->size();
    std::string text = "Players: " + std::to_string(playerCount);

    float scale = 1.f;
    float lineHeight = D2D::getTextHeight(text, scale);
    float padding = 6.f;

    float panelWidth = D2D::getTextWidth(text, scale) + padding * 2.f;
    float panelHeight = lineHeight + padding * 2.f;

    float x = (screen.x - panelWidth) * 0.5f;
    float y = screen.y * 0.03f;

    PacketCounter* packetMod = ModuleManager::getModule<PacketCounter>();
    if(packetMod && packetMod->isEnabled())
        y += panelHeight + 2.f;

    Vec4<float> bg(x, y, x + panelWidth, y + panelHeight);
    D2D::fillRectangle(bg, UIColor(0, 0, 0, 170));
    D2D::drawRectangle(bg, accent, 8.f);

    Vec2<float> textPos(x + padding, y + padding);
    D2D::drawText(textPos, text, textColor, scale);
}
