#pragma once
//Entry point in player

#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include "HitResult.h"
#include "../../NetWork/Packets/PlayerListPacket.h"
#include "../../Utils/MemoryUtil.h"
#include "../../Core/mce.h"
#include "LevelData.h"
#include "../Actor/Components/ActorUniqueIDComponent.h"

class Actor;
struct Wrapper {
    uint8_t pad[0x10];
    void* ptrToT;

    template <typename T>
    T* get() const {
        return reinterpret_cast<T*>(ptrToT);
    }
};

class SkinClass {
   public:
    mce::PlayerSkin* serializedSkin;
};

class PlayerListEntry {
   public:
    ActorUniqueIDComponent mId;
    mce::UUID uuid;
    std::string name;
    std::string xuid;
    std::string platformonlineId;
    DWORD buildPlatform;
    SkinClass skin;

    mce::PlayerSkin* getPlayerSkin() {
        auto sig =
            "48 8B 0D ?? ?? ?? ?? 48 8B 01 BA ?? ?? ?? ?? 48 8B 40 ?? FF 15 ?? ?? ?? ?? 48 8B F8 "
            "48 85 C0 75 3C 48 C7 44 24 ?? ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 44 24 ?? 4C 8D "
            "0D ?? ?? ?? ?? 44 8D 47 ?? 48 8D 15 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? "
            "84 C0 74 05 E8 ?? ?? ?? ?? 48 89 7C 24 ?? 48 85 FF 74 33 C7 47 ?? ?? ?? ?? ?? C7 47 "
            "?? ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 07 33 D2 41 B8 ?? ?? ?? ?? 48 8D 4F ?? E8 "
            "?? ?? ?? ?? 48 8D 4F ?? E8 ?? ?? ?? ?? 90 0F 57 C0 0F 11 06 48 C7 46 ?? ?? ?? ?? ?? "
            "48 8D 4F";
        using getfunc_sirankedo = int64_t(__fastcall*)(void* vftable, int64_t param);
        void*** off_8483478 = reinterpret_cast<void***>(MemoryUtil::getVtableFromSig(sig));

        getfunc_sirankedo createInstance = reinterpret_cast<getfunc_sirankedo>((*off_8483478)[1]);

        void* raw = reinterpret_cast<void*>(createInstance(off_8483478, 0x220));
        if(!raw)
            return nullptr;

        void* ptr = *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(raw) + 0x10);
        return reinterpret_cast<mce::PlayerSkin*>(ptr);
    }
};



class Level {
public:
    CLASS_MEMBER(std::shared_ptr<LevelData>, mLevelData, 0x90);
   public:
    std::unordered_map<mce::UUID, PlayerListEntry>* getPlayerList()
    {
        
        return MemoryUtil::callVirtualFunc<std::unordered_map<mce::UUID, PlayerListEntry>*>(303, this);
    }
    //std::unordered_map<mce::UUID, PlayerListEntry>& getPlayerMap() {
    //    return hat::member_at<std::unordered_map<mce::UUID, PlayerListEntry>>(this, 0x1BC8); // need offset
    //}

    //std::unordered_map<mce::UUID, PlayerListEntry>& getPlayerMap() {}
	

std::vector<Actor*> getRuntimeActorList() {
	std::vector<Actor*> listOut;
	MemoryUtil::callVirtualFunc<decltype(&listOut)>(309, this, &listOut);
	return listOut;
}


HitResult* getHitResult() {
	return MemoryUtil::callVirtualFunc<HitResult*>(319, this);
}

};