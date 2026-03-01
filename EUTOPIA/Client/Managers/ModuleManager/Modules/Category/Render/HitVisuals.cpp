#include "HitVisuals.h"

#include "../../../../../../SDK/Render/Matrix.h"
#include "../../../ModuleManager.h"
#include "../Client/ColorsMod.h"

int styletorender = 0;

HitVisuals::HitVisuals() : Module("HitVisuals", "Cool hit visuals", Category::RENDER) {
    registerSetting(new EnumSetting(
        "Style", "Choose style", {"Sphere", "Spiral", "PulsingRing", "Cross"}, &styletorender, 0));
    for(int i = 0; i < sphereCount; i++)
        offsets.push_back(i * 2.f * 3.1415926f / sphereCount);
}

void HitVisuals::onD2DRender() {
    LocalPlayer* lp = GI::getLocalPlayer();
    if(!lp)
        return;

    std::vector<Actor*> actors = ActorUtils::getActorList(true);
    if(actors.empty())
        return;

    Actor* closest = nullptr;
    float minDist = FLT_MAX;
    for(Actor* a : actors) {
        if(!a || a == lp)
            continue;
        float d = lp->getPos().dist(a->getPos());
        if(d < minDist) {
            closest = a;
            minDist = d;
        }
    }

    if(!closest)
        return;

    target = closest;
    int currentHurt = target->getHurtTime();
    if(currentHurt <= 0)
        return;

    if(currentHurt > 0 && lastHurtTime == 0)
        angle = 0.f;
    lastHurtTime = currentHurt;
    angle += 0.05f;

    Vec3<float> pos = target->getPos();
    pos.y += 0.75f;
    UIColor mainColor = ModuleManager::getModule<Colors>()->getColor();

    if(styletorender == 0) {
        for(int i = 0; i < sphereCount; i++) {
            float a = angle + offsets[i];
            float xOff = cos(a) * 0.8f;
            float zOff = sin(a) * 0.8f;
            float yOff = sin(angle + i) * 0.3f;
            Vec3<float> spherePos = pos + Vec3<float>(xOff, yOff, zOff);
            Vec2<float> screenPos;
            if(Matrix::WorldToScreen(spherePos, screenPos)) {
                float dist = lp->getPos().dist(spherePos);
                float scale = 1.f / std::max(dist * 0.2f, 0.5f);
                D2D::fillCircle(screenPos, mainColor, 4.f * scale);
                D2D::drawCircle(screenPos, mainColor, 6.f * scale, 2.f);
            }
        }
    } else if(styletorender == 1) {
        for(int i = 0; i < sphereCount; i++) {
            float a = angle * 2.f + offsets[i];
            float xOff = cos(a) * (0.5f + i * 0.05f);
            float zOff = sin(a) * (0.5f + i * 0.05f);
            float yOff = i * 0.1f;
            Vec3<float> spherePos = pos + Vec3<float>(xOff, yOff, zOff);
            Vec2<float> screenPos;
            if(Matrix::WorldToScreen(spherePos, screenPos)) {
                float dist = lp->getPos().dist(spherePos);
                float scale = 1.f / std::max(dist * 0.25f, 0.4f);
                D2D::fillCircle(screenPos, mainColor, 3.5f * scale);
            }
        }
    } else if(styletorender == 2) {
        float radius = 0.5f + sin(angle * 3.f) * 0.3f;
        for(int i = 0; i < sphereCount; i++) {
            float a = offsets[i];
            float xOff = cos(a) * radius;
            float zOff = sin(a) * radius;
            Vec3<float> spherePos = pos + Vec3<float>(xOff, 0.f, zOff);
            Vec2<float> screenPos;
            if(Matrix::WorldToScreen(spherePos, screenPos)) {
                float dist = lp->getPos().dist(spherePos);
                float scale = 1.f / std::max(dist * 0.2f, 0.5f);
                D2D::drawCircle(screenPos, mainColor, 5.f * scale, 2.f);
            }
        }
    } else if(styletorender == 3) {
        for(int i = 0; i < 4; i++) {
            float xOff = (i < 2 ? -0.5f : 0.5f);
            float zOff = (i % 2 == 0 ? -0.5f : 0.5f);
            Vec3<float> spherePos = pos + Vec3<float>(xOff, 0.f, zOff);
            Vec2<float> screenPos;
            if(Matrix::WorldToScreen(spherePos, screenPos)) {
                float dist = lp->getPos().dist(spherePos);
                float scale = 1.f / std::max(dist * 0.3f, 0.5f);
                D2D::fillCircle(screenPos, mainColor, 4.f * scale);
            }
        }
    }
}
