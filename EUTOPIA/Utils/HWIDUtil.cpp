#include "HWIDUtil.h"
#include <windows.h>
#include <sstream>
#include <iomanip>
#include <winreg.h>
#include <string>

static std::string ReadMachineGuid() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
        return "";
    char buffer[64];
    DWORD bufSize = sizeof(buffer);
    if (RegQueryValueExA(hKey, "MachineGuid", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return "";
    }
    RegCloseKey(hKey);
    return std::string(buffer, bufSize - 1); // strip null
}

std::string GetHWID() {
    // Try MachineGuid first
    std::string guid = ReadMachineGuid();
    if (!guid.empty()) return guid;

    // Fallback: volume serial of system drive
    char sysDir[MAX_PATH];
    GetSystemWindowsDirectoryA(sysDir, MAX_PATH);
    char rootPath[5] = "C:\\";
    rootPath[0] = sysDir[0];

    DWORD volumeSerial = 0;
    if (!GetVolumeInformationA(rootPath, nullptr, 0, &volumeSerial, nullptr, nullptr, nullptr, 0)) {
        return "UNKNOWN";
    }
    std::stringstream ss;
    ss << std::uppercase << std::hex << volumeSerial;
    return ss.str();
} 