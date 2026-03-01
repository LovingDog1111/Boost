#pragma once
#include "../FuncHook.h"

class GetAveragePingHook : public FuncHook {
   private:
    using func_t = int(__thiscall*)(void*, std::string const&);
    static inline func_t oFunc;
    static inline int lastPing = 0;

    static int GetAveragePingCallback(void* _this, std::string const& addressOrGuid) {
        int ping = oFunc(_this, addressOrGuid);

        lastPing = ping;

        return ping;
    }

   public:
    GetAveragePingHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&GetAveragePingCallback;
    }

    static int getLastPing() {
        return lastPing;
    }
};