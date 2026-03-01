#pragma once
#include "../../../Client.h"
#include "CommandBase.h"

class TracerCommand : public CommandBase {
   public:
    TracerCommand();

    bool execute(const std::vector<std::string>& args) override;
};
