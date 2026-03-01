#include "Toggle.h"

#include "../../../Managers/ModuleManager/ModuleManager.h"

Toggle::Toggle() : CommandBase("toggle", "Toggle a module on/off", "<moduleName>", {}) {}

bool Toggle::execute(const std::vector<std::string>& args) {
    if(args.size() < 2) {
        Client::DisplayClientMessage("Usage: .toggle <moduleName>", MCTF::RED);
        return false;
    }

    std::string name = args[1];
    auto module = ModuleManager::getModuleByName(name);
    if(!module) {
        Client::DisplayClientMessage(("Module '" + name + "' not found.").c_str(), MCTF::RED);
        return false;
    }

    module->setEnabled(!module->isEnabled());
    std::string status = module->isEnabled() ? "enabled" : "disabled";
    Client::DisplayClientMessage(("Module '" + name + "' " + status).c_str(), MCTF::GREEN);
    return true;
}
