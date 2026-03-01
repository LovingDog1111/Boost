#pragma once

#include <map>
#include <string>
#include <vector>

#include "../../../Client.h"
#include "CommandBase.h"

class AssistantCommand : public CommandBase {
   public:
    AssistantCommand();
    bool execute(const std::vector<std::string>& args) override;

   private:
    std::string askAI(const std::string& prompt, int userId);
    std::string queryAI(const std::string& prompt);
    std::string processAIResponse(const std::string& prompt);
    bool detectAction(const std::string& prompt, std::string& action, std::string& moduleName);
    std::string getNextApiKey();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static inline std::map<int, std::vector<std::string>> conversationHistory;
    static inline std::vector<std::string> apiKeys = {"AIzaSyAu1KVtFJRXDBF8SJOnhzzxT4dk0-ApSGI",
                                                      "AIzaSyCA-4d5jOJDJFUuhvShRd-Ptckbcr4p8IM"};
    inline static int currentApiKeyIndex = 0;
};
