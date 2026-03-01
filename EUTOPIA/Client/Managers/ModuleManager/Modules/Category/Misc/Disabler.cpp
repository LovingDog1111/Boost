#include "Disabler.h"

#include <../SDK/NetWork/Packets/PlayerAuthInputPacket.h>
#include <Windows.h>

#include "..\SDK\NetWork\Packets\NetworkStackLatencyPacket.h"
#include "..\Utils\PlusUtils.h"

Disabler::Disabler() : Module("Disabler", "Disable the anticheat", Category::MISC) {
    registerSetting(new EnumSetting("Server", "Change server",
                                    {"LbSML", "LbSM", "LbSky", "LbTest", "Sentinel"}, &Mode, 0));
    registerSetting(
        new SliderSetting<int>("TargetPing", "Target Ping (ms)", &targetPing, 100, 1, 3500));
    registerSetting(new SliderSetting<float>("JitterRange", "Jitter Range (ms)", &jitterRange, 10.f,
                                             1.0f, 100.0f));
    registerSetting(
        new BoolSetting("DebugMessages", "Enable debug messages", &debugMessages, false));
}

std::string Disabler::getModeText() {
    switch(Mode) {
        case 0:
            return "LbSML";
        case 1:
            return "LbSM";
        case 2:
            return "LbSky";
        case 3:
            return "LbTest";
        case 4:
            return "Sentinel";
        default:
            return "Unknown";
    }
}

static __int64 ms;
static DWORD lastMS = GetTickCount();
static __int64 timeMS = -1;

static DWORD getCurrentMs() {
    return GetTickCount64();
}

static __int64 getElapsedTime() {
    return getCurrentMs() - ms;
}

static void resetTime() {
    lastMS = getCurrentMs();
    timeMS = getCurrentMs();
}

static bool hasTimedElapsed(__int64 time, bool reset) {
    if(getCurrentMs() - lastMS > time) {
        if(reset)
            resetTime();
        return true;
    }
    return false;
}

void Disabler::onSendPacket(Packet* packet) {
    if(Mode == 0 &&
       (packet->getName() == "PlayerAuthInputPacket" || packet->getName() == "MovePlayerPacket")) {
        auto* paip = (PlayerAuthInputPacket*)packet;
        auto* mpp = (MovePlayerPacket*)packet;
        if(paip) {
            float perc = static_cast<float>(paip->mClientTick % 3) / 3.0f;
            float targetY = (perc < 0.5f) ? 0.02f : -0.02f;
            paip->mPos.y = Math::lerp(paip->mPos.y, paip->mPos.y + targetY, perc);
            paip->mMove.y = -(1.0f / 3.0f);
            if(paip->mClientTick % 3 == 0) {
                paip->mInputData |= static_cast<AuthInputAction>(
                    1ULL << static_cast<int>(PlayerAuthInputPacket::InputData::StartJumping));
            }
            paip->mInputData |= static_cast<AuthInputAction>(
                1ULL << static_cast<int>(PlayerAuthInputPacket::InputData::Jumping));
        }
        if(mpp) {
            float perc = static_cast<float>(mpp->mTick % 3) / 3.0f;
            float targetY = (perc < 0.5f) ? 0.02f : -0.02f;
            mpp->mPos.y = Math::lerp(mpp->mPos.y, mpp->mPos.y + targetY, perc);
            mpp->mOnGround = true;
        }
    }

    if(Mode == 1 &&
       (packet->getName() == "PlayerAuthInputPacket" || packet->getName() == "MovePlayerPacket")) {
        auto* paip = (PlayerAuthInputPacket*)packet;
        auto* mpp = (MovePlayerPacket*)packet;
        mpp->mTeleportTick = 0;
        mpp->mRuntimeId = 0;
        paip->TicksAlive = 0;
    }

    if(Mode == 2 &&
       (packet->getName() == "PlayerAuthInputPacket" || packet->getName() == "MovePlayerPacket")) {
        auto* paip = (PlayerAuthInputPacket*)packet;
        auto* mpp = (MovePlayerPacket*)packet;
        mpp->mTeleportTick = 0;
        mpp->mRuntimeId = 0;
        paip->TicksAlive = 0;
    }

    if(Mode == 3 &&
       (packet->getName() == "PlayerAuthInputPacket" || packet->getName() == "MovePlayerPacket")) {
        auto* paip = (PlayerAuthInputPacket*)packet;
        auto* mpp = (MovePlayerPacket*)packet;
        if(paip) {
            float phase = static_cast<float>(paip->mClientTick % 4) / 4.0f;
            float offset = (phase < 0.5f) ? 0.025f : -0.025f;
            paip->mPos.y += offset * phase;
            paip->mMove.y = -(1.0f / 4.0f);
            if(paip->TicksAlive % 4 == 0) {
                paip->mInputData |= static_cast<AuthInputAction>(
                    1ULL << static_cast<int>(PlayerAuthInputPacket::InputData::StartJumping));
            }
        }
        if(mpp) {
            float phase = static_cast<float>(mpp->mTick % 4) / 4.0f;
            float offset = (phase < 0.5f) ? 0.025f : -0.025f;
            mpp->mPos.y += offset * phase;
            mpp->mOnGround = true;
            mpp->mTeleportTick = (mpp->mTick % 10 == 0) ? 0 : 1;
            mpp->mRuntimeId = (mpp->mTick % 5 == 0) ? 0 : mpp->mRuntimeId;
        }
    }

    if(Mode == 4) {
        Mode == 0;
        GI::DisplayClientMessage("No");
        return;
        if(!sentinelChecked) {
            sentinelAllowed = PlusUtils::isPlusActive();
            sentinelChecked = true;
        }

        if(!sentinelAllowed) {
            GI::DisplayClientMessage(
                "Unauthorized HWID! This is a Boost+ feature. Please upgrade your plan to use it.");
            Mode = 0;
            this->setEnabled(false);
            return;
        }

        uint64_t now = GetTickCount64();

        static uint64_t lastDebugTime = 0;
        if(now - lastDebugTime > 5000) { 
            if(debugMessages) {
                GI::DisplayClientMessage("[DEBUG] Sentinel Mode Active");
            }
            lastDebugTime = now;  
        }

        auto sender = GI::getPacketSender();
        if(!sender)
            return;

        uint64_t baseDelay = static_cast<uint64_t>(targetPing);
        uint64_t targetSendTime = now + baseDelay;

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(-static_cast<int>(jitterRange),
                                            static_cast<int>(jitterRange));
        int jitter = dis(gen);

        uint64_t actualDelay = baseDelay + jitter;

        if(static_cast<int64_t>(actualDelay) < 0)
            actualDelay = 0;

        QueuedLatencyPacket queuedPacket;
        queuedPacket.originalTimestamp = now;
        queuedPacket.sendTime = targetSendTime + actualDelay;

        mPacketQueue.push_back(queuedPacket);

        if(debugMessages) {
            GI::DisplayClientMessage(
                ("[DEBUG] Sending Packet after delay: " + std::to_string(actualDelay) + "ms")
                    .c_str());
        }


        if(!mPacketQueue.empty()) {
            for(const auto& queuedPacket : mPacketQueue) {
                uint64_t packetReceivedTime = GetTickCount64();
                uint64_t delayTime = packetReceivedTime - queuedPacket.originalTimestamp;


                if(debugMessages) {
                    GI::DisplayClientMessage(
                        ("[DEBUG] Received Packet Info - Original Timestamp: " +
                         std::to_string(queuedPacket.originalTimestamp) +
                         ", Delay: " + std::to_string(delayTime) + "ms")
                            .c_str());
                }
            }
        }

        return;
    }
}

void Disabler::processLatencyQueue() {
    if(mPacketQueue.empty())
        return;

    auto sender = GI::getPacketSender();
    if(!sender)
        return;

    uint64_t now = GetTickCount64();
    std::vector<QueuedLatencyPacket> remainingPackets;

    for(const auto& queuedPacket : mPacketQueue) {
        if(now >= queuedPacket.sendTime) {
            auto responsePacket = MinecraftPacket::createPacket<NetworkStackLatencyPacket>();
            responsePacket->mCreateTime = queuedPacket.originalTimestamp;
            responsePacket->mFromServer = false;

            sender->sendToServer(responsePacket.get());
        } else {
            remainingPackets.push_back(queuedPacket);
        }
    }

    mPacketQueue = std::move(remainingPackets);
}

void Disabler::onNormalTick(LocalPlayer* player) {
    if(Mode == 4) {
        processLatencyQueue();
    }
}
