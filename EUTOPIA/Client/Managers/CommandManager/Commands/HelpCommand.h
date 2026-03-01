#pragma once
#include "CommandBase.h"

class HelpCommand : public CommandBase {
public:
	HelpCommand();
	bool execute(const std::vector<std::string>& args) override;
};
