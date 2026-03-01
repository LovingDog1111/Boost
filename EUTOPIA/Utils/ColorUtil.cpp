#include "ColorUtil.h"

#include <math.h>

#include <iomanip>
#include <sstream>

#include "TimerUtil.h"

uint32_t ColorUtil::ColorToUInt(const UIColor& color) {
    return (static_cast<uint32_t>(color.a) << 24) | (static_cast<uint32_t>(color.r) << 16) |
           (static_cast<uint32_t>(color.g) << 8) | static_cast<uint32_t>(color.b);
}

std::string ColorUtil::ColorToHexString(const UIColor& color) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.r)
       << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.g) << std::hex
       << std::setw(2) << std::setfill('0') << static_cast<int>(color.b) << std::hex << std::setw(2)
       << std::setfill('0') << static_cast<int>(color.a);
    return ss.str();
}

UIColor ColorUtil::HexStringToColor(const std::string& hexString) {
    std::string hex = hexString;
    if(hex.substr(0, 2) == "0x") {
        hex = hex.substr(2);
    }

    if(hex.length() != 8) {
        return UIColor(0, 0, 0, 255);
    }

    try {
        uint32_t colorValue = std::stoul(hex, nullptr, 16);
        return UIColor(static_cast<uint8_t>((colorValue >> 24) & 0xFF),
                       static_cast<uint8_t>((colorValue >> 16) & 0xFF),
                       static_cast<uint8_t>((colorValue >> 8) & 0xFF),
                       static_cast<uint8_t>(colorValue & 0xFF));
    } catch(...) {
        return UIColor(0, 0, 0, 255);
    }
}

void ColorUtil::ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g,
                                     float& out_b) {
    if(s == 0.0f) {
        out_r = out_g = out_b = v;
        return;
    }

    h = fmodf(h, 1.0f) / (60.0f / 360.0f);
    int i = (int)h;
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch(i) {
        case 0:
            out_r = v;
            out_g = t;
            out_b = p;
            break;
        case 1:
            out_r = q;
            out_g = v;
            out_b = p;
            break;
        case 2:
            out_r = p;
            out_g = v;
            out_b = t;
            break;
        case 3:
            out_r = p;
            out_g = q;
            out_b = v;
            break;
        case 4:
            out_r = t;
            out_g = p;
            out_b = v;
            break;
        case 5:
        default:
            out_r = v;
            out_g = p;
            out_b = q;
            break;
    }
}

UIColor ColorUtil::lerp(const UIColor& start, const UIColor& end, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return UIColor(static_cast<uint8_t>(start.r + (end.r - start.r) * t),
                   static_cast<uint8_t>(start.g + (end.g - start.g) * t),
                   static_cast<uint8_t>(start.b + (end.b - start.b) * t),
                   static_cast<uint8_t>(start.a + (end.a - start.a) * t));
}

UIColor ColorUtil::lerpHSV(const UIColor& start, const UIColor& end, float t) {
    auto rgbToHsv = [](const UIColor& color) -> std::tuple<float, float, float> {
        float r = color.r / 255.0f;
        float g = color.g / 255.0f;
        float b = color.b / 255.0f;

        float max = std::max({r, g, b});
        float min = std::min({r, g, b});
        float delta = max - min;

        float h = 0, s = 0, v = max;

        if(delta != 0) {
            s = delta / max;
            if(max == r)
                h = 60 * fmodf((g - b) / delta, 6);
            else if(max == g)
                h = 60 * ((b - r) / delta + 2);
            else
                h = 60 * ((r - g) / delta + 4);
        }

        if(h < 0)
            h += 360;
        return {h, s, v};
    };

    auto [h1, s1, v1] = rgbToHsv(start);
    auto [h2, s2, v2] = rgbToHsv(end);

    float dh = h2 - h1;
    if(dh > 180)
        dh -= 360;
    else if(dh < -180)
        dh += 360;

    float h = h1 + dh * t;
    float s = s1 + (s2 - s1) * t;
    float v = v1 + (v2 - v1) * t;
    float a = (start.a + (end.a - start.a) * t) / 255.0f;

    return UIColor::FromHSV(h, s, v, a);
}

UIColor ColorUtil::getRainbowColor(float time, float saturation, float brightness, float speed) {
    float hue = fmodf(time * speed * 60.0f, 360.0f);
    return UIColor::FromHSV(hue, saturation, brightness);
}

UIColor ColorUtil::getWaveColor(const UIColor& color1, const UIColor& color2, float time,
                                float speed) {
    float wave = (sinf(time * speed * 6.28318f) + 1.0f) * 0.5f;
    return lerp(color1, color2, wave);
}

UIColor ColorUtil::getBreathingColor(const UIColor& color, float time, float speed) {
    float alpha = (sinf(time * speed * 6.28318f) + 1.0f) * 0.5f;
    UIColor result = color;
    result.a = static_cast<uint8_t>(result.a * alpha);
    return result;
}

UIColor ColorUtil::getRainbowColor2(float seconds, float saturation, float brightness, long index) {
    float currentHue =
        (((TimerUtil::getCurrentMs() + index) % (int)(seconds * 1000)) / (float)(seconds * 1000));
    float red, green, blue = 0.0f;
    ColorConvertHSVtoRGB(currentHue, saturation, brightness, red, green, blue);

    return UIColor((int)(red * 255.f), (int)(green * 255.f), (int)(blue * 255.f));
}

UIColor ColorUtil::getPulseColor(const UIColor& color, float time, float intensity) {
    float pulse = (sinf(time * 8.0f) + 1.0f) * 0.5f;
    pulse = powf(pulse, 2.0f) * intensity;

    return UIColor(std::min(255, static_cast<int>(color.r + pulse * 100)),
                   std::min(255, static_cast<int>(color.g + pulse * 100)),
                   std::min(255, static_cast<int>(color.b + pulse * 100)), color.a);
}

UIColor ColorUtil::getOceanWaveColor(float time, float phase) {
    float wave1 = sinf(time * 2.0f + phase) * 0.3f + 0.7f;
    float wave2 = sinf(time * 3.2f + phase + 1.57f) * 0.2f + 0.8f;
    float wave3 = sinf(time * 1.8f + phase + 3.14f) * 0.15f + 0.85f;

    float h = 200.0f + sinf(time * 0.5f + phase) * 30.0f;
    float s = 0.6f + wave1 * 0.4f;
    float v = 0.7f + wave2 * 0.3f;

    return UIColor::FromHSV(h, s, v);
}

UIColor ColorUtil::getAquaGradient(float progress, float alpha) {
    progress = std::clamp(progress, 0.0f, 1.0f);

    const std::vector<UIColor> keyColors = {UIColor(15, 82, 186, 255), UIColor(0, 150, 255, 255),
                                            UIColor(64, 224, 255, 255), UIColor(173, 216, 230, 255),
                                            UIColor(240, 248, 255, 255)};

    return getMultiGradient(keyColors, progress);
}

UIColor ColorUtil::getCrystalShine(float time, float phase) {
    float shimmer = powf(sinf(time * 4.0f + phase), 4.0f);
    float base = 0.4f + shimmer * 0.6f;

    return UIColor(static_cast<uint8_t>(150 * base + shimmer * 105),
                   static_cast<uint8_t>(200 * base + shimmer * 55),
                   static_cast<uint8_t>(255 * base), 255);
}

UIColor ColorUtil::getLiquidFlow(float time, float phase) {
    float flow1 = sinf(time * 2.5f + phase) * 0.4f + 0.6f;
    float flow2 = sinf(time * 3.7f + phase + 2.0f) * 0.3f + 0.7f;
    float turbulence = sinf(time * 8.0f + phase) * 0.1f + 0.9f;

    float h = 190.0f + flow1 * 40.0f;
    float s = 0.7f + flow2 * 0.3f;
    float v = 0.8f * turbulence;

    return UIColor::FromHSV(h, s, v);
}

UIColor ColorUtil::getPlasmaEffect(float time, float phase) {
    float x = sinf(time * 3.0f + phase);
    float y = sinf(time * 2.0f + phase + 1.57f);
    float z = sinf(time * 4.0f + phase + 3.14f);

    float r = (sinf(x + y) + sinf(y + z) + sinf(z + x)) / 3.0f;

    float hue = fmodf((r + 1.0f) * 180.0f + 180.0f, 360.0f);
    return UIColor::FromHSV(hue, 0.8f, 0.9f);
}

UIColor ColorUtil::getSweepGradient(float progress, float baseHue) {
    float hue = fmodf(baseHue + progress * 120.0f, 360.0f);
    float saturation = 0.7f + sinf(progress * 12.56f) * 0.3f;
    float brightness = 0.8f + sinf(progress * 18.84f) * 0.2f;

    return UIColor::FromHSV(hue, saturation, brightness);
}

UIColor ColorUtil::getGlowPulse(const UIColor& baseColor, float time, float intensity) {
    float pulse = powf((sinf(time * 6.0f) + 1.0f) * 0.5f, 2.0f);
    float glowFactor = 1.0f + pulse * intensity * 0.8f;

    return UIColor(std::min(255, static_cast<int>(baseColor.r * glowFactor)),
                   std::min(255, static_cast<int>(baseColor.g * glowFactor)),
                   std::min(255, static_cast<int>(baseColor.b * glowFactor)), baseColor.a);
}

UIColor ColorUtil::getEnergyFlow(float time, float phase) {
    float energy = sinf(time * 5.0f + phase) * 0.5f + 0.5f;
    energy = powf(energy, 0.5f);

    float h = 220.0f - energy * 20.0f;
    float s = 1.0f - energy * 0.3f;
    float v = 0.6f + energy * 0.4f;

    return UIColor::FromHSV(h, s, v);
}

UIColor ColorUtil::getNeonGlow(float time, float hue, float intensity) {
    float glow = (sinf(time * 7.0f) + 1.0f) * 0.5f;
    glow = powf(glow, 3.0f) * intensity;

    float s = 0.8f + glow * 0.2f;
    float v = 0.7f + glow * 0.3f;

    return UIColor::FromHSV(hue, s, v);
}

UIColor ColorUtil::getMultiGradient(const std::vector<UIColor>& colors, float progress) {
    if(colors.empty())
        return UIColor();
    if(colors.size() == 1)
        return colors[0];

    progress = std::clamp(progress, 0.0f, 1.0f);
    float scaledProgress = progress * (colors.size() - 1);
    int index = static_cast<int>(scaledProgress);
    float localProgress = scaledProgress - index;

    if(index >= static_cast<int>(colors.size()) - 1) {
        return colors.back();
    }

    return lerp(colors[index], colors[index + 1], localProgress);
}

UIColor ColorUtil::getAquaThemeColor(int variant, float alpha) {
    const std::vector<UIColor> aquaColors = {UIColor(64, 158, 255),  UIColor(0, 191, 255),
                                             UIColor(30, 144, 255),  UIColor(135, 206, 250),
                                             UIColor(173, 216, 230), UIColor(224, 255, 255),
                                             UIColor(100, 149, 237), UIColor(70, 130, 180),
                                             UIColor(176, 196, 222), UIColor(95, 158, 160)};

    int index = std::clamp(variant, 0, static_cast<int>(aquaColors.size()) - 1);
    UIColor color = aquaColors[index];
    color.a = static_cast<uint8_t>(alpha * 255);
    return color;
}

UIColor ColorUtil::getTwoColorFade(const UIColor& color1, const UIColor& color2, float time,
                                   float speed, float offset) {
    float wave1 = sinf(time * speed * 6.28318f + offset) * 0.5f + 0.5f;
    float wave2 = sinf(time * speed * 1.5f * 6.28318f + offset * 2.0f) * 0.5f + 0.5f;
    float wave3 = sinf(time * speed * 0.75f * 6.28318f + offset * 0.5f) * 0.5f + 0.5f;

    float combinedWave = (wave1 + wave2 + wave3) / 3.0f;

    return lerp(color1, color2, combinedWave);
}

// New functions from the provided code
UIColor ColorUtil::lerpColor(UIColor color1, UIColor color2, float amount) {
    amount = std::min(1.f, std::max(0.f, amount));
    return UIColor(
        Math::lerpInt(color1.r, color2.r, amount), Math::lerpInt(color1.g, color2.g, amount),
        Math::lerpInt(color1.b, color2.b, amount), Math::lerpInt(color1.a, color2.a, amount));
}

UIColor ColorUtil::Rainbow(float seconds, float saturation, float brightness, int index) {
    float hue =
        ((TimerUtil::getCurrentMs() + index) % (int)(seconds * 1000)) / (float)(seconds * 1000);
    float r, g, b = 0;
    ColorConvertHSVtoRGB(hue, saturation, brightness, r, g, b);
    return UIColor(r * 255, g * 255, b * 255);
}

UIColor ColorUtil::getAstolfoRainbow(float seconds, float saturation, float brightness, int index) {
    float hue = 0;
    float speed = seconds * 1450.f;
    float tempHue = (float)((TimerUtil::getCurrentMs() + index) % (int)speed);
    while(tempHue > speed)
        tempHue -= speed;
    tempHue /= speed;
    if(tempHue > 0.5)
        tempHue = 0.5F - (tempHue - 0.5f);
    tempHue += 0.5F;
    hue = tempHue;

    float r, g, b = 0;
    ColorConvertHSVtoRGB(hue, saturation, brightness, r, g, b);
    return UIColor(r * 240, g * 240, b * 240);
}

UIColor ColorUtil::getCustomColors(float seconds, int index,
                                   const std::vector<std::pair<float, UIColor>>& colorMap) {
    float hue =
        ((TimerUtil::getCurrentMs() + index) % (int)(seconds * 1000)) / (float)(seconds * 1000);

    UIColor currentColor = UIColor(0, 0, 0);
    for(int i = 0; i < colorMap.size() - 1; i++) {
        if(hue >= colorMap[i].first && hue <= colorMap[i + 1].first) {
            float amount = (hue - colorMap[i].first) / (colorMap[i + 1].first - colorMap[i].first);
            currentColor = lerpColor(colorMap[i].second, colorMap[i + 1].second, amount);
            break;
        }
    }
    return UIColor(currentColor.r, currentColor.g, currentColor.b);
}

UIColor ColorUtil::getClientColor(float seconds, float saturation, float brightness, int index) {
    std::vector<std::pair<float, UIColor>> colorMap;

    
    int clientColorMode = 0;  

    switch(clientColorMode) {
        case 0:  // Blue and Green
            colorMap = {{0.0f, UIColor(65, 201, 163)},
                        {0.5f, UIColor(0, 13, 175)},
                        {1.0f, UIColor(65, 201, 163)}};
            return getCustomColors(seconds, index * 2, colorMap);
        case 1:  // Astolfo
            return getAstolfoRainbow(seconds, saturation * 0.5f, brightness, index * 2);
        case 2:  // Bubble Gum
            colorMap = {{0.0f, UIColor(255, 192, 203)}, {0.2f, UIColor(255, 153, 204)},
                        {0.4f, UIColor(255, 102, 178)}, {0.6f, UIColor(255, 51, 180)},
                        {0.8f, UIColor(255, 0, 128)},   {1.0f, UIColor(255, 192, 203)}};
            return getCustomColors(seconds, index * 2, colorMap);
        case 3:  // Rainbow
            return Rainbow(seconds, saturation, brightness, index * 2);
        case 4:  // Tenacity
            colorMap = {{0.0f, UIColor(236, 133, 209)},
                        {0.5f, UIColor(28, 167, 222)},
                        {1.0f, UIColor(236, 133, 209)}};
            return getCustomColors(seconds, index * 2, colorMap);
        case 5:  // Wave
            colorMap = {{0.0f, UIColor(33, 33, 33)},
                        {0.5f, UIColor(255, 255, 255)},
                        {1.0f, UIColor(33, 33, 33)}};
            return getCustomColors(seconds, index * 2, colorMap);
        case 6:  // Cherry
            colorMap = {{0.0f, UIColor(139, 0, 0)},
                        {0.25f, UIColor(205, 92, 92)},
                        {0.5f, UIColor(220, 20, 60)},
                        {0.75f, UIColor(255, 36, 0)},
                        {1.0f, UIColor(139, 0, 0)}};
            return getCustomColors(seconds, index * 2, colorMap);
        case 7:  // Xextreame
            colorMap = {{0.0f, UIColor(255, 255, 255)},
                        {0.5f, UIColor(226, 13, 15)},
                        {1.0f, UIColor(255, 255, 255)}};
            return getCustomColors(seconds, index, colorMap);
        case 8:  // White
            return UIColor(255, 255, 255);
        default:
            return UIColor(255, 255, 255);
    }
}

ID2D1LinearGradientBrush* ColorUtil::createLinearGradientBrush(
    ID2D1RenderTarget* renderTarget, const std::vector<GradientStop>& stops,
    const D2D1_POINT_2F& startPoint, const D2D1_POINT_2F& endPoint) {
    if(!renderTarget || stops.empty())
        return nullptr;

    std::vector<D2D1_GRADIENT_STOP> d2dStops;
    d2dStops.reserve(stops.size());

    for(const auto& stop : stops) {
        D2D1_GRADIENT_STOP d2dStop;
        d2dStop.position = std::clamp(stop.position, 0.0f, 1.0f);
        d2dStop.color = stop.color.toD2D1Color();
        d2dStops.push_back(d2dStop);
    }

    ID2D1GradientStopCollection* stopCollection = nullptr;
    HRESULT hr = renderTarget->CreateGradientStopCollection(
        d2dStops.data(), static_cast<UINT32>(d2dStops.size()), &stopCollection);

    if(FAILED(hr))
        return nullptr;

    ID2D1LinearGradientBrush* brush = nullptr;
    hr = renderTarget->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(startPoint, endPoint), stopCollection, &brush);

    stopCollection->Release();
    return SUCCEEDED(hr) ? brush : nullptr;
}

ID2D1RadialGradientBrush* ColorUtil::createRadialGradientBrush(
    ID2D1RenderTarget* renderTarget, const std::vector<GradientStop>& stops,
    const D2D1_POINT_2F& center, float radiusX, float radiusY) {
    if(!renderTarget || stops.empty())
        return nullptr;

    std::vector<D2D1_GRADIENT_STOP> d2dStops;
    d2dStops.reserve(stops.size());

    for(const auto& stop : stops) {
        D2D1_GRADIENT_STOP d2dStop;
        d2dStop.position = std::clamp(stop.position, 0.0f, 1.0f);
        d2dStop.color = stop.color.toD2D1Color();
        d2dStops.push_back(d2dStop);
    }

    ID2D1GradientStopCollection* stopCollection = nullptr;
    HRESULT hr = renderTarget->CreateGradientStopCollection(
        d2dStops.data(), static_cast<UINT32>(d2dStops.size()), &stopCollection);

    if(FAILED(hr))
        return nullptr;

    ID2D1RadialGradientBrush* brush = nullptr;
    hr = renderTarget->CreateRadialGradientBrush(
        D2D1::RadialGradientBrushProperties(center, D2D1::Point2F(0, 0), radiusX, radiusY),
        stopCollection, &brush);

    stopCollection->Release();
    return SUCCEEDED(hr) ? brush : nullptr;
}

std::vector<ColorUtil::GradientStop> ColorUtil::getAquaGradientStops(float alpha) {
    return {{0.0f, UIColor(15, 82, 186, static_cast<uint8_t>(alpha * 255))},
            {0.25f, UIColor(0, 150, 255, static_cast<uint8_t>(alpha * 255))},
            {0.5f, UIColor(64, 224, 255, static_cast<uint8_t>(alpha * 255))},
            {0.75f, UIColor(173, 216, 230, static_cast<uint8_t>(alpha * 255))},
            {1.0f, UIColor(240, 248, 255, static_cast<uint8_t>(alpha * 255))}};
}

std::vector<ColorUtil::GradientStop> ColorUtil::getOceanGradientStops(float time, float alpha) {
    float wave = sinf(time * 2.0f) * 0.3f + 0.7f;
    uint8_t a = static_cast<uint8_t>(alpha * 255);

    return {{0.0f, UIColor(static_cast<uint8_t>(25 * wave), static_cast<uint8_t>(25 * wave),
                           static_cast<uint8_t>(112 * wave), a)},
            {0.3f, UIColor(static_cast<uint8_t>(0 * wave), static_cast<uint8_t>(105 * wave),
                           static_cast<uint8_t>(148 * wave), a)},
            {0.6f, UIColor(static_cast<uint8_t>(64 * wave), static_cast<uint8_t>(224 * wave),
                           static_cast<uint8_t>(255 * wave), a)},
            {1.0f, UIColor(static_cast<uint8_t>(240 * wave), static_cast<uint8_t>(248 * wave),
                           static_cast<uint8_t>(255 * wave), a)}};
}

std::vector<ColorUtil::GradientStop> ColorUtil::getSunsetGradientStops(float alpha) {
    uint8_t a = static_cast<uint8_t>(alpha * 255);
    return {{0.0f, UIColor(255, 94, 77, a)},
            {0.33f, UIColor(255, 154, 0, a)},
            {0.66f, UIColor(255, 206, 84, a)},
            {1.0f, UIColor(255, 238, 173, a)}};
}

std::vector<ColorUtil::GradientStop> ColorUtil::getNeonGradientStops(float time, float alpha) {
    float pulse = (sinf(time * 5.0f) + 1.0f) * 0.5f;
    uint8_t intensity = static_cast<uint8_t>(pulse * 255);
    uint8_t a = static_cast<uint8_t>(alpha * 255);

    return {{0.0f, UIColor(intensity, 0, intensity, a)},
            {0.5f, UIColor(0, intensity, intensity, a)},
            {1.0f, UIColor(intensity, intensity, 255, a)}};
}