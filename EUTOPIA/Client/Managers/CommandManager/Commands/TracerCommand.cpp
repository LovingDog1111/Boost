#include "TracerCommand.h"
#include "CommandBase.h"
#include "../../../Client.h"
#include "../../Client/Managers/ModuleManager/ModuleManager.h"

TracerCommand::TracerCommand()
    : CommandBase("tracer", "Set BlockTracer target block", "<block>", {}) {}

bool TracerCommand::execute(const std::vector<std::string>& args) {
    if(args.size() < 2) {
        Client::DisplayClientMessage("Usage: .tracer <blockId|name>", MCTF::RED);
        return false;
    }

    std::string input = args[1];
    int blockId = -1;

 
    try {
        blockId = std::stoi(input);
    } catch(...) {
        
        static std::unordered_map<std::string, int> blockNameToId = {
            {"hopper", 145},        {"enchanting_table", 116}, {"chest", 54},  {"bookshelf", 47},
            {"trapped_chest", 146}, {"ender_chest", 130},      {"anvil", 149}, {"furnace", 117}};

        auto it = blockNameToId.find(input);
        if(it != blockNameToId.end()) {
            blockId = it->second;
        }
    }

    if(blockId == -1) {
        Client::DisplayClientMessage("Invalid block ID or name.", MCTF::RED);
        return false;
    }

    auto blockTracer = ModuleManager::getModule<BlockTracer>();
    if(!blockTracer) {
        Client::DisplayClientMessage("BlockTracer module not found.", MCTF::RED);
        return false;
    }

    blockTracer->setTargetBlock(blockId);

    std::string confirmMsg = "BlockTracer target set to ID: " + std::to_string(blockId);
    Client::DisplayClientMessage(confirmMsg.c_str(), MCTF::GREEN);
    return true;
}
