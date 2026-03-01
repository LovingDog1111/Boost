#include "Tracer.h"

#include "../SDK/Render/MCR.h"
#include "../SDK/Render/MeshHelpers.h"
#include "../Utils/DrawUtil.h"
#include "../Utils/Minecraft/TargetUtil.h"

Tracer::Tracer() : Module("Tracer", "Draws lines to ESP highlighted entities.", Category::RENDER) {
    registerSetting(new ColorSetting("Color", "NULL", &color, UIColor(255, 255, 255, 175)));
    registerSetting(new BoolSetting("Mobs", "NULL", &mobs, false));
}

void Tracer::onLevelRender() {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    if(!localPlayer)
        return;

    Tessellator* tessellator = DrawUtil::tessellator;
    if(!tessellator)
        return;

    float yaw = localPlayer->getActorRotationComponent()->mYaw;
    float pitch = localPlayer->getActorRotationComponent()->mPitch;

    float calcYaw = (yaw + 90.f) * (PI / 180.f);
    float calcPitch = (pitch) * -(PI / 180.f);

    Vec3<float> moveVec;
    moveVec.x = cos(calcYaw) * cos(calcPitch) * 0.2f;
    moveVec.y = sin(calcPitch) * 0.2f;
    moveVec.z = sin(calcYaw) * cos(calcPitch) * 0.2f;

    Vec3<float> origin = Game.getClientInstance()->getLevelRenderer()->renderplayer->origin;
    Vec3<float> start = moveVec;

    DrawUtil::setColor(color);

    for(auto& actor : localPlayer->level->getRuntimeActorList()) {
        if(TargetUtil::isTargetValid(actor, mobs)) {
            Vec3<float> end = actor->getEyePos().sub(origin);
            tessellator->begin(VertextFormat::LINE_LIST, 2);
            tessellator->vertex(start.x, start.y, start.z);
            tessellator->vertex(end.x, end.y, end.z);
            MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, tessellator,
                                               DrawUtil::blendMaterial);
        }
    }
}
