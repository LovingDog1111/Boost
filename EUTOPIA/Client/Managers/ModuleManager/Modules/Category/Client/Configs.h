#pragma once

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "../../ModuleBase/Module.h"

class Configs : public Module {
   public:
    Configs();
    void renderUI();
    void onD2DRender() override;
    void onDisable() override;
    void onKeyUpdate(int key, bool isDown) override;
    bool onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown);

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
    void fetchConfigs();

    float scrollOffset = 0.0f;
    float maxScroll = 0.0f;


    std::string searchQuery;
    bool searchFocused = false;
    mutable bool searchBoxClicked = false;
    mutable std::chrono::steady_clock::time_point searchClickTime;

    std::vector<std::string> configList;
    std::map<std::string, Vec4<float>> buttonRects;
};
