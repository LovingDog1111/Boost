#include "FriendJoin.h"

#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <unordered_set>

#include "../../Client/Client.h"

FriendJoin::FriendJoin()
    : Module("FriendJoin", "Show messages when friends join/leave", Category::MISC) {}

static std::unordered_map<std::string, std::chrono::steady_clock::time_point> lastAlertTime;

void FriendJoin::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    std::vector<std::string> currentPlayerList = getCurrentPlayerList();

    static std::unordered_set<std::string> seenPlayers;
    auto now = std::chrono::steady_clock::now();

  
    for(const auto& player : currentPlayerList) {
        if(seenPlayers.find(player) == seenPlayers.end()) {
            
            if(friends.find(player) != friends.end()) {
                GI::DisplayClientMessage(("%s[FRIEND]%s " + player + " joined!").c_str(),
                                         MCTF::GREEN, MCTF::WHITE);
                lastAlertTime[player] = now;
            }
            seenPlayers.insert(player);
        }
    }

    
    for(auto it = seenPlayers.begin(); it != seenPlayers.end();) {
        if(std::find(currentPlayerList.begin(), currentPlayerList.end(), *it) ==
           currentPlayerList.end()) {
            if(friends.find(*it) != friends.end()) {
                GI::DisplayClientMessage(("%s[FRIEND]%s " + *it + " left.").c_str(), MCTF::RED,
                                         MCTF::WHITE);
                lastAlertTime.erase(*it);
            }
            it = seenPlayers.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<std::string> FriendJoin::getCurrentPlayerList() {
    std::vector<std::string> playerList;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    Level* level = localPlayer ? localPlayer->level : nullptr;
    if(level) {
        auto* playerMap = level->getPlayerList();
        if(playerMap) {
            for(const auto& pair : *playerMap) {
                playerList.push_back(pair.second.name);
            }
        }
    }
    return playerList;
}
