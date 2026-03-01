#pragma once
#include <cstdint>  // 添加这个头文件

#include "../../FuncHook.h"

class ActorSetRotHook : public FuncHook {
   private:
    using func_t = __int64(__fastcall*)(float*, uint32_t*, __int64);
    static inline func_t oFunc;

    static __int64 ActorSetRotCallback(float* a1, uint32_t* a2, __int64 a3) {
        // a3 应该是Actor指针
        Actor* actor = reinterpret_cast<Actor*>(a3);
        auto lp = GI::getLocalPlayer();
         if(actor == lp) {
            __int64 result = oFunc(a1, a2, a3);
             ModuleManager::onUpdateRotation(lp);
             return result;
        }

        return oFunc(a1, a2, a3);
    }

   public:
    ActorSetRotHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&ActorSetRotCallback;
    }
};