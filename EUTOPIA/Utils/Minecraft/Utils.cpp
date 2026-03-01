#include "Utils.h"

#include "../../SDK/GlobalInstance.h"


void Utils::setTimer(float timer) {
	if(auto ci = GI::getClientInstance()) {
		if(auto mc = ci->minecraftSim) {
			mc->setSimTimer(timer);
			mc->setSimSpeed(timer / 20.f);
		}
	}
}

void Utils::resetTimer() {
    setTimer(20.f);
}

ActorType Utils::getMob(const std::string& name) {
    static const std::unordered_map<std::string, ActorType> mobMap = {
        {"Chicken", ActorType::Chicken},
        {"Cow", ActorType::Cow},
        {"Pig", ActorType::Pig},
        {"Sheep", ActorType::Sheep},
        {"Wolf", ActorType::Wolf},
        {"Villager", ActorType::Villager},
        {"MushroomCow", ActorType::MushroomCow},
        {"Squid", ActorType::Squid},
        {"Rabbit", ActorType::Rabbit},
        {"Bat", ActorType::Bat},
        {"IronGolem", ActorType::IronGolem},
        {"SnowGolem", ActorType::SnowGolem},
        {"Ocelot", ActorType::Ocelot},
        {"Horse", ActorType::Horse},
        {"Donkey", ActorType::Donkey},
        {"Mule", ActorType::Mule},
        {"SkeletonHorse", ActorType::SkeletonHorse},
        {"ZombieHorse", ActorType::ZombieHorse},
        {"PolarBear", ActorType::PolarBear},
        {"Llama", ActorType::Llama},
        {"Parrot", ActorType::Parrot},
        {"Dolphin", ActorType::Dolphin},
        {"Zombie", ActorType::Zombie},
        {"Creeper", ActorType::Creeper},
        {"Skeleton", ActorType::Skeleton},
        {"Player", ActorType::Player}};


    std::string key = name;
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    for(const auto& [mobName, type] : mobMap) {
        std::string mobLower = mobName;
        std::transform(mobLower.begin(), mobLower.end(), mobLower.begin(), ::tolower);
        if(mobLower == key) {
            return type;
        }
    }

    return ActorType::Undefined;  
}

void Utils::Sneak() {
    LocalPlayer* player = GI::getLocalPlayer();
    if(!player)
        return;
    player->setSneaking(true);
}
