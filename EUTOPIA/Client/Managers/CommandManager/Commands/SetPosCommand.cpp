#include "SetPosCommand.h"

#include "../../Client/Managers/ModuleManager/ModuleManager.h"

SetPosCommand::SetPosCommand()
    : CommandBase("setPos", "Teleport to a target using SMTeleport.", "<x> <y> <z>", {}) {}

bool SetPosCommand::execute(const std::vector<std::string>& args) {
    if(args.size() < 4) {
        Client::DisplayClientMessage("Usage: .setPos <x> <y> <z>", MCTF::RED);
        return false;
    }

    try {
        float x = std::stof(args[1]);
        float y = std::stof(args[2]);
        float z = std::stof(args[3]);

        auto smTeleport = ModuleManager::getModule<SMTeleport>();
        if(!smTeleport) {
            Client::DisplayClientMessage("SMTeleport module not found.", MCTF::RED);
            return false;
        }

        smTeleport->setTargetPos(Vec3<float>(x, y, z));

        std::string confirmMsg = "Teleport target set to: X" + std::to_string(x) + " Y" +
                                 std::to_string(y) + " Z" + std::to_string(z);
        Client::DisplayClientMessage(confirmMsg.c_str(), MCTF::GREEN);
        return true;
    } catch(...) {
        Client::DisplayClientMessage("Invalid coordinates. Please enter valid numbers.", MCTF::RED);
        return false;
    }
}
