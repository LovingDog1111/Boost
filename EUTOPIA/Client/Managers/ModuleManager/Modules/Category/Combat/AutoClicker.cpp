#include "AutoClicker.h"

#include <Windows.h>

#include <random>

#include "..\Client\Managers\ModuleManager\ModuleManager.h"

int minCPS = 8;
int maxCPS = 12;
bool rightclick = false;
bool holdclick;

int getRandomTicksFromCPS(int a, int b) {
    if(a > b)
        std::swap(a, b);
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(a, b);
    int cps = dist(rng);
    return std::max(1, 20 / cps);
}

bool isLeftClickHeld() {
    return GetAsyncKeyState(VK_LBUTTON) & 0x8000;
}

bool isRightClickHeld() {
    return GetAsyncKeyState(VK_RBUTTON) & 0x8000;
}

Actor* getActorFromEntityId2(EntityId id) {
    auto player = GI::getLocalPlayer();
    for(auto a : player->level->getRuntimeActorList())
        if(a->getEntityContext()->entity.rawId == id.rawId)
            return a;
    return nullptr;
}

AutoClicker::AutoClicker()
    : Module("AutoClicker", "Automatically clicks for you", Category::COMBAT) {
    registerSetting(
        new BoolSetting("RightClick", "Allow the clicker to right click", &rightclick, false));
    registerSetting(new BoolSetting("Hold", "Click when holding", &holdclick, false));
    registerSetting(new SliderSetting<float>("Min CPS", "Minimum CPS", &minCPS, minCPS, 1, 20));
    registerSetting(new SliderSetting<float>("Max CPS", "Maximum CPS", &maxCPS, maxCPS, 1, 20));
}

void AutoClicker::onEnable() {}

void AutoClicker::onLevelRender() {
    auto player = GI::getLocalPlayer();
    if(!player)
        return;
    auto gm = player->getgamemode();
    if(!gm)
        return;
    if(GI::getClientInstance()->getScreenName() != "hud_screen")
        return;

    auto clickgui = ModuleManager::getModule<ClickGUI>();
    if(clickgui && clickgui->isEnabled() && GI::getLocalPlayer() != nullptr)
        return;

    static int tick = 0;
    static int wait = getRandomTicksFromCPS(minCPS, maxCPS);
    tick++;

    bool mk = GI::canUseMoveKeys();
    bool leftHeld = isLeftClickHeld();
    bool rightHeld = isRightClickHeld();

    if(holdclick) {
        if(!leftHeld && !(rightclick && rightHeld)) {
            tick = 0;
            return;
        }
    } else {
        if(!mk) {
            tick = 0;
            return;
        }
    }

    if(tick >= wait) {
        if(leftHeld || !holdclick) {
            player->swing();
            auto h = player->level->getHitResult();

            if(h->type == HitResultType::ENTITY) {
                Actor* a = getActorFromEntityId2(h->entity.id);
                if(a &&
                   a->getEntityContext()->entity.rawId != player->getEntityContext()->entity.rawId)
                    gm->attack(a);
            }
        }

        if(rightclick && rightHeld) {
            auto h = player->level->getHitResult();
            gm->buildBlock(h->blockPos, h->selectedFace, true);
        }

        tick = 0;
        wait = getRandomTicksFromCPS(minCPS, maxCPS);
    }
}
