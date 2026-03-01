#include "Velocity.h"
#include "../../../../../../SDK/NetWork/Packets/SetActorMotionPacket.h"

Velocity::Velocity() : Module("Velocity", "Prevent you from knockback", Category::MOVEMENT) {
    registerSetting(new SliderSetting<float>("Horizontal", "NULL", &horizontal, horizontal, 0.f, 100.f));
    registerSetting(new SliderSetting<float>("Vertical", "NULL", &vertical, vertical, 0.f, 100.f));
}


void Velocity::onReceivePacket(Packet* packet, bool* cancel) {
    if (!packet) return;
    
    auto ci = GI::getClientInstance();
    if (!ci || !ci->getLocalPlayer()) return;
    
    auto runtimecomponent = ci->getLocalPlayer()->getRuntimeIDComponent();
    if (!runtimecomponent) return;

    if (packet->getId() == PacketID::SetActorMotion) {
        auto setactormotion = reinterpret_cast<SetActorMotionPacket*>(packet);
        if (!setactormotion) return;

        if (setactormotion->mRuntimeID == runtimecomponent->mRuntimeID) {
            if (horizontal == 0.f && vertical == 0.f) {
                Vec3<float> newMotion = setactormotion->mMotion;
                newMotion.x *= 0;
                newMotion.y *= 0;
                newMotion.z *= 0;
                setactormotion->mMotion = newMotion;
            } else {
                Vec3<float> newMotion = setactormotion->mMotion;
                newMotion.x *= (horizontal / 100.f);
                newMotion.y *= (vertical / 100.f);
                newMotion.z *= (horizontal / 100.f);
                setactormotion->mMotion = newMotion;
            }
        }
    }
}

std::string Velocity::getModeText() {
    static char textStr[15];
    sprintf_s(textStr, 15, "H%i%%V%i%%", (int)(std::round(horizontal)), (int)(std::round(vertical)));
    return std::string(textStr);
}