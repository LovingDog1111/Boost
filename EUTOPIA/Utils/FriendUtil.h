#pragma once
#include <mutex>
#include <string>
#include <vector>

class FriendManager {
   public:
    static void addFriend(const std::string& name);
    static void removeFriend(const std::string& name);
    static bool isFriend(const std::string& name);
    static const std::vector<std::string>& getFriends();

   private:
    static std::vector<std::string> friendList;
    static std::mutex friendMutex;
};

