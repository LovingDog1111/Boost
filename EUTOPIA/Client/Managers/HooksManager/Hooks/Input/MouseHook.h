#pragma once
#include "../FuncHook.h"

class MouseHook : public FuncHook {
   private:
    using func_t = void*(__fastcall*)(void* _this, char actionButtonId, int buttonData, short x,
                                      short y, short dx, short dy, uint8_t forceMotionlessPointer);
    static inline func_t oFunc = nullptr;    
    static inline void* thisMouse = nullptr; 

    static void* MouseCallback(void* _this, char actionButtonId, int buttonData, short x, short y,
                               short dx, short dy, uint8_t forceMotionlessPointer) {
        thisMouse = _this;  


        return oFunc(_this, actionButtonId, buttonData, x, y, dx, dy, forceMotionlessPointer);
    }

   public:
    MouseHook() {

        OriginFunc = (void*)&oFunc;
        func = (void*)&MouseCallback;
    }

    static void simulateMouseInput(char actionButtonId, int buttonData, short x, short y, short dx,
                                   short dy) {
     
        if(!thisMouse) {
            return;  
        }

        using func_t = void*(__fastcall*)(void*, char, int, short, short, short, short, uint8_t);
        auto fn = reinterpret_cast<func_t>(oFunc);

        fn(thisMouse, actionButtonId, buttonData, x, y, dx, dy,
           2);  
    }
};
