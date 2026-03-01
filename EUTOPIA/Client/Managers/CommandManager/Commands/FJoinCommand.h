#pragma once
#include "../../../Client.h"
#include "CommandBase.h"

class FJoinCommand : public CommandBase {
   public:
    FJoinCommand();

    bool execute(const std::vector<std::string>& args) override;
};
