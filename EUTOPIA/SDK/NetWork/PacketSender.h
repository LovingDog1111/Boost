#pragma once
#include "../../Utils/MemoryUtil.h"
#include "Packets/Packet.h"
#include "NetworkSystem.h"
class PacketSender   {
  public:
      CLASS_MEMBER(NetworkSystem*, mNetworkSystem, 0x20);

      // credit: tozic
  public:


  
      virtual ~PacketSender();
       virtual bool isInitialized() = 0;
      // Packets sent with this function will trigger the PacketOutEvent.
      virtual void send(Packet* packet);
       virtual void sendTo(void* packet) = 0;
      // Calling this function will NOT trigger the PacketOutEvent.
      virtual void sendToServer(Packet* packet);
      virtual __int64 sendToClient(__int64 UserEntityIdentifierComponent, Packet* const&);
      virtual __int64 sendToClient(const void* networkIdentifier, const Packet* packet, int a4);
      virtual __int64 sendToClients(const void* networkIdentifier, const Packet* packet);
      virtual void sendBroadcast(Packet* packet);
  };