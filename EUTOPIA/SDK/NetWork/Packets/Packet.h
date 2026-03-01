#pragma once
#include "../../Utils/MemoryUtil.h"
#include "PacketID.h"
#include <string>
#include "../PacketHandlerDispatcherInstance.h"

class Packet {
  public:
    CLASS_MEMBER(PacketHandlerDispatcherInstance*, packetHandler, 0x20);
  private:
    char pad_0x0[0x30];
  public:
    PacketID getId() {
      return MemoryUtil::callVirtualFunc<PacketID>(1,this);
    }
  
    std::string getName() {
      return *MemoryUtil::callVirtualFunc<std::string*>(2,this, std::string());
    }
  };