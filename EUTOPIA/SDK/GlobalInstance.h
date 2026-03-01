#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "Core/ClientInstance.h"
#include "Core/GuiData.h"
#include "Network/PacketSender.h"
#include "Render/Font.h"
#include "Render/LevelRender.h"
#include "Render/MinecraftUIRenderContext.h"
#include "Render/ScreenView.h"
#include "Render/Tessellator.h"
#include "World/Actor/LocalPlayer.h"
#include "World/Level/Level.h"
#include "world/Level/BlockSource.h"
#include "..\Utils\PlusUtils.h"

// class LocalPlayer;
// class ClientInstance;
// class MinecraftUIRenderContext;
// class Font;
// class ScreenView;
// class Tessellator;
// class BlockSource;
// class Level;
// class LevelRenderer;
// class PacketSender;
// class GuiData;

struct Color {
    unsigned char r, g, b, a;

    Color() : r(255), g(255), b(255), a(255) {}
    Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}

    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    unsigned int toHex() const {
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
};

class GlobalConfig {
   public:
    static std::string clientVersion;
    static std::string clientname;
    static int clientBuildNumber;

    static float fps;
    static float ping;
    static float deltaTime;
    static int perspective;
    
    static bool isFirstPerson() { return perspective == 0; }
    static bool isThirdPerson() { return perspective == 1; }
    static bool isThirdPersonBack() { return perspective == 2; }
    static void setPerspective(int newPerspective) { perspective = newPerspective; }

    static Color primaryColor;
    static Color secondaryColor;
    static Color accentColor;

    static void* keyMapPtr;
    static void* keyMousePtr;
    static Vec2<float> Fov;
    static void initialize();

    static void reset();
};

class InstanceManager {
   public:
    InstanceManager() = default;

    std::unordered_map<std::type_index, void*> instances;

    static InstanceManager& getInstance() {
        static InstanceManager instance;
        return instance;
    }

   public:
    InstanceManager(const InstanceManager&) = delete;
    InstanceManager& operator=(const InstanceManager&) = delete;

    template <typename T>
    static void set(T* instance) {
        getInstance().instances[typeid(T)] = static_cast<void*>(instance);
    }

    // Get an instance of a specific type
    template <typename T>
    static T* get() {
        auto& instanceMap = getInstance().instances;
        auto it = instanceMap.find(typeid(T));
        if(it != instanceMap.end() && it->second != nullptr) {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }

    // Check if an instance exists and is valid
    template <typename T>
    static bool has() {
        auto& instanceMap = getInstance().instances;
        auto it = instanceMap.find(typeid(T));
        return (it != instanceMap.end() && it->second != nullptr);
    }

    // Clear a specific instance
    template <typename T>
    static void clear() {
        auto& instanceMap = getInstance().instances;
        auto it = instanceMap.find(typeid(T));
        if(it != instanceMap.end()) {
            it->second = nullptr;
        }
    }

    static void clearAll() {
        getInstance().instances.clear();
    }

    static size_t getInstanceCount() {
        return getInstance().instances.size();
    }

    static ClientInstance* getClientInstance() {
        return get<ClientInstance>();
    }





    static LocalPlayer* getLocalPlayer();
    static BlockSource* getRegion();
    static Level* getLevel();
    static LevelRenderer* getLevelRenderer();
    static PacketSender* getPacketSender();
    static GuiData* getGuiData();
    static std::string getScreenName();
    static MinecraftUIRenderContext* getRenderContext() {
        return get<MinecraftUIRenderContext>();
    }

    static Font* getFont() {
        return get<Font>();
    }

    static ScreenView* getScreenView() {
        return get<ScreenView>();
    }

    static Tessellator* getTessellator() {
        return get<Tessellator>();
    }

    static bool isLocalPlayer(Actor* actor);
    static bool isRenderEnvSafe();
    static bool isKeyDown(uint32_t key);
    static bool isLeftClickDown();
    static bool isRightClickDown();
    static void DisplayClientMessage(const char* fmt, ...);
    static bool canUseMoveKeys();
    static bool isInHudScreen();
};

extern InstanceManager Game;
using GI = InstanceManager;
using G = InstanceManager;
using game = InstanceManager;
using GC = GlobalConfig;