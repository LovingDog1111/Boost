#include "AssistantCommand.h"

#include <algorithm>
#include <cctype>
#include <fstream>

#include "../../ModuleManager/ModuleManager.h"

AssistantCommand::AssistantCommand()
    : CommandBase("assistant", "Perform actions via AI assistant.", "<sentence>", {}) {}

std::string AssistantCommand::askAI(const std::string& prompt, int userId) {
    conversationHistory[userId].push_back("User: " + prompt);
    std::string reply = "AI response to: " + prompt;
    conversationHistory[userId].push_back("AI: " + reply);
    std::ofstream out("conversation_" + std::to_string(userId) + ".txt");
    for(const auto& line : conversationHistory[userId])
        out << line << "\n";
    return reply;
}

bool AssistantCommand::detectAction(const std::string& prompt, std::string& action,
                                    std::string& moduleName) {
    std::string lowerPrompt = prompt;
    std::transform(lowerPrompt.begin(), lowerPrompt.end(), lowerPrompt.begin(), ::tolower);

    if(lowerPrompt.find("toggle") != std::string::npos)
        action = "toggle";
    else if(lowerPrompt.find("enable") != std::string::npos)
        action = "enable";
    else if(lowerPrompt.find("disable") != std::string::npos)
        action = "disable";
    else
        return false;

    size_t pos = lowerPrompt.find(action) + action.length();
    std::string remaining = prompt.substr(pos);
    remaining.erase(
        std::remove_if(remaining.begin(), remaining.end(),
                       [](char c) { return c == '"' || c == '[' || c == ']' || c == ','; }),
        remaining.end());
    while(!remaining.empty() && std::isspace(remaining.front()))
        remaining.erase(remaining.begin());
    while(!remaining.empty() && std::isspace(remaining.back()))
        remaining.pop_back();

    std::string lowerRem = remaining;
    std::transform(lowerRem.begin(), lowerRem.end(), lowerRem.begin(), ::tolower);

    for(Module* mod : ModuleManager::moduleList) {
        std::string name = mod->getModuleName();
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        if(lowerRem.find(lowerName) != std::string::npos) {
            moduleName = name;
            return true;
        }
    }
    return false;
}

bool AssistantCommand::execute(const std::vector<std::string>& args) {
    if(args.size() < 2) {
        Client::DisplayClientMessage("Usage: .assistant <sentence>", MCTF::RED);
        return false;
    }

    std::string prompt;
    for(size_t i = 1; i < args.size(); ++i)
        prompt += args[i] + " ";

    std::string action, moduleName;
    bool hasAction = detectAction(prompt, action, moduleName);

    std::string aiReply = askAI(prompt, 0);

    if(hasAction) {
        Module* target = ModuleManager::getModuleByName(moduleName);
        if(target) {
            if(action == "toggle")
                target->toggle();
            else if(action == "enable")
                target->setEnabled(true);
            else if(action == "disable")
                target->setEnabled(false);
            aiReply = "Executed action " + action + " on module: " + target->getModuleName() + ".";
        } else {
            aiReply = "Module not found: " + moduleName + ".";
        }
    }

    Client::DisplayClientMessage(("AI: " + aiReply).c_str(), MCTF::GREEN);
    return true;
}
