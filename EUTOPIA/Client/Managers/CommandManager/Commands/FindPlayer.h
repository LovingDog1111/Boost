#pragma once
#include "../../../Client.h"
#include "CommandBase.h"

class FindPlayer : public CommandBase {
   public:
    FindPlayer();

    bool execute(const std::vector<std::string>& args) override;
    std::vector<std::string> getCurrentPlayerList2();


};
