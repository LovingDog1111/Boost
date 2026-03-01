#pragma once

#include "../../ModuleBase/Module.h"

class Spawn : public Module {
public:
    Spawn();
    ~Spawn();

    virtual void onEnable() override;
};
