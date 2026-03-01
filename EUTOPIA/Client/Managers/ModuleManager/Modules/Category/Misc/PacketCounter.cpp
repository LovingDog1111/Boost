#include "PacketCounter.h"

#include "../../Utils/DrawUtil.h"
#include "../Client/Managers/ModuleManager/ModuleManager.h"
#include "../Client/Managers/ModuleManager/Modules/Category/Client/ColorsMod.h"

float refreshRate = 20.f;
bool resetOnReach;

PacketCounter::PacketCounter()
    : Module("PacketCounter", "Counts outgoing packets", Category::MISC) {
    registerSetting(
        new EnumSetting("Mode", "Render mode", {"Mojangles", "Custom"}, &rendermode, 0));

    registerSetting(new EnumSetting("Counter", "Which packets to count",
                                    {"Ingoing", "Outgoing", "Both"}, &counterMode, 0));
    registerSetting(new BoolSetting("Reset Packet Count",
                                    "Reset packet count after reaching the set amount",
                                    &resetOnReach, false));
    registerSetting(new SliderSetting<float>(
        "Reset", "Reset when this amount of packets is reached", &refreshRate, 20, 1, 100));
}

void PacketCounter::onSendPacket(Packet* packet) {
    if(counterMode == 1 || counterMode == 2) {
        outgoingCount++;
    }
    if(resetOnReach && outgoingCount >= refreshRate) {
        outgoingCount = 0;
    }
}

void PacketCounter::onReceivePacket(Packet* packet, bool* cancel) {
    if(counterMode == 0 || counterMode == 2) {
        incomingCount++;
    }
    if(resetOnReach && incomingCount >= refreshRate) {
        incomingCount = 0;
    }
}

void PacketCounter::onMCRender(MinecraftUIRenderContext* ctx) {
    if(rendermode != 0)
        return;

    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    Vec2<float> screen = GI::getClientInstance()->guiData->windowSizeScaled;
    Colors* colorsModule = ModuleManager::getModule<Colors>();
    UIColor color = colorsModule->getColor();

    std::string text;
    if(counterMode == 2) {
        text = "Packets In: " + std::to_string(incomingCount) +
               " | Packets Out: " + std::to_string(outgoingCount);
    } else if(counterMode == 1) {
        text = "Packets Out: " + std::to_string(outgoingCount);
    } else {
        text = "Packets In: " + std::to_string(incomingCount);
    }

    float textWidth = DrawUtil::getTextWidth(text, 1.f);
    float x = (screen.x / 2.f) - (textWidth / 2.f);
    float y = 10.f;

    DrawUtil::drawText({x, y}, text, color, 1.f);
}

void PacketCounter::onDisable() {
    incomingCount = 0;
    outgoingCount = 0;
}

void PacketCounter::onD2DRender() {
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

    std::string text;
    if(counterMode == 2) {
        text = "Packets In: " + std::to_string(incomingCount) +
               " | Packets Out: " + std::to_string(outgoingCount);
    } else if(counterMode == 1) {
        text = "Packets Out: " + std::to_string(outgoingCount);
    } else {
        text = "Packets In: " + std::to_string(incomingCount);
    }

    float scale = 1.f;
    float lineHeight = RenderUtil::getTextHeight(text, scale);
    float padding = 6.f;

    float panelWidth = RenderUtil::getTextWidth(text, scale) + padding * 2.f;
    float panelHeight = lineHeight + padding * 2.f;

    float x = (screen.x - panelWidth) * 0.5f;
    float y = screen.y * 0.03f;

    Vec4<float> bg(x, y, x + panelWidth, y + panelHeight);
    RenderUtil::fillRoundedRectangle(bg, UIColor(0, 0, 0, 170), 8.f);
    RenderUtil::drawRoundedRectangle(bg, accent, 8.f, 1.f);

    Vec2<float> textPos(x + padding, y + padding);
    RenderUtil::drawText(textPos, text, textColor, scale);
}
