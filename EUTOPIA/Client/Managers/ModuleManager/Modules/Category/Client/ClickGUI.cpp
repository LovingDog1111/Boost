#include "ClickGUI.h"

#include "../../../../../../Libs/json.hpp"
#include "../../../../../../Utils/TimerUtil.h"
#include "../../../ModuleManager.h"

ClickGUI::ClickGUI() : Module("ClickGUI", "Display all modules", Category::CLIENT, VK_INSERT) {
    registerSetting(new ColorSetting("Color", "NULL", &mainColor, mainColor));
    registerSetting(new SliderSetting<float>("Blur", "Background blur intensity", &blurStrength,
                                             4.f, 0.f, 20.f));
    registerSetting(new BoolSetting("Description", "Show Description", &showDescription, true));
}

ClickGUI::~ClickGUI() {
    for(auto& window : windowList) {
        delete window;
    }
    windowList.clear();
}

ClickGUI::ClickWindow::ClickWindow(std::string windowName, Vec2<float> startPos, Category c) {
    this->name = windowName;
    this->category = c;
    this->pos = startPos;
    this->extended = true;

    for(auto& mod : ModuleManager::moduleList) {
        if(mod->getCategory() == c) {
            this->moduleList.push_back(mod);
        }
    }

    std::sort(this->moduleList.begin(), this->moduleList.end(),
              [](Module* lhs, Module* rhs) { return lhs->getModuleName() < rhs->getModuleName(); });
}

void ClickGUI::onDisable() {
    GI::getClientInstance()->grabVMouse();

    isLeftClickDown = false;
    isRightClickDown = false;
    isHoldingLeftClick = false;
    isHoldingRightClick = false;

    draggingWindowPtr = nullptr;

    capturingKbSettingPtr = nullptr;
    draggingSliderSettingPtr = nullptr;
    capturingStringSettingPtr = nullptr;

    openAnim = 0.0f;
}

void ClickGUI::onEnable() {
    GI::getClientInstance()->releaseVMouse();
    openAnim = 1.0f;

    for(auto& window : windowList) {
        float totalTargetHeight =
            2.f * 1.f * ((float)ModuleManager::getModule<CustomFont>()->fontSize / 25.f);
        for(auto& mod : window->moduleList) {
            float modTargetHeight = D2D::getTextHeight("", 1.f) + (0.f * 2.f);
            if(mod->extended) {
                modTargetHeight +=
                    2.f * 1.f * ((float)ModuleManager::getModule<CustomFont>()->fontSize / 25.f);
                for(auto& setting : mod->getSettingList()) {
                    if(setting->dependOn.has_value() && !setting->dependOn.value()())
                        continue;
                    if(setting->type == SettingType::COLOR_S) {
                        ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
                        modTargetHeight += D2D::getTextHeight("", 1.f) + (0.f * 2.f);
                        if(colorSetting->extended) {
                            modTargetHeight +=
                                2.f * 1.f *
                                ((float)ModuleManager::getModule<CustomFont>()->fontSize / 25.f);
                            for(auto& slider : colorSetting->colorSliders) {
                                modTargetHeight +=
                                    D2D::getTextHeight("", 1.f) + (0.f * 2.f) +
                                    2.f * 1.f *
                                        ((float)ModuleManager::getModule<CustomFont>()->fontSize /
                                         25.f);
                            }
                            if(colorSetting->showSynced) {
                                modTargetHeight +=
                                    D2D::getTextHeight("", 1.f) + (0.f * 2.f) +
                                    2.f * 1.f *
                                        ((float)ModuleManager::getModule<CustomFont>()->fontSize /
                                         25.f);
                            }
                            modTargetHeight -=
                                2.f * 1.f *
                                ((float)ModuleManager::getModule<CustomFont>()->fontSize / 25.f);
                        }
                    } else if(setting->type == SettingType::VEC3_S) {
                        Vec3SettingBase* vecSetting = static_cast<Vec3SettingBase*>(setting);
                        modTargetHeight += D2D::getTextHeight("", 1.f) + (0.f * 2.f);
                        if(vecSetting->extended) {
                            modTargetHeight +=
                                2.f * 1.f *
                                ((float)ModuleManager::getModule<CustomFont>()->fontSize / 25.f);
                            for(int i = 0; i < 3; i++) {
                                modTargetHeight +=
                                    D2D::getTextHeight("", 1.f) + (0.f * 2.f) +
                                    2.f * 1.f *
                                        ((float)ModuleManager::getModule<CustomFont>()->fontSize /
                                         25.f);
                            }
                            modTargetHeight -=
                                2.f * 1.f *
                                ((float)ModuleManager::getModule<CustomFont>()->fontSize / 25.f);
                        }
                    } else {
                        modTargetHeight += D2D::getTextHeight("", 1.f) + (0.f * 2.f);
                    }
                    modTargetHeight +=
                        2.f * 1.f *
                        ((float)ModuleManager::getModule<CustomFont>()->fontSize / 25.f);
                }
            }
            modTargetHeight +=
                2.f * 1.f * ((float)ModuleManager::getModule<CustomFont>()->fontSize / 25.f);
            totalTargetHeight += modTargetHeight;

            mod->selectedAnim = 0.f;
            mod->currentTextColor =
                mod->isEnabled() ? UIColor(255, 255, 255) : UIColor(175, 175, 175);
            mod->enabledAnimWidth =
                mod->isEnabled()
                    ? 230.f * ((float)ModuleManager::getModule<CustomFont>()->fontSize / 25.f) +
                          (3.f * 1.f * 2.f) -
                          (3.5f * 1.f *
                           ((float)ModuleManager::getModule<CustomFont>()->fontSize / 25.f) * 2.f)
                    : 0.f;
        }
        window->currentHeight = window->extended ? totalTargetHeight : 0.f;
    }
}

bool ClickGUI::isVisible() {
    return false;
}

void ClickGUI::onKeyUpdate(int key, bool isDown) {
    if(!isEnabled()) {
        if(key == getKeybind() && isDown)
            setEnabled(true);
        return;
    }

    if(key == VK_SHIFT) {
        isHoldingShift = isDown;
        return;
    }

    if(!isDown)
        return;

    if(capturingStringSettingPtr != nullptr) {
        if(key == VK_RETURN || key == VK_ESCAPE) {
            capturingStringSettingPtr->isCapturing = false;
            capturingStringSettingPtr = nullptr;
            return;
        }

        if(key == VK_BACK) {
            if(!capturingStringSettingPtr->value->empty())
                capturingStringSettingPtr->value->pop_back();
            return;
        }

        std::string& str = *capturingStringSettingPtr->value;

        if(key >= 'A' && key <= 'Z') {
            char c = (char)key;
            if(!isHoldingShift)
                c = (char)tolower(c);
            str.push_back(c);
            return;
        }

        if(key >= '0' && key <= '9') {
            if(isHoldingShift) {
                switch(key) {
                    case '1':
                        str.push_back('!');
                        return;
                    default:
                        return;
                }
            }
            str.push_back((char)key);
            return;
        }

        BYTE keyboardState[256];
        GetKeyboardState(keyboardState);

        WCHAR unicodeBuffer[4];
        int result =
            ToUnicode(key, MapVirtualKey(key, MAPVK_VK_TO_VSC), keyboardState, unicodeBuffer, 4, 0);

        if(result > 0) {
            char utf8Buffer[8];
            int len = WideCharToMultiByte(CP_UTF8, 0, unicodeBuffer, result, utf8Buffer,
                                          sizeof(utf8Buffer), NULL, NULL);

            if(len > 0)
                str.append(utf8Buffer, len);
        }

        return;
    }

    if(key < 192 && capturingKbSettingPtr != nullptr) {
        if(key != VK_ESCAPE)
            *capturingKbSettingPtr->value = key;
        capturingKbSettingPtr = nullptr;
        return;
    }

    if(key == getKeybind() || key == VK_ESCAPE)
        setEnabled(false);
}

void ClickGUI::onMouseUpdate(Vec2<float> mousePosA, char mouseButton, char isDown) {
    // MouseButtons
    // 0 = mouse move
    // 1 = left click
    // 2 = right click
    // 3 = middle click
    // 4 = scroll   (isDown: 120 (SCROLL UP) and -120 (SCROLL DOWN))

    switch(mouseButton) {
        case 0:
            mousePos = mousePosA;
            break;
        case 1:
            isLeftClickDown = isDown;
            isHoldingLeftClick = isDown;
            break;
        case 2:
            isRightClickDown = isDown;
            isHoldingRightClick = isDown;
            break;
        case 4:
            float moveVec = (isDown < 0) ? -15.f : 15.f;
            for(auto& window : windowList) {
                if(window == draggingWindowPtr)
                    continue;

                if(!isHoldingShift)
                    window->pos.y += moveVec;
                else
                    window->pos.x += moveVec;
            }
            break;
    }

    if(draggingWindowPtr != nullptr) {
        if(!isHoldingLeftClick)
            draggingWindowPtr = nullptr;
    }

    if(capturingKbSettingPtr != nullptr) {
        if(isRightClickDown) {
            *capturingKbSettingPtr->value = 0;
            capturingKbSettingPtr = nullptr;
            isRightClickDown = false;
        }
    }

    if(draggingSliderSettingPtr != nullptr) {
        if(!isHoldingLeftClick)
            draggingSliderSettingPtr = nullptr;
    }
}

void ClickGUI::InitClickGUI() {
    setEnabled(false);

    Vec2<float> startPos = Vec2<float>(30.f, 55.f);

    windowList.push_back(new ClickWindow("Combat", startPos, Category::COMBAT));
    startPos.x += 230.f + 8.f;

    windowList.push_back(new ClickWindow("Movement", startPos, Category::MOVEMENT));
    startPos.x += 230.f + 8.f;

    windowList.push_back(new ClickWindow("Render", startPos, Category::RENDER));
    startPos.x += 230.f + 8.f;

    windowList.push_back(new ClickWindow("Player", startPos, Category::PLAYER));
    startPos.x += 230.f + 8.f;

    windowList.push_back(new ClickWindow("Client", startPos, Category::CLIENT));
    startPos.x += 230.f + 8.f;

    windowList.push_back(new ClickWindow("Misc", startPos, Category::MISC));
    startPos.x += 230.f + 8.f;

    windowList.push_back(new ClickWindow("World", startPos, Category::WORLD));

    initialized = true;
}

void ClickGUI::renderParticles() {}

void ClickGUI::Render() {
    if(!initialized)
        return;
    if(GI::canUseMoveKeys())
        GI::getClientInstance()->releaseVMouse();

    renderParticles();
    static Vec2<float> oldMousePos = mousePos;
    mouseDelta = mousePos.sub(oldMousePos);
    oldMousePos = mousePos;

    Vec2<float> screenSize = GI::getClientInstance()->guiData->windowSizeReal;
    float deltaTime = D2D::deltaTime;

    float textSize = 1.f;
    float textPaddingX = 3.f * textSize;
    float textPaddingY = 0.f * textSize;
    float textHeight = D2D::getTextHeight("", textSize);

    std::string descriptionText = "NULL";

    openAnim += deltaTime * 2.f;
    if(openAnim > 1.f)
        openAnim = 1.f;

    {
        if(blurStrength > 0.1f) {
            D2D::addBlur(Vec4<float>(0.f, 0.f, screenSize.x, screenSize.y),
                         blurStrength * openAnim);
        }

        float anim = openAnim;
        Vec2<float> center = Vec2<float>(screenSize.x * 0.5f, screenSize.y * 0.5f);
        float maxDim = std::max(screenSize.x, screenSize.y);

        D2D::fillCircleGradient(Vec2<float>(center.x - maxDim * 0.35f, center.y - maxDim * 0.15f),
                                maxDim * 0.45f, UIColor(140, 90, 255, (int)(123 * anim)),
                                UIColor(140, 90, 255, 0));

        D2D::fillCircleGradient(Vec2<float>(center.x + maxDim * 0.30f, center.y + maxDim * 0.20f),
                                maxDim * 0.40f, UIColor(90, 180, 255, (int)(121 * anim)),
                                UIColor(90, 180, 255, 0));

        D2D::fillCircleGradient(Vec2<float>(screenSize.x - maxDim * 0.15f, maxDim * 0.15f),
                                maxDim * 0.35f, UIColor(90, 180, 255, (int)(60 * anim)),
                                UIColor(90, 180, 255, 0));

        D2D::fillCircleGradient(Vec2<float>(center.x, center.y + maxDim * 0.35f), maxDim * 0.50f,
                                UIColor(255, 120, 200, (int)(131 * anim)),
                                UIColor(255, 120, 200, 0));

        D2D::fillRectangle(Vec4<float>(0.f, 0.f, screenSize.x, screenSize.y),
                           UIColor(0, 0, 0, (int)(110 * anim)));
    }

    for(auto& window : windowList) {
        if(window->pos.x > screenSize.x)
            window->pos.x = screenSize.x - 265.f;
        if(window == draggingWindowPtr)
            window->pos = window->pos.add(mouseDelta);

        static CustomFont* customFontMod = ModuleManager::getModule<CustomFont>();
        float fontPercent = ((float)customFontMod->fontSize / 25.f);

        Vec4<float> hRectPos =
            Vec4<float>(window->pos.x, window->pos.y,
                        window->pos.x + (int)(230.f * fontPercent) + (textPaddingX * 2.f),
                        window->pos.y + textHeight + (textPaddingY * 2.f));

        Vec2<float> hTextPos = Vec2<float>(hRectPos.x + textPaddingX, hRectPos.y + textPaddingY);

        if(hRectPos.contains(mousePos)) {
            if(isLeftClickDown) {
                draggingWindowPtr = window;
                isLeftClickDown = false;
            } else if(isRightClickDown) {
                window->extended = !window->extended;
                isRightClickDown = false;
            }
        }

        updateSelectedAnimRect(hRectPos, window->selectedAnim);

        D2D::fillRectangle(hRectPos, UIColor(0, 0, 0, 75));
        D2D::fillRectangle(hRectPos, mainColor);
        D2D::drawText(hTextPos, window->name, UIColor(255, 255, 255), textSize);
        D2D::fillRectangle(hRectPos, UIColor(255, 255, 255, (int)(25 * window->selectedAnim)));

        float moduleSpace = 2.f * textSize * fontPercent;
        float settingSpace = 2.f * textSize * fontPercent;
        float animSpeed = 8.f * deltaTime;

        // First pass: Calculate target heights for all modules
        float totalTargetHeight = moduleSpace;
        std::vector<float> moduleTargetHeights;
        moduleTargetHeights.reserve(window->moduleList.size());

        for(auto& mod : window->moduleList) {
            float modTargetHeight = textHeight + (textPaddingY * 2.f);
            float settingsTargetHeight = 0.f;

            if(mod->extended) {
                settingsTargetHeight += settingSpace;
                for(auto& setting : mod->getSettingList()) {
                    if(setting->dependOn.has_value() && !setting->dependOn.value()())
                        continue;
                    if(setting->type == SettingType::COLOR_S) {
                        ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
                        settingsTargetHeight += textHeight + (textPaddingY * 2.f);
                        if(colorSetting->extended) {
                            settingsTargetHeight += settingSpace;
                            for(auto& slider : colorSetting->colorSliders) {
                                settingsTargetHeight +=
                                    textHeight + (textPaddingY * 2.f) + settingSpace;
                            }
                            if(colorSetting->showSynced) {
                                settingsTargetHeight +=
                                    textHeight + (textPaddingY * 2.f) + settingSpace;
                            }
                            settingsTargetHeight -= settingSpace;
                        }
                    } else if(setting->type == SettingType::VEC3_S) {
                        Vec3SettingBase* vecSetting = static_cast<Vec3SettingBase*>(setting);
                        settingsTargetHeight += textHeight + (textPaddingY * 2.f);
                        if(vecSetting->extended) {
                            settingsTargetHeight += settingSpace;
                            for(int i = 0; i < 3; i++) {
                                settingsTargetHeight +=
                                    textHeight + (textPaddingY * 2.f) + settingSpace;
                            }
                            settingsTargetHeight -= settingSpace;
                        }
                    } else {
                        settingsTargetHeight += textHeight + (textPaddingY * 2.f);
                    }
                    settingsTargetHeight += settingSpace;
                }
            }

            modTargetHeight += settingsTargetHeight + moduleSpace;
            moduleTargetHeights.push_back(settingsTargetHeight);
            totalTargetHeight += modTargetHeight;
        }

        // Second pass: Apply same interpolation to all heights simultaneously
        float windowTargetHeight = window->extended ? totalTargetHeight : 0.f;
        window->currentHeight += (windowTargetHeight - window->currentHeight) * animSpeed;
        if(std::abs(windowTargetHeight - window->currentHeight) < 0.5f)
            window->currentHeight = windowTargetHeight;

        for(size_t i = 0; i < window->moduleList.size(); i++) {
            auto& mod = window->moduleList[i];
            float targetModHeight = mod->extended ? moduleTargetHeights[i] : 0.f;
            mod->currentHeight += (targetModHeight - mod->currentHeight) * animSpeed;
            if(std::abs(targetModHeight - mod->currentHeight) < 0.5f)
                mod->currentHeight = targetModHeight;
        }

        // Use actual current height for background, not target
        float bgHeight = window->extended ? window->currentHeight : 0.f;
        if(window->currentHeight > 0.1f) {
            float wbgPaddingX = 2.f * textSize * fontPercent;
            Vec4<float> clipRect =
                Vec4<float>(hRectPos.x + wbgPaddingX, hRectPos.w, hRectPos.z - wbgPaddingX,
                            hRectPos.w + window->currentHeight);
            D2D::PushAxisAlignedClip(clipRect, true);
            D2D::fillRectangle(clipRect, UIColor(0, 0, 0, 75));

            float yOffset = hRectPos.w + moduleSpace;
            for(auto& mod : window->moduleList) {
                float modPaddingX = wbgPaddingX + (2.f * textSize * fontPercent);
                Vec4<float> mRectPos =
                    Vec4<float>(hRectPos.x + modPaddingX, yOffset, hRectPos.z - modPaddingX,
                                yOffset + textHeight + (textPaddingY * 2.f));

                if(mRectPos.y > hRectPos.w + window->currentHeight) {
                    yOffset += textHeight + (textPaddingY * 2.f) + mod->currentHeight + moduleSpace;
                    continue;
                }

                Vec2<float> mTextPos =
                    Vec2<float>(mRectPos.x + textPaddingX, mRectPos.y + textPaddingY);

                if(mRectPos.contains(mousePos)) {
                    descriptionText = mod->getDescription();
                    if(isLeftClickDown) {
                        mod->toggle();
                        isLeftClickDown = false;
                    } else if(isRightClickDown) {
                        mod->extended = !mod->extended;
                        isRightClickDown = false;
                    }
                }

                updateSelectedAnimRect(mRectPos, mod->selectedAnim);

                float targetEnabledWidth = mod->isEnabled() ? (mRectPos.z - mRectPos.x) : 0.f;
                mod->enabledAnimWidth += (targetEnabledWidth - mod->enabledAnimWidth) * animSpeed;
                if(std::abs(targetEnabledWidth - mod->enabledAnimWidth) < 0.5f)
                    mod->enabledAnimWidth = targetEnabledWidth;

                UIColor targetColor =
                    mod->isEnabled() ? UIColor(255, 255, 255)
                                     : ColorUtil::lerp(UIColor(175, 175, 175),
                                                       UIColor(255, 255, 255), mod->selectedAnim);
                mod->currentTextColor =
                    ColorUtil::lerp(mod->currentTextColor, targetColor, animSpeed);

                if(mod->enabledAnimWidth > 0.1f) {
                    D2D::fillRectangle(Vec4<float>(mRectPos.x, mRectPos.y,
                                                   mRectPos.x + mod->enabledAnimWidth, mRectPos.w),
                                       mainColor);
                }
                D2D::fillRectangle(mRectPos, UIColor(0, 0, 0, 35));
                D2D::drawText(mTextPos, mod->getModuleName(), mod->currentTextColor, textSize);
                D2D::fillRectangle(mRectPos, UIColor(255, 255, 255, (int)(25 * mod->selectedAnim)));

                yOffset += textHeight + (textPaddingY * 2.f);

                if(mod->currentHeight > 0.1f) {
                    float settingPaddingX = 3.5f * textSize * fontPercent;
                    float settingPaddingZ = 3.5f * textSize * fontPercent;
                    Vec4<float> modClipRect = Vec4<float>(mRectPos.x, mRectPos.w, mRectPos.z,
                                                          mRectPos.w + mod->currentHeight);
                    D2D::PushAxisAlignedClip(modClipRect, true);

                    float settingYOffset = mRectPos.w + settingSpace;
                    for(auto& setting : mod->getSettingList()) {
                        if(setting->dependOn.has_value() && !setting->dependOn.value()())
                            continue;

                        std::string settingName = setting->name;
                        Vec4<float> sRectPos =
                            Vec4<float>(mRectPos.x + settingPaddingX, settingYOffset,
                                        mRectPos.z - settingPaddingZ,
                                        settingYOffset + textHeight + (textPaddingY * 2.f));

                        if(sRectPos.y > mRectPos.w + mod->currentHeight) {
                            settingYOffset += textHeight + (textPaddingY * 2.f);
                            if(setting->type == SettingType::COLOR_S &&
                               static_cast<ColorSetting*>(setting)->extended) {
                                ColorSetting* cs = static_cast<ColorSetting*>(setting);
                                settingYOffset += settingSpace;
                                for(auto& slider : cs->colorSliders)
                                    settingYOffset +=
                                        textHeight + (textPaddingY * 2.f) + settingSpace;
                                if(cs->showSynced)
                                    settingYOffset +=
                                        textHeight + (textPaddingY * 2.f) + settingSpace;
                                settingYOffset -= settingSpace;
                            } else if(setting->type == SettingType::VEC3_S &&
                                      static_cast<Vec3SettingBase*>(setting)->extended) {
                                settingYOffset +=
                                    settingSpace +
                                    3.f * (textHeight + (textPaddingY * 2.f) + settingSpace) -
                                    settingSpace;
                            }
                            settingYOffset += settingSpace;
                            continue;
                        }

                        Vec2<float> sTextPos =
                            Vec2<float>(sRectPos.x + textPaddingX, sRectPos.y + textPaddingY);

                        if(sRectPos.contains(mousePos))
                            descriptionText = setting->description;

                        updateSelectedAnimRect(sRectPos, setting->selectedAnim);

                        switch(setting->type) {
                            case SettingType::BOOL_S: {
                                BoolSetting* boolSetting = static_cast<BoolSetting*>(setting);
                                bool& boolValue = (*boolSetting->value);
                                if(sRectPos.contains(mousePos) && isLeftClickDown) {
                                    boolValue = !boolValue;
                                    isLeftClickDown = false;
                                }
                                if(boolValue)
                                    D2D::fillRectangle(sRectPos, mainColor);
                                D2D::drawText(sTextPos, settingName, UIColor(255, 255, 255),
                                              textSize);
                                settingYOffset += textHeight + (textPaddingY * 2.f);
                                break;
                            }
                            case SettingType::STRING_S: {
                                StringSetting* stringSetting = static_cast<StringSetting*>(setting);

                                if(sRectPos.contains(mousePos) && isLeftClickDown) {
                                    capturingStringSettingPtr = stringSetting;
                                    stringSetting->isCapturing = true;
                                    isLeftClickDown = false;
                                }

                                std::string displayText = stringSetting->isCapturing
                                                              ? (*stringSetting->value + "_")
                                                              : *stringSetting->value;

                                float rightBound = sRectPos.z - textPaddingX;
                                float leftBound = sRectPos.x + textPaddingX +
                                                  D2D::getTextWidth(settingName + ":", textSize) +
                                                  6.f;

                                float clipWidth = rightBound - leftBound;
                                float fullTextWidth = D2D::getTextWidth(displayText, textSize);

                                float drawX = rightBound - fullTextWidth;
                                Vec2<float> valueTextPos(drawX, sTextPos.y);

                                D2D::drawText(sTextPos, settingName + ":", UIColor(255, 255, 255),
                                              textSize);

                                D2D::PushAxisAlignedClip(
                                    Vec4<float>(leftBound, sRectPos.y, rightBound, sRectPos.w),
                                    false);
                                D2D::drawText(valueTextPos, displayText, UIColor(255, 255, 255),
                                              textSize);
                                D2D::PopAxisAlignedClip();

                                settingYOffset += textHeight + (textPaddingY * 2.f);
                                break;
                            }
                            case SettingType::KEYBIND_S: {
                                KeybindSetting* keybindSetting =
                                    static_cast<KeybindSetting*>(setting);
                                int& keyValue = (*keybindSetting->value);
                                if(sRectPos.contains(mousePos) && isLeftClickDown) {
                                    capturingKbSettingPtr =
                                        (capturingKbSettingPtr == keybindSetting) ? nullptr
                                                                                  : keybindSetting;
                                    isLeftClickDown = false;
                                }
                                std::string keybindName =
                                    (setting == capturingKbSettingPtr)
                                        ? "..."
                                        : ((keyValue != 0) ? KeyNames[keyValue] : "None");
                                Vec2<float> keybindTextPos =
                                    Vec2<float>(sRectPos.z - textPaddingX -
                                                    D2D::getTextWidth(keybindName, textSize),
                                                sTextPos.y);
                                D2D::drawText(sTextPos, settingName + ":", UIColor(255, 255, 255),
                                              textSize);
                                D2D::drawText(keybindTextPos, keybindName, UIColor(255, 255, 255),
                                              textSize);
                                settingYOffset += textHeight + (textPaddingY * 2.f);
                                break;
                            }
                            case SettingType::ENUM_S: {
                                EnumSetting* enumSetting = static_cast<EnumSetting*>(setting);
                                int& enumValue = (*enumSetting->value);
                                if(sRectPos.contains(mousePos)) {
                                    if(isLeftClickDown) {
                                        enumValue = (enumValue + 1) % enumSetting->enumList.size();
                                        isLeftClickDown = false;
                                    } else if(isRightClickDown) {
                                        enumValue = (enumValue - 1 + enumSetting->enumList.size()) %
                                                    enumSetting->enumList.size();
                                        isRightClickDown = false;
                                    }
                                }
                                std::string modeName = enumSetting->enumList[enumValue];
                                Vec2<float> modeTextPos =
                                    Vec2<float>(sRectPos.z - textPaddingX -
                                                    D2D::getTextWidth(modeName, textSize),
                                                sTextPos.y);
                                D2D::drawText(sTextPos, settingName + ":", UIColor(255, 255, 255),
                                              textSize);
                                D2D::drawText(modeTextPos, modeName, UIColor(255, 255, 255),
                                              textSize);
                                settingYOffset += textHeight + (textPaddingY * 2.f);
                                break;
                            }
                            case SettingType::COLOR_S: {
                                ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
                                if(sRectPos.contains(mousePos) && isRightClickDown) {
                                    colorSetting->extended = !colorSetting->extended;
                                    isRightClickDown = false;
                                }

                                float colorBoxSize = std::round(textHeight / 1.5f);
                                float colorBoxPaddingX = textPaddingX + (2.f * textSize);
                                Vec4<float> colorBoxRect =
                                    Vec4<float>(sRectPos.z - colorBoxPaddingX - colorBoxSize,
                                                (sRectPos.y + sRectPos.w - colorBoxSize) / 2.f,
                                                sRectPos.z - colorBoxPaddingX,
                                                (sRectPos.y + sRectPos.w + colorBoxSize) / 2.f);
                                D2D::drawText(sTextPos, settingName + ":", UIColor(255, 255, 255),
                                              textSize);
                                D2D::fillRectangle(colorBoxRect, (*colorSetting->colorPtr));
                                settingYOffset += textHeight + (textPaddingY * 2.f);

                                float expandedHeight = settingSpace;
                                for(auto& slider : colorSetting->colorSliders) {
                                    expandedHeight +=
                                        textHeight + (textPaddingY * 2.f) + settingSpace;
                                }
                                if(colorSetting->showSynced) {
                                    expandedHeight +=
                                        textHeight + (textPaddingY * 2.f) + settingSpace;
                                }
                                expandedHeight -= settingSpace;

                                float targetHeight = colorSetting->extended ? expandedHeight : 0.f;

                                colorSetting->currentHeight +=
                                    (targetHeight - colorSetting->currentHeight) * animSpeed;
                                if(std::abs(targetHeight - colorSetting->currentHeight) < 0.5f)
                                    colorSetting->currentHeight = targetHeight;

                                float layoutAdvance = colorSetting->currentHeight > 0.1f
                                                          ? colorSetting->currentHeight
                                                          : 0.f;

                                if(colorSetting->currentHeight > 0.1f) {
                                    Vec4<float> colorClipRect =
                                        Vec4<float>(sRectPos.x, sRectPos.w, sRectPos.z,
                                                    sRectPos.w + colorSetting->currentHeight);

                                    D2D::PushAxisAlignedClip(colorClipRect, true);

                                    float internalYOffset = sRectPos.w + settingSpace;

                                    for(auto& slider : colorSetting->colorSliders) {
                                        if(internalYOffset >
                                           sRectPos.w + colorSetting->currentHeight)
                                            break;

                                        Vec4<float> colorSliderRect = Vec4<float>(
                                            sRectPos.x + settingPaddingX, internalYOffset,
                                            sRectPos.z - settingPaddingZ,
                                            internalYOffset + textHeight + (textPaddingY * 2.f));
                                        Vec2<float> colorSliderTextPos =
                                            Vec2<float>(colorSliderRect.x + textPaddingX,
                                                        colorSliderRect.y + textPaddingY);
                                        updateSelectedAnimRect(colorSliderRect,
                                                               slider->selectedAnim);
                                        if(colorSliderRect.contains(mousePos) && isLeftClickDown) {
                                            draggingSliderSettingPtr = slider;
                                            isLeftClickDown = false;
                                        }
                                        uint8_t& value = (*slider->valuePtr);
                                        float minValue = (float)slider->minValue;
                                        float maxValue = (float)slider->maxValue;
                                        if(draggingSliderSettingPtr == slider) {
                                            float draggingPercent =
                                                (mousePos.x - colorSliderRect.x) /
                                                (colorSliderRect.z - colorSliderRect.x);
                                            draggingPercent =
                                                std::max(0.f, std::min(1.f, draggingPercent));
                                            value = (int)minValue +
                                                    (int)std::round((maxValue - minValue) *
                                                                    draggingPercent);
                                        }
                                        float valuePercent =
                                            (float)(value - minValue) / (maxValue - minValue);
                                        valuePercent = std::max(0.f, std::min(1.f, valuePercent));
                                        Vec4<float> valueRectPos = Vec4<float>(
                                            colorSliderRect.x, colorSliderRect.y,
                                            colorSliderRect.x +
                                                (colorSliderRect.z - colorSliderRect.x) *
                                                    valuePercent,
                                            colorSliderRect.w);
                                        char valueText[10];
                                        sprintf_s(valueText, 10, "%i", (int)value);
                                        std::string valueTextStr(valueText);
                                        Vec2<float> valueTextPos = Vec2<float>(
                                            colorSliderRect.z - textPaddingX -
                                                D2D::getTextWidth(
                                                    valueTextStr, textSize,
                                                    (draggingSliderSettingPtr != slider)),
                                            colorSliderTextPos.y);
                                        D2D::fillRectangle(valueRectPos, mainColor);
                                        D2D::drawText(colorSliderTextPos, slider->name + ":",
                                                      UIColor(255, 255, 255), textSize);
                                        D2D::drawText(valueTextPos, valueTextStr,
                                                      UIColor(255, 255, 255), textSize,
                                                      (draggingSliderSettingPtr != slider));
                                        D2D::fillRectangle(
                                            colorSliderRect,
                                            UIColor(255, 255, 255,
                                                    (int)(25 * slider->selectedAnim)));
                                        internalYOffset +=
                                            textHeight + (textPaddingY * 2.f) + settingSpace;
                                    }

                                    if(colorSetting->showSynced &&
                                       internalYOffset <=
                                           sRectPos.w + colorSetting->currentHeight) {
                                        bool& boolValue = (colorSetting->colorSynced);
                                        Vec4<float> syncRectPos = Vec4<float>(
                                            sRectPos.x + settingPaddingX, internalYOffset,
                                            sRectPos.z - settingPaddingZ,
                                            internalYOffset + textHeight + (textPaddingY * 2.f));
                                        Vec2<float> syncTextPos =
                                            Vec2<float>(syncRectPos.x + textPaddingX,
                                                        syncRectPos.y + textPaddingY);
                                        updateSelectedAnimRect(syncRectPos,
                                                               colorSetting->syncSelectedAnim);
                                        if(syncRectPos.contains(mousePos) && isLeftClickDown) {
                                            boolValue = !boolValue;
                                            isLeftClickDown = false;
                                        }
                                        if(boolValue)
                                            D2D::fillRectangle(syncRectPos, mainColor);
                                        D2D::fillRectangle(
                                            syncRectPos,
                                            UIColor(255, 255, 255,
                                                    (int)(25 * colorSetting->syncSelectedAnim)));
                                        D2D::drawText(syncTextPos, "Synced", UIColor(255, 255, 255),
                                                      textSize);
                                    }

                                    D2D::PopAxisAlignedClip();
                                }

                                settingYOffset += layoutAdvance;
                                break;
                            }
                            case SettingType::VEC3_S: {
                                Vec3SettingBase* vecSetting =
                                    static_cast<Vec3SettingBase*>(setting);
                                if(sRectPos.contains(mousePos) && isRightClickDown) {
                                    vecSetting->extended = !vecSetting->extended;
                                    isRightClickDown = false;
                                }
                                D2D::drawText(sTextPos, settingName, UIColor(255, 255, 255),
                                              textSize);
                                settingYOffset += textHeight + (textPaddingY * 2.f);

                                float expandedHeight = settingSpace;
                                int sliderCount = 0;
                                if(vecSetting->valueType == Vec3Type::FLOAT_T) {
                                    Vec3Setting<float>* vec =
                                        static_cast<Vec3Setting<float>*>(vecSetting);
                                    sliderCount = (int)vec->vecSliders.size();
                                } else if(vecSetting->valueType == Vec3Type::INT_T) {
                                    Vec3Setting<int>* vec =
                                        static_cast<Vec3Setting<int>*>(vecSetting);
                                    sliderCount = (int)vec->vecSliders.size();
                                }
                                for(int i = 0; i < sliderCount; i++) {
                                    expandedHeight +=
                                        textHeight + (textPaddingY * 2.f) + settingSpace;
                                }
                                expandedHeight -= settingSpace;

                                float targetHeight = vecSetting->extended ? expandedHeight : 0.f;
                                vecSetting->currentHeight +=
                                    (targetHeight - vecSetting->currentHeight) * animSpeed;
                                if(std::abs(targetHeight - vecSetting->currentHeight) < 0.5f)
                                    vecSetting->currentHeight = targetHeight;

                                float layoutAdvance = vecSetting->currentHeight > 0.1f
                                                          ? vecSetting->currentHeight
                                                          : 0.f;

                                if(vecSetting->currentHeight > 0.1f) {
                                    Vec4<float> vecClipRect =
                                        Vec4<float>(sRectPos.x, settingYOffset, sRectPos.z,
                                                    settingYOffset + vecSetting->currentHeight);
                                    D2D::PushAxisAlignedClip(vecClipRect, true);

                                    float internalYOffset = settingYOffset + settingSpace;
                                    float startY = internalYOffset;

                                    if(vecSetting->valueType == Vec3Type::FLOAT_T) {
                                        Vec3Setting<float>* vec =
                                            static_cast<Vec3Setting<float>*>(vecSetting);
                                        for(auto& slider : vec->vecSliders) {
                                            if(internalYOffset >
                                               settingYOffset + vecSetting->currentHeight)
                                                break;

                                            Vec4<float> sliderRect = Vec4<float>(
                                                sRectPos.x + settingPaddingX, internalYOffset,
                                                sRectPos.z - settingPaddingZ,
                                                internalYOffset + textHeight +
                                                    (textPaddingY * 2.f));
                                            const Vec2<float>& sliderTextPos =
                                                Vec2<float>(sliderRect.x + textPaddingX,
                                                            sliderRect.y + textPaddingY);
                                            updateSelectedAnimRect(sliderRect,
                                                                   slider->selectedAnim);
                                            if(sliderRect.contains(mousePos) && isLeftClickDown) {
                                                draggingSliderSettingPtr = slider;
                                                isLeftClickDown = false;
                                            }
                                            float& value = (*slider->valuePtr);
                                            float minValue = (float)slider->minValue;
                                            float maxValue = (float)slider->maxValue;
                                            if(draggingSliderSettingPtr == slider) {
                                                float draggingPercent = (mousePos.x - sRectPos.x) /
                                                                        (sRectPos.z - sRectPos.x);
                                                draggingPercent =
                                                    std::max(0.f, std::min(1.f, draggingPercent));
                                                value = minValue +
                                                        (maxValue - minValue) * draggingPercent;
                                            }
                                            float valuePercent =
                                                (value - minValue) / (maxValue - minValue);
                                            valuePercent =
                                                std::max(0.f, std::min(1.f, valuePercent));
                                            Vec4<float> valueRectPos = Vec4<float>(
                                                sliderRect.x, sliderRect.y,
                                                sliderRect.x +
                                                    (sliderRect.z - sliderRect.x) * valuePercent,
                                                sliderRect.w);
                                            char valueText[10];
                                            sprintf_s(valueText, 10, "%.2f", value);
                                            std::string valueTextStr(valueText);
                                            Vec2<float> valueTextPos = Vec2<float>(
                                                sliderRect.z - textPaddingX -
                                                    D2D::getTextWidth(
                                                        valueTextStr, textSize,
                                                        (draggingSliderSettingPtr != slider)),
                                                sliderRect.y);
                                            D2D::fillRectangle(valueRectPos, mainColor);
                                            D2D::drawText(sliderTextPos, slider->name + ":",
                                                          UIColor(255, 255, 255), textSize);
                                            D2D::drawText(valueTextPos, valueTextStr,
                                                          UIColor(255, 255, 255), textSize,
                                                          (draggingSliderSettingPtr != slider));
                                            D2D::fillRectangle(
                                                sliderRect,
                                                UIColor(255, 255, 255,
                                                        (int)(25 * slider->selectedAnim)));
                                            internalYOffset +=
                                                textHeight + (textPaddingY * 2.f) + settingSpace;
                                        }
                                        internalYOffset -= settingSpace;
                                        const float EndY = internalYOffset;
                                        const float SLineWidth = 4.f * textSize * fontPercent;
                                        const float SLinePaddingX = 1.f * textSize;
                                        const Vec4<float>& SLineRect = Vec4<float>(
                                            sRectPos.x + SLinePaddingX, startY,
                                            sRectPos.x + SLinePaddingX + SLineWidth, EndY);
                                    } else if(vecSetting->valueType == Vec3Type::INT_T) {
                                        Vec3Setting<int>* vec =
                                            static_cast<Vec3Setting<int>*>(vecSetting);
                                        for(auto& slider : vec->vecSliders) {
                                            if(internalYOffset >
                                               settingYOffset + vecSetting->currentHeight)
                                                break;

                                            Vec4<float> sliderRect = Vec4<float>(
                                                sRectPos.x + settingPaddingX, internalYOffset,
                                                sRectPos.z - settingPaddingZ,
                                                internalYOffset + textHeight +
                                                    (textPaddingY * 2.f));
                                            const Vec2<float>& sliderTextPos =
                                                Vec2<float>(sliderRect.x + textPaddingX,
                                                            sliderRect.y + textPaddingY);
                                            updateSelectedAnimRect(sliderRect,
                                                                   slider->selectedAnim);
                                            if(sliderRect.contains(mousePos) && isLeftClickDown) {
                                                draggingSliderSettingPtr = slider;
                                                isLeftClickDown = false;
                                            }
                                            int& value = (*slider->valuePtr);
                                            float minValue = (float)slider->minValue;
                                            float maxValue = (float)slider->maxValue;
                                            if(draggingSliderSettingPtr == slider) {
                                                float draggingPercent =
                                                    (mousePos.x - sliderRect.x) /
                                                    (sliderRect.z - sliderRect.x);
                                                draggingPercent =
                                                    std::max(0.f, std::min(1.f, draggingPercent));
                                                value = (int)minValue +
                                                        (int)std::round((maxValue - minValue) *
                                                                        draggingPercent);
                                            }
                                            float valuePercent =
                                                (float)(value - minValue) / (maxValue - minValue);
                                            valuePercent =
                                                std::max(0.f, std::min(1.f, valuePercent));
                                            Vec4<float> valueRectPos = Vec4<float>(
                                                sliderRect.x, sliderRect.y,
                                                sliderRect.x +
                                                    (sliderRect.z - sliderRect.x) * valuePercent,
                                                sliderRect.w);
                                            char valueText[10];
                                            sprintf_s(valueText, 10, "%.i", (int)value);
                                            std::string valueTextStr(valueText);
                                            Vec2<float> valueTextPos = Vec2<float>(
                                                sliderRect.z - textPaddingX -
                                                    D2D::getTextWidth(
                                                        valueTextStr, textSize,
                                                        (draggingSliderSettingPtr != slider)),
                                                sliderRect.y);
                                            D2D::fillRectangle(valueRectPos, mainColor);
                                            D2D::drawText(sliderTextPos, slider->name + ":",
                                                          UIColor(255, 255, 255), textSize);
                                            D2D::drawText(valueTextPos, valueTextStr,
                                                          UIColor(255, 255, 255), textSize,
                                                          (draggingSliderSettingPtr != slider));
                                            D2D::fillRectangle(
                                                sliderRect,
                                                UIColor(255, 255, 255,
                                                        (int)(25 * slider->selectedAnim)));
                                            internalYOffset +=
                                                textHeight + (textPaddingY * 2.f) + settingSpace;
                                        }
                                        internalYOffset -= settingSpace;
                                        const float EndY = internalYOffset;
                                        const float SLineWidth = 4.f * textSize * fontPercent;
                                        const float SLinePaddingX = 1.f * textSize;
                                        Vec4<float> SLineRect = Vec4<float>(
                                            sRectPos.x + SLinePaddingX, startY,
                                            sRectPos.x + SLinePaddingX + SLineWidth, EndY);
                                    }

                                    D2D::PopAxisAlignedClip();
                                }

                                settingYOffset += layoutAdvance;
                                break;
                            }
                            case SettingType::SLIDER_S: {
                                SliderSettingBase* sliderSettingBase =
                                    static_cast<SliderSettingBase*>(setting);
                                if(sRectPos.contains(mousePos) && isLeftClickDown) {
                                    draggingSliderSettingPtr = sliderSettingBase;
                                    isLeftClickDown = false;
                                }
                                if(sliderSettingBase->valueType == ValueType::INT_T) {
                                    SliderSetting<int>* intSlider =
                                        static_cast<SliderSetting<int>*>(sliderSettingBase);
                                    int& value = (*intSlider->valuePtr);
                                    float minValue = (float)intSlider->minValue;
                                    float maxValue = (float)intSlider->maxValue;
                                    if(draggingSliderSettingPtr == sliderSettingBase) {
                                        float draggingPercent =
                                            (mousePos.x - sRectPos.x) / (sRectPos.z - sRectPos.x);
                                        draggingPercent =
                                            std::max(0.f, std::min(1.f, draggingPercent));
                                        value =
                                            (int)minValue + (int)std::round((maxValue - minValue) *
                                                                            draggingPercent);
                                    }
                                    float valuePercent =
                                        (float)(value - minValue) / (maxValue - minValue);
                                    valuePercent = std::max(0.f, std::min(1.f, valuePercent));
                                    Vec4<float> valueRectPos = Vec4<float>(
                                        sRectPos.x, sRectPos.y,
                                        sRectPos.x + (sRectPos.z - sRectPos.x) * valuePercent,
                                        sRectPos.w);
                                    char valueText[10];
                                    sprintf_s(valueText, 10, "%i", value);
                                    std::string valueTextStr(valueText);
                                    Vec2<float> valueTextPos = Vec2<float>(
                                        sRectPos.z - textPaddingX -
                                            D2D::getTextWidth(
                                                valueTextStr, textSize,
                                                (draggingSliderSettingPtr != sliderSettingBase)),
                                        sTextPos.y);
                                    D2D::fillRectangle(valueRectPos, mainColor);
                                    D2D::drawText(sTextPos, settingName + ":",
                                                  UIColor(255, 255, 255), textSize);
                                    D2D::drawText(valueTextPos, valueTextStr,
                                                  UIColor(255, 255, 255), textSize,
                                                  (draggingSliderSettingPtr != sliderSettingBase));
                                } else if(sliderSettingBase->valueType == ValueType::FLOAT_T) {
                                    SliderSetting<float>* floatSlider =
                                        static_cast<SliderSetting<float>*>(sliderSettingBase);
                                    float& value = (*floatSlider->valuePtr);
                                    float minValue = floatSlider->minValue;
                                    float maxValue = floatSlider->maxValue;
                                    if(draggingSliderSettingPtr == sliderSettingBase) {
                                        float draggingPercent =
                                            (mousePos.x - sRectPos.x) / (sRectPos.z - sRectPos.x);
                                        draggingPercent =
                                            std::max(0.f, std::min(1.f, draggingPercent));
                                        value = minValue + (maxValue - minValue) * draggingPercent;
                                    }
                                    float valuePercent = (value - minValue) / (maxValue - minValue);
                                    valuePercent = std::max(0.f, std::min(1.f, valuePercent));
                                    Vec4<float> valueRectPos = Vec4<float>(
                                        sRectPos.x, sRectPos.y,
                                        sRectPos.x + (sRectPos.z - sRectPos.x) * valuePercent,
                                        sRectPos.w);
                                    char valueText[10];
                                    sprintf_s(valueText, 10, "%.2f", value);
                                    std::string valueTextStr(valueText);
                                    Vec2<float> valueTextPos = Vec2<float>(
                                        sRectPos.z - textPaddingX -
                                            D2D::getTextWidth(
                                                valueTextStr, textSize,
                                                (draggingSliderSettingPtr != sliderSettingBase)),
                                        sTextPos.y);
                                    D2D::fillRectangle(valueRectPos, mainColor);
                                    D2D::drawText(sTextPos, settingName + ":",
                                                  UIColor(255, 255, 255), textSize);
                                    D2D::drawText(valueTextPos, valueTextStr,
                                                  UIColor(255, 255, 255), textSize,
                                                  (draggingSliderSettingPtr != sliderSettingBase));
                                }
                                settingYOffset += textHeight + (textPaddingY * 2.f);
                                break;
                            }
                        }
                        D2D::fillRectangle(
                            sRectPos, UIColor(255, 255, 255, (int)(25 * setting->selectedAnim)));
                        settingYOffset += settingSpace;
                    }
                    D2D::PopAxisAlignedClip();
                }

                yOffset += mod->currentHeight + moduleSpace;
            }
            D2D::PopAxisAlignedClip();
        }
    }

    if(showDescription && descriptionText != "NULL" && draggingWindowPtr == nullptr &&
       draggingSliderSettingPtr == nullptr) {
        Vec2<float> mousePadding(15.f, 15.f);
        Vec2<float> targetPos = mousePos.add(mousePadding);

        float lerpSpeed = 1.f;
        tooltipLerpPos.x += (targetPos.x - tooltipLerpPos.x) * lerpSpeed;
        tooltipLerpPos.y += (targetPos.y - tooltipLerpPos.y) * lerpSpeed;

        float textWidth = D2D::getTextWidth(descriptionText, 0.8f);
        float textHeight = D2D::getTextHeight(descriptionText, 0.8f);

        Vec4<float> rectPos(tooltipLerpPos.x, tooltipLerpPos.y, tooltipLerpPos.x + textWidth + 4.f,
                            tooltipLerpPos.y + textHeight);

        Vec2<float> textPos(rectPos.x + 2.f, rectPos.y);

        D2D::fillRectangle(rectPos, UIColor(0, 0, 0, 125));
        D2D::drawText(textPos, descriptionText, UIColor(255, 255, 255), 0.8f);
    }
}

void ClickGUI::updateSelectedAnimRect(Vec4<float>& rect, float& anim) {
    bool shouldUp = rect.contains(mousePos);

    if(draggingWindowPtr != nullptr)
        shouldUp = false;

    if(draggingSliderSettingPtr != nullptr) {
        if(&draggingSliderSettingPtr->selectedAnim != &anim)
            shouldUp = false;
        else
            shouldUp = true;
    }

    // anim += D2D::deltaTime * (shouldUp ? 5.f : -2.f);
    if(shouldUp)
        anim = 1.f;
    else
        anim -= D2D::deltaTime * 2.f;

    if(anim > 1.f)
        anim = 1.f;
    if(anim < 0.f)
        anim = 0.f;
}

using json = nlohmann::json;

void ClickGUI::onLoadConfig(void* confVoid) {
    json* config = reinterpret_cast<json*>(confVoid);
    std::string modName = this->getModuleName();
    try {
        if(config->contains(modName)) {
            json obj = config->at(getModuleName());
            if(obj.is_null())
                return;

            for(Setting* setting : this->getSettingList()) {
                if(obj.contains(setting->name)) {
                    if(setting->type == SettingType::KEYBIND_S) {
                        KeybindSetting* keySetting = (KeybindSetting*)setting;
                        *keySetting->value = obj[keySetting->name].get<int>();
                    } else if(setting->type == SettingType::BOOL_S) {
                        BoolSetting* boolSetting = (BoolSetting*)setting;
                        *boolSetting->value = obj[boolSetting->name].get<bool>();
                    } else if(setting->type == SettingType::ENUM_S) {
                        EnumSetting* enumSetting = (EnumSetting*)setting;
                        *enumSetting->value = obj[enumSetting->name].get<int>();
                    } else if(setting->type == SettingType::STRING_S) {
                        StringSetting* strSetting = (StringSetting*)setting;
                        *strSetting->value = obj[strSetting->name].get<std::string>();
                    } else if(setting->type == SettingType::COLOR_S) {
                        ColorSetting* colorSetting = (ColorSetting*)setting;
                        json colorObj = obj[colorSetting->name];
                        if(colorObj.contains("Red"))
                            colorSetting->colorPtr->r = colorObj["Red"].get<int>();
                        if(colorObj.contains("Green"))
                            colorSetting->colorPtr->g = colorObj["Green"].get<int>();
                        if(colorObj.contains("Blue"))
                            colorSetting->colorPtr->b = colorObj["Blue"].get<int>();
                        if(colorObj.contains("Alpha"))
                            colorSetting->colorPtr->a = colorObj["Alpha"].get<int>();
                        if(colorObj.contains("Synced"))
                            colorSetting->colorSynced = colorObj["Synced"].get<bool>();
                    } else if(setting->type == SettingType::SLIDER_S) {
                        SliderSettingBase* sliderSetting = (SliderSettingBase*)setting;
                        if(sliderSetting->valueType == ValueType::INT_T) {
                            SliderSetting<int>* intSlider = (SliderSetting<int>*)sliderSetting;
                            *intSlider->valuePtr = obj[intSlider->name].get<int>();
                        } else if(sliderSetting->valueType == ValueType::FLOAT_T) {
                            SliderSetting<float>* floatSlider =
                                (SliderSetting<float>*)sliderSetting;
                            *floatSlider->valuePtr = obj[floatSlider->name].get<float>();
                        }
                    }
                }
            }

            for(auto& window : windowList) {
                std::string windowName = window->name;

                if(obj.contains(windowName)) {
                    json confValue = obj.at(windowName);
                    if(confValue.is_null())
                        continue;

                    if(confValue.contains("isExtended")) {
                        window->extended = confValue["isExtended"].get<bool>();
                    }

                    if(confValue.contains("pos")) {
                        window->pos.x = confValue["pos"]["x"].get<float>();
                        window->pos.y = confValue["pos"]["y"].get<float>();
                    }
                }
            }
        }
    } catch(const std::exception& e) {
    }
}

void ClickGUI::onSaveConfig(void* confVoid) {
    json* currentConfig = reinterpret_cast<json*>(confVoid);
    try {
        json obj;
        for(Setting* setting : this->getSettingList()) {
            if(setting->type == SettingType::KEYBIND_S) {
                KeybindSetting* keySetting = (KeybindSetting*)setting;
                obj[keySetting->name] = *keySetting->value;
            } else if(setting->type == SettingType::BOOL_S) {
                BoolSetting* boolSetting = (BoolSetting*)setting;
                obj[boolSetting->name] = *boolSetting->value;
            } else if(setting->type == SettingType::ENUM_S) {
                EnumSetting* enumSetting = (EnumSetting*)setting;
                obj[enumSetting->name] = *enumSetting->value;
            } else if(setting->type == SettingType::STRING_S) {
                StringSetting* strSetting = (StringSetting*)setting;
                obj[strSetting->name] = *strSetting->value;
            } else if(setting->type == SettingType::COLOR_S) {
                ColorSetting* colorSetting = (ColorSetting*)setting;
                obj[colorSetting->name]["Red"] = colorSetting->colorPtr->r;
                obj[colorSetting->name]["Green"] = colorSetting->colorPtr->g;
                obj[colorSetting->name]["Blue"] = colorSetting->colorPtr->b;
                obj[colorSetting->name]["Alpha"] = colorSetting->colorPtr->a;
                obj[colorSetting->name]["Synced"] = colorSetting->colorSynced;
            } else if(setting->type == SettingType::SLIDER_S) {
                SliderSettingBase* sliderSetting = (SliderSettingBase*)setting;
                if(sliderSetting->valueType == ValueType::INT_T) {
                    SliderSetting<int>* intSlider = (SliderSetting<int>*)sliderSetting;
                    obj[intSlider->name] = *intSlider->valuePtr;
                } else if(sliderSetting->valueType == ValueType::FLOAT_T) {
                    SliderSetting<float>* floatSlider = (SliderSetting<float>*)sliderSetting;
                    obj[floatSlider->name] = *floatSlider->valuePtr;
                }
            }
        }

        for(auto& window : windowList) {
            obj[window->name]["isExtended"] = window->extended;
            obj[window->name]["pos"]["x"] = window->pos.x;
            obj[window->name]["pos"]["y"] = window->pos.y;
        }
        (*currentConfig)[this->getModuleName()] = obj;
    } catch(const std::exception& e) {
    }
}