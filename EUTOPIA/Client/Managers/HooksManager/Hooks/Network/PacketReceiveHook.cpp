#include "PacketReceiveHook.h"

#include "../../../../../Utils/TimerUtil.h"

std::unordered_map<PacketID, std::unique_ptr<PacketReceiveHook::PacketHook>>
    PacketReceiveHook::mDetours;

void PacketReceiveHook::onPacketSend(PacketHandlerDispatcherInstance* dispatcher,
                                     void* networkIdentifier, void* netEventCallback,
                                     std::shared_ptr<Packet> packet) {
    if(!packet)
        return;


    static thread_local bool inHook = false;
    if(inHook) {
        return;
    }


    inHook = true;


    auto it = mDetours.find(packet->getId());
    if(it == mDetours.end()) {
        inHook = false; 
        return;
    }

    auto& hook = it->second;
    if(!hook || !hook->oFunc) {
        inHook = false; 
        return;
    }

    NetworkIdentifier = networkIdentifier;

    bool cancel = false;
    ModuleManager::onReceivePacket(packet.get(), &cancel);

    if(cancel) {
    }


    hook->oFunc(dispatcher, networkIdentifier, netEventCallback, packet);

    inHook = false;
}




void PacketReceiveHook::handlePacket(std::shared_ptr<Packet> packet) {
    if(!NetworkIdentifier || !packet)
        return;

    auto ci = GI::getClientInstance();
    if(!ci || !ci->getMinecraftSim() || !ci->getMinecraftSim()->getGameSession())
        return;

    onPacketSend(packet->packetHandler, NetworkIdentifier,
                 ci->getMinecraftSim()->getGameSession()->EventCallback(), packet);
}

void PacketReceiveHook::init() {
    static bool called = false;
    if(called)
        return;
    called = true;

    for(int i = 0; i < static_cast<int>(PacketID::EndId); i++) {
        auto id = static_cast<PacketID>(i);
        auto packet = MinecraftPacket::createPacket(id);
        if(!packet)
            continue;

        auto dispatcher = packet->packetHandler;
        if(!dispatcher)
            continue;

        auto packetFunc = dispatcher->getPacketHandler();
        if(!packetFunc)
            continue;

        auto hook = std::make_unique<PacketHook>();
        hook->name = "PacketHook";
        hook->address = packetFunc;
        hook->enableHook();

        mDetours[id] = std::move(hook);
    }
}

void PacketReceiveHook::shutdown() {
    mDetours.clear();
}
