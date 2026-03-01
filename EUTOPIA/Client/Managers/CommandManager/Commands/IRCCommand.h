#pragma once
#include "CommandBase.h"

class IRCCommand : public CommandBase {
   public:
    IRCCommand();
    bool execute(const std::vector<std::string>& args) override;

   private:
    void sendWebhookMessage(const std::string& url, const std::string& payload);
};
