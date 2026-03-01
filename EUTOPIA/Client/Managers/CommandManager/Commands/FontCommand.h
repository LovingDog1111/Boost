#pragma once
#include "CommandBase.h"

class FontCommand : public CommandBase {
   public:
    FontCommand();
    bool execute(const std::vector<std::string>& args) override;
};
