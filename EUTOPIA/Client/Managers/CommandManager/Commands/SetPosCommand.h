#pragma once
#include "../../../Client.h"
#include "CommandBase.h"

class SetPosCommand : public CommandBase {
   public:
    SetPosCommand();

    bool execute(const std::vector<std::string>& args) override;
};
