#pragma once
#include <d2d1.h>
#include <d2d1_3.h>
#include <stdint.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "../SDK/Core/mce.h"
#include "../Utils/TimerUtil.h"

struct UIColor {
    union {
        struct {
            uint8_t r, g, b, a;
        };
        int arr[4];
    };

    static UIColor FromHSV(float h, float s, float v, float a = 1.0f) {
        h = fmodf(h, 360.0f);
        if(h < 0)
            h += 360.0f;

        s = std::clamp(s, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);

        float c = v * s;
        float x = c * (1.0f - std::abs(fmodf(h / 60.0f, 2.0f) - 1.0f));
        float m = v - c;

        float r1, g1, b1;
        if(h < 60.0f) {
            r1 = c;
            g1 = x;
            b1 = 0;
        } else if(h < 120.0f) {
            r1 = x;
            g1 = c;
            b1 = 0;
        } else if(h < 180.0f) {
            r1 = 0;
            g1 = c;
            b1 = x;
        } else if(h < 240.0f) {
            r1 = 0;
            g1 = x;
            b1 = c;
        } else if(h < 300.0f) {
            r1 = x;
            g1 = 0;
            b1 = c;
        } else {
            r1 = c;
            g1 = 0;
            b1 = x;
        }

        return UIColor(static_cast<uint8_t>((r1 + m) * 255), static_cast<uint8_t>((g1 + m) * 255),
                       static_cast<uint8_t>((b1 + m) * 255), static_cast<uint8_t>(a * 255));
    }

    UIColor(uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255, uint8_t alpha = 255) {
        this->r = red;
        this->g = green;
        this->b = blue;
        this->a = alpha;
    }

    UIColor(const mce::Color& color) {
        this->r = static_cast<uint8_t>(std::clamp(color.r * 255.0f, 0.0f, 255.0f));
        this->g = static_cast<uint8_t>(std::clamp(color.g * 255.0f, 0.0f, 255.0f));
        this->b = static_cast<uint8_t>(std::clamp(color.b * 255.0f, 0.0f, 255.0f));
        this->a = static_cast<uint8_t>(std::clamp(color.a * 255.0f, 0.0f, 255.0f));
    }

    bool operator==(const UIColor& other) const {
        return (r == other.r && g == other.g && b == other.b && a == other.a);
    }

    D2D1_COLOR_F toD2D1Color() const {
        return D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    mce::Color toMCColor() const {
        return mce::Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }
};

namespace ColorUtil {
// Base utility functions
uint32_t ColorToUInt(const UIColor& color);
UIColor HexStringToColor(const std::string& hexString);
std::string ColorToHexString(const UIColor& color);
void ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b);

// Interpolation functions
UIColor lerp(const UIColor& start, const UIColor& end, float t);
UIColor lerpHSV(const UIColor& start, const UIColor& end, float t);

// Dynamic color generation
UIColor getRainbowColor(float time, float saturation = 1.0f, float brightness = 1.0f,
                        float speed = 1.0f);
UIColor getWaveColor(const UIColor& color1, const UIColor& color2, float time, float speed = 1.0f);
UIColor getBreathingColor(const UIColor& color, float time, float speed = 1.0f);
UIColor getPulseColor(const UIColor& color, float time, float intensity = 1.0f);
UIColor getRainbowColor2(float seconds, float saturation, float brightness, long index);

// Advanced color effects
UIColor getOceanWaveColor(float time, float phase = 0.0f);
UIColor getAquaGradient(float progress, float alpha = 1.0f);
UIColor getCrystalShine(float time, float phase = 0.0f);
UIColor getLiquidFlow(float time, float phase = 0.0f);
UIColor getPlasmaEffect(float time, float phase = 0.0f);
UIColor getSweepGradient(float progress, float baseHue = 200.0f);
UIColor getGlowPulse(const UIColor& baseColor, float time, float intensity = 1.0f);
UIColor getEnergyFlow(float time, float phase = 0.0f);
UIColor getNeonGlow(float time, float hue = 200.0f, float intensity = 1.0f);
UIColor getTwoColorFade(const UIColor& color1, const UIColor& color2, float time,
                        float speed = 1.0f, float offset = 0.0f);

// Multi-point gradients
UIColor getMultiGradient(const std::vector<UIColor>& colors, float progress);
UIColor getAquaThemeColor(int variant, float alpha = 1.0f);


UIColor lerpColor(UIColor color1, UIColor color2, float amount);
UIColor Rainbow(float seconds, float saturation, float brightness, int index);
UIColor getAstolfoRainbow(float seconds, float saturation, float brightness, int index);
UIColor getCustomColors(float seconds, int index,
                        const std::vector<std::pair<float, UIColor>>& colorMap);
UIColor getClientColor(float seconds, float saturation, float brightness, int index);

// D2D native gradient support
struct GradientStop {
    float position;
    UIColor color;
};

ID2D1LinearGradientBrush* createLinearGradientBrush(ID2D1RenderTarget* renderTarget,
                                                    const std::vector<GradientStop>& stops,
                                                    const D2D1_POINT_2F& startPoint,
                                                    const D2D1_POINT_2F& endPoint);

ID2D1RadialGradientBrush* createRadialGradientBrush(ID2D1RenderTarget* renderTarget,
                                                    const std::vector<GradientStop>& stops,
                                                    const D2D1_POINT_2F& center, float radiusX,
                                                    float radiusY);

// Preset gradients
std::vector<GradientStop> getAquaGradientStops(float alpha = 1.0f);
std::vector<GradientStop> getOceanGradientStops(float time, float alpha = 1.0f);
std::vector<GradientStop> getSunsetGradientStops(float alpha = 1.0f);
std::vector<GradientStop> getNeonGradientStops(float time, float alpha = 1.0f);
};  // namespace ColorUtil