#pragma once
#include <chrono>
#include <deque>
#include <string>

#include "../../ModuleBase/Module.h"
struct ChatMessage {
    std::string text;
    std::time_t sentTime;
};
class SearchBox : public Module {
   public:
    SearchBox();
    void renderUI();
    void onDisable() override;
    void onKeyUpdate(int key, bool isDown) override;
    bool onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown);
    void onD2DRender() override;

    float size = 1.0f;
    bool dragging = false;
    Vec2<float> dragOffset = {0.f, 0.f};
    Vec2<float> panelPos = {150.f, 80.f};
    Vec2<float> panelSize = {600.f, 400.f};
    bool isSearchFocused() const {
        return searchFocused;
    }
    static void addIncomingMessage(const std::string& msg);

   private:
    bool shouldRender() const;
    std::string searchQuery;
    bool searchFocused = false;
    mutable bool searchBoxClicked = false;
    mutable std::chrono::steady_clock::time_point searchClickTime;
    static std::deque<ChatMessage> messageLog;
    static const int MAX_MESSAGES = 50;
    static std::string sanitizeText(const std::string& text);
    static void SendDiscordEmbed(const std::string& payloadJson);
};