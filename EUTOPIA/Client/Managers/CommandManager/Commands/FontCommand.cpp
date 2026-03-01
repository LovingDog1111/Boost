#include "FontCommand.h"

#include "../../../Client.h"
#include "..\Client\Managers\ModuleManager\ModuleManager.h"


FontCommand::FontCommand() : CommandBase("font", "Set font size", "<size>", {"f"}) {}

bool FontCommand::execute(const std::vector<std::string>& args) {
    if(args.size() < 2)
        return false;

    int size = std::stoi(args[1]);
    if(size < 15)
        size = 15;
    if(size > 30)
        size = 30;

    auto module = ModuleManager::getModule<CustomFont>();
    if(!module)
        return true;

    module->fontSize = size;
    Client::DisplayClientMessage("Font size set to %d", size);
    return true;
}
