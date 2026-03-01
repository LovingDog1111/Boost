#include <Windows.h>
#include <wininet.h>
// one of the extra switches
#include <algorithm>
#include <string>

#pragma comment(lib, "wininet.lib")

std::string DownloadString(const std::string& url) {
    HINTERNET hInternet = InternetOpenA("switch", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if(!hInternet)
        return "";

    HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if(!hFile) {
        InternetCloseHandle(hInternet);
        return "";
    }

    char buffer[1024];
    DWORD bytesRead;
    std::string result;
    while(InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        result.append(buffer, bytesRead);
    }

    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
    return result;
}

DWORD WINAPI KillSwitchThread(LPVOID) {
    const std::string killSwitchURL = "https://pastebin.com/raw/E6ni9g1T";

    while(true) {
        std::string content = DownloadString(killSwitchURL);
        std::transform(content.begin(), content.end(), content.begin(), ::tolower);

        if(content.find("on") != std::string::npos) {
            TerminateProcess(GetCurrentProcess(), 0);
        }

        Sleep(1000);
    }

    return 0;
}

void StartKillSwitch() {
    CreateThread(nullptr, 0, KillSwitchThread, nullptr, 0, nullptr);
}
