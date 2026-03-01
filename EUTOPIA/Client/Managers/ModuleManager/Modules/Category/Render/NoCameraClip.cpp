#include "NoCameraClip.h"

#include "..\SDK\World\Actor\Components\CameraComponent.h"

int cameramode = 0;

NoCameraClip::NoCameraClip() : Module("NoCameraClip", "Remove camera clip", Category::RENDER) {
    registerSetting(
        new EnumSetting("Mode", "Camera mode", {"Third person", "Unavaiable"}, &cameramode, 0));
}

void NoCameraClip::onEnable() {
    auto player = GI::getLocalPlayer();
    if(!player)
        return;

    auto ctx = player->getEntityContext();
    if(!ctx)
        return;

    auto& registry = ctx->enttRegistry;

    if(cameramode == 0) {
        auto view = registry.view<CameraThirdPersonComponent>();
        for(auto entity : view) {
            if(registry.all_of<CameraAvoidanceComponent>(entity))
                registry.remove<CameraAvoidanceComponent>(entity);
        }
    } else if(cameramode == 1) {
        auto view = registry.view<CameraComponent>();
        for(auto entity : view) {
            CameraComponent* cam = &registry.get<CameraComponent>(entity);
            if(!cam)
                continue;
            if(cam->getMode() == CameraMode::FirstPerson) {
                if(registry.all_of<CameraAvoidanceComponent>(entity))
                    registry.remove<CameraAvoidanceComponent>(entity);
                auto offsetView = registry.try_get<CameraOffsetComponent>(entity);
                if(offsetView)
                    offsetView->mPosOffset = Vec3<float>{0.f, 0.f, 0.f};
            }
        }
    }
}

void NoCameraClip::onDisable() {
    auto player = GI::getLocalPlayer();
    if(!player)
        return;

    auto ctx = player->getEntityContext();
    if(!ctx)
        return;

    auto& registry = ctx->enttRegistry;

    if(cameramode == 0) {
        auto view = registry.view<CameraThirdPersonComponent>();
        for(auto entity : view) {
            if(!registry.all_of<CameraAvoidanceComponent>(entity))
                registry.emplace<CameraAvoidanceComponent>(entity);
        }
    } else if(cameramode == 1) {
        auto view = registry.view<CameraComponent>();
        for(auto entity : view) {
            CameraComponent* cam = &registry.get<CameraComponent>(entity);
            if(!cam)
                continue;
            if(cam->getMode() == CameraMode::FirstPerson) {
                if(!registry.all_of<CameraAvoidanceComponent>(entity))
                    registry.emplace<CameraAvoidanceComponent>(entity);
            }
        }
    }
}
