#include "CommandManager.h"
#include "../../Client.h"
#include <sstream>
#include "../../../SDK/MCTextFormat.h"
#include "Commands/BindCommand.h"
#include "Commands/UnbindCommand.h"
#include "Commands/ConfigCommand.h"
#include "Commands/EjectCommand.h"
#include "Commands/FriendCommand.h"
#include "Commands/HelpCommand.h"
#include "Commands/IRCCommand.h"
//#include "Commands/AICommand.h"
//#include "Commands/KeyCommand.h" poo shiddy
#include "Commands/FindPlayer.h"
#include "Commands/SetPosCommand.h"
#include "Commands/Toggle.h"
#include "Commands/TracerCommand.h"
#include "Commands/FJoinCommand.h"
#include "Commands/SetSMCommand.h"
#include "Commands/AssistantCommand.h"
#include "Commands/FontCommand.h"
#include "Commands/ThemeCommand.h"

void CommandManager::init() {
	// Register all available commands here
    commandList.push_back(new BindCommand());
    commandList.push_back(new AssistantCommand());
    commandList.push_back(new ThemeCommand());
	commandList.push_back(new UnbindCommand());
	commandList.push_back(new ConfigCommand());
    commandList.push_back(new SetPosCommand());
	commandList.push_back(new EjectCommand());
    commandList.push_back(new FriendCommand());
    commandList.push_back(new HelpCommand());
    commandList.push_back(new IRCCommand());
    //commandList.push_back(new KeyCommand());
    commandList.push_back(new FindPlayer());
    commandList.push_back(new Toggle());
    commandList.push_back(new SetSMCommand());
    commandList.push_back(new FJoinCommand());
    commandList.push_back(new TracerCommand());
    commandList.push_back(new FontCommand());
    //commandList.push_back(new AICommand());
    //commandList.push_back(new SetPosCommand());
}

void CommandManager::shutdown() {
	for (auto& command : commandList) {
		delete command;
		command = nullptr;
	}
	commandList.clear();
}

void CommandManager::execute(const std::string& message) {
	std::string currentMessage = message;
	currentMessage.erase(currentMessage.begin());

	std::vector<std::string> args;
	std::stringstream ss(currentMessage);
	std::string word;

	while (ss >> word) {
		std::transform(word.begin(), word.end(), word.begin(), ::tolower);
		args.push_back(word);
	}

	if (args.empty())
		return;

	std::string cmd = args[0];
	for (auto& command : commandList) {
		std::vector<std::string> checkList;
		checkList.push_back(command->name);
		for (const std::string& alias : command->aliases)
			checkList.push_back(alias);

		for (std::string check : checkList) {
			std::transform(check.begin(), check.end(), check.begin(), ::tolower);
			if (cmd == check) {
				if (!command->execute(args)) {
					Client::DisplayClientMessage("Failed on execute command '%s%s%s'", MCTF::GRAY, message.c_str(), MCTF::WHITE);
					Client::DisplayClientMessage("%sUsage: %s%c%s %s", MCTF::BOLD, MCTF::RESET, prefix, command->name.c_str(), command->usage.c_str());
				}

				return;
			}
		}
	}

	Client::DisplayClientMessage("Couldn't find command %s%s%s!", MCTF::GRAY, cmd.c_str(), MCTF::WHITE);
}