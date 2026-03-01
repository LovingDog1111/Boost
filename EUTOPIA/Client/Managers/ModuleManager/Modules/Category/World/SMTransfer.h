#pragma once
#include "../../ModuleBase/Module.h"

class SMTransfer : public Module {
   private:
    int targetSMIndex = -1;

    void sendTransferCommand(int smIndex);

   public:
    SMTransfer();
    ~SMTransfer();

    void onEnable() override;
    void setMainSMIndex(int smIndex);
    void setMainSMLIndex(int smIndex);
};
