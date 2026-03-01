#pragma once
#include "../FuncHook.h"
#include "../../../../../SDK/World/Inventory/ContainerScreenController.h"

class ContainerScreenControllerHook : public FuncHook {
private:
    using func_t = uint32_t(__thiscall*)(ContainerScreenController*);
    static inline func_t oFunc;

    static uint32_t ContainerScreenTickCallback(ContainerScreenController* _this) {
        // Future: add custom handling for container screen ticks here
        auto player = GI::getLocalPlayer();
        if(!player)
            return oFunc(_this);  // just in case
        if(player->getContainerManagerModel()->mContainerType != ContainerType::Container) {
            return oFunc(_this);
        }
        ModuleManager::onChestScreen(_this);
        return oFunc(_this);
    }
public:
    ContainerScreenControllerHook() {
        OriginFunc = (void*)&oFunc;
        func       = (void*)&ContainerScreenTickCallback;
    }
};
