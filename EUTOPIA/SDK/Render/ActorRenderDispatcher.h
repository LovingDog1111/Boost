#pragma once
#include "../World/Actor/Actor.h"
#include "BaseActorRenderContext.h"
#include "../../Utils/Maths.h"

class ActorRenderDispatcher {
public:
    virtual void render(BaseActorRenderContext* entityRenderContext, Actor* entity, Vec3<float>* cameraTargetPos, Vec3<float>* pos, Vec2<float>* rot, bool ignoreLighting) = 0;
    
    virtual ~ActorRenderDispatcher() = default;
};