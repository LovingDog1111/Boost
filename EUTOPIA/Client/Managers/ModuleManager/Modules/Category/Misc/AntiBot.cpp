#include "AntiBot.h"



AntiBot::AntiBot() : Module("AntiBot", "Filters out bots from the entity list", Category::MISC) {
    registerSetting(new EnumSetting("Mode", "AntiBot mode", {"Simple", "Custom"}, &Mode, 0));
    registerSetting(new EnumSetting("Entitylist Mode", "How to access entity list",
                                    {"EnttView", "Level"}, &EntitylistMode, 0));
    registerSetting(new BoolSetting("Hitbox Check", "Check player hitbox", &HitboxCheck, true));
    registerSetting(
        new BoolSetting("Player Check", "Check if actor is a player", &PlayerCheck, true));
    registerSetting(
        new BoolSetting("Invisible Check", "Check if actor is invisible", &InvisibleCheck, true));
    registerSetting(
        new BoolSetting("Name Check", "Check if actor name has newline", &NameCheck, true));
    registerSetting(
        new BoolSetting("PlayerList Check", "Check against playerlist", &PlayerListCheck, true));
    registerSetting(
        new BoolSetting("Has Armor Check", "Check if actor has armor", &HasArmorCheck, false));
    registerSetting(new EnumSetting("Armor Mode", "How to check armor", {"Full", "One Element"},
                                    &ArmorMode, 0));
}

void AntiBot::onEnable() {
    Game.DisplayClientMessage("[AntiBot] Enabled");
}

void AntiBot::onDisable() {
    Game.DisplayClientMessage("[AntiBot] Disabled");
}


void AntiBot::onNormalTick(LocalPlayer* localPlayer) {
    if(Mode == 0) {
        HitboxCheck = true;
        PlayerCheck = true;
        InvisibleCheck = true;
        NameCheck = true;
    }
}

constexpr float NORMAL_PLAYER_HEIGHT_MAX = 1.81f;
constexpr float NORMAL_PLAYER_HEIGHT_MIN = 1.35f;
constexpr float NORMAL_PLAYER_WIDTH_MIN = 0.54f;
constexpr float NORMAL_PLAYER_WIDTH_MAX = 0.66f;

std::vector<std::string> AntiBot::getPlayerNames() {
    auto player = Game.getLocalPlayer();
    std::vector<std::string> playerNames;
    if(!player)
        return playerNames;

    auto playerList = GI::getLevel()->getPlayerList();
    for(const auto& entry : *playerList | std::views::values) {
        playerNames.emplace_back(entry.name);
    }

    return playerNames;
}

bool AntiBot::isBot(Actor* actor) {
    if(!this->isEnabled())
        return false;
    if(PlayerCheck && !actor->isPlayer())
        return true;

    auto aabbShapeComponent = actor->getAABBShapeComponent();
    if(!aabbShapeComponent)
        return true;

    float hitboxWidth = aabbShapeComponent->mWidth;
    float hitboxHeight = aabbShapeComponent->mHeight;

    if(HitboxCheck &&
       (hitboxWidth < NORMAL_PLAYER_WIDTH_MIN || hitboxWidth > NORMAL_PLAYER_WIDTH_MAX ||
        hitboxHeight < NORMAL_PLAYER_HEIGHT_MIN || hitboxHeight > NORMAL_PLAYER_HEIGHT_MAX))
        return true;

    if(InvisibleCheck && actor->getStatusFlag(ActorFlags::Invisible))
        return true;

    if(NameCheck) {
        std::string nameTagString = actor->getNameTag();
        if(std::ranges::count(nameTagString, '\n') > 0)
            return true;
    }

    if(PlayerListCheck) {
        auto playerList = getPlayerNames();
        std::string nickName = actor->getNameTag();
        if(std::find(playerList.begin(), playerList.end(), nickName) != playerList.end())
            return true;
    }

    if(HasArmorCheck && !hasArmor(actor))
        return true;

    return false;
}

bool AntiBot::hasArmor(Actor* actor) {
    auto player = GI::getLocalPlayer();
    if(!player)
        return false;
    if(!actor->isPlayer())
        return false;

    ItemStack* helmetItem = actor->getArmorContainer()->getItem(0);
    ItemStack* chestplateItem = actor->getArmorContainer()->getItem(1);
    ItemStack* legginsItem = actor->getArmorContainer()->getItem(2);
    ItemStack* bootsItem = actor->getArmorContainer()->getItem(3);

    if(ArmorMode == 0)
        return helmetItem->mItem && chestplateItem->mItem && legginsItem->mItem && bootsItem->mItem;
    else
        return helmetItem->mItem || chestplateItem->mItem || legginsItem->mItem || bootsItem->mItem;
}
