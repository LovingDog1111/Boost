#include "SearchBox.h"

#include <Windows.h>
#include <wininet.h>

#include <chrono>
#include <deque>
#include <string>
#include <unordered_set>
//#include "ircboi.h"
#include "..\Client\Managers\ModuleManager\ModuleManager.h"
#pragma comment(lib, "wininet.lib")

std::deque<ChatMessage> SearchBox::messageLog;
std::unordered_set<std::string> messageSet;


static float getTimeSeconds() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    auto now = steady_clock::now();
    return duration_cast<duration<float>>(now - start).count();
}

SearchBox::SearchBox() : Module("IRC Chat", "Movable IRC Chat Box", Category::CLIENT) {
    searchQuery = "";
    searchFocused = false;
    searchBoxClicked = false;
    registerSetting(new SliderSetting<float>("Size", "UI Scale", &size, 1.0f, 0.7f, 1.5f));
}

bool SearchBox::shouldRender() const {
    auto clickGui = ModuleManager::getModule<ClickGUI>();
    return clickGui && clickGui->isEnabled();
}
void SearchBox::renderUI() {
    if(!shouldRender())
        return;

    auto colors = ModuleManager::getModule<Colors>();
    if(!colors)
        return;

    auto colortouse = colors->getColor();

    float s = size;
    Vec2<float> pos = {panelPos.x * s, panelPos.y * s};
    Vec2<float> panelSz = {panelSize.x * s, panelSize.y * s};
    Vec4<float> mainRect = {pos.x, pos.y, pos.x + panelSz.x, pos.y + panelSz.y};

    RenderUtil::fillRoundedRectangle(mainRect, UIColor(25, 25, 25, 240), 12.f * s);
    RenderUtil::drawRoundedRectangle(mainRect, colortouse, 12.f * s, 3.f * s);

   

    const float padding = 15.f * s;
    const float searchHeight = 28.f * s;
    const float bottomPadding = 15.f * s;
    float headerHeight = 50.f * s;
    float headerTop = pos.y + padding;
    float headerBottom = headerTop + headerHeight;
    float headerLeft = pos.x + padding;
    float headerRight = pos.x + panelSz.x - padding;

    float bulletRadius = 6.f * s;
    Vec2<float> bulletPos = {headerLeft + bulletRadius, headerTop + headerHeight / 2.f};
    RenderUtil::fillCircle(bulletPos, colortouse, bulletRadius);

    std::string title = "Boost IRC";
    std::vector<ColorUtil::GradientStop> titleGradient = {{0.f, colortouse},
                                                          {1.f, colortouse}};

    RenderUtil::drawGradientTextWithSweep(
        {bulletPos.x + bulletRadius * 2.f + 6.f * s, bulletPos.y - 12.f * s}, title, titleGradient,
        1.2f * s, true, getTimeSeconds(), 1.0f, 0.3f, 1.0f, 3.0f);

    float chatTopY = headerBottom + 10.f * s;
    float chatBottomY = pos.y + panelSz.y - bottomPadding - searchHeight - 10.f * s;
    Vec4<float> chatRect = {headerLeft, chatTopY, headerRight, chatBottomY};
    RenderUtil::fillRoundedRectangle(chatRect, UIColor(30, 30, 30, 180), 6.f * s);
    RenderUtil::drawRoundedRectangle(chatRect, UIColor(80, 80, 80, 150), 6.f * s, 1.5f * s);

    Vec4<float> searchRect = {headerLeft, pos.y + panelSz.y - bottomPadding - searchHeight,
                              headerRight, pos.y + panelSz.y - bottomPadding};

    UIColor searchBg, borderColor;
    bool showClickEffect = false;
    if(searchBoxClicked) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - searchClickTime).count();
        if(elapsed < 300)
            showClickEffect = true;
        else
            searchBoxClicked = false;
    }

    if(searchFocused) {
        searchBg = showClickEffect ? UIColor(80, 80, 80, 255) : UIColor(60, 60, 60, 255);
        borderColor = showClickEffect ? UIColor(150, 200, 255, 255) : UIColor(100, 150, 255, 200);
    } else {
        searchBg = UIColor(40, 40, 40, 200);
        borderColor = UIColor(80, 80, 80, 150);
    }

    RenderUtil::fillRoundedRectangle(searchRect, searchBg, 6.f * s);
    Vec4<float> borderRect = {searchRect.x - 1.f * s, searchRect.y - 1.f * s,
                              searchRect.z + 1.f * s, searchRect.w + 1.f * s};
    RenderUtil::drawRoundedRectangle(borderRect, borderColor, 6.f * s, 1.5f * s);

    std::string displayText = searchQuery.empty() ? (searchFocused ? "|" : "Type a message...")
                                                  : searchQuery + (searchFocused ? "|" : "");
    UIColor textColor = searchQuery.empty() && !searchFocused ? UIColor(120, 120, 120, 255)
                                                              : UIColor(220, 220, 220, 255);
    RenderUtil::drawText({searchRect.x + 8.f * s, searchRect.y + 6.f * s}, displayText, textColor,
                         0.75f * s);

    float chatEndY = searchRect.y - 60.f * s;
    float lineHeight = 42.f * s;
    float nameSize = 0.7f * s;
    float msgSize = 0.6f * s;
    float nameMsgSpacing = 8.f * s;
    float indent = 20.f * s;
    float boxPadding = 6.f * s;
    float textOffset = 6.f * s;
    int shown = 0;

    for(auto it = messageLog.rbegin(); it != messageLog.rend() && shown < 15; ++it, ++shown) {
        float y = chatEndY - lineHeight * shown;
        if(y < chatRect.y + 10.f * s)
            break;

        const ChatMessage& chatMsg = *it;
        std::string msgText = chatMsg.text;

        const std::string prefix = "IRC: ";
        if(msgText.find(prefix) == 0)
            msgText = msgText.substr(prefix.size());

        size_t delim = msgText.find(": ");
        if(delim != std::string::npos) {
            std::string name = msgText.substr(0, delim);
            std::string msg = msgText.substr(delim + 2);

            std::time_t msgTime = chatMsg.sentTime;
            std::tm timeInfo;
            localtime_s(&timeInfo, &msgTime);
            char timeBuffer[16];
            std::strftime(timeBuffer, sizeof(timeBuffer), "%I:%M %p", &timeInfo);
            std::string timeStr(timeBuffer);

            std::vector<ColorUtil::GradientStop> nameGradient = {{0.f, colortouse},
                                                                 {1.f, colortouse}};

            Vec4<float> msgRect = {
                headerLeft + indent - boxPadding, y - boxPadding, headerRight - indent + boxPadding,
                y + nameSize * 16.f + msgSize * 18.f + nameMsgSpacing + boxPadding};
            RenderUtil::fillRoundedRectangle(msgRect, UIColor(50, 50, 50, 180), 6.f * s);
            RenderUtil::drawRoundedRectangle(msgRect, UIColor(80, 80, 80, 200), 6.f * s, 1.5f * s);

            Vec4<float> glowLine = {msgRect.x + 2.f * s, msgRect.y + 2.f * s, msgRect.x + 6.f * s,
                                    msgRect.w - 2.f * s};
            RenderUtil::fillRoundedRectangle(glowLine, colortouse, 4.f * s);

            RenderUtil::drawGradientTextWithSweep({headerLeft + indent + textOffset, y}, name,
                                                  nameGradient, nameSize, true, getTimeSeconds(),
                                                  1.0f, 0.3f, 1.0f, 3.0f);

            float nameWidth = RenderUtil::getTextWidth(name, nameSize);

            RenderUtil::drawText({headerLeft + indent + textOffset + nameWidth + 10.f, y + 2.f},
                                 timeStr, UIColor(200, 200, 200, 180), nameSize * 0.8f);

            RenderUtil::drawText(
                {headerLeft + indent + textOffset, y + nameSize * 18.f + nameMsgSpacing}, msg,
                UIColor(220, 220, 220, 255), msgSize);
        } else {
            RenderUtil::drawText({headerLeft + indent + textOffset, y}, msgText,
                                 UIColor(220, 220, 220, 255), msgSize);
        }
    }
}

void SearchBox::onKeyUpdate(int key, bool isDown) {
    if(!shouldRender() || !searchFocused || !isDown)
        return;

    if(key == VK_RETURN && !searchQuery.empty()) {
        std::string playerName = sanitizeText(GI::getLocalPlayer()->getNameTag());
        std::string payload = "{\"content\": \"" + playerName + ": " + searchQuery + "\"}";
        SendDiscordEmbed(payload);
        searchQuery.clear();
        searchFocused = false;
        return;
    }

    if(key == VK_ESCAPE) {
        searchFocused = false;
        return;
    }

    if(key == VK_BACK && !searchQuery.empty()) {
        searchQuery.pop_back();
        return;
    }

    char ch = 0;
    if(key == VK_SPACE)
        ch = ' ';
    else if(key >= '0' && key <= '9')
        ch = static_cast<char>(key);
    else if(key >= 'A' && key <= 'Z')
        ch =
            (GetKeyState(VK_SHIFT) & 0x8000) ? static_cast<char>(key) : static_cast<char>(key + 32);
    else if(key == VK_OEM_PERIOD)
        ch = '.';
    else if(key == VK_OEM_COMMA)
        ch = ',';

    if(ch != 0 && searchQuery.size() < 100)
        searchQuery += ch;
}

bool SearchBox::onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown) {
    if(!shouldRender())
        return false;

    float s = size;
    Vec2<float> pos = {panelPos.x * s, panelPos.y * s};
    Vec2<float> panelSz = {panelSize.x * s, panelSize.y * s};

    Vec4<float> panelRect = {pos.x, pos.y, pos.x + panelSz.x, pos.y + panelSz.y};
    bool insidePanel = mousePos.x >= panelRect.x && mousePos.x <= panelRect.z &&
                       mousePos.y >= panelRect.y && mousePos.y <= panelRect.w;

    if(!insidePanel)
        return false;

    const float padding = 15.f * s;
    const float searchHeight = 28.f * s;
    const float bottomPadding = 15.f * s;

    Vec4<float> searchRect = {pos.x + padding, pos.y + panelSz.y - bottomPadding - searchHeight,
                              pos.x + panelSz.x - padding, pos.y + panelSz.y - bottomPadding};

    if(mouseButton == 1 && isDown) {
        if(mousePos.x >= searchRect.x && mousePos.x <= searchRect.z && mousePos.y >= searchRect.y &&
           mousePos.y <= searchRect.w) {
            searchFocused = true;
            searchBoxClicked = true;
            searchClickTime = std::chrono::steady_clock::now();
            return true;
        } else {
            searchFocused = false;
        }
    }

    if(mouseButton == 1 && isDown) {
        if(!dragging && !(mousePos.x >= searchRect.x && mousePos.x <= searchRect.z &&
                          mousePos.y >= searchRect.y && mousePos.y <= searchRect.w)) {
            dragging = true;
            dragOffset = {mousePos.x - pos.x, mousePos.y - pos.y};
        }
    } else if(mouseButton == 0 && dragging) {
        panelPos = {(mousePos.x - dragOffset.x) / s, (mousePos.y - dragOffset.y) / s};
    } else if(mouseButton == 1 && !isDown) {
        dragging = false;
    }

    return false;
}

void SearchBox::onD2DRender() {
    renderUI();
}

void SearchBox::onDisable() {
    searchFocused = false;
    searchQuery.clear();
}

std::string SearchBox::sanitizeText(const std::string& text) {
    std::string out;
    bool wasValid = true;
    for(char c : text) {
        bool isValid = static_cast<unsigned char>(c) <= 127;
        if(wasValid) {
            if(!isValid)
                wasValid = false;
            else
                out += c;
        } else {
            wasValid = isValid;
        }
    }
    return out;
}

void SearchBox::SendDiscordEmbed(const std::string& payloadJson) {
}

void SearchBox::addIncomingMessage(const std::string& msg) {
    if(messageSet.count(msg))
        return;

    constexpr size_t MAX_MESSAGE_LENGTH = 65; 

    std::string truncatedMsg = msg;
    if(truncatedMsg.size() > MAX_MESSAGE_LENGTH) {
        truncatedMsg = truncatedMsg.substr(0, MAX_MESSAGE_LENGTH) + "...";
    }

    ChatMessage chatMsg;
    chatMsg.text = msg;
    chatMsg.sentTime = std::time(nullptr);
    messageLog.push_back(chatMsg);
    messageSet.insert(msg);

    if(messageLog.size() > MAX_MESSAGES) {
        messageSet.erase(messageLog.front().text);
        messageLog.pop_front();
    }
}
