#pragma once
#include <glm/glm.hpp>
#include <mutex>
#include <string>
#include <vector>
#include "..\Libs\glm\glm\vec3.hpp"
#include "..\Libs\glm\glm\vec2.hpp"
#include "../../ModuleBase/Module.h"


struct ItemInfo {
    Vec3<float> pos;  
    Vec2<float> size;  
    std::string name;
    std::vector<std::pair<std::string, int>> enchants;
};




class ItemESP : public Module {
   public:
    ItemESP();;
    void onLevelRender() override;
    void onNormalTick(LocalPlayer* player) override;

   private:
    bool showNames = true;
    float distance = 100.f;
    bool renderFilled = false;
    bool themedColor = true;
    bool showItemNames = true;
    bool distanceScaledFont = true;
    float fontSize = 20.f;
    float scalingMultiplier = 1.25f;
    bool highlightUsefulItems = true;
    bool showEnchant = true;

    std::vector<ItemInfo> items;
    std::mutex itemMutex;
};
