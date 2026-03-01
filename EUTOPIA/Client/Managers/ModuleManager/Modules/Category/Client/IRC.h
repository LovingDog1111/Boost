#pragma once


#include "../../ModuleBase/Module.h"

class IRC : public Module {
   public:
    IRC();
    bool isIRCEnabled() const;

   private:
    bool enabledByUser = true;
};