#include "FJoinCommand.h"

#include "../../Client/Managers/ModuleManager/ModuleManager.h"

FJoinCommand::FJoinCommand()
    : CommandBase("fjoin", "Manage friends for join alerts", "<add|remove|list> [player]", {}) {}

bool FJoinCommand::execute(const std::vector<std::string>& args) {
    auto friendModule = ModuleManager::getModule<FriendJoin>();
    if(!friendModule) {
        Client::DisplayClientMessage("FriendJoin module not found.", MCTF::RED);
        return false;
    }

    if(args.size() < 2) {
        Client::DisplayClientMessage("Usage: .fjoin <add|remove|list> [player]", MCTF::RED);
        return false;
    }

    std::string action = args[1];

    if(action == "add") {
        if(args.size() < 3) {
            Client::DisplayClientMessage("Usage: .fjoin add <player>", MCTF::RED);
            return false;
        }
        std::string playerName = args[2];
        friendModule->addFriend(playerName);
        Client::DisplayClientMessage(("Added friend: " + playerName).c_str(), MCTF::GREEN);
        return true;
    } else if(action == "remove") {
        if(args.size() < 3) {
            Client::DisplayClientMessage("Usage: .fjoin remove <player>", MCTF::RED);
            return false;
        }
        std::string playerName = args[2];
        friendModule->removeFriend(playerName);
        Client::DisplayClientMessage(("Removed friend: " + playerName).c_str(), MCTF::GREEN);
        return true;
    } else if(action == "list") {
        auto friends = friendModule->getFriends();
        if(friends.empty()) {
            Client::DisplayClientMessage("No friends added.", MCTF::YELLOW);
            return true;
        }

        std::string list = "Friends: ";
        for(const auto& f : friends)
            list += f + ", ";
        list = list.substr(0, list.size() - 2);  
        Client::DisplayClientMessage(list.c_str(), MCTF::GREEN);
        return true;
    }

    Client::DisplayClientMessage("Invalid action. Use add, remove, or list.", MCTF::RED);
    return false;
}
