#pragma once
#include "Actor.h"

class Mob : public Actor {  // idek lol 
   public:
    void setSprinting(bool shouldSprint) {
        MemoryUtil::callVirtualFunc<void, bool>(140, this, shouldSprint);
    }

    void setSneaking(bool shouldSneak) {
        MemoryUtil::callVirtualFunc<void, bool>(139, this,
                                                shouldSneak); 
    }

    void setJumping(bool shouldJump) {
        MemoryUtil::callVirtualFunc<void, bool>(141, this, shouldJump);  
    }
};
