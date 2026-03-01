#pragma once
#include "../../../Client.h"
#include "../../Client/Managers/ModuleManager/ModuleManager.h"
#include "..\Client\Managers\CommandManager\Commands\CommandBase.h"


class ThemeCommand : public CommandBase {
   public:
    ThemeCommand();
    bool execute(const std::vector<std::string>& args) override;

   private:
    UIColor parseColorName(const std::string& name);
};
