#pragma once
#include "../FuncHook.h"

class KeyMouseHook : public FuncHook {
   private:
    using func_t = void(__fastcall*)(__int64, char, char, __int16, __int16, __int16, __int16, char);
    static inline func_t oFunc;

    static void mouseInputCallback(__int64 a1, char mouseButton, char isDown, __int16 mouseX,
                                   __int16 mouseY, __int16 relativeMovementX,
                                   __int16 relativeMovementY, char a8) {
        GC::keyMousePtr = (void*)(a1 + 0x10);
        D2D::mpos = Vec2<float>((float)mouseX, (float)mouseY);

        static ClickGUI* clickGuiMod = ModuleManager::getModule<ClickGUI>();

        if(clickGuiMod->isEnabled()) {
            clickGuiMod->onMouseUpdate(Vec2<float>((float)mouseX, (float)mouseY), mouseButton,
                                       isDown);
            return;
        }

        oFunc(a1, mouseButton, isDown, mouseX, mouseY, relativeMovementX, relativeMovementY, a8);
    }

   public:
    KeyMouseHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&mouseInputCallback;
    }

    static void simulate(char mouseButton, bool isDown, short mouseX, short mouseY) {
        if(!GC::keyMousePtr || !oFunc)
            return;
        auto fn = reinterpret_cast<func_t>(oFunc);
        fn((__int64)GC::keyMousePtr, mouseButton, isDown ? 1 : 0, mouseX, mouseY, 0, 0, 0);
    }
};
