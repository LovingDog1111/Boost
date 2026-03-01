#include "GlobalInstance.h"

// Initialize static members of GlobalConfig


std::string GlobalConfig::clientVersion = "0.0.1";
int GlobalConfig::clientBuildNumber = 100;
std::string GlobalConfig::clientname = "Boost";

float GlobalConfig::fps = 0.0f;
float GlobalConfig::ping = 0.0f;
float GlobalConfig::deltaTime = 0.0f;

Color GlobalConfig::primaryColor(0, 0, 139);      // Dark Blue
Color GlobalConfig::secondaryColor(128, 0, 128);  // Purple
Color GlobalConfig::accentColor(0, 0, 255);       // Blue


Vec2<float> GlobalConfig::Fov = Vec2<float>(70.0f, 70.0f);  // Default FOV

void* GlobalConfig::keyMapPtr = nullptr;
void* GlobalConfig::keyMousePtr = nullptr;
int GlobalConfig::perspective = 0;

// Initialize default values
void GlobalConfig::initialize() {
    clientVersion = "1.0.0";
    clientBuildNumber = 100;
    
    fps = 0.0f;
    ping = 0.0f;
    deltaTime = 0.0f;
    
    primaryColor = Color(74, 144, 226);
    secondaryColor = Color(51, 255, 187);
    accentColor = Color(255, 255, 255);
    Fov = Vec2<float>(70.0f, 70.0f);  // Default FOV


    keyMapPtr = nullptr;
    keyMousePtr = nullptr;
    perspective = 0;

}

// Reset all values to defaults
void GlobalConfig::reset() {
    initialize();
}

// InstanceManager convenience methods implementation

LocalPlayer* InstanceManager::getLocalPlayer() {
    ClientInstance* client = get<ClientInstance>();
    if (client) {
       
        return client->getLocalPlayer(); // Replace with actual implementation
    }
    return nullptr;
}



BlockSource* InstanceManager::getRegion() {
    ClientInstance* client = get<ClientInstance>();
    if (client) {
        return client->getRegion(); // Assuming ClientInstance has getRegion method
    }
    return nullptr;
}

Level* InstanceManager::getLevel() {
    ClientInstance* client = get<ClientInstance>();
    if (client) {
        return client->getLevel(); // Assuming ClientInstance has getLevel method
    }
    return nullptr;
}

LevelRenderer* InstanceManager::getLevelRenderer() {
    ClientInstance* client = get<ClientInstance>();
    if (client) {
        return client->getLevelRenderer(); // Assuming ClientInstance has getLevelRenderer method
    }
    return nullptr;
}

PacketSender* InstanceManager::getPacketSender() {
    ClientInstance* client = get<ClientInstance>();
    if (client) {
        return client->getpacketSender(); // Assuming ClientInstance has getPacketSender method
    }
    return nullptr;
}
GuiData* InstanceManager::getGuiData() {
    ClientInstance* client = get<ClientInstance>();
    if (client) {
        return client->getguiData(); // Assuming ClientInstance has getGuiData method
    }
    return nullptr;
}

std::string InstanceManager::getScreenName() {
    ClientInstance* client = get<ClientInstance>();
    if (client) {
        return client->getScreenName(); // Assuming ClientInstance has getScreenName method
    }
    return "no_screen";
}

bool InstanceManager::isInHudScreen() {
    ClientInstance* client = get<ClientInstance>();
    if (client) {
        std::string screenName = client->getScreenName();
        return screenName == "hud_screen"; // Assuming "hud_screen" is the name for the HUD screen
    }
    return false;
}
bool InstanceManager::isLocalPlayer(Actor* actor) {
    if (!actor) return false;
    
    LocalPlayer* localPlayer = getLocalPlayer();
    if (localPlayer) {
        return actor == (Actor*)localPlayer;
    }
    return false;
}

bool InstanceManager::isRenderEnvSafe() {
    // Check if we have essential game objects
    return has<ClientInstance>() && 
           getLevel() != nullptr && 
           getLocalPlayer() != nullptr;
}

bool InstanceManager::isKeyDown(uint32_t key) {
    if(GlobalConfig::keyMapPtr == nullptr) {
        return false;
    }

    return *reinterpret_cast<bool*>((uintptr_t)GlobalConfig::keyMapPtr + ((uintptr_t)key * 0x4));
}

bool InstanceManager::isLeftClickDown() {
    if(GlobalConfig::keyMapPtr == nullptr) {
        return false;
    }

    return *reinterpret_cast<bool*>((uintptr_t)GlobalConfig::keyMapPtr + 1);
}

bool InstanceManager::isRightClickDown() {
    if(GlobalConfig::keyMapPtr == nullptr) {
        return false;
    }

    return *reinterpret_cast<bool*>((uintptr_t)GlobalConfig::keyMapPtr + 2);
}

bool InstanceManager::canUseMoveKeys() {
    ClientInstance* client = InstanceManager::getClientInstance();
    if (!client) {
        return false; // No client instance available
    }
    // Check if the player is in a valid state to move
    return client->minecraftGame->getmouseGrabbed();
}

void InstanceManager::DisplayClientMessage(const char* fmt, ...) {
    LocalPlayer* player = InstanceManager::getLocalPlayer();
    auto GuiData = InstanceManager::getGuiData();
    auto Screen = InstanceManager::getClientInstance()->getScreenName();

    if(player == nullptr || GuiData == nullptr)
        return;
    if(Screen == "chat_screen")
        return;  // idfk why
    va_list arg;
    va_start(arg, fmt);
    char message[300];
    vsprintf_s(message, 300, fmt, arg);
    va_end(arg);

#ifdef _DEBUG
    MCString mcMsg(message);
    getGuiData()->displayMessage(mcMsg)

#else
    std::string stdMsg(message);
    GuiData->displayMessage(stdMsg);
#endif
}
