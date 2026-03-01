#pragma once
#include "CommandBase.h"

class FriendCommand : public CommandBase {
   public:
    FriendCommand();
    bool execute(const std::vector<std::string>& args) override;
};