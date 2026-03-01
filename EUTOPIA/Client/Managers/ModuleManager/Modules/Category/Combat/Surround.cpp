#include "Surround.h"

#include <DrawUtil.h>
#include <Minecraft/InvUtil.h>
#include <Minecraft/PlayerUtil.h>
#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

Surround::Surround()
    : Module("FeetTrap", "Surrounds you in blocks to prevent massive crystal damage",
             Category::COMBAT) {
    registerSetting(new SliderSetting<int>(
        "Place Delay", "Delay, in ticks, between block placements", &placeDelay, 1, 0, 20));
    registerSetting(new SliderSetting<int>("Blocks Per Tick", "How many blocks to place each tick",
                                           &placePerTick, 1, 1, 10));

    registerSetting(new BoolSetting("Air Place", "Place on air\n(Not recommanded to enable this!)",
                                    &airPlace, false));
    registerSetting(new BoolSetting(
        "Packet Place",
        "Send packet to server to place\n(Placements can be slower but can prevents ghost blocks)",
        &packetPlace, false));
    registerSetting(new BoolSetting("Rotate", "Rotates to placement", &rotate, true));
    registerSetting(
        new BoolSetting("Center", "Teleports you to the center of the block", &center, true));
    registerSetting(new BoolSetting("Dynamic",
                                    "Normally surround only place 1x1 around you\nThis setting "
                                    "will make surround auto place fit your hitbox",
                                    &dynamic, true));
    registerSetting(new BoolSetting(
        "Nukkit AABB", "Nukkit aabb is fatter so this will make the dynamic mode adjust for it",
        &nukkitAABB, true));
    registerSetting(new BoolSetting("Auto Disable", "Auto disable module after finish surround",
                                    &disableOnComplete, false));
    registerSetting(
        new BoolSetting("Jump Disable", "Auto disable module when jump", &disableOnJump, true));
    registerSetting(new BoolSetting("No MultiTask", "Stop when eating", &eatStop, false));
    registerSetting(new BoolSetting("Crystal Check", "Attacks end crystals", &crystalCheck, true));
    registerSetting(new BoolSetting("Crystal Blocker", "NULL", &crystalBlocker, false));
    registerSetting(new BoolSetting("Button", "Places button below", &button, true));
    registerSetting(new BoolSetting("E-Chest", "Places echest below", &echest, true));

    registerSetting(new EnumSetting("Switch", "Switch to obsidian",
                                    {"None", "Switch", "SwitchBack", "Spoof", "Packet"},
                                    &switchMode, 2));
    registerSetting(new BoolSetting(
        "Render", "Renders a block overlay where the obsidian will be placed", &render, true));
    registerSetting(new ColorSetting("Color", "NULL", &color, UIColor(125, 0, 255), false));
    registerSetting(new SliderSetting<int>("Alpha", "NULL", &alpha, 40, 0, 255));
    registerSetting(new SliderSetting<int>("LineAlpha", "NULL", &lineAlpha, 90, 0, 255));
    registerSetting(new BoolSetting("Fade", "Fade animation for render", &fade, false));
}

void Surround::onDisable() {
    placeList.clear();
    fadeBlockList.clear();
    shouldSurround = false;
    if(oldSlot != -1) {
        LocalPlayer* localPlayer = GI::getLocalPlayer();
        if(localPlayer != nullptr) {
            localPlayer->supplies->mSelectedSlot = oldSlot;
        }
        oldSlot = -1;
    }
    wasSurrounding = false;
}

void Surround::onEnable() {
    placeList.clear();
    shouldSurround = false;
    oldSlot = -1;
    wasSurrounding = false;

    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(localPlayer == nullptr) {
        return;
    }

    Vec3<float> playerPos = localPlayer->getPos();

    if(center) {
        Vec3<float> centerPos = Vec3<float>(floor(playerPos.x), playerPos.y, floor(playerPos.z));
        centerPos.x += 0.5f;
        centerPos.z += 0.5f;
        localPlayer->SetPosition(Game.getLocalPlayer()->getStateVectorComponent(), centerPos);
    }
    shouldSurround = true;
}

void Surround::onNormalTick(LocalPlayer* localPlayer) {
    if(localPlayer == nullptr) {
        return;
    }

    // 缓存实体列表，避免重复获取
    static std::vector<Actor*> entityList;
    static std::vector<Actor*> crystalList;
    entityList.clear();
    crystalList.clear();

    entityList.push_back(localPlayer);

    // 一次性遍历所有实体，分类处理
    for(Actor* actor : localPlayer->level->getRuntimeActorList()) {
        if(actor->getActorTypeComponent()->id == ActorType::EnderCrystal) {
            crystalList.push_back(actor);
            continue;
        }
        if(TargetUtil::isTargetValid(actor, true, false)) {
            entityList.push_back(actor);
        }
    }

    if(eatStop && localPlayer->getItemUseDuration() > 0) {
        return;
    }

    if(!shouldSurround) {
        return;
    }

    if(disableOnJump && GI::canUseMoveKeys() && GI::isKeyDown(VK_SPACE)) {
        this->setEnabled(false);
        return;
    }

    BlockSource* region = GI::getRegion();
    if(region == nullptr) {
        return;
    }

    int obsidianSlot = InvUtil::findItemSlotInHotbar(49);
    if(obsidianSlot == -1) {
        this->setEnabled(false);
        return;
    }

    if(!placeList.empty()) {
        placeList.clear();
    } else {
        if(wasSurrounding) {
            wasSurrounding = false;
            if(disableOnComplete) {
                this->setEnabled(false);
                return;
            }
        }
    }

    if(entityList.empty()) {
        return;
    }

    // 优化：使用缓存的水晶列表
    if(crystalCheck) {
        Vec3<float> playerEyePos = localPlayer->getEyePos();
        for(Actor* crystal : crystalList) {
            if(crystal->getEyePos().dist(playerEyePos) <= 4.f) {
                localPlayer->gamemode->attack(crystal);
            }
        }
    }

    AABB localAABB = localPlayer->getAABBShapeComponent()->getAABB();
    Vec3<float> feetPos = localAABB.getCenter();
    feetPos.y = localAABB.lower.y;

    BlockPos currentPos = feetPos.add(Vec3<float>(0.f, 0.5f, 0.f)).floor().CastTo<int>();

    int xStart = -1;
    int zStart = -1;
    int xEnd = 1;
    int zEnd = 1;

    if(dynamic) {
        // 预计算边界检查位置的AABB，避免重复计算
        AABB xStartAABB = getBlockAABB(currentPos.add(BlockPos(xStart, 0, 0)));
        AABB zStartAABB = getBlockAABB(currentPos.add(BlockPos(0, 0, zStart)));
        AABB xEndAABB = getBlockAABB(currentPos.add(BlockPos(xEnd, 0, 0)));
        AABB zEndAABB = getBlockAABB(currentPos.add(BlockPos(0, 0, zEnd)));

        for(Actor* actor : entityList) {
            AABB actorAABB = actor->getAABBShapeComponent()->getAABB();

            if(actor->getActorTypeComponent()->id == ActorType::Player) {
                actorAABB = TargetUtil::makeAABB(0.6f, actor->getAABBShapeComponent()->mHeight,
                                                 actor->getEyePos());
                if(nukkitAABB) {
                    actorAABB.lower = actorAABB.lower.sub(Vec3<float>(0.1f, 0, 0.1f));
                    actorAABB.upper = actorAABB.upper.add(Vec3<float>(0.1f, 0.f, 0.1f));
                }
            }

            if(actorAABB.intersects(xStartAABB)) {
                xStart -= 1;
                xStartAABB = getBlockAABB(currentPos.add(BlockPos(xStart, 0, 0)));
            }
            if(actorAABB.intersects(zStartAABB)) {
                zStart -= 1;
                zStartAABB = getBlockAABB(currentPos.add(BlockPos(0, 0, zStart)));
            }
            if(actorAABB.intersects(xEndAABB)) {
                xEnd += 1;
                xEndAABB = getBlockAABB(currentPos.add(BlockPos(xEnd, 0, 0)));
            }
            if(actorAABB.intersects(zEndAABB)) {
                zEnd += 1;
                zEndAABB = getBlockAABB(currentPos.add(BlockPos(0, 0, zEnd)));
            }
        }
    }

    // 预分配placeList容量，避免动态扩容
    placeList.reserve((xEnd - xStart + 1) * (zEnd - zStart + 1));

    for(int x = xStart; x <= xEnd; x++) {
        for(int z = zStart; z <= zEnd; z++) {
            // Skip corners
            if((x == xStart || x == xEnd) && (z == zStart || z == zEnd)) {
                continue;
            }

            // Handle interior positions (place below)
            if(x > xStart && x < xEnd && z > zStart && z < zEnd) {
                const BlockPos& placePos = currentPos.add(BlockPos(x, -1, z));
                if(WorldUtil::isValidPlacePos(placePos)) {
                    placeList.push_back(placePos);
                }
                continue;
            }

            // Handle perimeter positions
            const BlockPos& placePos = currentPos.add(BlockPos(x, 0, z));

            if(WorldUtil::isValidPlacePos(placePos)) {
                placeList.push_back(placePos);
            } else {
                const BlockPos& blockBelow = BlockPos(placePos.x, placePos.y - 1, placePos.z);
                if(WorldUtil::isValidPlacePos(blockBelow)) {
                    placeList.push_back(blockBelow);
                }
            }
        }
    }

    oldSlot = InvUtil::getSelectedSlot();

    if(echest) {
        const BlockPos& chestPos =
            localPlayer->getPos().floor().sub(Vec3<float>(0.f, 2.f, 0.f)).CastTo<int>();
        uint8_t chestSlot = InvUtil::findItemSlotInHotbar(143);

        if(switchMode != 0) {
            if(switchMode != 4)
                InvUtil::switchSlot(chestSlot);
            if(switchMode == 4 && PlayerUtil::selectedSlotServerSide != chestSlot)
                InvUtil::switchSlot(chestSlot);
        }
        WorldUtil::placeBlock(chestPos, -1);
    }

    if(!placeList.empty()) {
        // 优化：只在需要时排序，并使用更高效的排序方式
        if(placeList.size() > 1) {
            Vec3<float> playerFeetPos = feetPos;
            std::sort(placeList.begin(), placeList.end(),
                      [playerFeetPos](const BlockPos& a1, const BlockPos& a2) {
                          return (WorldUtil::distanceToBlock(playerFeetPos, a1) <
                                  WorldUtil::distanceToBlock(playerFeetPos, a2));
                      });
        }

        if(switchMode != 0) {
            if(switchMode != 4) {
                InvUtil::switchSlot(obsidianSlot);
            }
            if(switchMode == 4 && PlayerUtil::selectedSlotServerSide != obsidianSlot) {
                InvUtil::switchSlot(obsidianSlot);
            }
        }

        if(InvUtil::getSelectedSlot() == obsidianSlot || switchMode == 4) {
            static int oTick = 0;
            if(oTick >= placeDelay) {
                int placed = 0;

                if(crystalBlocker) {
                    for(Actor* crystal : crystalList) {
                        for(BlockPos& blockPos : placeList) {
                            Vec3<float> crystalPos = crystal->getEyePos();
                            if(crystalPos == blockPos.CastTo<float>() ||
                               crystal->getPos() == blockPos.CastTo<float>()) {
                                placeList.push_back(crystalPos.CastTo<int>());
                                localPlayer->gamemode->attack(crystal);
                            }
                        }
                    }
                }

                for(BlockPos& blockPos : placeList) {
                    if(WorldUtil::isValidPlacePos(blockPos)) {
                        WorldUtil::placeBlock(blockPos, -1);
                        if(fade)
                            fadeBlockList[blockPos] = 1.f;
                        placed++;
                    }
                    if(placed >= placePerTick) {
                        break;
                    }
                }
                oTick = 0;
            } else {
                oTick++;
            }
        }
        wasSurrounding = true;
    }

    if(button) {
        const BlockPos& buttonPos =
            localPlayer->getPos().floor().sub(Vec3<float>(0.f, 1.f, 0.f)).CastTo<int>();
        uint8_t buttonSlot = InvUtil::findItemSlotInHotbar(143);

        if(switchMode != 0) {
            if(switchMode != 4)
                InvUtil::switchSlot(buttonSlot);
            if(switchMode == 4 && PlayerUtil::selectedSlotServerSide != buttonSlot)
                InvUtil::switchSlot(buttonSlot);
        }
        WorldUtil::placeBlock(buttonPos, -1);
    }

    if(switchMode == 3 || switchMode == 4) {
        if(switchMode == 3) {
            InvUtil::switchSlot(oldSlot);
        } else {
            InvUtil::switchSlot(oldSlot);
        }
    }
}

void Surround::onLevelRender() {
    if(!render)
        return;

    for(BlockPos& blockPos : placeList) {
        DrawUtil::drawBox3dFilled(getBlockAABB(blockPos), UIColor(color.r, color.g, color.b, alpha),
                                  UIColor(color.r, color.g, color.b, lineAlpha));
    }

    for(auto it = fadeBlockList.begin(); it != fadeBlockList.end();) {
        if(it->second > 0.f) {
            DrawUtil::drawBox3dFilled(
                getBlockAABB(it->first),
                UIColor(color.r, color.g, color.b, (int)(alpha * it->second)),
                UIColor(color.r, color.g, color.b, (int)(lineAlpha * it->second)));
            it->second -= DrawUtil::deltaTime * 1.5f;
            it++;
        } else {
            it = fadeBlockList.erase(it);
        }
    }
}

void Surround::onSendPacket(Packet* packet, bool& cancel) {
    if(!rotate)
        return;

    LocalPlayer* lp = GI::getLocalPlayer();
    if(lp == nullptr) {
        return;
    }

    if(packet->getId() == PacketID::PlayerAuthInput) {
        PlayerAuthInputPacket* authPkt = (PlayerAuthInputPacket*)packet;

        for(const BlockPos& blockPos : placeList) {
            if(WorldUtil::canBuildOn(blockPos)) {
                const Vec2<float>& angle = lp->getEyePos().CalcAngle(blockPos.CastTo<float>());
                authPkt->mRot = angle;
                authPkt->mYHeadRot = angle.y;
                break;
            }
        }
    }
}