#include "MidClick.h"

#include <FriendUtil.h>
#include <Windows.h>

MidClick::MidClick()
    : Module("MidClick", "Middle-click to friend/unfriend players", Category::PLAYER) {}

Actor* getActorFromEntityId(LocalPlayer* player, EntityId id) {
    if(!player || !player->level)
        return nullptr;
    for(auto actor : player->level->getRuntimeActorList()) {
        if(actor->getEntityContext()->entity.rawId == id.rawId)
            return actor;
    }
    return nullptr;
}

void MidClick::onNormalTick(LocalPlayer* player) {
    if(!player || !player->level)
        return;

    static bool wasDown = false;
    bool midClickPressed = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;

    if(midClickPressed && !wasDown) {
        HitResult* hit = player->level->getHitResult();
        if(!hit || hit->type != HitResultType::ENTITY)
            return;

        Actor* target = getActorFromEntityId(player, hit->entity.id);
        if(!target || target == player)
            return;

        std::string name = target->getNameTag();
        if(name.empty())
            return;

        if(FriendManager::isFriend(name)) {
            FriendManager::removeFriend(name);
            GI::DisplayClientMessage("%sRemoved %s%s%s from your friends list.", MCTF::RED,
                                     MCTF::WHITE, name.c_str(), MCTF::RED);
        } else {
            FriendManager::addFriend(name);
            GI::DisplayClientMessage("%sAdded %s%s%s to your friends list.", MCTF::GREEN,
                                     MCTF::WHITE, name.c_str(), MCTF::GREEN);
        }
    }

    wasDown = midClickPressed;
}