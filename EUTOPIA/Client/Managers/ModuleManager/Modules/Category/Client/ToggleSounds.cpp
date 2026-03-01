#include "ToggleSounds.h"
#include <iostream>
#include "../../../ModuleManager.h"
#include "AudioData.h"

ToggleSounds::ToggleSounds()
    : Module("ToggleSounds", "Play sounds when toggling modules", Category::CLIENT) {
    registerSetting(
        new EnumSetting("Mode", "Which sound set to use", {"Celestial"}, &soundMode, 0));
    std::cout << "ToggleSounds constructor called" << std::endl;
}

void ToggleSounds::onEnable() {
    std::cout << "ToggleSounds enabled" << std::endl;
    testAudioPlayback();
    moduleStates.clear();
    for(auto mod : ModuleManager::moduleList) {
        if(mod != this) {
            moduleStates[mod->getModuleName()] = mod->isEnabled();
            std::cout << "Tracking module: " << mod->getModuleName()
                      << " (Enabled: " << mod->isEnabled() << ")" << std::endl;
        }
    }
}

void ToggleSounds::onDisable() {
    std::cout << "ToggleSounds disabled" << std::endl;
    moduleStates.clear();
}

void ToggleSounds::onNormalTick(LocalPlayer* localPlayer) {
    if(!isEnabled() || localPlayer == nullptr)
        return;

    for(auto mod : ModuleManager::moduleList) {
        if(mod == this)
            continue;

        std::string moduleName = mod->getModuleName();
        bool currentState = mod->isEnabled();

        if(moduleStates.find(moduleName) != moduleStates.end()) {
            bool previousState = moduleStates[moduleName];

            if(previousState != currentState) {
                std::string soundFile = getSoundPath(currentState);
                std::cout << "Module " << moduleName << " toggled " << (currentState ? "ON" : "OFF")
                          << ", playing: " << soundFile << std::endl;

                uint32_t dataSize = 0;
                const uint8_t* soundData = AudioData::getSoundData(soundFile, dataSize);

                if(soundData && dataSize > 0) {
                    std::cout << "Playing from hex data, size: " << dataSize << " bytes"
                              << std::endl;
                    int result = audioPlayer.PlayFromMemory(soundData, dataSize, 1.0f, false);
                    if(result != 0) {
                        std::cout << "Hex audio play failed: " << result
                                  << ", trying file playback..." << std::endl;
                        audioPlayer.Play(soundFile, 1.0f, false);
                    }
                } else {
                    std::cout << "No hex data for " << soundFile << ", using file playback..."
                              << std::endl;
                    audioPlayer.Play(soundFile, 1.0f, false);
                }
            }
        }

        moduleStates[moduleName] = currentState;
    }
}

std::string ToggleSounds::getSoundPath(bool enabled) {
    return enabled ? "celestial.wav" : "celestial_off.wav";
}

void ToggleSounds::testAudioPlayback() {
    std::cout << "Testing hex audio playback..." << std::endl;

    uint32_t dataSize = 0;
    const uint8_t* soundData = AudioData::getSoundData("celestial.wav", dataSize);

    if(soundData && dataSize > 0) {
        std::cout << "Found celestial.wav hex data, size: " << dataSize << " bytes" << std::endl;
        int result = audioPlayer.PlayFromMemory(soundData, dataSize, 1.0f, false);
        std::cout << "Hex audio test result: " << result << std::endl;

        if(result != 0) {
            std::cout << "Falling back to file playback test..." << std::endl;
            audioPlayer.Play("celestial.wav", 1.0f, false);
        }
    } else {
        std::cout << "No hex data available, testing file playback..." << std::endl;
        audioPlayer.Play("celestial.wav", 1.0f, false);
    }
}
