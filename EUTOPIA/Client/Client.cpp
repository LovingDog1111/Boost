#include "Client.h"
#include <NetworkUtil.h>
#include <Windows.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Notifications.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <regex>
#include "../SDK/Core/MinecraftGame.h"
#include "../SDK/GlobalInstance.h"
#include "../SDK/MCTextFormat.h"
#include "Managers/CommandManager/CommandManager.h"
#include "Managers/ConfigManager/ConfigManager.h"
#include "Managers/HooksManager/HookManager.h"
#include "Managers/ModuleManager/ModuleManager.h"
#include "Managers/NotificationManager/NotificationManager.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.UI.Core.h"
#include "winrt/Windows.UI.ViewManagement.h"
#include "winrt/windows.applicationmodel.core.h"
#include "winrt/windows.system.h"
#include <ConsoleUtil.h>

void Client::DisplayIRCMessage(const char* fmt, ...) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    auto GuiData = GI::getGuiData();
    if(localPlayer == nullptr)
        return;

    va_list arg;
    va_start(arg, fmt);
    char message[300];
    vsprintf_s(message, 300, fmt, arg);
    va_end(arg);

    static std::string headerMessage;
    if(headerMessage.empty()) {
        char headerCStr[50];
        sprintf_s(headerCStr, 50, "%s[%sIRC%s] %s", MCTF::GRAY, MCTF::ORANGE, MCTF::GRAY,
                  MCTF::WHITE);
        headerMessage = std::string(headerCStr);
    }

    std::string messageStr = headerMessage + std::string(message);
    GuiData->displayMessage(messageStr);
}

void Client::DisplayClientMessage(const char* fmt, ...) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    auto GuiData = GI::getGuiData();
    if(localPlayer == nullptr)
        return;

    va_list arg;
    va_start(arg, fmt);
    char message[300];
    vsprintf_s(message, 300, fmt, arg);
    va_end(arg);

    static std::string headerMessage;
    if(headerMessage.empty()) {
        char headerCStr[50];
        sprintf_s(headerCStr, 50, "%s[%sBoostV4%s] %s", MCTF::GRAY, MCTF::BLUE, MCTF::GRAY,
                  MCTF::WHITE);
        headerMessage = std::string(headerCStr);
    }

    std::string messageStr = headerMessage + std::string(message);
    GuiData->displayMessage(messageStr);
}

void AddFont(const std::string& fontPath) {
    std::wstring temp = std::wstring(fontPath.begin(), fontPath.end());
    AddFontResource(temp.c_str());
}

void RenameWindow(const char* newTitle) {
    HWND hWnd = GetForegroundWindow();
    if(hWnd) {
        SetWindowTextA(hWnd, newTitle);
    }
}

void Client::init() {
    std::string ClientPath = FileUtil::getClientPath();
    if(!FileUtil::doesFilePathExist(ClientPath)) {
        FileUtil::createPath(ClientPath);
    }

    if(GI::getClientInstance() && GI::getClientInstance()->minecraftGame) {
        GI::getClientInstance()->minecraftGame->playUI("random.levelup", 1.0f, 1.0f);
    }

    if(GI::getClientInstance() && GI::getClientInstance()->minecraftGame) {
        GI::getClientInstance()->playUi("random.chest", 1.0f, 1.0f);
    }

    RenameWindow("BoostV4 1.21.9X");

    Logger::init();
    ConfigManager::init();
    MCTF::initMCTF();
    ModuleManager::init();
    CommandManager::init();
    HookManager::init();

    if(GI::getClientInstance() == nullptr)
        return;

    initialized = true;

    NotificationManager::addNotification("Successfully injected Boost!", 5.f);

    
    if(!ConfigManager::doesConfigExist("default"))
        ConfigManager::createNewConfig("default");
    ConfigManager::loadConfig("default");
}

void Client::shutdown() {
    ConfigManager::saveConfig();
    initialized = false;
}