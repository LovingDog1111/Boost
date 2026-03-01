#pragma once

#include <memory>

#include "Event.h"

class PacketInEvent : public CancelableEvent {
   public:

    explicit PacketInEvent(std::shared_ptr<class Packet> packet, void* networkIdentifier,
                           void* netEventCallback, bool* cancelPtr = nullptr)
        : mPacket(packet),
          mNetworkIdentifier(networkIdentifier),
          mNetEventCallback(netEventCallback),
          cancel(cancelPtr) {}

    std::shared_ptr<class Packet> mPacket;
    void* mNetworkIdentifier;
    void* mNetEventCallback;
    bool* cancel;  

    template <typename T>
    std::shared_ptr<T> getPacket() const {
        return std::reinterpret_pointer_cast<T>(mPacket);
    }
};
