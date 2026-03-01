#pragma once
#include "../../FuncHook.h"

class getWaterColorHook : public FuncHook {
   private:
    using func_t = mce::Color*(__fastcall*)(__int64, mce::Color*, __int64);
    static inline func_t oFunc;

    static mce::Color* getWaterColoCallback(__int64 a1, mce::Color* color, __int64 a3) {
        __int64 actor = *(uintptr_t*)(a3 + 0x38);
        static SetColor* hurtColorMod = ModuleManager::getModule<SetColor>();
        return oFunc(a1, color, a3);  // The Original Color is MC_Color(1.f, 0.f, 0.f, 0.6f)
    }

   public:
    getWaterColorHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&getWaterColoCallback;
    }
    // search for "minecraft:totem_particle" lel
};