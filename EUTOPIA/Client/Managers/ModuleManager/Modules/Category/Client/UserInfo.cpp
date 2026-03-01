#include "UserInfo.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "../../../../HooksManager/Hooks/Network/GetAvgPingHook.h"
#include "../../../ModuleManager.h"

UserInfo::UserInfo() : Module("UserInfo", "Displays username & system time", Category::CLIENT) {
    registerSetting(new BoolSetting("ShowUsername", "Display username", &showUsername, true));
    registerSetting(new BoolSetting("ShowTime", "Display system local time", &showTime, true));
    registerSetting(new BoolSetting("ShowServer", "Display server", &showServer, true));
    registerSetting(new BoolSetting("ShowPing", "Display ping", &showPing, true));
    registerSetting(new BoolSetting("ShowPlaytime", "Display playtime", &showPlaytime, true));
    registerSetting(new SliderSetting<int>("Opacity", "Background opacity", &opacity, 130, 0, 255));
    registerSetting(new SliderSetting<int>("Offset", "Distance from corner", &offset, 10, 0, 30));
    registerSetting(
        new ColorSetting("TextColor", "Text color for values", &textColor, UIColor(255, 255, 255)));
}

UserInfo::~UserInfo() {}

std::chrono::steady_clock::time_point sessionStart;
std::chrono::seconds accumulatedPlayTime{0};
bool wasConnected = false;

void UserInfo::onD2DRender() {
    Vec2<float> windowSize = GI::getClientInstance()->guiData->windowSizeReal;

    LocalPlayer* lp = GI::getLocalPlayer();
    if(!lp)
        return;

    std::string title = "UserInfo";
    std::string username = "Username: " + lp->getNameTag();

    std::string serverIP =
        GI::getPacketSender()->getmNetworkSystem()->getremoteConnector()->rakNet->mserverAddress;
    bool isConnected = !serverIP.empty() && serverIP != "0.0.0.0";
    if(!isConnected)
        serverIP = "Local World";
    std::string serverStr = "Server: " + serverIP;

    int ping = GetAveragePingHook::getLastPing();
    std::string pingStr = "Ping: " + std::to_string(ping) + " ms";

    std::time_t now = std::time(nullptr);
    std::tm timeInfo;
    localtime_s(&timeInfo, &now);
    char timeBuffer[16];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%I:%M:%S %p", &timeInfo);
    std::string timeStr = std::string("Time: ") + timeBuffer;

    if(isConnected) {
        if(!wasConnected) {
            sessionStart = std::chrono::steady_clock::now();
            wasConnected = true;
        }
    } else {
        if(wasConnected) {
            accumulatedPlayTime += std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - sessionStart);
            wasConnected = false;
        }
    }

    std::chrono::seconds totalPlayTime = accumulatedPlayTime;
    if(wasConnected) {
        totalPlayTime += std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - sessionStart);
    }

    int hours = totalPlayTime.count() / 3600;
    int minutes = (totalPlayTime.count() % 3600) / 60;
    int seconds = totalPlayTime.count() % 60;

    std::ostringstream playtimeStream;
    playtimeStream << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2)
                   << std::setfill('0') << minutes << ":" << std::setw(2) << std::setfill('0')
                   << seconds;
    std::string playtimeStr = playtimeStream.str();

    float textSize = 1.0f;
    float padding = 8.f;
    float spacing = 4.f;
    float radius = 6.f;

    float titleWidth = RenderUtil::getTextWidth(title, textSize + 0.2f);
    float userWidth = RenderUtil::getTextWidth(username, textSize);
    float timeWidth = RenderUtil::getTextWidth(timeStr, textSize);
    float serverWidth = RenderUtil::getTextWidth(serverStr, textSize);
    float pingWidth = RenderUtil::getTextWidth(pingStr, textSize);
    float playtimeWidth = RenderUtil::getTextWidth(playtimeStr, textSize);

    float contentWidth = titleWidth;
    if(showUsername)
        contentWidth = std::max(contentWidth, userWidth);
    if(showTime)
        contentWidth = std::max(contentWidth, timeWidth);
    if(showServer)
        contentWidth = std::max(contentWidth, serverWidth);
    if(showPing)
        contentWidth = std::max(contentWidth, pingWidth);
    if(showPlaytime)
        contentWidth = std::max(contentWidth, playtimeWidth);

    float lineHeight = RenderUtil::getTextHeight(username, textSize);
    float titleHeight = RenderUtil::getTextHeight(title, textSize + 0.2f);

    int linesToShow = 0;
    if(showUsername)
        linesToShow++;
    if(showTime)
        linesToShow++;
    if(showServer)
        linesToShow++;
    if(showPing)
        linesToShow++;
    if(showPlaytime)
        linesToShow++;

    float totalHeight = titleHeight + spacing + 1.f + spacing + lineHeight * linesToShow +
                        spacing * (linesToShow - 1);

    float posX = offset;
    float posY = windowSize.y - offset - totalHeight;

    Vec4<float> rect =
        Vec4<float>(posX, posY, posX + contentWidth + padding * 2.f, posY + totalHeight);

    UIColor bgColor = UIColor(30, 30, 40, opacity);
    UIColor glowColor = UIColor(80, 120, 200, (int)(opacity * 0.2f));
    UIColor outlineColor = UIColor(100, 200, 255, (int)(opacity * 0.6f));
    UIColor aquaColor = UIColor(0, 255, 255);
    UIColor valueColor = textColor;

    RenderUtil::fillRoundedRectangle(rect, bgColor, radius);
    RenderUtil::fillRoundedRectangle(Vec4<float>(rect.x + 1, rect.y + 1, rect.z - 1, rect.w - 1),
                                     glowColor, radius - 1);
    RenderUtil::drawRoundedRectangle(rect, outlineColor, radius, 1.5f);

    float textX = posX + padding;
    float textY = posY + spacing;

    RenderUtil::drawText(Vec2<float>(textX, textY), title, aquaColor, textSize + 0.2f, true);

    float underlineY = textY + titleHeight + 1.f;
    RenderUtil::fillRectangle(Vec4<float>(textX, underlineY, textX + titleWidth, underlineY + 1.f),
                              aquaColor);

    textY = underlineY + spacing * 2.f;

    if(showUsername) {
        RenderUtil::drawText(Vec2<float>(textX, textY), "Username: ", aquaColor, textSize);
        RenderUtil::drawText(
            Vec2<float>(textX + RenderUtil::getTextWidth("Username: ", textSize), textY),
            lp->getNameTag(), valueColor, textSize);
        textY += lineHeight + spacing;
    }

    if(showTime) {
        RenderUtil::drawText(Vec2<float>(textX, textY), "Time: ", aquaColor, textSize);
        RenderUtil::drawText(
            Vec2<float>(textX + RenderUtil::getTextWidth("Time: ", textSize), textY), timeBuffer,
            valueColor, textSize);
        textY += lineHeight + spacing;
    }

    if(showServer) {
        RenderUtil::drawText(Vec2<float>(textX, textY), "Server: ", aquaColor, textSize);
        RenderUtil::drawText(
            Vec2<float>(textX + RenderUtil::getTextWidth("Server: ", textSize), textY), serverIP,
            valueColor, textSize);
        textY += lineHeight + spacing;
    }

    if(showPing) {
        UIColor pingColor;
        if(ping < 50)
            pingColor = UIColor(100, 255, 100, 255);  // Green
        else if(ping < 100)
            pingColor = UIColor(255, 255, 100, 255);  // Yellow
        else if(ping < 200)
            pingColor = UIColor(255, 150, 100, 255);  // Orange
        else
            pingColor = UIColor(255, 100, 100, 255);  // Red

        RenderUtil::drawText(Vec2<float>(textX, textY), "Ping: ", aquaColor, textSize);
        RenderUtil::drawText(
            Vec2<float>(textX + RenderUtil::getTextWidth("Ping: ", textSize), textY),
            std::to_string(ping) + " ms", pingColor, textSize);
        textY += lineHeight + spacing;
    }

    RenderUtil::drawText(Vec2<float>(textX, textY), "Playtime: ", aquaColor, textSize);
    RenderUtil::drawText(
        Vec2<float>(textX + RenderUtil::getTextWidth("Playtime: ", textSize), textY), playtimeStr,
        valueColor, textSize);
}