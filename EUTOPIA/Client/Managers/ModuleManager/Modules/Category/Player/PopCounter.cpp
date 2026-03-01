#include "PopCounter.h"

#include <AnimationUtil.h>
#include <ColorUtil.h>
#include <DrawUtil.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../../../../../SDK/GlobalInstance.h"
#include "../../../../../../SDK/NetWork/MinecraftPacket.h"
#include "../../../../../../SDK/NetWork/Packets/PacketID.h"
#include "../../../../../../SDK/NetWork/Packets/TextPacket.h"
#include "../../../../../../SDK/World/Item/Item.h"
#include "../../../../../../Utils/Minecraft/TargetUtil.h"
#include "../../../../../../Utils/Minecraft/WorldUtil.h"
#include "../../../../../Client.h"

using namespace std;

PopCounter::PopCounter() : Module("PopCounter", "PopCounter", Category::PLAYER) {
    registerSetting(new BoolSetting("SendChat", "NULL", &sendchat, false));
    registerSetting(new BoolSetting("PopCham", "NULL", &popcham, true));

    // 初始化随机数种子
    srand(static_cast<unsigned int>(time(nullptr)));
}

static std::vector<Actor*> playerlist;

void sendMessage1(const std::string& str) {
    auto packet = MinecraftPacket::createPacket<TextPacket>();
    auto* pkt = packet.get();

    pkt->mType = TextPacketType::Chat;
    pkt->mMessage = str;
    pkt->mPlatformId = "";
    pkt->mLocalize = false;
    pkt->mXuid = "";
    pkt->mAuthor = "";

    GI::getPacketSender()->sendToServer(pkt);
}

static std::unordered_map<std::string, int> popCounts;
static std::unordered_map<std::string, bool> lastHadTotem;

void PopCounter::onNormalTick(LocalPlayer* localPlayer) {
    playerlist.clear();
    Level* level = localPlayer->level;
    if(!level)
        return;
    if(localPlayer == nullptr)
        return;

    if(!localPlayer->isAlive()) {
        popCounts.clear();
        lastHadTotem.clear();
    }

    for(auto& entity : level->getRuntimeActorList()) {
        if(TargetUtil::isTargetValid(entity, false)) {
            float rangeCheck = 100;
            if(WorldUtil::distanceToEntity(localPlayer->getPos(), entity) <= rangeCheck) {
                playerlist.push_back(entity);
            }
        }
    }

    for(auto* player : playerlist) {
        std::string playerName = player->getNameTag().c_str();

        bool hasTotem = false;
        auto* offhand = player->getOffhandSlot();
        if(offhand) {
            Item* item = offhand->getItem();
            if(item && item->mItemId == 601) {
                hasTotem = true;
            }
        }

        if(lastHadTotem[playerName] && !hasTotem) {
            popCounts[playerName] += 1;
            Client::DisplayClientMessage("%s Popped %d Totems!", playerName.c_str(),
                                         popCounts[playerName]);
            if(sendchat) {
                std::string message = "> @" + playerName + " popped " +
                                      std::to_string(popCounts[playerName]) +
                                      " totems thanks to Boost!";
                sendMessage1(message);
            }

            // 添加爆图腾动画
            if(popcham) {
                Vec3<float> popPos = player->getPos().add(Vec3<float>(0, 0, 0));
                float pitch = player->getActorRotationComponent()->mPitch;
                float yaw = player->getActorRotationComponent()->mYaw;

                popAnimations.emplace_back(popPos, pitch, yaw);
            }
        }

        lastHadTotem[playerName] = hasTotem;
    }
}

void PopCounter::onLevelRender() {
    if(!popcham)
        return;
    if(!GI::isInHudScreen())
        return;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    float currentTime = GetTickCount64() / 1000.0f;

    // 更新和渲染所有爆图腾动画
    for(auto it = popAnimations.begin(); it != popAnimations.end();) {
        PopAnimationData& anim = *it;

        // 计算动画进度 (0.0 到 1.0)
        float elapsed = currentTime - anim.startTime;
        float progress = elapsed / anim.duration;

        if(progress >= 1.0f) {
            // 动画完成，移除
            it = popAnimations.erase(it);
            continue;
        }
        float spantime = GetTickCount64() / 500.0f;
        // 计算上升位置 - 直接使用线性插值实现缓慢上升
        float riseHeight = 3.0f;  // 上升5个方块高度
        float currentHeight = AnimationUtil::lerp(0.0f, riseHeight, progress);
        anim.currentPos = anim.startPos.add(Vec3<float>(0, currentHeight, 0));

        Vec3<float> finalPos = anim.currentPos;

        // 计算渐变透明度 (开始时较亮，结束时完全透明)
        float alphaProgress = 1.0f - progress;  // 反向进度，越到后面越透明

        // 应用透明度到颜色
        UIColor currentColor = anim.color;
        currentColor.a = (uint8_t)(anim.color.a * alphaProgress);

        // 渲染玩家模型 - 只渲染填充，不渲染轮廓
        DrawUtil::drawPlayerModel(finalPos, anim.pitch, anim.yaw, currentColor,
                                  UIColor(0, 0, 0, 0));  // 轮廓颜色设为完全透明
        DrawUtil::drawWings(finalPos.add(Vec3<float>(0,-0.6,0)), anim.yaw, 2.0f, spantime, currentColor,
                            UIColor(0, 0, 0, 0));

        ++it;
    }
}