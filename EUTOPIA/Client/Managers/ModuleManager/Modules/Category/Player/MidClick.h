#include "../../ModuleBase/Module.h"

class MidClick : public Module {
   public:
    MidClick();
    void onNormalTick(LocalPlayer* player) override;

   private:
    bool wasDown = false;
};