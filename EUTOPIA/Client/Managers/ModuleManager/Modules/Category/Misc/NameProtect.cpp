#pragma once
#include "NameProtect.h"

#include <regex>

#include "..\SDK\NetWork\Packets\TextPacket.h"

NameProtect::NameProtect() : Module("NameProtect", "Hide your real nametag", Category::MISC) {
    newName = "Boost"; 
}

void NameProtect::onEnable() {
    auto player = GI::getLocalPlayer();
    if(!player)
        return;

    oldNametag = player->getNameTag();
}

void NameProtect::onDisable() {
    auto player = GI::getLocalPlayer();
    if(!player)
        return;

    player->setNametag(oldNametag);
}

void NameProtect::onNormalTick(LocalPlayer* player) {
    auto playerInstance = GI::getLocalPlayer();
    if(!playerInstance)
        return;

    playerInstance->setNametag(newName);
}

void NameProtect::onSendPacket(Packet* packet) {
    if(!packet || packet->getId() != PacketID::Text)
        return;



    auto* textPacket = reinterpret_cast<TextPacket*>(packet);
    if(!textPacket)
        return;

    std::string& message = textPacket->mMessage;

    if(message.find(oldNametag) != std::string::npos) {
        std::regex oldNameRegex(oldNametag);
        message = std::regex_replace(message, oldNameRegex, newName);
    }
}
