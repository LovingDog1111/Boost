#include "PlayerInfo.h"

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

#include "../Utils/NetworkUtil.h"
#include "json.hpp"

using json = nlohmann::json;

PlayerInfo::PlayerInfo() : Module("PlayerInfo", "Middle-click player info", Category::PLAYER) {}

Actor* getActorById(LocalPlayer* lp, EntityId id) {
    if(!lp || !lp->level)
        return nullptr;
    for(auto a : ActorUtils::getActorList(true))
        if(a->getEntityContext()->entity.rawId == id.rawId)
            return a;
    return nullptr;
}

std::vector<std::string> boostList;
std::chrono::steady_clock::time_point lastFetch = std::chrono::steady_clock::now();

void updateBoostList() {
    std::string cont;
    if(NetworkUtil::httpGet("http://43.226.0.155:5000/get_users", cont) && !cont.empty()) {
        try {
            json j = json::parse(cont);
            if(j.is_array()) {
                boostList.clear();
                for(auto& u : j)
                    if(u.is_string())
                        boostList.push_back(u.get<std::string>());
            }
        } catch(...) {
        }
    }
}

void PlayerInfo::onNormalTick(LocalPlayer* lp) {
    if(!lp || !lp->level)
        return;

    auto now = std::chrono::steady_clock::now();
    if(std::chrono::duration_cast<std::chrono::seconds>(now - lastFetch).count() > 30) {
        updateBoostList();
        lastFetch = now;
    }

    static bool wasDown = false;
    bool pressed = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;

    if(pressed && !wasDown) {
        HitResult* hr = lp->level->getHitResult();
        if(hr && hr->type == HitResultType::ENTITY) {
            Actor* tgt = getActorById(lp, hr->entity.id);
            if(tgt && tgt != lp) {
                std::string name = tgt->getNameTag();
                if(!name.empty()) {
                    float hp = tgt->getHealth();
                    float maxHp = tgt->getMaxHealth();
                    int hunger = 0;
                    if(tgt->isPlayer()) {
                        auto hAttr = tgt->getAttribute(AttributeHashes::HUNGER);
                        if(hAttr)
                            hunger = static_cast<int>(hAttr->mCurrentValue);
                    }
                    Vec3<float> pos = tgt->getPos();
                    bool isBoost =
                        std::find(boostList.begin(), boostList.end(), name) != boostList.end();

                    GI::DisplayClientMessage(
                        "%sName: %s%s | %sHP: %s%d/%d | %sHunger: %s%d | %sPos: %s%.1f %.1f %.1f | "
                        "%sBoost: %s%s",
                        MCTF::GRAY, MCTF::WHITE, name.c_str(), MCTF::GRAY, MCTF::GREEN, (int)hp,
                        (int)maxHp, MCTF::GRAY, MCTF::YELLOW, hunger, MCTF::GRAY, MCTF::AQUA, pos.x,
                        pos.y, pos.z, MCTF::GRAY, isBoost ? MCTF::RED : MCTF::GREEN,
                        isBoost ? "Yes" : "No");
                }
            }
        }
    }

    wasDown = pressed;
}
