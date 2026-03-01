#include "Hud.h"
#include "../../../ModuleManager.h"

Hud::Hud() : Module("Hud", "Displays position and potion effects", Category::RENDER) {
    registerSetting(new BoolSetting("ShowPosition", "Display coordinates", &showPosition, true));
    registerSetting(new BoolSetting("ShowPotions", "Display active potion effects", &showPotions, true));
    registerSetting(new SliderSetting<float>("TextSize", "Font scale", &textSize, 1.f, 0.5f, 2.f));
    registerSetting(new SliderSetting<float>("Spacing", "Space between elements", &spacing, 2.f, 0.f, 10.f));
    registerSetting(new BoolSetting("ShowDimension", "Show dimension name", &showDimension, true));
    registerSetting(new ColorSetting("PosColor", "Primary text color", &textColor, UIColor(255, 255, 255, 255)));
    registerSetting(new ColorSetting("PotionColor", "Potion effect color", &potionColor, UIColor(200, 255, 200, 255)));
    registerSetting(new BoolSetting("Glow", "Adds a glow effect to the list.", &shouldGlow, shouldGlow));
}

inline std::string Hud::getEffectTimeLeftStr(MobEffectInstance* mEffectInstance) {
    uint32_t timeLeft = mEffectInstance->mDuration;
    uint32_t timeReal = (uint32_t)(timeLeft / 20);
    std::string m = std::to_string(timeReal / 60);
    std::string s;
    if (timeReal % 60 < 10)
        s += "0";
    s += std::to_string(timeReal % 60);
    return m + ":" + s;
}

std::string Hud::getDimensionName(LocalPlayer* lp) {
    if (!lp || !lp->getDimensionTypeComponent()) return "Unknown";
    int dimId = lp->getDimensionTypeComponent()->type;
    switch (dimId) {
    case 0: return "Overworld";
    case 1: return "Nether";
    case 2: return "End";
    default: return "Unknown";
    }
}

void Hud::onD2DRender() {
    auto lp = GI::getLocalPlayer();
    if (!lp) return;

    Vec2<float> windowSize = D2D::getWindowSize();
    float scale = textSize;
    float pad = 4.f * scale;
    float lineHeight = D2D::getTextHeight("A", scale) + spacing * scale;

    float x = pad;
    float y = windowSize.y + pad;

    std::vector<std::pair<std::string, UIColor>> renderLines;

    if (showPosition) {
        Vec3<float> pos = lp->getPos();
        std::string posStr = "X: " + std::to_string((int)pos.x) + " Y: " + std::to_string((int)pos.y) + " Z: " + std::to_string((int)pos.z);
        if (showDimension) {
            posStr += " [" + getDimensionName(lp) + "]";
        }
        renderLines.push_back({ posStr, textColor });
    }

    std::vector<std::pair<std::string, UIColor>> potionLines;
    /*
    if (showPotions) {
        for (uint32_t effectId = 1; effectId < 37; effectId++) {
            MobEffect* mEffect = MobEffect::getById(effectId);
            if (mEffect == nullptr)
                continue;
            if (lp->hasEffect(mEffect)) {
                MobEffectInstance* mEffectInstance = lp->getEffect(mEffect);
                if (mEffectInstance == nullptr)
                    continue;

                std::string text1 = mEffectInstance->getDisplayName() + " ";
                std::string text2 = getEffectTimeLeftStr(mEffectInstance);
                std::string fullText = text1 + text2;

                UIColor color = potionColor;
                potionLines.push_back({ fullText, color });
            }
        }
    }
    */
    //
    float totalHeight = renderLines.size() * lineHeight + potionLines.size() * lineHeight;
    if (!renderLines.empty() && !potionLines.empty()) totalHeight += lineHeight * 0.5f;

    y -= totalHeight;

    for (const auto& line : potionLines) {
        D2D::drawGlowingText(Vec2<float>(x, y), line.first, line.second, scale, shouldGlow ? 4.5f : 0.f);
        y += lineHeight;
    }

    if (!potionLines.empty() && !renderLines.empty()) {
        y += lineHeight * 0.3f;
    }

    for (const auto& line : renderLines) {
        D2D::drawGlowingText(Vec2<float>(x, y), line.first, line.second, scale, shouldGlow ? 4.5f : 0.f);
        y += lineHeight;
    }
}