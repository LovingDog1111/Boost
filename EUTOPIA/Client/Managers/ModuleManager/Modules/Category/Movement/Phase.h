#pragma once
#pragma once
#include "../../ModuleBase/Module.h"

class Phase : public Module {
private:
    bool getPlayerAABB(AABB*& aabb);  // 添加辅助函数声明
    
public:
    Phase();
    virtual void onDisable() override;
    virtual void onLevelTick(Level* level) override;
};
