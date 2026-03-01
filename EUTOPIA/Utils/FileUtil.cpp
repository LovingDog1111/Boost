#include "FileUtil.h"

std::string FileUtil::getRoamingStatePath() {
    char* appDataPath = nullptr;
    size_t len = 0;
    if (_dupenv_s(&appDataPath, &len, "AppData") == 0 && appDataPath != nullptr) {
        std::string RoamingStatePath = std::string(appDataPath) + "\\..\\Local\\Packages\\Microsoft.MinecraftUWP_8wekyb3d8bbwe\\RoamingState\\";
        free(appDataPath); // Free the allocated memory
        return RoamingStatePath;
    }
    return ""; // Return an empty string if the environment variable is not found
}

std::string FileUtil::getClientPath() {
	static std::string ClientPath = getRoamingStatePath() + "Boostv4\\";
	return ClientPath;
}