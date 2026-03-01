#include "notifs.h"

#include "../../../ModuleManager.h"

notifs::notifs() : Module("Notifications", "Displays module notifications", Category::CLIENT) {}

void notifs::onEnable() {
    moduleStates.clear();
    for(auto mod : ModuleManager::moduleList) {
        if(mod != this)
            moduleStates[mod->getModuleName()] = mod->isEnabled();
    }
}

void notifs::onDisable() {
    moduleStates.clear();
}

void notifs::onNormalTick(LocalPlayer* player) {
    for(auto mod : ModuleManager::moduleList) {
        if(mod == this)
            continue;

        std::string moduleName = mod->getModuleName();
        bool currentState = mod->isEnabled();

        if(moduleStates.find(moduleName) != moduleStates.end()) {
            bool previousState = moduleStates[moduleName];

            if(previousState != currentState) {
                std::string status = currentState ? "enabled" : "disabled";
                NotificationManager::addNotification(moduleName + " was " + status, 3.f);
            }
        }

        moduleStates[moduleName] = currentState;
    }
}
