#pragma once
#include <iostream>
#include <string>

#include "../../../../../../SDK/Render/Tessellator.h"
#include "../../../../../../SDK/World/Level/BlockTessellator.h"
#include "../../FuncHook.h"

class TessellateBlockInWorldHook : public FuncHook {
   private:
    // 更新函数签名以匹配新版本
    using func_t = __int64(__fastcall*)(BlockTessellator*, int, Tessellator*, Block*, const BlockPos&, void*);
    static inline func_t oFunc;

    static __int64 TessellateBlockInWorldCallBack(BlockTessellator* _this,
                                                  int a2,  // 新增的参数
                                                  Tessellator* tessellator, Block* block,
                                                  const BlockPos& pos,  // BlockPos (压缩为int)
                                                  void* airAndSimpleBlocks) {
        // 调用原函数
        __int64 result = oFunc(_this, a2, tessellator, block, pos, airAndSimpleBlocks);

        // 基本的安全检查
        if(!tessellator || !block || !_this) {
            return result;
        }

        auto blok = block;
        Vec3<int> Pos = pos;


        return result;
    }

   public:
    TessellateBlockInWorldHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&TessellateBlockInWorldCallBack;
    }
};