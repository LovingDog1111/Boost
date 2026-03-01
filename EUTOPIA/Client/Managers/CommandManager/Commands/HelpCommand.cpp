#include "HelpCommand.h"
#include "../../../Client.h"
#include "../../../../SDK/MCTextFormat.h"
#include <algorithm>
#include "../../ModuleManager/Modules/ModuleBase/Module.h"
#include "../../ModuleManager/ModuleManager.h"
#include "../CommandManager.h"

HelpCommand::HelpCommand()
    : CommandBase("help", "Lists all available commands", "[command]", {"h"}) {}

bool HelpCommand::execute(const std::vector<std::string>& args) {
    if(args.size() == 1) {
        Client::DisplayClientMessage("%sAvailable Commands:", MCTF::BOLD);
        for(auto& cmd : CommandManager::commandList) {
            std::string aliasesStr;
            for(const auto& alias : cmd->aliases) {
                aliasesStr += alias + ", ";
            }
            if(!aliasesStr.empty())
                aliasesStr = aliasesStr.substr(0, aliasesStr.size() - 2);

            Client::DisplayClientMessage("  %s%s%s - %s%s", MCTF::GRAY, cmd->name.c_str(),
                                         MCTF::WHITE, MCTF::ITALIC, cmd->description.c_str());
        }
        Client::DisplayClientMessage("Type %s.help <command>%s for more info.", MCTF::GRAY,
                                     MCTF::WHITE);
        return true;
    }

    if(args.size() == 2) {
        std::string targetCmd = args[1];

        for(auto& cmd : CommandManager::commandList) {
            std::string cmdName = cmd->name;
            std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(), ::tolower);
            if(cmdName == targetCmd) {
                Client::DisplayClientMessage("%sCommand: %s%s", MCTF::BOLD, MCTF::RESET,
                                             cmd->name.c_str());
                Client::DisplayClientMessage("%sUsage: %s%c%s %s", MCTF::BOLD, MCTF::RESET,
                                             CommandManager::prefix, cmd->name.c_str(),
                                             cmd->usage.c_str());
                Client::DisplayClientMessage("%sDescription: %s%s", MCTF::BOLD, MCTF::RESET,
                                             cmd->description.c_str());
                if(!cmd->aliases.empty()) {
                    std::string aliasList;
                    for(auto& alias : cmd->aliases)
                        aliasList += alias + ", ";
                    aliasList = aliasList.substr(0, aliasList.size() - 2);
                    Client::DisplayClientMessage("%sAliases: %s%s", MCTF::BOLD, MCTF::RESET,
                                                 aliasList.c_str());
                }
                return true;
            }
        }

        Client::DisplayClientMessage("%sUnknown command: %s%s", MCTF::RED, MCTF::RESET,
                                     targetCmd.c_str());
        return true;
    }

    return false;
}
