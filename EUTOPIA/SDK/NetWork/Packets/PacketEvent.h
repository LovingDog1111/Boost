#pragma once

#include <memory>

#include "Packet.h"  

class PacketEvent {
   public:
    PacketEvent(std::shared_ptr<Packet> pkt) : mPacket(pkt), mCancelled(false) {}


    std::shared_ptr<Packet> getPacket() const {
        return mPacket;
    }

    bool isCancelled() const {
        return mCancelled;
    }
    void setCancelled(bool cancelled) {
        mCancelled = cancelled;
    }
    
   private:
    std::shared_ptr<Packet> mPacket;  
    bool mCancelled;                  
};
