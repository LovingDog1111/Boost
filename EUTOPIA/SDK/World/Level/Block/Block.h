#pragma once
//reigon-block
#include "BlockLegacy.h"
#include "../HitResult.h"
#include <MemoryUtil.h>

class Block {
public:
	CLASS_MEMBER(BlockLegacy*, blockLegcy, 0x78);
    CLASS_MEMBER(BlockLegacy*, blockLegacy, 0x78); 


public:
	uint32_t* getRuntimeID() {
		return hat::member_at<uint32_t*>(this, 0xc0);
	}
    bool isInteractiveBlock() {
        using func_t = bool(__fastcall*)(void*);
        static func_t Func = reinterpret_cast<func_t>(MemoryUtil::getFuncFromCall(
            MemoryUtil::findSignature("E8 ? ? ? ? 41 88 46 ? 49 8B 4E")));
        return Func(this);
    }
};

struct ActorBlockSyncMessage {
   public:
    // ActorBlockSyncMessage inner types define
    enum class MessageId : int {
        CREATE = 0x1,
        DESTROY = 0x2,
    };

    uint64_t mEntityUniqueID;
    ActorBlockSyncMessage::MessageId mMessage;
};