#include "AIChat.h"

#include <Windows.h>
#include <wininet.h>

#include "json.hpp"
using json = nlohmann::json;


#include <chrono>
#include <deque>
#include <map>
#include <string>

#include "..\Client\Managers\ModuleManager\ModuleManager.h"
#pragma comment(lib, "wininet.lib")

std::deque<ChatMessage2> AIChat::messageLog;
std::map<std::string, std::vector<std::string>> AIChat::conversationHistory;

std::vector<std::string> wrapText(const std::string& text, float maxWidth, float textSize) {
    std::vector<std::string> lines;
    std::string currentLine;
    std::string word;
    for(char c : text) {
        if(c == ' ') {
            std::string testLine = currentLine + word + " ";
            float width = RenderUtil::getTextWidth(testLine, textSize);
            if(width > maxWidth && !currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine = word + " ";
            } else {
                currentLine = testLine;
            }
            word.clear();
        } else {
            word += c;
        }
    }
    if(!word.empty()) {
        std::string testLine = currentLine + word;
        if(RenderUtil::getTextWidth(testLine, textSize) > maxWidth && !currentLine.empty()) {
            lines.push_back(currentLine);
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    if(!currentLine.empty())
        lines.push_back(currentLine);
    return lines;
}


static float getTimeSeconds() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    auto now = steady_clock::now();
    return duration_cast<duration<float>>(now - start).count();
}

static std::string getAPIKey() {
    std::vector<std::string> apiKeys = {
        "AIzaSyA89cJVPY_VTqM8R9kHpByLlKkVK052Uzk",
    };
    return apiKeys[0];
}

AIChat::AIChat() : Module("AI Chat", "Movable AI Chat Box", Category::CLIENT) {
    searchQuery = "";
    searchFocused = false;
    searchBoxClicked = false;
    registerSetting(new SliderSetting<float>("Size", "UI Scale", &size, 1.0f, 0.7f, 1.5f));
}

bool AIChat::shouldRender() const {
    auto clickGui = ModuleManager::getModule<ClickGUI>();
    return clickGui && clickGui->isEnabled();
}

void AIChat::renderUI() {
    if(!shouldRender())
        return;
    float s = size;
    Vec2<float> pos = {panelPos.x * s, panelPos.y * s};
    Vec2<float> panelSz = {panelSize.x * s, panelSize.y * s};
    Vec4<float> mainRect = {pos.x, pos.y, pos.x + panelSz.x, pos.y + panelSz.y};
    RenderUtil::fillRoundedRectangle(mainRect, UIColor(25, 25, 25, 240), 12.f * s);
    RenderUtil::drawRoundedRectangle(mainRect, UIColor(0, 180, 255, 120), 12.f * s, 3.f * s);
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
    RenderUtil::fillCircle(bulletPos, UIColor(0, 255, 200), bulletRadius);
    std::string title = "BoostGPT";
    std::vector<ColorUtil::GradientStop> titleGradient = {{0.f, UIColor(0, 255, 200)},
                                                          {1.f, UIColor(0, 180, 255)}};
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
    std::string displayText = searchQuery.empty() ? (searchFocused ? "|" : "Ask AI...")
                                                  : searchQuery + (searchFocused ? "|" : "");
    UIColor textColor = searchQuery.empty() && !searchFocused ? UIColor(120, 120, 120, 255)
                                                              : UIColor(220, 220, 220, 255);
    RenderUtil::drawText({searchRect.x + 8.f * s, searchRect.y + 6.f * s}, displayText, textColor,
                         0.75f * s);
    float chatEndY = searchRect.y - 60.f * s;
    float lineHeight = 42.f * s;
    float textSize = 0.7f * s;
    float indent = 20.f * s;
    float boxPadding = 6.f * s;
    float textOffset = 6.f * s;
    int shown = 0;
    for(auto it = messageLog.rbegin(); it != messageLog.rend() && shown < 15; ++it, ++shown) {
        const ChatMessage2& chatMsg = *it;
   
        float messageBoxWidth = headerRight - headerLeft - 2 * indent;
        std::vector<std::string> wrappedLines = wrapText(chatMsg.text, messageBoxWidth, textSize);

        float totalHeight = wrappedLines.size() * (textSize * 24.f);

  
        float y = chatEndY - shown * (totalHeight + 12.f);  


        if(y < chatRect.y + 10.f * s)
            break;

        Vec4<float> msgRect = {headerLeft + indent - boxPadding, y - boxPadding,
                               headerRight - indent + boxPadding, y + totalHeight + boxPadding};

        RenderUtil::fillRoundedRectangle(msgRect, UIColor(50, 50, 50, 180), 6.f * s);
        RenderUtil::drawRoundedRectangle(msgRect, UIColor(80, 80, 80, 200), 6.f * s, 1.5f * s);

        
        Vec4<float> glowLine = {msgRect.x + 2.f * s, msgRect.y + 2.f * s, msgRect.x + 6.f * s,
                                msgRect.w - 2.f * s};
        RenderUtil::fillRoundedRectangle(glowLine, UIColor(0, 180, 255, 160), 4.f * s);

        
        float lineY = y;
        for(const std::string& line : wrappedLines) {
            RenderUtil::drawText({headerLeft + indent + textOffset, lineY}, line,
                                 UIColor(220, 220, 220, 255), textSize);
            lineY += textSize * 24.f;
        }

        RenderUtil::fillRoundedRectangle(msgRect, UIColor(50, 50, 50, 180), 6.f * s);
        RenderUtil::drawRoundedRectangle(msgRect, UIColor(80, 80, 80, 200), 6.f * s, 1.5f * s);
        RenderUtil::fillRoundedRectangle(glowLine, UIColor(0, 180, 255, 160), 4.f * s);
        RenderUtil::drawText({headerLeft + indent + textOffset, y}, chatMsg.text,
                             UIColor(220, 220, 220, 255), textSize);
    }
}

void AIChat::onKeyUpdate(int key, bool isDown) {
    if(!shouldRender() || !searchFocused || !isDown)
        return;
    if(key == VK_RETURN && !searchQuery.empty()) {
        std::string userId = sanitizeText(GI::getLocalPlayer()->getNameTag());
        addMessage("You: " + searchQuery);
        std::string reply = sendToGemini(userId, searchQuery);
        addMessage("BoostGPT: " + reply);
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

bool AIChat::onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown) {
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

void AIChat::onD2DRender() {
    renderUI();
}

void AIChat::onDisable() {
    searchFocused = false;
    searchQuery.clear();
}

void AIChat::addMessage(const std::string& msg) {
    ChatMessage2 chatMsg;
    chatMsg.text = msg;
    chatMsg.sentTime = std::time(nullptr);
    messageLog.push_back(chatMsg);
    if(messageLog.size() > MAX_MESSAGES) {
        messageLog.pop_front();
    }
}

std::string AIChat::sanitizeText(const std::string& text) {
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

std::string AIChat::sendToGemini(const std::string& userId, const std::string& prompt) {
    conversationHistory[userId].push_back("{\"role\": \"user\", \"parts\": [{\"text\": \"" +
                                          prompt + "\"}]}");
    std::string apiKey = getAPIKey();
    std::string payload = "{\"contents\": [";
    for(const std::string& msg : conversationHistory[userId]) {
        payload += msg + ",";
    }
    if(!conversationHistory[userId].empty())
        payload.pop_back();
    payload += "]}";
    std::wstring host = L"generativelanguage.googleapis.com";
    std::wstring path = L"/v1beta/models/gemini-2.5-flash-preview-05-20:generateContent?key=" +
                        std::wstring(apiKey.begin(), apiKey.end());
    HINTERNET hInternet = InternetOpenW(L"AIClient", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if(!hInternet)
        return "Network error.";
    HINTERNET hConnect = InternetConnectW(hInternet, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT,
                                          NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if(!hConnect) {
        InternetCloseHandle(hInternet);
        return "Connection error.";
    }
    HINTERNET hRequest = HttpOpenRequestW(hConnect, L"POST", path.c_str(), NULL, NULL, NULL,
                                          INTERNET_FLAG_SECURE, 0);
    if(!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "Request error.";
    }
    std::string headers = "Content-Type: application/json\r\n";
    BOOL sent = HttpSendRequestA(hRequest, headers.c_str(), -1, (LPVOID)payload.c_str(),
                                 static_cast<DWORD>(payload.size()));
    std::string response;
    if(sent) {
        char buffer[4096];
        DWORD bytesRead;
        while(InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
            buffer[bytesRead] = 0;
            response += buffer;
        }
    }
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    try {
        auto jsonResponse = json::parse(response);
        if(!jsonResponse.contains("candidates") || !jsonResponse["candidates"].is_array())
            return "Invalid response: Missing candidates.";
        auto& candidates = jsonResponse["candidates"];
        if(candidates.empty() || !candidates[0].contains("content"))
            return "Invalid response: Empty candidates.";
        auto& content = candidates[0]["content"];
        if(!content.contains("parts") || !content["parts"].is_array())
            return "Invalid response: No parts found.";
        std::string reply = content["parts"][0]["text"];

        /**
        if(reply.size() > 50) {
            reply = reply.substr(0, 47) + "...";
            */
        

        conversationHistory[userId].push_back("{\"role\": \"model\", \"parts\": [{\"text\": \"" +
                                              reply + "\"}]}");
        return reply;
    } catch(const std::exception& e) {
        return std::string("JSON parse error: ") + e.what();
    }
}
