#include "FriendCommand.h"

#include "../../../../SDK/MCTextFormat.h"
#include "../../../Client.h"
#include "../../Utils/FriendUtil.h"

FriendCommand::FriendCommand()
    : CommandBase("friend", "Manage your friends list", "<add/remove/list> <name>") {}

bool FriendCommand::execute(const std::vector<std::string>& args) {
    if(args.size() < 2) {
        Client::DisplayClientMessage("Usage: %s.friend <add/remove/list> <name>", MCTF::GRAY);
        return true;
    }

    std::string action = args[1];

    if(action == "list") {
        const auto& friends = FriendManager::getFriends();
        if(friends.empty()) {
            Client::DisplayClientMessage("Friend list is empty.");
        } else {
            Client::DisplayClientMessage("Friends:");
            for(const auto& name : friends) {
                Client::DisplayClientMessage("- %s%s", MCTF::GRAY, name.c_str());
            }
        }
        return true;
    }

    if(args.size() < 3) {
        Client::DisplayClientMessage("Please specify a name.");
        return true;
    }

    std::string name = args[2];

    if(action == "add") {
        FriendManager::addFriend(name);
        Client::DisplayClientMessage("Added %s%s %sto your friends list.", MCTF::GRAY, name.c_str(),
                                     MCTF::WHITE);
    } else if(action == "remove") {
        FriendManager::removeFriend(name);
        Client::DisplayClientMessage("Removed %s%s %sfrom your friends list.", MCTF::GRAY,
                                     name.c_str(), MCTF::WHITE);
    } else {
        Client::DisplayClientMessage("Unknown action: %s%s", MCTF::GRAY, action.c_str());
    }
    
    return true;
}