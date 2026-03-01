#pragma once
#include "../FuncHook.h"

class RenderOutLineSelectionHook : public FuncHook {
   private:
    using func_t = void (__fastcall*)(LevelRendererPlayer*,   ScreenContext*,  Block*, BlockSource*,const BlockPos&);
    static inline func_t oFunc;

    static void RenderOutLineSelectionHookCallback(LevelRendererPlayer* obj,
                                                          ScreenContext* scn, Block* block,
                                                          BlockSource* region, const BlockPos & pos) {

        static BlockHighlight* blockMod = ModuleManager::getModule<BlockHighlight>();
        if(blockMod->isEnabled())
            return;
        oFunc(obj, scn, block, region,
                     pos);  
    }

   public:
    RenderOutLineSelectionHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&RenderOutLineSelectionHookCallback;
    }
};