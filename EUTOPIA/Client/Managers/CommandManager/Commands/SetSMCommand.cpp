#include "SetSMCommand.h"

#include "../../../Client.h"
#include "../../Client/Managers/ModuleManager/ModuleManager.h"

SetSMCommand::SetSMCommand()
    : CommandBase("setSM", "Set your main SM teleport target with a module transfer.",
                  "<sm|sml> <1-120>", {}) {}

bool SetSMCommand::execute(const std::vector<std::string>& args) {
    if(args.size() < 3) {
        Client::DisplayClientMessage("Usage: .setSM <sm|sml> <1-120>", MCTF::RED);
        return false;
    }

    std::string smType = args[1]; 
    int smIndex = -1;

    try {
        smIndex = std::stoi(args[2]);  
    } catch(...) {
        Client::DisplayClientMessage(
            "Invalid number for SM index. Please enter a valid number between 1 and 120.",
            MCTF::RED);
        return false;
    }

    if(smIndex < 1 || smIndex > 120) {
        Client::DisplayClientMessage("SM index must be between 1 and 120.", MCTF::RED);
        return false;
    }

    std::string targetSM =
        smType + std::to_string(smIndex); 

    auto smTransferModule = ModuleManager::getModule<SMTransfer>();
    if(!smTransferModule) {
        Client::DisplayClientMessage("SMTransfer module not found.", MCTF::RED);
        return false;
    }

    if(smType == "sm") {
        smTransferModule->setMainSMIndex(smIndex);  
    } else if(smType == "sml") {
        smTransferModule->setMainSMLIndex(smIndex); 
    } else {
        Client::DisplayClientMessage("Invalid SM type. Use 'sm' or 'sml'.", MCTF::RED);
        return false;
    }

    std::string confirmMsg = "Main SM teleport target set to " + targetSM;
    Client::DisplayClientMessage(confirmMsg.c_str(), MCTF::GREEN);
    return true;
}
