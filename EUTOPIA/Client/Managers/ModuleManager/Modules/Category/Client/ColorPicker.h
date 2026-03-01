#pragma once
#include "../../ModuleBase/Settings/ColorSetting.h"
#include "../../../../../../SDK/GlobalInstance.h"
#include "../../../../../../Utils/RenderUtil.h"

class ColorPicker {
private:
    // State variables
    ColorSetting* activeColorSetting = nullptr;
    bool isVisible = false;
    Vec2<float> position = Vec2<float>(0.f, 0.f);

    // Color picker dimensions
    float pickerSize = 200.f;
    float hueBarWidth = 15.f;
    float alphaBarHeight = 15.f;
    float buttonHeight = 25.f;
    float padding = 10.f;

    // Color state (HSV format for easier manipulation)
    float hue = 0.f;        // 0-360
    float saturation = 1.f; // 0-1
    float value = 1.f;      // 0-1
    float alpha = 1.f;      // 0-1


    float targetHue = 0.f;
    float targetSaturation = 0.f;
    float targetValue = 0.f;
    float targetAlpha = 1.f;
    float animSpeed = 0.25f;

    // Interaction state
    bool draggingColorArea = false;
    bool draggingHueBar = false;
    bool draggingAlphaBar = false;

    // Helper functions
    UIColor HSVtoRGB(float h, float s, float v, float a = 1.f);
    Vec3<float> RGBtoHSV(const UIColor& color);
    void updateColorFromHSV();
    bool isPointInRect(const Vec2<float>& point, const Vec4<float>& rect);

public:
    ColorPicker();
    ~ColorPicker();

    // Main interface
    void show(ColorSetting* colorSetting, Vec2<float> pos);
    void hide();
    bool isActive() const { return isVisible; }

    // Event handling
    bool handleMouseInput(const Vec2<float>& mousePos, bool isLeftClick, bool isHolding);

    // Rendering
    void render(float alpha = 1.f);

    // Utility
    void setGuiScale(float scale) { guiScale = scale; }

private:
    float guiScale = 1.f;

    // Render components
    void renderColorArea(const Vec4<float>& rect, float alpha);
    void renderHueBar(const Vec4<float>& rect, float alpha);
    void renderAlphaBar(const Vec4<float>& rect, float alpha);
    void renderButtons(const Vec4<float>& confirmRect, const Vec4<float>& cancelRect, float alpha);
    void renderBackground(const Vec4<float>& rect, float alpha);

    // Interaction helpers
    bool handleColorAreaClick(const Vec2<float>& mousePos, const Vec4<float>& colorArea);
    bool handleHueBarClick(const Vec2<float>& mousePos, const Vec4<float>& hueBar);
    bool handleAlphaBarClick(const Vec2<float>& mousePos, const Vec4<float>& alphaBar);
    bool handleButtonClick(const Vec2<float>& mousePos, const Vec4<float>& confirmRect, const Vec4<float>& cancelRect);

    // Position adjustment
    void adjustPosition(const Vec2<float>& screenSize);
};