#pragma once
#include "../../ModuleBase/Module.h"

class ClickGUI : public Module {
   private:
    struct ClickWindow {
        std::string name;
        Category category;
        Vec2<float> pos;
        bool extended = true;

        // Animation
        float selectedAnim = 0.f;

        std::vector<Module*> moduleList;

        ClickWindow(std::string windowName, Vec2<float> startPos, Category c);
    };

    std::vector<ClickWindow*> windowList;
    float openAnim = 0.0f;
    bool initialized = false;

    Vec2<float> mousePos = Vec2<float>(0.f, 0.f);
    Vec2<float> mouseDelta = Vec2<float>(0.f, 0.f);

    bool isLeftClickDown = false;
    bool isHoldingLeftClick = false;
    bool isRightClickDown = false;
    bool isHoldingRightClick = false;

    ClickWindow* draggingWindowPtr = nullptr;

    KeybindSetting* capturingKbSettingPtr = nullptr;
    SliderSettingBase* draggingSliderSettingPtr = nullptr;

    void updateSelectedAnimRect(Vec4<float>& rect, float& anim);

   private:
    UIColor mainColor = UIColor(255, 0, 0, 125);
    float blurStrength = 5.f;
    bool showDescription = true;
    bool yourBoolVariable;
    float topY = 0.f;
    float bottomY = 0.f;


   public:
    ClickGUI();
    ~ClickGUI();

    bool renderedThisFrame = false;
    void onDisable() override;
    void onEnable() override;
    bool isVisible() override;
    void onKeyUpdate(int key, bool isDown) override;
    void onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown);

    void InitClickGUI();
    void Render();

    void onLoadConfig(void* conf) override;
    void onSaveConfig(void* conf) override;
};