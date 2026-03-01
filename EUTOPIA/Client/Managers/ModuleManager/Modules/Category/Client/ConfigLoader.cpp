#include "ConfigLoader.h"

#include <windows.h>
#include <winhttp.h>

#include <fstream>

#include "..\Client\Managers\ConfigManager\ConfigManager.h"
#include "..\Utils\FileUtil.h"

int selectedConfig = 0;

int selectedConfig2 = 0;

ConfigLoader::ConfigLoader() : Module("ConfigLoader", "Load premade configs", Category::CLIENT) {
    registerSetting(new EnumSetting("Config", "Choose premade config", {"Cubecraft", "Lifeboat"},
                                    &selectedConfig2, 0));
}

void ConfigLoader::onEnable() {
    std::string configName;
    std::wstring url;
    std::string configId;

    if(selectedConfig2 == 0) {
        GI::DisplayClientMessage("WARNING: This config contains plus modules!");
        GI::DisplayClientMessage("Selected: Cubecraft premade config");
        GI::DisplayClientMessage("Binds: KillAura [V], Fly [C], ClickGUI [K]");
        configName = "CubecraftPremade.txt";
        configId = "CubecraftPremade";
        url = L"http://43.226.0.155:5000/get_audio?file=CubecraftPremade.txt";
    } else if(selectedConfig2 == 1) {
        GI::DisplayClientMessage("WARNING: This config contains plus modules!");
        GI::DisplayClientMessage("Selected: Lifeboat premade config");
        GI::DisplayClientMessage("Binds: KillAura [V], Fly [C], AutoMine [R], EnemyTP [CTRL]");
        configName = "LifeboatPremade.txt";
        configId = "LifeboatPremade";
        url = L"http://43.226.0.155:5000/get_audio?file=LifeboatPremade.txt";
    } else {
        this->setEnabled(false);
        return;
    }

    std::string premadePath = FileUtil::getClientPath() + "Configs\\" + configName;

    if(FileUtil::doesFilePathExist(premadePath)) {
        GI::DisplayClientMessage("Config already exists, loading...");
        ConfigManager::loadConfig(configId);
        this->setEnabled(false);
        return;
    }

    GI::DisplayClientMessage(("Downloading config: " + configName).c_str());

    HINTERNET hSession = WinHttpOpen(L"ConfigLoader/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if(!hSession) {
        GI::DisplayClientMessage("Failed to open WinHTTP session");
        this->setEnabled(false);
        return;
    }

    URL_COMPONENTS urlComp{};
    urlComp.dwStructSize = sizeof(urlComp);
    wchar_t hostName[256];
    wchar_t urlPath[1024];
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = _countof(hostName);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = _countof(urlPath);

    WinHttpCrackUrl(url.c_str(), url.length(), 0, &urlComp);

    HINTERNET hConnect = WinHttpConnect(hSession, hostName, urlComp.nPort, 0);
    if(!hConnect) {
        WinHttpCloseHandle(hSession);
        GI::DisplayClientMessage("Failed to connect to server");
        this->setEnabled(false);
        return;
    }

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, L"GET", urlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);

    if(!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        GI::DisplayClientMessage("Failed to create request");
        this->setEnabled(false);
        return;
    }

    if(!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0,
                           0, 0) ||
       !WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        GI::DisplayClientMessage("Failed to download config");
        this->setEnabled(false);
        return;
    }

    std::ofstream ofs(premadePath, std::ios::binary);
    DWORD bytesRead = 0;
    char buffer[8192];

    while(WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
        ofs.write(buffer, bytesRead);

    ofs.close();

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    GI::DisplayClientMessage("Download complete, loading config...");
    ConfigManager::loadConfig(configId);

    this->setEnabled(false);
}
