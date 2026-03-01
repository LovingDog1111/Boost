#pragma once
#include "../../../Client.h"
#include "CommandBase.h"

class Toggle : public CommandBase {
   public:
    Toggle();

    bool execute(const std::vector<std::string>& args) override;
};
