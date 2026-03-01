#include "FindPlayer.h"

#include <algorithm>

#include "../../Client/Managers/ModuleManager/ModuleManager.h"
#include "../../Utils/Minecraft/TargetUtil.h"

FindPlayer::FindPlayer()
    : CommandBase("FindPlayer", "Select a player entity by name.", "<name>", {}) {}

std::string stripFormatting(const std::string& name) {
    std::string out;
    for(size_t i = 0; i < name.size(); ++i) {
        if(name[i] == '§' && i + 1 < name.size()) {
            ++i;
            continue;
        }
        out += name[i];
    }
    return out;
}

std::vector<std::string> FindPlayer::getCurrentPlayerList2() {
    std::vector<std::string> playerList;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    Level* level = localPlayer ? localPlayer->level : nullptr;
    if(level) {
        for(auto& entity : level->getRuntimeActorList()) {
            if(!TargetUtil::isTargetValid(entity, false))
                continue;
            playerList.push_back(stripFormatting(entity->getNameTag()));
        }
    }
    return playerList;
}

bool FindPlayer::execute(const std::vector<std::string>& args) {
    if(args.size() < 2) {
        Client::DisplayClientMessage("Usage: .FindPlayer <name>", MCTF::RED);
        return false;
    }

    std::string inputName = args[1];
    auto toLower = [](const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(), ::tolower);
        return out;
    };

    std::vector<std::string> playerList = getCurrentPlayerList2();
    std::string correctedName;
    for(const auto& name : playerList) {
        if(toLower(name).find(toLower(inputName)) != std::string::npos) {
            correctedName = name;
            break;
        }
    }

    if(correctedName.empty()) {
        Client::DisplayClientMessage("Target not in range or not found", MCTF::RED);
        return false;
    }

    LocalPlayer* player = Game.getLocalPlayer();
    if(!player || !player->level)
        return false;

    Actor* found = nullptr;
    std::vector<Actor*> actors = ActorUtils::getActorList(false);
    for(auto* entity : actors) {
        std::string name = stripFormatting(entity->getNameTag());
        if(name == correctedName) {
            found = entity;
            break;
        }
    }

    if(!found) {
        Client::DisplayClientMessage("Player entity not found.", MCTF::RED);
        return false;
    }

    auto tpMod = ModuleManager::getModule<PlayerTP>();
    if(!tpMod) {
        Client::DisplayClientMessage("PlayerTP module not found.", MCTF::RED);
        return false;
    }

    tpMod->setTarget(found);
    std::string msg = "[FindPlayer] Target set to: " + stripFormatting(found->getNameTag());
    Client::DisplayClientMessage(msg.c_str(), MCTF::GREEN);


    return true;
}
