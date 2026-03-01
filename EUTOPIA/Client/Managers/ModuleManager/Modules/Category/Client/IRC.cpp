#include "IRC.h"

IRC::IRC() : Module("IRC", "Chat with other players via remote server", Category::CLIENT) {
    registerSetting(new BoolSetting("Enable IRC", "Turn IRC chat on/off", &enabledByUser, true));
}

bool IRC::isIRCEnabled() const {
    return enabledByUser;
}
