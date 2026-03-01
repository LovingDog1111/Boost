#pragma once
#include <MemoryUtil.h>
class InputLMode;
class GameMode {

public:
	CLASS_MEMBER(class Actor*, mPlayer, 0x8);
    CLASS_MEMBER(BlockPos, lastBreakPos, 0x10);
    CLASS_MEMBER(int32_t, lastBreakFace, 0x1C);
    CLASS_MEMBER(float, lastDestroyProgress, 0x20);
	CLASS_MEMBER(float, mBreakProgress, 0x24);
    CLASS_MEMBER(float, destroyProgress, 0x24);
    
    




    virtual ~GameMode() = 0;
    virtual __int64 startDestroyBlock(Vec3<int> const& pos, unsigned char blockSide,
                                      bool& isDestroyedOut);                  // 1
    virtual __int64 destroyBlock(const Vec3<int>&, unsigned char blockSide);  // 2
    virtual __int64 continueDestroyBlock(Vec3<int> const& a1, unsigned char a2,
                                         Vec3<float> const& a3, bool& a4);              // 3
    virtual __int64 stopDestroyBlock(Vec3<int> const&);                                 // 4
    virtual __int64 startBuildBlock(Vec3<int> const& a1, unsigned char a2, bool auth);  // 5
    virtual __int64 buildBlock(Vec3<int> const& a1, unsigned char a2, bool auth);       // 6
    virtual __int64 continueBuildBlock(Vec3<int> const& a1, unsigned char a2);          // 7
    virtual __int64 stopBuildBlock();                                                   // 8
    virtual __int64 tick();                                                             // 9
    virtual __int64 getPickRange(InputMode const& a1, bool a2);                         // 10
    virtual __int64 useItem(class ItemStack& a1);                                       // 11
    virtual __int64 useItemOn(ItemStack& a1, Vec3<int> const& a2, unsigned char a3,
                              Vec3<float> const& a4, class Block const* a5);  // 12
    virtual __int64 interact(Actor* a1, Vec3<float> const& a2);               // 13
    virtual __int64 attack(Actor*);                                           // 14
    virtual __int64 releaseUsingItem();                                       // 15

    float getDestroyRate(Block* block) {

        using func_t = float(__fastcall*)(GameMode*, Block*);
        static func_t Func = reinterpret_cast<func_t>(
            MemoryUtil::getFuncFromCall(MemoryUtil::findSignature("E8 ? ? ? ? 0F 28 F8 49 8B 4E")));
        return Func(this, block);


    }
    bool baseUseItem(ItemStack* itemStack) {
        using func_t = bool(__fastcall*)(GameMode*, ItemStack*);
        static func_t Func =
            reinterpret_cast<func_t>(MemoryUtil::getFuncFromCall(MemoryUtil::findSignature(
                "E8 ? ? ? ? 84 C0 74 ? 48 8B ? 48 8B ? 48 8B ? ? ? ? ? FF 15 ? ? ? ? 48 85")));
        return Func(this, itemStack);
    }
};