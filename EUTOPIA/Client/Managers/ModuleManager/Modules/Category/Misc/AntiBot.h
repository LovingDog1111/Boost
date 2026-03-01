#pragma once

#include <string>
#include <vector>

#include "../Client/Managers/ModuleManager/Modules/ModuleBase/Module.h"

class AntiBot : public Module {
   public:
    AntiBot();

    void onEnable() override;
    void onDisable() override;
    virtual void onNormalTick(LocalPlayer* localPlayer) override;

    bool isBot(class Actor* actor);
    bool hasArmor(class Actor* actor);

    static std::vector<std::string> getPlayerNames();

    static inline int Mode = 0;
    static inline int EntitylistMode = 0;
    static inline bool HitboxCheck = true;
    static inline bool PlayerCheck = true;
    static inline bool InvisibleCheck = true;
    static inline bool NameCheck = true;
    static inline bool PlayerListCheck = true;
    static inline bool HasArmorCheck = false;
    static inline int ArmorMode = 0;
};
