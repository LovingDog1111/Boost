#include "ColorPicker.h"
#include <algorithm>
#include <cmath>
#include "../../../../ConfigManager/ConfigManager.h"


ColorPicker::ColorPicker() {
    // Constructor
}

ColorPicker::~ColorPicker() {
    // Destructor
}

void ColorPicker::show(ColorSetting* colorSetting, Vec2<float> pos) {
    if (!colorSetting) return;

    activeColorSetting = colorSetting;
    isVisible = true;
    position = pos;

    // Convert current color to HSV
    Vec3<float> hsv = RGBtoHSV(*colorSetting->colorPtr);
    hue = hsv.x;
    saturation = hsv.y;
    value = hsv.z;
    alpha = colorSetting->colorPtr->a / 255.f;


    targetHue = hue;
    targetSaturation = saturation;
    targetValue = value;
    targetAlpha = alpha;

    // Adjust position to stay within screen bounds
    Vec2<float> screenSize =GI::getGuiData()->windowSizeReal;
    adjustPosition(screenSize);
}

void ColorPicker::hide() {
    isVisible = false;
    activeColorSetting = nullptr;
    draggingColorArea = false;
    draggingHueBar = false;
    draggingAlphaBar = false;
}

bool ColorPicker::handleMouseInput(const Vec2<float>& mousePos, bool isLeftClick, bool isHolding) {
    if (!isVisible) return false;

    float scaledPickerSize = pickerSize * guiScale;
    float scaledPadding = padding * guiScale;
    float scaledHueBarWidth = hueBarWidth * guiScale;
    float scaledAlphaBarHeight = alphaBarHeight * guiScale;
    float scaledButtonHeight = buttonHeight * guiScale;

    // Calculate component rectangles
    Vec4<float> mainRect = Vec4<float>(
        position.x, position.y,
        position.x + scaledPickerSize + scaledHueBarWidth + scaledPadding * 3,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledButtonHeight + scaledPadding * 4
    );

    Vec4<float> colorArea = Vec4<float>(
        position.x + scaledPadding,
        position.y + scaledPadding,
        position.x + scaledPadding + scaledPickerSize,
        position.y + scaledPadding + scaledPickerSize
    );

    Vec4<float> hueBar = Vec4<float>(
        position.x + scaledPickerSize + scaledPadding * 2,
        position.y + scaledPadding,
        position.x + scaledPickerSize + scaledHueBarWidth + scaledPadding * 2,
        position.y + scaledPadding + scaledPickerSize
    );

    Vec4<float> alphaBar = Vec4<float>(
        position.x + scaledPadding,
        position.y + scaledPickerSize + scaledPadding * 2,
        position.x + scaledPickerSize + scaledPadding,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledPadding * 2
    );

    float buttonWidth = (scaledPickerSize - scaledPadding) / 2.f;
    Vec4<float> confirmButton = Vec4<float>(
        position.x + scaledPadding,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledPadding * 3,
        position.x + scaledPadding + buttonWidth,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledButtonHeight + scaledPadding * 3
    );

    Vec4<float> cancelButton = Vec4<float>(
        position.x + scaledPadding + buttonWidth + scaledPadding,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledPadding * 3,
        position.x + scaledPickerSize + scaledPadding,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledButtonHeight + scaledPadding * 3
    );

    // Handle mouse input
    if (isLeftClick) {
        // Check if clicking outside the color picker
        if (!isPointInRect(mousePos, mainRect)) {
            hide();
            return true;
        }

        // Handle button clicks
        if (handleButtonClick(mousePos, confirmButton, cancelButton)) {
            return true;
        }

        // Start dragging operations
        if (handleColorAreaClick(mousePos, colorArea)) {
            draggingColorArea = true;
            return true;
        }

        if (handleHueBarClick(mousePos, hueBar)) {
            draggingHueBar = true;
            return true;
        }

        if (handleAlphaBarClick(mousePos, alphaBar)) {
            draggingAlphaBar = true;
            return true;
        }
    }

    // Handle dragging
    if (isHolding) {
        if (draggingColorArea) {
            handleColorAreaClick(mousePos, colorArea);
            return true;
        }

        if (draggingHueBar) {
            handleHueBarClick(mousePos, hueBar);
            return true;
        }

        if (draggingAlphaBar) {
            handleAlphaBarClick(mousePos, alphaBar);
            return true;
        }
    }
    else {
        // Stop dragging when mouse is released
        draggingColorArea = false;
        draggingHueBar = false;
        draggingAlphaBar = false;
    }

    // If we're inside the color picker, consume the event
    return isPointInRect(mousePos, mainRect);
}

void ColorPicker::render(float alpha_) {
    if (!isVisible) return;

    // --- 动画插值 ---
    bool changed = false;
#define LERP(a, b, t) ((a) + ((b)-(a)) * (t))
    float oldHue = hue, oldSaturation = saturation, oldValue = value, oldAlpha = alpha;
    hue = LERP(hue, targetHue, animSpeed);
    saturation = LERP(saturation, targetSaturation, animSpeed);
    value = LERP(value, targetValue, animSpeed);
    alpha = LERP(alpha, targetAlpha, animSpeed);
    if (std::abs(hue - oldHue) > 0.001f ||
        std::abs(saturation - oldSaturation) > 0.001f ||
        std::abs(value - oldValue) > 0.001f ||
        std::abs(alpha - oldAlpha) > 0.001f) {
        changed = true;
    }
    if (changed) updateColorFromHSV();

    float scaledPickerSize = pickerSize * guiScale;
    float scaledPadding = padding * guiScale;
    float scaledHueBarWidth = hueBarWidth * guiScale;
    float scaledAlphaBarHeight = alphaBarHeight * guiScale;
    float scaledButtonHeight = buttonHeight * guiScale;

    // Main background
    Vec4<float> mainRect = Vec4<float>(
        position.x, position.y,
        position.x + scaledPickerSize + scaledHueBarWidth + scaledPadding * 3,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledButtonHeight + scaledPadding * 4
    );

    renderBackground(mainRect, alpha_);

    // Color area
    Vec4<float> colorArea = Vec4<float>(
        position.x + scaledPadding,
        position.y + scaledPadding,
        position.x + scaledPadding + scaledPickerSize,
        position.y + scaledPadding + scaledPickerSize
    );
    renderColorArea(colorArea, alpha_);

    // Hue bar
    Vec4<float> hueBar = Vec4<float>(
        position.x + scaledPickerSize + scaledPadding * 2,
        position.y + scaledPadding,
        position.x + scaledPickerSize + scaledHueBarWidth + scaledPadding * 2,
        position.y + scaledPadding + scaledPickerSize
    );
    renderHueBar(hueBar, alpha_);

    // Alpha bar
    Vec4<float> alphaBar = Vec4<float>(
        position.x + scaledPadding,
        position.y + scaledPickerSize + scaledPadding * 2,
        position.x + scaledPickerSize + scaledPadding,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledPadding * 2
    );
    renderAlphaBar(alphaBar, alpha_);

    // Buttons
    float buttonWidth = (scaledPickerSize - scaledPadding) / 2.f;
    Vec4<float> confirmButton = Vec4<float>(
        position.x + scaledPadding,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledPadding * 3,
        position.x + scaledPadding + buttonWidth,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledButtonHeight + scaledPadding * 3
    );

    Vec4<float> cancelButton = Vec4<float>(
        position.x + scaledPadding + buttonWidth + scaledPadding,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledPadding * 3,
        position.x + scaledPickerSize + scaledPadding,
        position.y + scaledPickerSize + scaledAlphaBarHeight + scaledButtonHeight + scaledPadding * 3
    );

    renderButtons(confirmButton, cancelButton, alpha_);
}

// Helper function implementations

UIColor ColorPicker::HSVtoRGB(float h, float s, float v, float a) {
    float r, g, b;
    ColorUtil::ColorConvertHSVtoRGB(h / 360.f, s, v, r, g, b);
    return UIColor((int)(r * 255), (int)(g * 255), (int)(b * 255), (int)(a * 255));
}

Vec3<float> ColorPicker::RGBtoHSV(const UIColor& color) {
    float r = color.r / 255.f;
    float g = color.g / 255.f;
    float b = color.b / 255.f;

    float max = std::max({ r, g, b });
    float min = std::min({ r, g, b });
    float delta = max - min;

    float h = 0, s = 0, v = max;

    if (delta != 0) {
        s = delta / max;

        if (max == r) {
            h = 60 * fmod(((g - b) / delta), 6);
        }
        else if (max == g) {
            h = 60 * (((b - r) / delta) + 2);
        }
        else {
            h = 60 * (((r - g) / delta) + 4);
        }

        if (h < 0) h += 360;
    }

    return Vec3<float>(h, s, v);
}

void ColorPicker::updateColorFromHSV() {
    if (!activeColorSetting) return;

    UIColor newColor = HSVtoRGB(hue, saturation, value, alpha);
    *activeColorSetting->colorPtr = newColor;

    // Auto-save configuration when color changes
    ConfigManager::saveConfig();
}

bool ColorPicker::isPointInRect(const Vec2<float>& point, const Vec4<float>& rect) {
    return point.x >= rect.x && point.x <= rect.z && point.y >= rect.y && point.y <= rect.w;
}

void ColorPicker::adjustPosition(const Vec2<float>& screenSize) {
    float totalWidth = pickerSize * guiScale + hueBarWidth * guiScale + padding * guiScale * 3;
    float totalHeight = pickerSize * guiScale + alphaBarHeight * guiScale + buttonHeight * guiScale + padding * guiScale * 4;

    if (position.x + totalWidth > screenSize.x) {
        position.x = screenSize.x - totalWidth - 10.f;
    }
    if (position.y + totalHeight > screenSize.y) {
        position.y = screenSize.y - totalHeight - 10.f;
    }

    if (position.x < 10.f) position.x = 10.f;
    if (position.y < 10.f) position.y = 10.f;
}

// Render component implementations

void ColorPicker::renderBackground(const Vec4<float>& rect, float alpha) {
    // Main background
    UIColor bgColor = UIColor(30, 35, 40, (int)(240 * alpha));
   RenderUtil::fillRoundedRectangle(rect, bgColor, 8.f);

    // Border
    UIColor borderColor = UIColor(60, 65, 70, (int)(180 * alpha));
   RenderUtil::drawRoundedRectangle(rect, borderColor, 8.f, 1.f);
}

void ColorPicker::renderColorArea(const Vec4<float>& rect, float alpha) {
    // Draw color gradient (simplified approach using rectangles)
    float step = 2.f * guiScale;

    for (float x = rect.x; x < rect.z; x += step) {
        for (float y = rect.y; y < rect.w; y += step) {
            float s = (x - rect.x) / (rect.z - rect.x);
            float v = 1.f - (y - rect.y) / (rect.w - rect.y);

            UIColor pixelColor = HSVtoRGB(hue, s, v, alpha);
            Vec4<float> pixelRect = Vec4<float>(x, y, x + step, y + step);
           RenderUtil::fillRectangle(pixelRect, pixelColor);
        }
    }

    // Draw selection indicator
    Vec2<float> selectorPos = Vec2<float>(
        rect.x + saturation * (rect.z - rect.x),
        rect.y + (1.f - value) * (rect.w - rect.y)
    );

    // Outer circle (black)
   RenderUtil::drawCircle(selectorPos, UIColor(0, 0, 0, (int)(200 * alpha)), 6.f, 2.f);
    // Inner circle (white)
   RenderUtil::drawCircle(selectorPos, UIColor(255, 255, 255, (int)(255 * alpha)), 4.f, 2.f);
}

void ColorPicker::renderHueBar(const Vec4<float>& rect, float alpha) {
    // Draw hue gradient
    float step = 2.f * guiScale;

    for (float y = rect.y; y < rect.w; y += step) {
        float h = (y - rect.y) / (rect.w - rect.y) * 360.f;
        UIColor hueColor = HSVtoRGB(h, 1.f, 1.f, alpha);
        Vec4<float> hueRect = Vec4<float>(rect.x, y, rect.z, y + step);
       RenderUtil::fillRectangle(hueRect, hueColor);
    }

    // Draw selection indicator
    Vec2<float> hueIndicator = Vec2<float>(
        rect.x + (rect.z - rect.x) / 2.f,
        rect.y + (hue / 360.f) * (rect.w - rect.y)
    );

    // Indicator line
   RenderUtil::drawLine(
        Vec2<float>(rect.x - 2.f, hueIndicator.y),
        Vec2<float>(rect.z + 2.f, hueIndicator.y),
        UIColor(255, 255, 255, (int)(255 * alpha)), 2.f
    );
   RenderUtil::drawLine(
        Vec2<float>(rect.x - 1.f, hueIndicator.y),
        Vec2<float>(rect.z + 1.f, hueIndicator.y),
        UIColor(0, 0, 0, (int)(150 * alpha)), 1.f
    );
}

void ColorPicker::renderAlphaBar(const Vec4<float>& rect, float alpha) {
    // Draw checkered background for alpha
    float checkSize = 8.f * guiScale;
    for (float x = rect.x; x < rect.z; x += checkSize) {
        for (float y = rect.y; y < rect.w; y += checkSize) {
            int checkX = (int)((x - rect.x) / checkSize);
            int checkY = (int)((y - rect.y) / checkSize);
            UIColor checkColor = ((checkX + checkY) % 2 == 0) ? UIColor(200, 200, 200) : UIColor(150, 150, 150);
            checkColor.a = (int)(checkColor.a * alpha);

            Vec4<float> checkRect = Vec4<float>(x, y, std::min(x + checkSize, rect.z), std::min(y + checkSize, rect.w));
           RenderUtil::fillRectangle(checkRect, checkColor);
        }
    }

    // Draw alpha gradient
    float step = 2.f * guiScale;
    for (float x = rect.x; x < rect.z; x += step) {
        float a = (x - rect.x) / (rect.z - rect.x);
        UIColor alphaColor = HSVtoRGB(hue, saturation, value, a * alpha);
        Vec4<float> alphaRect = Vec4<float>(x, rect.y, x + step, rect.w);
       RenderUtil::fillRectangle(alphaRect, alphaColor);
    }

    // Draw selection indicator
    Vec2<float> alphaIndicator = Vec2<float>(
        rect.x + this->alpha * (rect.z - rect.x),
        rect.y + (rect.w - rect.y) / 2.f
    );

    // Indicator line
   RenderUtil::drawLine(
        Vec2<float>(alphaIndicator.x, rect.y - 2.f),
        Vec2<float>(alphaIndicator.x, rect.w + 2.f),
        UIColor(255, 255, 255, (int)(255 * alpha)), 2.f
    );
   RenderUtil::drawLine(
        Vec2<float>(alphaIndicator.x, rect.y - 1.f),
        Vec2<float>(alphaIndicator.x, rect.w + 1.f),
        UIColor(0, 0, 0, (int)(150 * alpha)), 1.f
    );
}

void ColorPicker::renderButtons(const Vec4<float>& confirmRect, const Vec4<float>& cancelRect, float alpha) {
    // Confirm button
    UIColor confirmColor = UIColor(50, 150, 50, (int)(200 * alpha));
   RenderUtil::fillRoundedRectangle(confirmRect, confirmColor, 4.f);

    // Cancel button
    UIColor cancelColor = UIColor(150, 50, 50, (int)(200 * alpha));
   RenderUtil::fillRoundedRectangle(cancelRect, cancelColor, 4.f);

    // Button text
    UIColor textColor = UIColor(255, 255, 255, (int)(255 * alpha));

    // Confirm text
    std::string confirmText = "OK";
    Vec2<float> confirmTextPos = Vec2<float>(
        confirmRect.x + (confirmRect.z - confirmRect.x -RenderUtil::getTextWidth(confirmText, 0.7f)) / 2.f,
        confirmRect.y + (confirmRect.w - confirmRect.y -RenderUtil::getTextHeight(confirmText, 0.7f)) / 2.f
    );
   RenderUtil::drawText(confirmTextPos, confirmText, textColor, 0.7f);

    // Cancel text
    std::string cancelText = "Cancel";
    Vec2<float> cancelTextPos = Vec2<float>(
        cancelRect.x + (cancelRect.z - cancelRect.x -RenderUtil::getTextWidth(cancelText, 0.7f)) / 2.f,
        cancelRect.y + (cancelRect.w - cancelRect.y -RenderUtil::getTextHeight(cancelText, 0.7f)) / 2.f
    );
   RenderUtil::drawText(cancelTextPos, cancelText, textColor, 0.7f);
}

// Interaction helper implementations

bool ColorPicker::handleColorAreaClick(const Vec2<float>& mousePos, const Vec4<float>& colorArea) {
    if (!isPointInRect(mousePos, colorArea)) return false;

    targetSaturation = (mousePos.x - colorArea.x) / (colorArea.z - colorArea.x);
    targetValue = 1.f - (mousePos.y - colorArea.y) / (colorArea.w - colorArea.y);

    targetSaturation = std::max(0.f, std::min(1.f, targetSaturation));
    targetValue = std::max(0.f, std::min(1.f, targetValue));

    return true;
}

bool ColorPicker::handleHueBarClick(const Vec2<float>& mousePos, const Vec4<float>& hueBar) {
    if (!isPointInRect(mousePos, hueBar)) return false;

    targetHue = (mousePos.y - hueBar.y) / (hueBar.w - hueBar.y) * 360.f;
    targetHue = std::max(0.f, std::min(360.f, targetHue));

    // 不直接 updateColorFromHSV
    return true;
}

bool ColorPicker::handleAlphaBarClick(const Vec2<float>& mousePos, const Vec4<float>& alphaBar) {
    if (!isPointInRect(mousePos, alphaBar)) return false;

    targetAlpha = (mousePos.x - alphaBar.x) / (alphaBar.z - alphaBar.x);
    targetAlpha = std::max(0.f, std::min(1.f, targetAlpha));

    // 不直接 updateColorFromHSV
    return true;
}

bool ColorPicker::handleButtonClick(const Vec2<float>& mousePos, const Vec4<float>& confirmRect, const Vec4<float>& cancelRect) {
    if (isPointInRect(mousePos, confirmRect)) {
        // Confirm button clicked - apply color and hide
        updateColorFromHSV();
        hide();
        return true;
    }

    if (isPointInRect(mousePos, cancelRect)) {
        // Cancel button clicked - just hide without applying
        hide();
        return true;
    }

    return false;
}
