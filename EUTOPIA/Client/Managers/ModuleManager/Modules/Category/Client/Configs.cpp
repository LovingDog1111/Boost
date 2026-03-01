#include "Configs.h"

#include <Windows.h>

#include <filesystem>
#include <string>
#include <vector>

#include "..\Client\Managers\ConfigManager\ConfigManager.h"
#include "..\Client\Managers\ModuleManager\ModuleManager.h"
#include "..\Utils\FileUtil.h"
#include <wincodec.h>
#include <winerror.h>
#include <Windows.h>    
#include <combaseapi.h> 
#include <shlobj.h>     



static float getTimeSeconds() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    auto now = steady_clock::now();
    return duration_cast<duration<float>>(now - start).count();
}

Configs::Configs() : Module("Configs", "Manage and load configs", Category::CLIENT) {
    searchQuery = "";
    searchFocused = false;
    searchBoxClicked = false;
    registerSetting(new SliderSetting<float>("Size", "UI Scale", &size, 1.0f, 0.7f, 1.5f));
    fetchConfigs();
}

bool Configs::shouldRender() const {
    auto clickGui = ModuleManager::getModule<ClickGUI>();
    return clickGui && clickGui->isEnabled();
}

void Configs::fetchConfigs() {
    configList.clear();

    // Get %LOCALAPPDATA%
    wchar_t* localAppData = nullptr;
    if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &localAppData))) {
        std::wstring path = localAppData;
        CoTaskMemFree(localAppData);

        path +=
            L"\\Packages\\Microsoft.MinecraftUWP_8wekyb3d8bbwe\\RoamingState\\Boostv4\\Configs\\";

        try {
            for(const auto& entry : std::filesystem::directory_iterator(path)) {
                if(entry.is_regular_file() && entry.path().extension() == L".json") {
                    std::string name = entry.path().stem().string();
                    configList.push_back(name);
                }
            }
        } catch(const std::filesystem::filesystem_error&) {
        }
    }
}


void Configs::renderUI() {
    if(!shouldRender())
        return;
    float s = size;
    Vec2<float> pos = {panelPos.x * s, panelPos.y * s};
    Vec2<float> panelSz = {panelSize.x * s, panelSize.y * s};
    Vec4<float> mainRect = {pos.x, pos.y, pos.x + panelSz.x, pos.y + panelSz.y};
    RenderUtil::fillRoundedRectangle(mainRect, UIColor(25, 25, 25, 240), 12.f * s);
    RenderUtil::drawRoundedRectangle(mainRect, UIColor(0, 180, 255, 120), 12.f * s, 3.f * s);
    const float padding = 15.f * s;
    float headerHeight = 50.f * s;
    float headerTop = pos.y + padding;
    float headerBottom = headerTop + headerHeight;
    float headerLeft = pos.x + padding;
    float headerRight = pos.x + panelSz.x - padding;
    std::string title = "Configs";
    std::vector<ColorUtil::GradientStop> titleGradient = {{0.f, UIColor(0, 255, 200)},
                                                          {1.f, UIColor(0, 180, 255)}};
    RenderUtil::drawGradientTextWithSweep({headerLeft, headerTop + 12.f * s}, title, titleGradient,
                                          1.4f * s, true, getTimeSeconds(), 1.0f, 0.3f, 1.0f, 3.0f);
    float searchBoxY = pos.y + panelSz.y - padding - 28.f * s;
    Vec4<float> searchRect = {headerLeft, searchBoxY, headerRight, searchBoxY + 28.f * s};
    UIColor searchBg = searchFocused ? UIColor(60, 60, 60, 255) : UIColor(40, 40, 40, 200);
    UIColor borderColor = searchFocused ? UIColor(100, 150, 255, 200) : UIColor(80, 80, 80, 150);
    RenderUtil::fillRoundedRectangle(searchRect, searchBg, 6.f * s);
    Vec4<float> borderRect = {searchRect.x - 1.f * s, searchRect.y - 1.f * s,
                              searchRect.z + 1.f * s, searchRect.w + 1.f * s};
    RenderUtil::drawRoundedRectangle(borderRect, borderColor, 6.f * s, 1.5f * s);
    std::string displayText = searchQuery.empty() ? (searchFocused ? "|" : "Search Configs...")
                                                  : searchQuery + (searchFocused ? "|" : "");
    UIColor textColor = searchQuery.empty() && !searchFocused ? UIColor(120, 120, 120, 255)
                                                              : UIColor(220, 220, 220, 255);
    RenderUtil::drawText({searchRect.x + 8.f * s, searchRect.y + 6.f * s}, displayText, textColor,
                         0.75f * s);
    float buttonY = headerBottom + 10.f * s;
    float buttonHeight = 32.f * s;
    float buttonSpacing = 6.f * s;
    for(const std::string& name : configList) {
        if(!searchQuery.empty() && name.find(searchQuery) == std::string::npos)
            continue;
        Vec4<float> btnRect = {headerLeft, buttonY, headerRight, buttonY + buttonHeight};
        RenderUtil::fillRoundedRectangle(btnRect, UIColor(40, 40, 40, 255), 5.f * s);
        RenderUtil::drawRoundedRectangle(btnRect, UIColor(70, 70, 70, 180), 5.f * s, 1.5f * s);
        RenderUtil::drawText({btnRect.x + 10.f * s, btnRect.y + 7.f * s}, name,
                             UIColor(220, 220, 220, 255), 0.75f * s);
        buttonRects[name] = btnRect;
        buttonY += buttonHeight + buttonSpacing;
    }
}

bool Configs::onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown) {
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
    Vec4<float> searchRect = {pos.x + padding, pos.y + panelSz.y - padding - 28.f * s,
                              pos.x + panelSz.x - padding, pos.y + panelSz.y - padding};
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
        for(const auto& [name, rect] : buttonRects) {
            if(mousePos.x >= rect.x && mousePos.x <= rect.z && mousePos.y >= rect.y &&
               mousePos.y <= rect.w) {
                ConfigManager::loadConfig(name);
                break;
            }
        }
    }
    return false;
}

void Configs::onKeyUpdate(int key, bool isDown) {
    if(!shouldRender() || !searchFocused || !isDown)
        return;
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

void Configs::onD2DRender() {
    renderUI();
}

void Configs::onDisable() {
    searchFocused = false;
    searchQuery.clear();
}
