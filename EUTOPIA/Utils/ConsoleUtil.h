#pragma once

#include <windows.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <unordered_set>

class ConsoleUtil {
   public:
    static void CreateConsole(const char* title) {
        if(!GetConsoleWindow()) {
            AllocConsole();
            freopen_s(&f, "CONOUT$", "w", stdout);
            freopen_s(&f, "CONIN$", "r", stdin);
            SetConsoleTitleA(title);
        } else {
            ShowWindow(GetConsoleWindow(), SW_SHOW);
        }
    }

    static void CloseConsole() {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        ClearConsole();
    }

    static void ClearConsole() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if(hConsole == INVALID_HANDLE_VALUE)
            return;

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD count;
        COORD homeCoords = {0, 0};

        if(!GetConsoleScreenBufferInfo(hConsole, &csbi))
            return;

        FillConsoleOutputCharacter(hConsole, ' ', csbi.dwSize.X * csbi.dwSize.Y, homeCoords,
                                   &count);
        SetConsoleCursorPosition(hConsole, homeCoords);
    }

    static void LogDebug(const std::string& message) {
        setColor(8);  // Gray
        std::cout << "[";
        setColor(11);  // Cyan
        std::cout << "Aqua";
        setColor(8);  // Gray
        std::cout << "] [";
        setColor(9);  // Blue
        std::cout << "Debug";
        setColor(8);  // Gray
        std::cout << "] ";
        resetColor();
        std::cout << message << std::endl;
    }

      static void Log(const std::string& tag, const std::string& message, int tagColor = 11, int msgColor = 7) {
        setColor(8);  // Gray for brackets
        std::cout << "[";
        setColor(tagColor);  // Custom tag color
        std::cout << tag;
        setColor(8);  // Gray
        std::cout << "] ";
        setColor(msgColor);  // Custom message color
        std::cout << message << std::endl;
        resetColor();
    }

    static void LogInfo(const std::string& message) {
        setColor(8);  // Gray
        std::cout << "[";
        setColor(11);  // Cyan
        std::cout << "Aqua";
        setColor(8);  // Gray
        std::cout << "] [";
        setColor(11);  // Cyan
        std::cout << "Info";
        setColor(8);  // Gray
        std::cout << "] ";
        resetColor();
        std::cout << message << std::endl;
    }

    static void LogCritical(const std::string& message) {
        setColor(8);  // Gray
        std::cout << "[";
        setColor(11);  // Cyan
        std::cout << "Aqua";
        setColor(8);  // Gray
        std::cout << "] [";
        setColor(12);  // Red
        std::cout << "Critical";
        setColor(8);  // Gray
        std::cout << "] ";
        resetColor();
        std::cout << message << std::endl;
    }

    static void LogHookStatus(const std::string& hookName, uintptr_t address, bool success) {
        LogInfo("Hook " + hookName + " at address 0x" + std::to_string(address) +
                (success ? " succeeded" : " failed"));
    }

    static void LogMemoryStatus(uintptr_t address, const std::string& description) {
        LogDebug("Memory at " + std::to_string(address) + ": " + description);
    }
    static std::unordered_set<std::string> getRunningWindowTitles() {
        std::unordered_set<std::string> titles;

        EnumWindows(
            [](HWND hwnd, LPARAM lParam) -> BOOL {
                char title[256];
                int length = GetWindowTextA(hwnd, title, sizeof(title));
                if(length > 0) {
                    auto* pTitles = reinterpret_cast<std::unordered_set<std::string>*>(lParam);
                    pTitles->insert(std::string(title, length));
                }
                return TRUE;  // continue enumeration
            },
            reinterpret_cast<LPARAM>(&titles));

        return titles;
    }

   private:
    static inline FILE* f = nullptr;

    static void setColor(int color) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    }

    static void resetColor() {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);  // Reset to default color
    }
};