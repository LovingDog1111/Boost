#pragma once
#pragma once

#include <string>

#include "../../ModuleBase/Module.h"

class Chat : public Module {
   public:
    Chat();
    ~Chat() {}
    void onSendPacket(Packet* packet) override;
    std::string generateClipString(int length);
    void setCustomSuffix(const std::string& suffix);

   private:
    int clipValue = 5;
    bool useSeparator = true;
    bool useSuffix = true;
    bool antiFilter = true;
    std::string suffixText = "Boost";
};
