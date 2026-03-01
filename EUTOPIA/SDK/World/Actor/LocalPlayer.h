#pragma once
#include <string>
#include "../../Utils/MemoryUtil.h"
#include "Player.h"
class wtd
{
public:
	char sb[0x120];
};


class LocalPlayer : public Player  
{
public:
#ifdef _DEBUG  // Debug mode
    void displayClientMessage(const std::string& message) {
        static std::unique_ptr<wtd> wtd1 = std::make_unique<wtd>();

        // Release version: directly use std::string
        MemoryUtil::callVirtualFunc<void>(200, this, message, wtd1.get());
    }
#else  // Release mode
    void displayClientMessage(const std::string& message) {
        static std::unique_ptr<wtd> wtd1 = std::make_unique<wtd>();
        
        // Release version: directly use std::string
        MemoryUtil::callVirtualFunc<void>(200, this, message, wtd1.get());
    }
#endif
};