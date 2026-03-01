#include "EjectCommand.h"
#include "../../../Client.h"
#include "../../../Managers/ModuleManager/ModuleManager.h"
//#include "../../../Managers/ModuleManager/Modules/Category/Client/ChatModule.h"

EjectCommand::EjectCommand() : CommandBase("eject", "Removes the cheat from the game.", "", { "uninject" }) {
}

bool EjectCommand::execute(const std::vector<std::string>& args) {
    // Gracefully stop external chat client first to avoid residual threads
    //if (auto chatMod = ModuleManager::getModule<ChatModule>()) {
    //    if (chatMod->isEnabled()) {
    //        chatMod->forceDisable();
    //    }
    //}
    //if (auto musicMod = ModuleManager::getModule<NeteaseMusicGUI>()) {
    //    if (musicMod->isEnabled()) musicMod->setEnabled(false);
    //}
	Client::shutdown();
	return true;
}