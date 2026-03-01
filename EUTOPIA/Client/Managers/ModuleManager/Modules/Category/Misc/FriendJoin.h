#pragma once
#include <string>
#include <unordered_set>
#include <vector>

#include "../../ModuleBase/Module.h"

class FriendJoin : public Module {
   private:
    std::unordered_set<std::string> friends;

   public:
    FriendJoin();

    void onNormalTick(LocalPlayer* localPlayer);

    std::vector<std::string> getCurrentPlayerList();

    
    void addFriend(const std::string& name) {
        friends.insert(name);
    }
    void removeFriend(const std::string& name) {
        friends.erase(name);
    }

    std::vector<std::string> getFriends() const {
        return std::vector<std::string>(friends.begin(), friends.end());
    }
};
