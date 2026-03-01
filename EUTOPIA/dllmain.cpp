#include <Windows.h>
#include <psapi.h>
#include <wininet.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "../Libs/miniaudio/miniaudio.h"
#include "../Utils/NetworkUtil.h"
#include "Client/Client.h"
#include "Client/Managers/CommandManager/CommandManager.h"
#include "Client/Managers/HooksManager/HookManager.h"
#include "Client/Managers/ModuleManager/ModuleManager.h"
#include "json.hpp"

DWORD WINAPI ClientThread(LPVOID lpParameter) {
    Sleep(3000);
    Client::init();

    while(Client::isInitialized()) {
        ModuleManager::onClientTick();
        Sleep(50);
    }

    Sleep(25);
    FreeLibraryAndExitThread((HMODULE)lpParameter, 1);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if(ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, ClientThread, hModule, 0, nullptr);
        if(hThread)
            CloseHandle(hThread);
    } else if(ul_reason_for_call == DLL_PROCESS_DETACH) {
        Client::shutdown();
        Sleep(50);
        D2D::Clean();
        HookManager::shutdown();
        ModuleManager::shutdown();
        CommandManager::shutdown();
        Client::DisplayClientMessage("%sEjected", MCTF::RED);
    }
    return TRUE;
}