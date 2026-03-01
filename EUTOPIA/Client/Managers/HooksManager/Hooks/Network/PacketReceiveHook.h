#pragma once
#include <memory>
#include <unordered_map>

#include "../../../../../SDK/NetWork/PacketHandlerDispatcherInstance.h"
#include "../FuncHook.h"

static inline void* NetworkIdentifier = nullptr;

class PacketReceiveHook {
   private:
    class PacketHook : public FuncHook {
       public:
        using func_t = void(__thiscall*)(PacketHandlerDispatcherInstance*, void*, void*,
                                         std::shared_ptr<Packet>);

        func_t oFunc = nullptr;

        PacketHook() {
            OriginFunc = &oFunc;
            func = (void*)&PacketCallback;
        }

        static void PacketCallback(PacketHandlerDispatcherInstance* dispatcher,
                                   void* networkIdentifier, void* netEventCallback,
                                   std::shared_ptr<Packet> packet) {
            PacketReceiveHook::onPacketSend(dispatcher, networkIdentifier, netEventCallback,
                                            packet);
        }
    };

    static std::unordered_map<PacketID, std::unique_ptr<PacketHook>> mDetours;

   public:
    static void onPacketSend(PacketHandlerDispatcherInstance* dispatcher, void* networkIdentifier,
                             void* netEventCallback, std::shared_ptr<Packet> packet);

    static void handlePacket(std::shared_ptr<Packet> packet);
    static void init();
    static void shutdown();
};
