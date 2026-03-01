#pragma once
#include "../../../Client.h"
#include "CommandBase.h"

class SetSMCommand : public CommandBase {
   public:
    SetSMCommand();

    bool execute(const std::vector<std::string>& args) override;
};
