#pragma once

#include "../../ModuleBase/Module.h"

class Hub : public Module {
public:
    Hub();
    ~Hub();

    virtual void onEnable() override;
};
