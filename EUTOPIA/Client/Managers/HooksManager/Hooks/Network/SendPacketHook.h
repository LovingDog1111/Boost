#pragma once
#include "../FuncHook.h"
#if __has_include("../../../../../Utils/ConsoleUtil.h")
#include "../../../../../Utils/ConsoleUtil.h"
#else
#include "../../../../Utils/ConsoleUtil.h"
#endif

int lastSlot = 0;

class SendPacketHook : public FuncHook {
   private:
    using func_t = void(__thiscall*)(PacketSender*, Packet*);
    static inline func_t oFunc;

    static void SendPacketCallback(PacketSender* _this, Packet* packet) {
        if(!packet) {
            oFunc(_this, packet);
            return;
        }

 

        static NoPacket* noPacketMod = ModuleManager::getModule<NoPacket>();
        static Freecam* freecam = ModuleManager::getModule<Freecam>();
        static KillauraReachY* kareach = ModuleManager::getModule<KillauraReachY>();
        static KillauraReachY2* kareach2 = ModuleManager::getModule<KillauraReachY2>();
        static AntiHit* antiHit = ModuleManager::getModule<AntiHit>();


        if(noPacketMod && noPacketMod->isEnabled() && GI::getLocalPlayer() != nullptr) {
            return;
        }

        if(kareach && kareach->isEnabled() && GI::getLocalPlayer() != nullptr) {
            return;
        }

        if(kareach2 && kareach2->isEnabled() && GI::getLocalPlayer() != nullptr) {
            return;
        }
        if(packet->getId() == PacketID::PlayerAuthInput) {
            if(antiHit && antiHit->isEnabled() && GI::getLocalPlayer() != nullptr) {
                return;
            }
        }

        if(packet->getId() == PacketID::PlayerAuthInput) {
            if(AntiHit::isTeleporting) {
                return;
            }
        }

         if(freecam && freecam->isEnabled() && GI::getLocalPlayer() != nullptr) {
            if(freecam->keyPressed) {
                ModuleManager::onSendPacket(packet);
                oFunc(_this, packet);
                return;
            } else {
                return;
            }
        }


        ModuleManager::onSendPacket(packet);

        oFunc(_this, packet);
    }

   public:
    SendPacketHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&SendPacketCallback;
    }
};