#include "../../../ModuleManager.h"
#include "KillAura.h"
#include "../SDK/Render/MeshHelpers.h"
#include "../../../../../../Utils/Minecraft/InvUtil.h"
#include "../SDK/Network/Packets/InteractPacket.h"

KillAura::KillAura()
    : Module("KillAura", "Attacks nearby players automatically", Category::COMBAT),
      lastAttackTick(0),
      mode(AttackMode::Vanilla) {
    registerSetting(new SliderSetting<float>("Range", "Attack range", &range, 6.f, 1.f, 12.f));
    registerSetting(new SliderSetting<int>("HitAttempts", "Number of attack attempts per target",
                                           &hitAttempts, 1, 1, 5));
    registerSetting(new BoolSetting("Mobs", "Attack mobs too", &mobs, false));
    registerSetting(new SliderSetting<int>("Attack Delay", "Delay between attacks in ticks",
                                           &attackDelay, 5, 0, 20));
    registerSetting(
        new EnumSetting("Type", "Attack method", {"Vanilla", "IDPredict"}, (int*)&mode, 0));
    registerSetting(new EnumSetting("Weapon", "Auto switch to best weapon",
                                    {"None", "Switch", "Spoof"}, &autoWeaponMode, 0));
    registerSetting(new BoolSetting("EatStop", "StopOnEat", &eatStop, false));
    registerSetting(new BoolSetting("Render", "NULL", &render, false));
    registerSetting(new ColorSetting("Color", "Render Color", &renderColor, renderColor, true, true,
                                     [&]() -> bool { return render; }));
}

void KillAura::onDisable() {
    targetList.clear();
}

int KillAura::getBestWeaponSlot(Actor* target) {
    if(!target)
        return InvUtil::getSelectedSlot();

    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return InvUtil::getSelectedSlot();

    PlayerInventory* inv = InvUtil::getPlayerInventory();
    if(!inv)
        return InvUtil::getSelectedSlot();

    uint8_t oldSlot = InvUtil::getSelectedSlot();
    float bestDamage = 0.f;
    int bestSlot = oldSlot;

    for(int i = 0; i < 9; i++) {
        ItemStack* stack = InvUtil::getItemStack(i);
        if(!InvUtil::isVaildItem(stack))
            continue;

        InvUtil::switchTo(i);
        float damage = localPlayer->calculateDamage(target);

        if(damage > bestDamage) {
            bestDamage = damage;
            bestSlot = i;
        }
    }

    InvUtil::switchTo(oldSlot);
    return bestSlot;
}
std::vector<Actor*> KillAura::getTargets(LocalPlayer* lp) {
    std::vector<Actor*> targets;
    for(Actor* a : lp->level->getRuntimeActorList()) {
        if(!a || a == lp)
            continue;
        if(!TargetUtil::isTargetValid(a, mobs, range))
            continue;
        if(a->getActorTypeComponent()->id == 71)
            continue;
        if(lp->getPos().dist(a->getPos()) > range)
            continue;
        targets.push_back(a);
    }
    return targets;
}

void KillAura::attackTarget(LocalPlayer* lp, Actor* target) {
    if(!lp || !lp->gamemode || !target)
        return;

    if(mode == AttackMode::Vanilla) {
        lp->gamemode->attack(target);
        lp->swing();
    } else if(mode == AttackMode::IDPredict) {
        auto* sender = GI::getClientInstance()->getpacketSender();
        if(!sender)
            return;

        int runtimeId = target->getRuntimeIDComponent()->mRuntimeID;

        for(int i = 0; i < 3; i++) {
            auto pkt = MinecraftPacket::createPacket<InteractPacket>();
            pkt->mAction = InteractPacket::Action::LeftClick;
            pkt->mTargetId = runtimeId + i;
            pkt->mPos = target->getPos();

            sender->send(pkt.get());
            lp->swing();
        }
    }
}

void KillAura::onNormalTick(LocalPlayer* lp) {
    if(!lp || !lp->level || !lp->gamemode)
        return;

    targetList.clear();

    if(eatStop && lp->getItemUseDuration() > 0)
        return;

    std::vector<Actor*> targets = getTargets(lp);
    Actor* bestTarget = nullptr;

    if(!targets.empty()) {
        bestTarget = *std::min_element(targets.begin(), targets.end(), [lp](Actor* a, Actor* b) {
            if(!a || !b)
                return false;
            return lp->getPos().dist(a->getPos()) < lp->getPos().dist(b->getPos());
        });
    }

    if(bestTarget)
        targetList.push_back(bestTarget);

    if(++lastAttackTick < attackDelay)
        return;

    lastAttackTick = 0;

    if(bestTarget) {
        uint8_t oldSlot = InvUtil::getSelectedSlot();
        int weaponSlot = getBestWeaponSlot(bestTarget);

        if(weaponSlot < 0 || weaponSlot > 8)
            weaponSlot = oldSlot;

        if(autoWeaponMode != 0) {
            if(InvUtil::getSelectedSlot() != weaponSlot)
                InvUtil::switchTo(weaponSlot);

            if(autoWeaponMode == 2)
                InvUtil::sendMobEquipment(static_cast<uint8_t>(weaponSlot));
        }

        for(int i = 0; i < hitAttempts; i++) {
            attackTarget(lp, bestTarget);
        }

        if(autoWeaponMode == 2) {
            if(InvUtil::getSelectedSlot() != oldSlot)
                InvUtil::switchTo(oldSlot);
        }
    }

    /*
    auto autoCrystalMod = ModuleManager::getModule<AutoCrystal>();
    if(autoCrystalMod && autoCrystalMod->isEnabled() && !autoCrystalMod->targetList.empty()) {
        auto actors = lp->level->getRuntimeActorList();
        for(Actor* actor : actors) {
            if(!actor)
                continue;

            auto* typeComp = actor->getActorTypeComponent();
            if(!typeComp || typeComp->id != 71)
                continue;

            if(lp->getPos().dist(actor->getPos()) > 5.f)
                continue;

            attackTarget(lp, actor);
        }
    }
    */
}

void KillAura::onLevelRender() {
    if(!render)
        return;
    if(targetList.empty())
        return;

    Vec3<float> origin =
        GI::getClientInstance()->getLevelRenderer()->getrenderplayer()->getorigin();

    Actor* target = targetList[0];
    AABBShapeComponent* targetAABBShape = target->getAABBShapeComponent();
    Vec3<float> targetPos = target->getEyePos();

    if(target->getActorTypeComponent()->id != 319)
        targetPos.y += targetAABBShape->mHeight;

    static float anim = 0.f;
    anim += DrawUtil::deltaTime * 45.f;
    const float coolAnim = 1.f + 1.f * sin((anim / 60.f) * PI);

    targetPos.y -= 1.6f;
    targetPos.y += coolAnim;

    Vec3<float> posList[360];
    for(int i = 0; i < 360; i++) {
        float calcYaw = (i + 90.f) * (PI / 180.f);
        Vec3<float> perm = Vec3<float>(cos(calcYaw) * 0.58f, 0.f, sin(calcYaw) * 0.58f);
        posList[i] = targetPos.add(perm).sub(origin);
    }

    DrawUtil::setColor(UIColor(renderColor.r, renderColor.g, renderColor.b, 5));
    DrawUtil::tessellator->begin(VertextFormat::QUAD);

    static float oldCoolAnim = 1.f;
    oldCoolAnim = Math::lerp(oldCoolAnim, coolAnim, DrawUtil::deltaTime * 5.f);
    float oldY = targetPos.y - coolAnim + oldCoolAnim;

    int quatily = 15;
    int times = 0;
    for(float yAdd = targetPos.y; yAdd != oldY;) {
        for(int i = 0; i < 360; i++) {
            Vec3<float> p1 = posList[i];
            Vec3<float> p2 =
                posList[(i + 1) % 360];  // loop back to first point when reaching end of list
            Vec3<float> p3 = Vec3<float>(p2.x, yAdd - origin.y, p2.z);  // bottom point of cylinder
            Vec3<float> p4 = Vec3<float>(p1.x, yAdd - origin.y, p1.z);  // bottom point of cylinder

            // Front side
            DrawUtil::tessellator->vertex(p1.x, p1.y, p1.z);
            DrawUtil::tessellator->vertex(p2.x, p2.y, p2.z);
            DrawUtil::tessellator->vertex(p3.x, p3.y, p3.z);
            DrawUtil::tessellator->vertex(p4.x, p4.y, p4.z);

            // Back side
            DrawUtil::tessellator->vertex(p4.x, p4.y, p4.z);
            DrawUtil::tessellator->vertex(p3.x, p3.y, p3.z);
            DrawUtil::tessellator->vertex(p2.x, p2.y, p2.z);
            DrawUtil::tessellator->vertex(p1.x, p1.y, p1.z);
        }

        yAdd = Math::lerp(yAdd, oldY, 0.08f);
        if(times >= quatily)
            break;

        times++;
    }

    MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, DrawUtil::tessellator,
                                       DrawUtil::blendMaterial);

    DrawUtil::setColor(renderColor);
    DrawUtil::tessellator->begin(VertextFormat::LINE_LIST);

    for(int i = 0; i < 360; i++) {
        Vec3<float> p1 = posList[i];
        Vec3<float> p2 =
            posList[(i + 1) % 360];  // loop back to first point when reaching end of list

        DrawUtil::tessellator->vertex(p1.x, p1.y, p1.z);
        DrawUtil::tessellator->vertex(p2.x, p2.y, p2.z);
    }

    MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, DrawUtil::tessellator,
                                       DrawUtil::blendMaterial);
}

std::string KillAura::getModeText() {
    switch(mode) {
        case AttackMode::Vanilla:
            return "Vanilla";
        case AttackMode::IDPredict:
            return "IDPredict";
        default:
            return "";
    }
}