#pragma once

#include <string>
#include "..\SDK\World\Actor\ActorType.h"

class Utils {
   public: 
    static void setTimer(float timer);
    static void resetTimer();
    static ActorType getMob(const std::string& name);
    static void Sneak();
};
