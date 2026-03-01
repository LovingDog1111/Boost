#include <algorithm>

#include "FriendUtil.h"

std::vector<std::string> FriendManager::friendList;
std::mutex FriendManager::friendMutex;
static std::string normalizeName(const std::string& input) {
    std::string output = input;
    output.erase(std::remove(output.begin(), output.end(), ' '), output.end());
    std::transform(output.begin(), output.end(), output.begin(), ::tolower);
    return output;
}
void FriendManager::addFriend(const std::string& name) {
    std::lock_guard<std::mutex> lock(friendMutex);
    std::string normName = normalizeName(name);
    if(std::find(friendList.begin(), friendList.end(), normName) == friendList.end()) {
        friendList.push_back(normName);
    }
}

void FriendManager::removeFriend(const std::string& name) {
    std::lock_guard<std::mutex> lock(friendMutex);
    std::string normName = normalizeName(name);
    friendList.erase(std::remove(friendList.begin(), friendList.end(), normName), friendList.end());
}

bool FriendManager::isFriend(const std::string& name) {
    std::lock_guard<std::mutex> lock(friendMutex);

    std::string normName = normalizeName(name);
    for(const std::string& friendName : friendList) {
        std::string normFriend = normalizeName(friendName);

        if(normName.find(normFriend) != std::string::npos) {
            return true;
        }
    }
    return false;
}

const std::vector<std::string>& FriendManager::getFriends() {
    return friendList;
}
