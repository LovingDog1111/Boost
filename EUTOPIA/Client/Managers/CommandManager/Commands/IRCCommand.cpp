#include "IRCCommand.h"

#include <windows.h>
#include <wininet.h>

#include <sstream>
#include <string>

#include "../../Client/Managers/ModuleManager/ModuleManager.h"

#pragma comment(lib, "wininet.lib")

static const std::string WEBHOOK_URL =
    "https://discord.com/api/webhooks/1404845880657580115/aa1HN2RPs_HJIAdLVubkUPLXVcRAmSwVjvbQR11U5yo3-gvjlGn89D3I-vYE21cYjJGC";

static bool envaledcharr(char c) {
    return !(c >= 0 && *reinterpret_cast<unsigned char*>(&c) < 128);
}

std::string sanitsiz(const std::string& text) {
    std::string out;
    bool wasValid = true;
    for(char c : text) {
        bool isValid = !envaledcharr(c);
        if(wasValid) {
            if(!isValid) {
                wasValid = false;
            } else {
                out += c;
            }
        } else {
            wasValid = isValid;
        }
    }
    return out;
}

static void SendDiscordEmbed2(const std::string& payloadJson) {
    HINTERNET hInternet = InternetOpenA("WebhookSender", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if(!hInternet)
        return;

    URL_COMPONENTSA urlComp{sizeof(urlComp)};
    char host[256] = {0}, path[2048] = {0};
    urlComp.lpszHostName = host;
    urlComp.dwHostNameLength = _countof(host);
    urlComp.lpszUrlPath = path;
    urlComp.dwUrlPathLength = _countof(path);

    InternetCrackUrlA(WEBHOOK_URL.c_str(), 0, 0, &urlComp);

    HINTERNET hConnect = InternetConnectA(hInternet, host, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL,
                                          INTERNET_SERVICE_HTTP, 0, 0);
    if(!hConnect) {
        InternetCloseHandle(hInternet);
        return;
    }

    HINTERNET hRequest =
        HttpOpenRequestA(hConnect, "POST", path, NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
    if(!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return;
    }

    const std::string headers = "Content-Type: application/json\r\n";
    HttpSendRequestA(hRequest, headers.c_str(), -1, (LPVOID)payloadJson.c_str(),
                     (DWORD)payloadJson.size());

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
}

IRCCommand::IRCCommand()
    : CommandBase("irc", "Sends a message to Discord via webhook", "[message]", {"ircmsg"}) {}

bool IRCCommand::execute(const std::vector<std::string>& args) {
    if(args.size() < 2) {
        GI::DisplayClientMessage("%sUsage: %s.irc <message>%s", MCTF::RED, MCTF::WHITE,
                                 MCTF::RESET);
        return false;
    }

    std::string message;
    for(size_t i = 1; i < args.size(); ++i)
        message += args[i] + " ";
    if(!message.empty())
        message.pop_back();

    
    auto removeMention = [&](const std::string& mention) {
        size_t pos = 0;
        while((pos = message.find(mention, pos)) != std::string::npos) {
            message.erase(pos, mention.length());
        }
    };
    removeMention("@everyone");
    removeMention("@here");

    std::string playerName = sanitsiz(GI::getLocalPlayer()->getNameTag());
    std::string payload = "{\"content\": \"" + playerName + ": " + message + "\"}";
    GI::DisplayClientMessage("%sMessage sent to Discord: %s", MCTF::GREEN, message.c_str());

    SendDiscordEmbed2(payload);
    return true;
}
