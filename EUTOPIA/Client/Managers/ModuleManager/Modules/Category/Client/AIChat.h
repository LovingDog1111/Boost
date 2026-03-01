#pragma once

#include <chrono>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "../../ModuleBase/Module.h"

struct ChatMessage2 {
    std::string text;
    std::time_t sentTime;
};

class AIChat : public Module {
   public:
    AIChat();
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

   private:
    bool shouldRender() const;
    std::string searchQuery;
    bool searchFocused = false;
    mutable bool searchBoxClicked = false;
    mutable std::chrono::steady_clock::time_point searchClickTime;

    static std::deque<ChatMessage2> messageLog;
    static const int MAX_MESSAGES = 50;

    static std::map<std::string, std::vector<std::string>> conversationHistory;

    static void addMessage(const std::string& msg);
    static std::string sanitizeText(const std::string& text);
    static std::string sendToGemini(const std::string& userId, const std::string& prompt);
};
