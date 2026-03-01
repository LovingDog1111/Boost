#include "Chat.h"


#include <winhttp.h>

#include <random>

#include <json.hpp>
#include "../../../../../../SDK/NetWork/Packets/TextPacket.h"
#pragma comment(lib, "winhttp.lib")

Chat::Chat() : Module("Chat", "Makes messages unique with clip formatting.", Category::CLIENT) {
    registerSetting(
        new SliderSetting<int>("Length", "Length of the clip string.", &clipValue, 5, 0, 16));
    registerSetting(new BoolSetting("Green", "Adds '>' prefix to messages.", &useSeparator, true));
    registerSetting(new BoolSetting("Suffix", "Toggle suffix usage.", &useSuffix, true));
    registerSetting(new BoolSetting("Anti Filter", "Prevents your curse words from being filtered.",
                                    &antiFilter, true));
}

void Chat::setCustomSuffix(const std::string& suffix) {
    suffixText = suffix;
}

void Chat::onSendPacket(Packet* packet) {
    if(packet->getId() == PacketID::Text) {
        auto* textPacket = reinterpret_cast<TextPacket*>(packet);
        if(textPacket->mType == TextPacketType::Chat && !textPacket->mMessage.empty()) {
            std::string message = textPacket->mMessage;

            if(antiFilter) {
                static const std::vector<std::string> curseWords = {
                    "fuck",   "shit", "bitch", "asshole", "bastard", "dick",    "cunt",  "nigger",
                    "faggot", "ass",  "damn",  "nigga",   "fag",     "shitter", "retard"};
                for(const std::string& word : curseWords) {
                    size_t pos = 0;
                    while((pos = message.find(word, pos)) != std::string::npos) {
                        std::string maskedWord = word;
                        size_t indexToMask = word.length() / 2;
                        maskedWord[indexToMask] = '*';
                        message.replace(pos, word.length(), maskedWord);
                        pos += maskedWord.length();
                    }
                }
            }

            std::string clip = generateClipString(clipValue);
            std::string finalMessage = message;

            if(useSeparator) {  // Changed from usePrefix to match the header
                finalMessage = "> " + finalMessage;
            }

            if(useSuffix) {
                finalMessage += " | " + suffixText;
            }

            finalMessage += " | " + clip;

            textPacket->mMessage = finalMessage;
            textPacket->mFilteredMessage = finalMessage;
            textPacket->mAuthor = "";
        }
    }
}

std::string Chat::generateClipString(int length) {
    static const char alphanum[] = "0123456789";
    std::string result;
    result.reserve(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    for(int i = 0; i < length; ++i) {
        result += alphanum[dis(gen)];
    }
    return result;
}
