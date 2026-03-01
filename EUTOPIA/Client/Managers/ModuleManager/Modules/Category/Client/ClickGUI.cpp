#include "ClickGUI.h"

#include "../../../../../../Libs/json.hpp"
#include "../../../../../../Utils/TimerUtil.h"
#include "../../../ModuleManager.h"
#include "../../ModuleBase/Settings/PageSetting.h"
#include "SearchBox.h"

ClickGUI::ClickGUI() : Module("ClickGUI", "Display all modules", Category::CLIENT, VK_INSERT) {
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
    renderedThisFrame = false;

    draggingWindowPtr = nullptr;

    capturingKbSettingPtr = nullptr;
    draggingSliderSettingPtr = nullptr;

    openAnim = 0.0f;
}

void ClickGUI::onEnable() {
    GI::getClientInstance()->releaseVMouse();
    openAnim = 0.0f;
}

bool ClickGUI::isVisible() {
    return false;
}

void ClickGUI::onKeyUpdate(int key, bool isDown) {
    static NeteaseMusicGUI* musicGuiMod = ModuleManager::getModule<NeteaseMusicGUI>();
    static SearchBox* searchBoxMod = ModuleManager::getModule<SearchBox>();
    static Configs* cs = ModuleManager::getModule<Configs>();
    static AIChat* ai = ModuleManager::getModule<AIChat>();
    if(musicGuiMod && musicGuiMod->isEnabled()) {
        musicGuiMod->onKeyUpdate(key, isDown);
        if(musicGuiMod->isSearchFocused()) {
            return;
        }
    }
    if(searchBoxMod && searchBoxMod->isEnabled()) {
        searchBoxMod->onKeyUpdate(key, isDown);
        if(searchBoxMod->isSearchFocused()) {
            return;  // Stop further GUI key processing
        }
    }

    if(ai && ai->isEnabled()) {
        ai->onKeyUpdate(key, isDown);
        if(ai->isSearchFocused()) {
            return;  // Stop further GUI key processing
        }
    }

    if(cs && cs->isEnabled()) {
        cs->onKeyUpdate(key, isDown);
        if(cs->isSearchFocused()) {
            return;  // Stop further GUI key processing
        }
    }
    if(!isEnabled()) {
        if(key == getKeybind() && isDown) {
            setEnabled(true);
        }
    } else {
        if(isDown) {
            if(key < 192) {
                if(capturingKbSettingPtr != nullptr) {
                    if(key != VK_ESCAPE)
                        *capturingKbSettingPtr->value = key;
                    capturingKbSettingPtr = nullptr;
                    return;
                }
            }
            if(key == getKeybind() || key == VK_ESCAPE) {
                setEnabled(false);
            }
        }
    }
}

void ClickGUI::onMouseUpdate(Vec2<float> mousePosA, char mouseButton, char isDown) {
    static SearchBox* searchBoxMod = ModuleManager::getModule<SearchBox>();
    static NeteaseMusicGUI* musicGuiMod = ModuleManager::getModule<NeteaseMusicGUI>();
    static AIChat* ai = ModuleManager::getModule<AIChat>();
    static Configs* cs = ModuleManager::getModule<Configs>();
    if(musicGuiMod && musicGuiMod->isEnabled() &&
       musicGuiMod->onMouseUpdate(mousePosA, mouseButton, isDown)) {
        return;
    }
    if(searchBoxMod && searchBoxMod->isEnabled() &&
       searchBoxMod->onMouseUpdate(mousePosA, mouseButton, isDown)) {
        return;
    }
    if(ai && ai->isEnabled() && ai->onMouseUpdate(mousePosA, mouseButton, isDown)) {
        return;
    }

    if(cs && cs->isEnabled() && cs->onMouseUpdate(mousePosA, mouseButton, isDown)) {
        return;
    }

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
                window->pos.y += moveVec;
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

inline float fremate() {
    return 61.f;  //+1 for a reaosn owo ok
}

inline float lerpSync(const float& a, const float& b, const float& duration) {
    const float result = (float)(a + (b - a) * duration * (60.f / fremate()));
    if(a < b && result > b)
        return b;
    else if(a > b && result < b)
        return b;
    return result;
}

inline float RandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}
void ClickGUI::InitClickGUI() {
    setEnabled(false);

    // Delete old windows first
    for(auto window : windowList) {
        delete window;
    }
    windowList.clear();

    // Recreate windows
    Vec2<float> startPos = Vec2<float>(25.f, 35.f);
    windowList.push_back(new ClickWindow("Combat", startPos, Category::COMBAT));
    startPos.x += 237.f;
    windowList.push_back(new ClickWindow("Movement", startPos, Category::MOVEMENT));
    startPos.x += 237.f;
    windowList.push_back(new ClickWindow("Render", startPos, Category::RENDER));
    startPos.x += 237.f;
    windowList.push_back(new ClickWindow("Player", startPos, Category::PLAYER));
    startPos.x += 237.f;
    windowList.push_back(new ClickWindow("World", startPos, Category::WORLD));
    startPos.x += 237.f;
    windowList.push_back(new ClickWindow("Misc", startPos, Category::MISC));
    startPos.x += 237.f;
    windowList.push_back(new ClickWindow("Client", startPos, Category::CLIENT));

    initialized = true;
}

void ClickGUI::Render() {
    if(!initialized)
        return;
    static std::unordered_map<void*, float> displayValueMap;

    mainColor = ModuleManager::getModule<Colors>()->getColor();
    if(GI::canUseMoveKeys())
        GI::getClientInstance()->releaseVMouse();

    /*
    openAnim = lerpSync(openAnim, this->isEnabled() ? 1.f : 0.f, 0.1f);

    float guiScale = 0.85f + 0.15f * openAnim;
    */

    static Vec2<float> oldMousePos = mousePos;
    mouseDelta = mousePos.sub(oldMousePos);
    oldMousePos = mousePos;
    std::string descriptionText = "NULL";

    Vec2<float> screenSize = GI::getClientInstance()->guiData->windowSizeReal;
    float deltaTime = RenderUtil::deltaTime;
    if(yourBoolVariable) {
        static float hue = 0.f;
        hue += deltaTime * 0.1f;
        if(hue > 1.f)
            hue = 0.f;

        float rF, gF, bF;
        ColorUtil::ColorConvertHSVtoRGB(hue, 1.f, 1.f, rF, gF, bF);
        mainColor = UIColor((int)(rF * 255.f), (int)(gF * 255.f), (int)(bF * 255.f), 255);
    } else {
        mainColor = mainColor;
    }

    static float opacity = 0.f;
    opacity = lerpSync(opacity, this->isEnabled() ? 1.f : 0.f, 0.15f);
    float textSize = 1.f;
    float textPaddingX = 3.f * textSize;
    float textPaddingY = 0.f * textSize;
    float textHeight = RenderUtil::getTextHeight("", textSize);

    if(blurStrength > 0.1f)
        RenderUtil::addBlur(Vec4<float>(0.f, 0.f, screenSize.x, screenSize.y), blurStrength);
    RenderUtil::fillRectangle(Vec4<float>(0.f, 0.f, screenSize.x, screenSize.y),
                              UIColor(0, 0, 0, (int)(110)));

    int steps = 100;
    float stepHeight = screenSize.y / steps;

    for(int i = 0; i < steps; i++) {
        float t = (float)i / (steps - 1);

        UIColor currentColor = ColorUtil::lerp(
            UIColor(0, 0, 0, 100), UIColor(mainColor.r, mainColor.g, mainColor.b, 100), t);

        float top = i * stepHeight;
        float bottom = top + stepHeight;

        RenderUtil::fillRectangle(Vec4<float>(0.f, top, screenSize.x, bottom), currentColor);
    }

    for(auto& window : windowList) {
        if(window == draggingWindowPtr) {
            window->pos = window->pos.add(mouseDelta);
        }

        static CustomFont* customFontMod = ModuleManager::getModule<CustomFont>();
        float fontPercent = (float)customFontMod->fontSize / 25.f;
        // Calculate original window rectangle
        float windowWidth = 250.f * fontPercent + (textPaddingX * 2.f);
        float windowHeight = textHeight + (textPaddingY * 2.f);
        Vec4<float> hRectPos =
            Vec4<float>(window->pos.x, window->pos.y, window->pos.x + windowWidth,
                        window->pos.y + windowHeight);

        // Calculate original text position
        Vec2<float> hTextPos = Vec2<float>(
            hRectPos.x +
                (hRectPos.z - hRectPos.x - RenderUtil::getTextWidth(window->name, textSize)) / 2.f,
            hRectPos.y + textPaddingY);

        // Zoom / pop-in animation scale
        float scale = 0.5f + 0.5f;  // adjust start/end scale

        // Center of the window
        Vec2<float> center =
            Vec2<float>((hRectPos.x + hRectPos.z) / 2.f, (hRectPos.y + hRectPos.w) / 2.f);

        // Apply scaling around the center
        Vec4<float> scaledRect;
        scaledRect.x = center.x + (hRectPos.x - center.x) * scale;
        scaledRect.y = center.y + (hRectPos.y - center.y) * scale;
        scaledRect.z = center.x + (hRectPos.z - center.x) * scale;
        scaledRect.w = center.y + (hRectPos.w - center.y) * scale;

        Vec2<float> scaledTextPos = Vec2<float>(center.x + (hTextPos.x - center.x) * scale,
                                                center.y + (hTextPos.y - center.y) * scale);

        RenderUtil::fillRectangle(scaledRect, mainColor);
        std::vector<std::string> plusModules = {"KillAuraH"};

        UIColor textColor = UIColor(255, 255, 255);
        if(std::find(plusModules.begin(), plusModules.end(), window->name) != plusModules.end()) {
            textColor = UIColor(255, 215, 0);
        }

        RenderUtil::drawText(scaledTextPos, window->name, textColor, textSize * scale);

        if(hRectPos.contains(mousePos)) {
            if(isLeftClickDown) {
                draggingWindowPtr = window;
                isLeftClickDown = false;
            } else if(isRightClickDown) {
                window->extended = !window->extended;
                isRightClickDown = false;
            }
        }

        std::string icon;
        switch(window->category) {
            case Category::COMBAT:
                icon = std::string(reinterpret_cast<const char*>(u8"🗡️"));
                break;
            case Category::MOVEMENT:
                icon = std::string(reinterpret_cast<const char*>(u8"🏃"));
                break;
            case Category::RENDER:
                icon = std::string(reinterpret_cast<const char*>(u8"👁️"));
                break;
            case Category::PLAYER:
                icon = std::string(reinterpret_cast<const char*>(u8"👤"));
                break;
            case Category::WORLD:
                icon = std::string(reinterpret_cast<const char*>(u8"🌍"));
                break;
            case Category::MISC:
                icon = std::string(reinterpret_cast<const char*>(u8"🔧"));
                break;
            case Category::CLIENT:
                icon = std::string(reinterpret_cast<const char*>(u8"💻"));
                break;
            default:
                icon = "";
                break;
        }

        float scaledTextSize = textSize * 0.91f;
        float iconHeight = RenderUtil::getTextHeight(icon, scaledTextSize);
        float iconWidth = RenderUtil::getTextWidth(icon, scaledTextSize);
        float iconXOffset = (hRectPos.z - hRectPos.x) * 0.15f;
        Vec2<float> iconPos =
            Vec2<float>(hRectPos.x + iconXOffset - (iconWidth / 2.f),
                        hRectPos.y + ((hRectPos.w - hRectPos.y - iconHeight) / 2.f));

        updateSelectedAnimRect(hRectPos, window->selectedAnim);

        RenderUtil::drawText(iconPos, icon, UIColor(255, 255, 255), scaledTextSize);

        float nameWidth = RenderUtil::getTextWidth(window->name, textSize);

        Vec2<float>(hRectPos.x + (hRectPos.z - hRectPos.x - nameWidth) / 2.f + iconWidth * 0.4f,
                    hRectPos.y + ((hRectPos.w - hRectPos.y - textHeight) / 2.f));

        RenderUtil::drawText(hTextPos, window->name, UIColor(255, 255, 255), textSize);

        RenderUtil::fillRectangle(hRectPos,
                                  UIColor(255, 255, 255, (int)(25 * window->selectedAnim)));

        if(window->extended) {
            float moduleSpace = 2.f * textSize * fontPercent;
            float settingSpace = 2.f * textSize * fontPercent;
            float yHeight = 0.f;
            yHeight += moduleSpace;

            for(auto& mod : window->moduleList) {
                yHeight += textHeight + (textPaddingY * 2.f);
                if(mod->extended) {
                    yHeight += settingSpace;
                    for(auto& setting : mod->getSettingList()) {
                        int currentPage =
                            (mod->modulePagePtr != nullptr) ? *mod->modulePagePtr : -1;

                        if(setting->type != SettingType::PAGE_S && setting->name != "Visible" &&
                           setting->name != "Keybind" && setting->name != "Toggle" &&
                           (mod->modulePagePtr != nullptr && setting->settingPage != currentPage))
                            continue;
                        if(setting->type != SettingType::COLOR_S) {
                            yHeight += textHeight + (textPaddingY * 2.f);
                        } else {
                            ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
                            yHeight += textHeight + (textPaddingY * 2.f);

                            if(colorSetting->extended) {
                                yHeight += settingSpace;

                                yHeight += 120.f;
                            }
                        }
                        yHeight += settingSpace;
                    }
                }
                yHeight += moduleSpace;
            }

            float wbgPaddingX = 2.f * textSize * fontPercent;
            Vec4<float> wbgRectPos = Vec4<float>(hRectPos.x + wbgPaddingX, hRectPos.w,
                                                 hRectPos.z - wbgPaddingX, hRectPos.w + yHeight);
            RenderUtil::fillRectangle(wbgRectPos, UIColor(0, 0, 0, 75));

            float yOffset = hRectPos.w + moduleSpace;
            for(auto& mod : window->moduleList) {
                float modPaddingX = wbgPaddingX + (2.f * textSize * fontPercent);
                Vec4<float> mRectPos =
                    Vec4<float>(hRectPos.x + modPaddingX, yOffset, hRectPos.z - modPaddingX,
                                yOffset + textHeight + (textPaddingY * 2.f));
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
                RenderUtil::fillRectangle(mRectPos,
                                          mod->isEnabled() ? mainColor : UIColor(0, 0, 0, 35));
                static std::vector<std::string> plusmodules = {"KillAuraH", "DisablerCC"};

                RenderUtil::drawText(
                    mTextPos, mod->getModuleName(),
                    std::find(plusmodules.begin(), plusmodules.end(), mod->getModuleName()) !=
                            plusmodules.end()
                        ? UIColor(255, 215, 0)  // gold xddd
                        : (mod->isEnabled()
                               ? UIColor(255, 255, 255)
                               : ColorUtil::lerp(UIColor(175, 175, 175), UIColor(255, 255, 255),
                                                 mod->selectedAnim)),
                    textSize);

                RenderUtil::fillRectangle(mRectPos,
                                          UIColor(255, 255, 255, (int)(25 * mod->selectedAnim)));
                yOffset += textHeight + (textPaddingY * 2.f);
                float borderWidth = 2.0f;
                if(&mod == &window->moduleList.front()) {
                    topY = mRectPos.y;
                }

                if(&mod == &window->moduleList.back()) {
                    bottomY = mRectPos.w + moduleSpace;
                }

                if(mod->extended) {
                    yOffset += settingSpace;
                    float startY = yOffset;

                    float settingPaddingX = 7.5f * textSize * fontPercent;
                    float settingPaddingZ = 3.5f * textSize * fontPercent;

                    for(auto& setting : mod->getSettingList()) {
                        std::string settingName = setting->name;

                        Vec4<float> sRectPos = Vec4<float>(
                            mRectPos.x + settingPaddingX, yOffset, mRectPos.z - settingPaddingZ,
                            yOffset + textHeight + (textPaddingY * 2.f));

                        Vec2<float> sTextPos =
                            Vec2<float>(sRectPos.x + textPaddingX, sRectPos.y + textPaddingY);

                        int currentPage = -1;
                        if(mod->modulePagePtr != nullptr) {
                            currentPage = *mod->modulePagePtr;
                        }

                        if(setting->type != SettingType::PAGE_S && setting->name != "Visible" &&
                           setting->name != "Keybind" && setting->name != "Toggle" &&
                           (mod->modulePagePtr != nullptr && setting->settingPage != currentPage))
                            continue;

                        if(sRectPos.contains(mousePos)) {
                            descriptionText = setting->description;
                        }

                        updateSelectedAnimRect(sRectPos, setting->selectedAnim);

                        switch(setting->type) {
                            case SettingType::BOOL_S: {
                                BoolSetting* boolSetting = static_cast<BoolSetting*>(setting);
                                bool& boolValue = (*boolSetting->value);

                                if(sRectPos.contains(mousePos)) {
                                    if(isLeftClickDown) {
                                        boolValue = !boolValue;
                                        isLeftClickDown = false;
                                    }
                                }

                                if(boolValue)
                                    RenderUtil::fillRectangle(sRectPos, mainColor);

                                RenderUtil::drawText(sTextPos, settingName, UIColor(255, 255, 255),
                                                     textSize);

                                yOffset += textHeight + (textPaddingY * 2.f);
                                break;
                            }
                            case SettingType::KEYBIND_S: {
                                KeybindSetting* keybindSetting =
                                    static_cast<KeybindSetting*>(setting);
                                int& keyValue = (*keybindSetting->value);

                                if(sRectPos.contains(mousePos)) {
                                    if(isLeftClickDown) {
                                        if(capturingKbSettingPtr == keybindSetting)
                                            capturingKbSettingPtr = nullptr;
                                        else
                                            capturingKbSettingPtr = keybindSetting;

                                        isLeftClickDown = false;
                                    }
                                }

                                std::string keybindName;
                                if(setting == capturingKbSettingPtr) {
                                    keybindName = "...";
                                } else {
                                    if(keyValue != 0)
                                        keybindName = KeyNames[keyValue];
                                    else
                                        keybindName = "None";
                                }

                                Vec2<float> keybindTextPos =
                                    Vec2<float>(sRectPos.z - textPaddingX -
                                                    RenderUtil::getTextWidth(keybindName, textSize),
                                                sTextPos.y);

                                RenderUtil::drawText(sTextPos, settingName + ":",
                                                     UIColor(255, 255, 255), textSize);
                                RenderUtil::drawText(keybindTextPos, keybindName,
                                                     UIColor(255, 255, 255), textSize);

                                yOffset += textHeight + (textPaddingY * 2.f);
                                break;
                            }
                            case SettingType::ENUM_S: {
                                EnumSetting* enumSetting = static_cast<EnumSetting*>(setting);
                                int& enumValue = (*enumSetting->value);

                                if(sRectPos.contains(mousePos)) {
                                    if(isLeftClickDown) {
                                        enumValue++;
                                        if(enumValue > enumSetting->enumList.size() - 1)
                                            enumValue = 0;
                                        isLeftClickDown = false;
                                    } else if(isRightClickDown) {
                                        enumValue--;
                                        if(enumValue < 0)
                                            enumValue = (int)enumSetting->enumList.size() - 1;
                                        isRightClickDown = false;
                                    }
                                }

                                std::string modeName = enumSetting->enumList[enumValue];
                                Vec2<float> modeTextPos =
                                    Vec2<float>(sRectPos.z - textPaddingX -
                                                    RenderUtil::getTextWidth(modeName, textSize),
                                                sTextPos.y);

                                RenderUtil::drawText(sTextPos, settingName + ":",
                                                     UIColor(255, 255, 255), textSize);
                                RenderUtil::drawText(modeTextPos, modeName, UIColor(255, 255, 255),
                                                     textSize);

                                yOffset += textHeight + (textPaddingY * 2.f);
                                break;
                            }
                            case SettingType::COLOR_S: {
                                ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);

                                if(sRectPos.contains(mousePos)) {
                                    if(isRightClickDown) {
                                        colorSetting->extended = !colorSetting->extended;
                                        isRightClickDown = false;
                                    }
                                }

                                float colorBoxSize = std::round(textHeight / 1.5f);
                                float colorBoxPaddingX = textPaddingX + (2.f * textSize);
                                Vec4<float> colorBoxRect =
                                    Vec4<float>(sRectPos.z - colorBoxPaddingX - colorBoxSize,
                                                (sRectPos.y + sRectPos.w - colorBoxSize) / 2.f,
                                                sRectPos.z - colorBoxPaddingX,
                                                (sRectPos.y + sRectPos.w + colorBoxSize) / 2.f);
                                if(!colorSetting->extended) {
                                    yOffset += textHeight + (textPaddingY * 2.f) + settingSpace;

                                    RenderUtil::drawText(sTextPos, settingName + ":",
                                                         UIColor(255, 255, 255), textSize);
                                    RenderUtil::fillRectangle(colorBoxRect,
                                                              (*colorSetting->colorPtr));
                                }
                                RenderUtil::drawText(sTextPos, settingName + ":",
                                                     UIColor(255, 255, 255), textSize);
                                RenderUtil::fillRectangle(colorBoxRect, (*colorSetting->colorPtr));

                                if(colorSetting->extended) {
                                    yOffset += textHeight + (textPaddingY * 2.f);

                                    auto* col = colorSetting->colorPtr;

                                    float pickerWidth = 136.f;
                                    float pickerHeight = 100.f;
                                    float barWidth = 10.f;

                                    Vec4<float> pickerBgRect = Vec4<float>(
                                        sRectPos.x, yOffset,
                                        sRectPos.x + pickerWidth + barWidth * 2.f + 20.f,
                                        yOffset + pickerHeight + 10.f);
                                    RenderUtil::fillRectangle(pickerBgRect, UIColor(30, 30, 30));
                                    Vec4<float> hsvRect =
                                        Vec4<float>(pickerBgRect.x + 5.f, pickerBgRect.y + 5.f,
                                                    pickerBgRect.x + 5.f + pickerWidth,
                                                    pickerBgRect.y + 5.f + pickerHeight);
                                    Vec4<float> cHue;
                                    ImGui::ColorConvertHSVtoRGB(colorSetting->hueDuration, 1, 1,
                                                                cHue.x, cHue.y, cHue.z);

                                    RenderUtil::drawHSVBox(hsvRect, cHue);
                                    Vec2<float> dotPos =
                                        Vec2<float>(hsvRect.x + colorSetting->pos.x,
                                                    hsvRect.y + colorSetting->pos.y);
                                    RenderUtil::fillCircle(dotPos, UIColor(255, 255, 255), 3.f);
                                    Vec4<float> hueBarRect =
                                        Vec4<float>(hsvRect.z + 5.f, hsvRect.y,
                                                    hsvRect.z + 5.f + barWidth, hsvRect.w);
                                    RenderUtil::drawHueBar(hueBarRect);
                                    float hueY = hueBarRect.y + (hueBarRect.w - hueBarRect.y) *
                                                                    colorSetting->hueDuration;
                                    RenderUtil::fillRectangle(Vec4<float>(hueBarRect.x, hueY - 1.f,
                                                                          hueBarRect.z, hueY + 1.f),
                                                              UIColor(255, 255, 255));
                                    Vec4<float> alphaBarRect =
                                        Vec4<float>(hueBarRect.z + 5.f, hsvRect.y,
                                                    hueBarRect.z + 5.f + barWidth, hsvRect.w);
                                    RenderUtil::drawAlphaBar(alphaBarRect, *col);
                                    float alphaY =
                                        alphaBarRect.y +
                                        (alphaBarRect.w - alphaBarRect.y) * (1.f - col->a / 255.f);
                                    RenderUtil::fillRectangle(
                                        Vec4<float>(alphaBarRect.x, alphaY - 1.f, alphaBarRect.z,
                                                    alphaY + 1.f),
                                        UIColor(255, 255, 255));
                                    if(hsvRect.contains(mousePos) && isLeftClickDown) {
                                        colorSetting->isDragging = true;
                                        isLeftClickDown = false;
                                    }
                                    if(hueBarRect.contains(mousePos) && isLeftClickDown) {
                                        colorSetting->isDragging2 = true;
                                        isLeftClickDown = false;
                                    }
                                    if(alphaBarRect.contains(mousePos) && isLeftClickDown) {
                                        colorSetting->isDragging3 = true;
                                        isLeftClickDown = false;
                                    }

                                    if(colorSetting->isDragging) {
                                        if(!isHoldingLeftClick)
                                            colorSetting->isDragging = false;
                                        else {
                                            float x = std::clamp(mousePos.x - hsvRect.x, 0.f,
                                                                 pickerWidth);
                                            float y = std::clamp(mousePos.y - hsvRect.y, 0.f,
                                                                 pickerHeight);
                                            colorSetting->pos = Vec2<float>(x, y);
                                        }
                                    }
                                    if(colorSetting->isDragging2) {
                                        if(!isHoldingLeftClick)
                                            colorSetting->isDragging2 = false;
                                        else {
                                            float h = (mousePos.y - hueBarRect.y) /
                                                      (hueBarRect.w - hueBarRect.y);
                                            colorSetting->hueDuration = std::clamp(h, 0.f, 1.f);
                                        }
                                    }
                                    if(colorSetting->isDragging3) {
                                        if(!isHoldingLeftClick)
                                            colorSetting->isDragging3 = false;
                                        else {
                                            float a = 1.f - ((mousePos.y - alphaBarRect.y) /
                                                             (alphaBarRect.w - alphaBarRect.y));
                                            col->a = (int)std::clamp(a * 255.f, 0.f, 255.f);
                                        }
                                    }
                                    float sat = colorSetting->pos.x / pickerWidth;
                                    float brightness = 1.f - colorSetting->pos.y / pickerHeight;
                                    float r, g, b;
                                    ImGui::ColorConvertHSVtoRGB(colorSetting->hueDuration, sat,
                                                                brightness, r, g, b);
                                    col->r = (int)(r * 255.f);
                                    col->g = (int)(g * 255.f);
                                    col->b = (int)(b * 255.f);

                                    yOffset += (pickerBgRect.w - pickerBgRect.y + settingSpace);
                                }
                                break;
                            }
                            case SettingType::PAGE_S: {
                                PageSetting* pageSetting = static_cast<PageSetting*>(setting);
                                int& pageValue = (*pageSetting->valuePtr);

                                if(sRectPos.contains(mousePos)) {
                                    if(isLeftClickDown) {
                                        pageValue++;
                                        if(pageValue >= (int)pageSetting->pageNames.size())
                                            pageValue = 0;
                                        isLeftClickDown = false;
                                    } else if(isRightClickDown) {
                                        pageValue--;
                                        if(pageValue < 0)
                                            pageValue = (int)pageSetting->pageNames.size() - 1;
                                        isRightClickDown = false;
                                    }
                                }

                                std::string pageName = pageSetting->pageNames[pageValue];
                                Vec2<float> pageTextPos =
                                    Vec2<float>(sRectPos.z - textPaddingX -
                                                    RenderUtil::getTextWidth(pageName, textSize),
                                                sTextPos.y);

                                RenderUtil::drawText(sTextPos, settingName + ":",
                                                     UIColor(255, 255, 255), textSize);
                                RenderUtil::drawText(pageTextPos, pageName, UIColor(255, 255, 255),
                                                     textSize);

                                yOffset += textHeight + (textPaddingY * 2.f);
                                break;
                            }
                            case SettingType::SLIDER_S: {
                                SliderSettingBase* sliderSettingBase =
                                    static_cast<SliderSettingBase*>(setting);

                                if(sRectPos.contains(mousePos)) {
                                    if(isLeftClickDown) {
                                        draggingSliderSettingPtr = sliderSettingBase;
                                        isLeftClickDown = false;
                                    }
                                }

                                float minValue = 0.f, maxValue = 1.f, rawValue = 0.f;

                                // Unique animated value per slider
                                static std::unordered_map<void*, float> animatedMap;

                                if(sliderSettingBase->valueType == ValueType::INT_T) {
                                    SliderSetting<int>* intSlider =
                                        static_cast<SliderSetting<int>*>(sliderSettingBase);
                                    int& value = (*intSlider->valuePtr);

                                    minValue = (float)intSlider->minValue;
                                    maxValue = (float)intSlider->maxValue;

                                    if(draggingSliderSettingPtr == sliderSettingBase) {
                                        float draggingPercent = std::clamp(
                                            (mousePos.x - sRectPos.x) / (sRectPos.z - sRectPos.x),
                                            0.f, 1.f);
                                        value = (int)(minValue + std::round((maxValue - minValue) *
                                                                            draggingPercent));
                                    }

                                    rawValue = (float)value;
                                } else if(sliderSettingBase->valueType == ValueType::FLOAT_T) {
                                    SliderSetting<float>* floatSlider =
                                        static_cast<SliderSetting<float>*>(sliderSettingBase);
                                    float& value = (*floatSlider->valuePtr);

                                    minValue = floatSlider->minValue;
                                    maxValue = floatSlider->maxValue;

                                    if(draggingSliderSettingPtr == sliderSettingBase) {
                                        float draggingPercent = std::clamp(
                                            (mousePos.x - sRectPos.x) / (sRectPos.z - sRectPos.x),
                                            0.f, 1.f);
                                        value = minValue + (maxValue - minValue) * draggingPercent;
                                    }

                                    rawValue = value;
                                }

                                // Create if doesn't exist
                                if(!animatedMap.count(sliderSettingBase))
                                    animatedMap[sliderSettingBase] = rawValue;

                                float& animatedValue = animatedMap[sliderSettingBase];
                                animatedValue = lerpSync(animatedValue, rawValue, 0.08f);

                                if(std::abs(animatedValue - rawValue) < 0.01f)
                                    animatedValue = rawValue;

                                // Progress bar draw
                                float valuePercent = std::clamp(
                                    (animatedValue - minValue) / (maxValue - minValue), 0.f, 1.f);
                                Vec4<float> valueRectPos = Vec4<float>(
                                    sRectPos.x, sRectPos.y,
                                    sRectPos.x + (sRectPos.z - sRectPos.x) * valuePercent,
                                    sRectPos.w);

                                RenderUtil::fillRectangle(valueRectPos, mainColor);

                                // Value text draw
                                std::string valueTextStr;
                                if(sliderSettingBase->valueType == ValueType::INT_T)
                                    valueTextStr = std::to_string((int)rawValue);
                                else {
                                    char buf[10];
                                    sprintf_s(buf, "%.2f", rawValue);
                                    valueTextStr = buf;
                                }

                                Vec2<float> valueTextPos = Vec2<float>(
                                    sRectPos.z - textPaddingX -
                                        RenderUtil::getTextWidth(
                                            valueTextStr, textSize,
                                            (draggingSliderSettingPtr != sliderSettingBase)),
                                    sTextPos.y);

                                RenderUtil::drawText(sTextPos, settingName + ":",
                                                     UIColor(255, 255, 255), textSize);
                                RenderUtil::drawText(
                                    valueTextPos, valueTextStr, UIColor(255, 255, 255), textSize,
                                    (draggingSliderSettingPtr != sliderSettingBase));

                                yOffset += textHeight + (textPaddingY * 2.f);
                                break;
                            }
                        }
                        RenderUtil::fillRectangle(
                            sRectPos, UIColor(255, 255, 255, (int)(25 * setting->selectedAnim)));
                        yOffset += settingSpace;
                    }
                    float endY = yOffset - settingSpace;

                    float sLineWidth = borderWidth;
                    float sLinePaddingX = 1.f * textSize;
                    Vec4<float> sLineRect =
                        Vec4<float>(mRectPos.x + sLinePaddingX, startY,
                                    mRectPos.x + sLinePaddingX + sLineWidth, endY);

                    Vec4<float> sLineRectRight =
                        Vec4<float>(mRectPos.z - sLinePaddingX - sLineWidth, startY,
                                    mRectPos.z - sLinePaddingX, endY);
                }

                yOffset += moduleSpace;
                if(&mod == &window->moduleList.back()) {
                    float bottomLineHeight = 1.f;
                    Vec4<float> finalBottomLine =
                        Vec4<float>(mRectPos.x, yOffset, mRectPos.z, yOffset + bottomLineHeight);
                    // RenderUtil::fillRectangle(finalBottomLine, mainColor);
                    renderedThisFrame = true;
                }
            }
        }
    }

    if(showDescription && descriptionText != "NULL" && draggingWindowPtr == nullptr &&
       draggingSliderSettingPtr == nullptr) {
        Vec2<float> mousePadding = Vec2<float>(15.f, 15.f);
        Vec4<float> rectPos = Vec4<float>(
            mousePos.x + mousePadding.x, mousePos.y + mousePadding.y,
            mousePos.x + mousePadding.x + RenderUtil::getTextWidth(descriptionText, 0.8f) +
                2.f * 2.f,
            mousePos.y + mousePadding.y + RenderUtil::getTextHeight(descriptionText, 0.8f));

        Vec2<float> textPos = Vec2<float>(rectPos.x, rectPos.y).add(Vec2<float>(2.f, 0.f));
        RenderUtil::fillRectangle(rectPos, UIColor(0, 0, 0, 125));
        RenderUtil::drawText(textPos, descriptionText, UIColor(255, 255, 255), 0.8f);
    }

    isLeftClickDown = false;
    isRightClickDown = false;

    auto searchBox = ModuleManager::getModule<SearchBox>();
    if(searchBox && searchBox->isEnabled()) {
        searchBox->renderUI();
    }
    auto cs = ModuleManager::getModule<Configs>();
    if(cs && cs->isEnabled()) {
        cs->renderUI();
    }

    auto ai = ModuleManager::getModule<AIChat>();

    if(ai && ai->isEnabled()) {
        ai->renderUI();
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

    // anim += RenderUtil::deltaTime * (shouldUp ? 5.f : -2.f);
    if(shouldUp)
        anim = 1.f;
    else
        anim -= RenderUtil::deltaTime * 2.f;

    if(anim > 1.f)
        anim = 1.f;
    if(anim < 0.f)
        anim = 0.f;
}

using json = nlohmann::json;

void ClickGUI::onLoadConfig(void* confVoid) {
    json* conf = reinterpret_cast<json*>(confVoid);
    std::string modName = this->getModuleName();

    if(conf->contains(modName)) {
        json obj = conf->at(modName);
        if(obj.is_null())
            return;

        if(obj.contains("enabled")) {
            this->setEnabled(obj.at("enabled").get<bool>());
        }

        for(auto& setting : getSettingList()) {
            std::string settingName = setting->name;

            if(obj.contains(settingName)) {
                json confValue = obj.at(settingName);
                if(confValue.is_null())
                    continue;

                switch(setting->type) {
                    case SettingType::BOOL_S: {
                        BoolSetting* boolSetting = static_cast<BoolSetting*>(setting);
                        (*boolSetting->value) = confValue.get<bool>();
                        break;
                    }
                    case SettingType::KEYBIND_S: {
                        KeybindSetting* keybindSetting = static_cast<KeybindSetting*>(setting);
                        (*keybindSetting->value) = confValue.get<int>();
                        break;
                    }
                    case SettingType::ENUM_S: {
                        EnumSetting* enumSetting = static_cast<EnumSetting*>(setting);
                        (*enumSetting->value) = confValue.get<int>();
                        break;
                    }
                    case SettingType::COLOR_S: {
                        ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
                        (*colorSetting->colorPtr) =
                            ColorUtil::HexStringToColor(confValue.get<std::string>());
                        break;
                    }
                    case SettingType::SLIDER_S: {
                        SliderSettingBase* sliderSettingBase =
                            static_cast<SliderSettingBase*>(setting);
                        if(sliderSettingBase->valueType == ValueType::INT_T) {
                            SliderSetting<int>* intSlider =
                                static_cast<SliderSetting<int>*>(sliderSettingBase);
                            (*intSlider->valuePtr) = confValue.get<int>();
                        } else if(sliderSettingBase->valueType == ValueType::FLOAT_T) {
                            SliderSetting<float>* floatSlider =
                                static_cast<SliderSetting<float>*>(sliderSettingBase);
                            (*floatSlider->valuePtr) = confValue.get<float>();
                        }
                        break;
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
}

void ClickGUI::onSaveConfig(void* confVoid) {
    json* conf = reinterpret_cast<json*>(confVoid);
    std::string modName = this->getModuleName();
    json obj = (*conf)[modName];

    obj["enabled"] = this->isEnabled();

    for(auto& setting : getSettingList()) {
        std::string settingName = setting->name;

        switch(setting->type) {
            case SettingType::BOOL_S: {
                BoolSetting* boolSetting = static_cast<BoolSetting*>(setting);
                obj[settingName] = (*boolSetting->value);
                break;
            }
            case SettingType::KEYBIND_S: {
                KeybindSetting* keybindSetting = static_cast<KeybindSetting*>(setting);
                obj[settingName] = (*keybindSetting->value);
                break;
            }
            case SettingType::ENUM_S: {
                EnumSetting* enumSetting = static_cast<EnumSetting*>(setting);
                obj[settingName] = (*enumSetting->value);
                break;
            }
            case SettingType::COLOR_S: {
                ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
                obj[settingName] = ColorUtil::ColorToHexString((*colorSetting->colorPtr));
                break;
            }
            case SettingType::SLIDER_S: {
                SliderSettingBase* sliderSettingBase = static_cast<SliderSettingBase*>(setting);
                if(sliderSettingBase->valueType == ValueType::INT_T) {
                    SliderSetting<int>* intSlider =
                        static_cast<SliderSetting<int>*>(sliderSettingBase);
                    obj[settingName] = (*intSlider->valuePtr);
                } else if(sliderSettingBase->valueType == ValueType::FLOAT_T) {
                    SliderSetting<float>* floatSlider =
                        static_cast<SliderSetting<float>*>(sliderSettingBase);
                    obj[settingName] = (*floatSlider->valuePtr);
                }
                break;
            }
        }
    }

    for(auto& window : windowList) {
        obj[window->name]["isExtended"] = window->extended;
        obj[window->name]["pos"]["x"] = window->pos.x;
        obj[window->name]["pos"]["y"] = window->pos.y;
    }

    (*conf)[modName] = obj;
}