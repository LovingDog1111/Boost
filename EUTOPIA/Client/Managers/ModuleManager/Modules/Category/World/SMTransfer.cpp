#include "SMTransfer.h"

#include "../../SDK/GlobalInstance.h"
#include "../../SDK/NetWork/Packets/CommandRequestPacket.h"
int transfertarget = 0;

SMTransfer::SMTransfer()
    : Module("SMTransfer", "Teleport to the specified SM target", Category::WORLD) {
    registerSetting(new EnumSetting("Mode", "change mode", {"SML", "SM", "Random"}, &transfertarget, 0));
}

SMTransfer::~SMTransfer() {}

void sendTransferCommand3(int smIndex) {
    std::shared_ptr<Packet> packet = MinecraftPacket::createPacket(PacketID::CommandRequest);
    auto* pkt = reinterpret_cast<CommandRequestPacket*>(packet.get());

    std::ostringstream command;
    if(transfertarget == 0) {
        command << "/transfer sml" << smIndex;
    }

    if(transfertarget == 1) {
        command << "/transfer sm" << smIndex;
    } 
    if(transfertarget == 2) {  
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);  

        int randomMode = dis(gen);  


        int randomIndex =
            rand() % 100;  

        if(randomMode == 0) {  
            command << "/transfer sml" << randomIndex;
        } else {  
            command << "/transfer sm" << randomIndex;
        }
    }

    pkt->mCommand = command.str();
    pkt->mInternalSource = false;
    pkt->mOrigin.mType = CommandOriginType::Player;
    pkt->mOrigin.mPlayerId = 0;
    pkt->mOrigin.mRequestId = "0";
    pkt->mOrigin.mUuid = mce::UUID();

    auto client = GI::getClientInstance();
    if(!client || !client->packetSender)
        return;

    client->packetSender->sendToServer(pkt);
}

void SMTransfer::onEnable() {
    if(targetSMIndex != -1) {
        sendTransferCommand3(targetSMIndex);
        this->setEnabled(false);
    } else {
        auto player = Game.getLocalPlayer();
        if(player)
            player->displayClientMessage("No SM target set. Use .setSM to set your target.");
        this->setEnabled(false);
    }
}

void SMTransfer::setMainSMIndex(int smIndex) {
    targetSMIndex = smIndex;
    auto player = Game.getLocalPlayer();
    if(player) {
        std::ostringstream ss; // ?? wtf is this loool
        ss << "Main SM set to " << smIndex; 
        player->displayClientMessage(ss.str());
    }
}

void SMTransfer::setMainSMLIndex(int smIndex) {
    targetSMIndex = smIndex;
    auto player = Game.getLocalPlayer();
    if(player) {
        std::ostringstream ss;
        ss << "Main SML set to " << smIndex;
        player->displayClientMessage(ss.str());
    }
}
