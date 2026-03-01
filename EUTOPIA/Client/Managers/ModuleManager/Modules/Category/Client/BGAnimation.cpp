#include "BGAnimation.h"
#include <cmath>
#include "../../../ModuleManager.h"
#include "../../../../../../Utils/TimerUtil.h"
#include <Windows.h>

// Forward declaration for mouse position helper
static Vec2<float> getMousePosScreen();

static inline float randomFloat(float a, float b) {
    return a + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (b - a)));
}

BGAnimation::BGAnimation() : Module("BGAnimation", "Background animation for ClickGUI", Category::CLIENT, 0) {
    // Hide by default
    // visible flag is private, so we keep isVisible override to false
    srand(static_cast<unsigned>(time(nullptr)));

    registerSetting(new BoolSetting("Enable Raindrops", "Show raindrop effect", &enableRaindrops, false));
    registerSetting(new SliderSetting<float>("Raindrop Speed", "Raindrop speed multiplier", &raindropSpeedMul, 1.0f, 0.5f, 5.0f));
    registerSetting(new SliderSetting<float>("Raindrop Size", "Raindrop length multiplier", &raindropSizeMul, 0.8f, 0.5f, 1.2f));
    registerSetting(new SliderSetting<int>("Raindrop Count", "Number of raindrops", &maxRaindrops, 120, 50, 300)); // 降低默认数量

    // Star settings
    registerSetting(new BoolSetting("Enable Stars", "Show starfield effect", &enableStars, false));
    registerSetting(new SliderSetting<int>("Star Count", "Number of stars", &maxStars, 150, 50, 500)); // 降低默认数量
    registerSetting(new SliderSetting<float>("Star Twinkle", "Star twinkle speed multiplier", &starTwinkleSpeedMul, 0.7f, 0.5f, 3.0f)); // 降低默认闪烁速度
    registerSetting(new SliderSetting<float>("Star Size", "Star size multiplier", &starSizeMul, 0.8f, 0.5f, 2.0f)); // 降低默认大小
    registerSetting(new SliderSetting<float>("Star Speed", "Star upward speed multiplier", &starSpeedMul, 0.5f, 0.1f, 5.0f)); // 降低默认上升速度

    registerSetting(new BoolSetting("Star Trails", "Show star movement trails", &enableStarTrails, false));
    registerSetting(new SliderSetting<float>("Trail Length", "Length of star trails", &starTrailLength, 0.6f, 0.3f, 3.0f)); // 降低默认拖尾长度
    registerSetting(new BoolSetting("Vary Star Sizes", "Use different star sizes", &varySizes, true));
    registerSetting(new BoolSetting("Vary Star Colors", "Use different star colors", &varyColors, true));

    // Spider web settings
    registerSetting(new BoolSetting("Enable Spider Web", "Show spider-web background", &enableSpiderWeb, true));
    registerSetting(new SliderSetting<int>("Web Node Count", "Number of web nodes", &webNodeCount, 150, 20, 300));
    registerSetting(new SliderSetting<float>("Web Connection Dist", "Max distance between nodes to draw a web line", &webConnectionDist, 130.0f, 50.0f, 300.0f));
    registerSetting(new SliderSetting<float>("Web Line Width", "Line width of web", &webLineWidth, 1.5f, 0.5f, 3.0f));
    registerSetting(new SliderSetting<float>("Web Speed", "Web node speed multiplier", &webNodeSpeedMul, 1.9f, 0.1f, 5.0f));
    registerSetting(new SliderSetting<float>("Mouse Repel Radius", "Radius that mouse repels nodes", &mouseRepelRadius, 120.0f, 20.0f, 500.0f));

    registerSetting(new BoolSetting("Colored Background", "Use deep blue background instead of black", &enableBackgroundColor, true));
    registerSetting(new SliderSetting<int>("Background Alpha", "Background opacity", &backgroundAlpha, 30, 0, 255));
}

void BGAnimation::ensureCounts() {
    if ((int)raindrops.size() != maxRaindrops) {
        raindrops.resize(maxRaindrops);
        for (auto& d : raindrops) resetRaindrop(d);
    }

    if ((int)stars.size() != maxStars) {
        stars.resize(maxStars);
        for (auto& s : stars) resetStar(s);
    }

    if ((int)webNodes.size() != webNodeCount) {
        webNodes.resize(webNodeCount);
        for (auto& n : webNodes) resetWebNode(n);
    }
}

void BGAnimation::updateEffects(float dt) {
    ensureCounts();


    // Update raindrops
    if (enableRaindrops) {
        for (auto& d : raindrops) {
            d.pos.y += d.speed * dt;
            if (d.pos.y - d.length > RenderUtil::getWindowSize().y) {
                resetRaindrop(d);
            }
        }
    }

    // Update stars (twinkle phase and movement)
    if (enableStars) {
        for (auto& s : stars) {
            s.phase += s.twinkleSpeed * dt;
            s.pos.y -= s.speed * dt; // move upwards

            if (s.pos.y + s.radius < 0) {
                // Re-spawn at bottom when off the top
                resetStar(s);
                s.pos.y = RenderUtil::getWindowSize().y + s.radius;
            }

            if (s.phase > 6.28318530718f) s.phase -= 6.28318530718f; // keep within 0-2π
        }
    }

    // Update spider web nodes
    if (enableSpiderWeb) {
        updateSpiderWeb(dt);
    }


}

void BGAnimation::resetRaindrop(Raindrop& drop) {
    Vec2<float> winSize = RenderUtil::getWindowSize();
    drop.pos.x = randomFloat(0.f, winSize.x);
    drop.pos.y = randomFloat(-winSize.y, 0.f);
    drop.length = randomFloat(10.f, 20.f) * raindropSizeMul;
    drop.speed = randomFloat(300.f, 600.f) * raindropSpeedMul;
    drop.alpha = randomFloat(50.f, 150.f); // 0-255
}

UIColor BGAnimation::getStarBaseColor(int type) {
    if (!varyColors) {
        return UIColor(255, 255, 255, 255); // White
    }

    switch (type) {
    case 0: // Small stars (more blue/white)
        return UIColor(200, 220, 255, 255);
    case 1: // Medium stars (white)
        return UIColor(255, 255, 255, 255);
    case 2: // Large stars (slightly yellow)
        return UIColor(255, 255, 240, 255);
    case 3: // Very bright stars (slight cyan tint)
        return UIColor(220, 255, 255, 255);
    default:
        return UIColor(255, 255, 255, 255);
    }
}

void BGAnimation::resetStar(Star& star) {
    Vec2<float> winSize = RenderUtil::getWindowSize();
    star.pos.x = randomFloat(0.f, winSize.x);
    star.pos.y = randomFloat(0.f, winSize.y);

    // Determine star type with weighted distribution
    float typeDist = randomFloat(0.f, 1.0f);
    if (typeDist < 0.7f) {
        star.type = 0; // 70% small stars
    }
    else if (typeDist < 0.9f) {
        star.type = 1; // 20% medium stars
    }
    else if (typeDist < 0.98f) {
        star.type = 2; // 8% large stars
    }
    else {
        star.type = 3; // 2% very bright stars
    }

    // Size based on type
    float baseRadius = 0.0f;
    switch (star.type) {
    case 0: baseRadius = randomFloat(0.5f, 1.0f); break;
    case 1: baseRadius = randomFloat(1.0f, 1.5f); break;
    case 2: baseRadius = randomFloat(1.5f, 2.0f); break;
    case 3: baseRadius = randomFloat(2.0f, 3.0f); break;
    }

    star.radius = baseRadius * (varySizes ? starSizeMul : 1.0f);
    star.glowRadius = star.radius * (1.5f + star.type);
    star.twinkleSpeed = randomFloat(0.5f, 1.5f) * starTwinkleSpeedMul;
    star.phase = randomFloat(0.f, 6.28318530718f);

    // Speed based partly on type - larger stars appear to move faster
    float typeSpeedBonus = 1.0f + 0.3f * star.type;
    star.speed = randomFloat(5.f, 20.f) * starSpeedMul * typeSpeedBonus;

    // Trail length - larger stars have longer trails
    star.tailLength = star.speed * (0.05f + 0.02f * star.type) * starTrailLength;

    // Color based on type
    star.color = getStarBaseColor(star.type);
}

void BGAnimation::resetWebNode(WebNode& node) {
    Vec2<float> winSize = RenderUtil::getWindowSize();
    node.pos.x = randomFloat(0.f, winSize.x);
    node.pos.y = randomFloat(0.f, winSize.y);
    float angle = randomFloat(0.f, 6.28318530718f);
    float speed = randomFloat(20.f, 60.f) * webNodeSpeedMul;
    node.vel.x = cosf(angle) * speed;
    node.vel.y = sinf(angle) * speed;
}

void BGAnimation::updateSpiderWeb(float dt) {
    Vec2<float> winSize = RenderUtil::getWindowSize();
    Vec2<float> mousePos = getMousePosScreen();

    for (auto& node : webNodes) {
        // Move
        node.pos.x += node.vel.x * dt;
        node.pos.y += node.vel.y * dt;

        // Bounce off edges
        if (node.pos.x < 0.f) {
            node.pos.x = 0.f;
            node.vel.x = fabsf(node.vel.x);
        }
        else if (node.pos.x > winSize.x) {
            node.pos.x = winSize.x;
            node.vel.x = -fabsf(node.vel.x);
        }

        if (node.pos.y < 0.f) {
            node.pos.y = 0.f;
            node.vel.y = fabsf(node.vel.y);
        }
        else if (node.pos.y > winSize.y) {
            node.pos.y = winSize.y;
            node.vel.y = -fabsf(node.vel.y);
        }

        // Mouse repulsion
        float dx = node.pos.x - mousePos.x;
        float dy = node.pos.y - mousePos.y;
        float dist2 = dx * dx + dy * dy;
        float repelR2 = mouseRepelRadius * mouseRepelRadius;
        if (dist2 < repelR2 && dist2 > 1.0f) {
            float dist = sqrtf(dist2);
            float factor = (mouseRepelRadius - dist) / mouseRepelRadius; // 0..1
            node.vel.x += (dx / dist) * mouseRepelStrength * factor * dt;
            node.vel.y += (dy / dist) * mouseRepelStrength * factor * dt;
        }
    }
}

void BGAnimation::renderSpiderWeb() {
    if (!enableSpiderWeb) return;

    size_t n = webNodes.size();
    float maxDist = webConnectionDist;
    float maxDist2 = maxDist * maxDist;

    for (size_t i = 0; i < n; ++i) {
        const Vec2<float>& p1 = webNodes[i].pos;
        for (size_t j = i + 1; j < n; ++j) {
            const Vec2<float>& p2 = webNodes[j].pos;
            float dx = p1.x - p2.x;
            float dy = p1.y - p2.y;
            float dist2 = dx * dx + dy * dy;
            if (dist2 < maxDist2) {
                float dist = sqrtf(dist2);
                float alphaFactor = 1.0f - dist / maxDist;
                unsigned char alpha = static_cast<unsigned char>(alphaFactor * 180.f); // semi transparent
                UIColor lineColor(255, 255, 255, alpha);
                RenderUtil::drawLine(p1, p2, lineColor, webLineWidth);
            }
        }
    }
}


void BGAnimation::onEnable() {
    raindrops.clear();
    raindrops.resize(raindropCount);
    for (auto& d : raindrops) resetRaindrop(d);

    stars.clear();
    stars.resize(maxStars);
    for (auto& s : stars) resetStar(s);

    webNodes.clear();
    webNodes.resize(webNodeCount);
    for (auto& n : webNodes) resetWebNode(n);
}



void BGAnimation::onDisable() {
    raindrops.clear();
    stars.clear();

    webNodes.clear();
}



void BGAnimation::onClientTick() {
    Vec2<float> winSize = RenderUtil::getWindowSize();
    float dt = RenderUtil::deltaTime;

    // Update raindrops
    if (enableRaindrops) {
        for (auto& d : raindrops) {
            d.pos.y += d.speed * dt;
            if (d.pos.y - d.length > winSize.y) {
                resetRaindrop(d);
            }
        }
    }
}


void BGAnimation::renderStar(const Star& s, float dt) {
    // Calculate alpha based on twinkle phase
    float alphaFactor = (sinf(s.phase) + 1.2f) * 0.5f; // 0.1-1.1 range
    if (alphaFactor > 1.0f) alphaFactor = 1.0f;

    // Adjust alpha by star type - brighter for larger stars
    float typeAlphaBonus = 1.0f + 0.2f * s.type;
    unsigned char alpha = (unsigned char)(150 + 105 * alphaFactor * typeAlphaBonus);
    if (alpha > 255) alpha = 255;

    // Create color with proper alpha
    UIColor starColor = s.color;
    starColor.a = alpha;

    // Draw star trail if enabled
    if (enableStarTrails && s.tailLength > 0.1f) {
        // Trail points from current position downward
        Vec2<float> tailEnd(s.pos.x, s.pos.y + s.tailLength);

        // Gradient trail with decreasing alpha
        for (int i = 0; i < 3; i++) {
            float trailSegment = 1.0f - (i / 3.0f);
            Vec2<float> segPos(
                s.pos.x,
                s.pos.y + s.tailLength * (i / 3.0f)
            );

            UIColor trailColor = starColor;
            trailColor.a = (unsigned char)(starColor.a * 0.7f * trailSegment);

            float segRadius = s.radius * (0.8f + 0.4f * trailSegment);
            RenderUtil::fillCircle(segPos, trailColor, segRadius);
        }

        // Very faint full trail line
        UIColor trailLineColor = starColor;
        trailLineColor.a = (unsigned char)(starColor.a * 0.2f);
        RenderUtil::drawLine(s.pos, tailEnd, trailLineColor, s.radius * 0.5f);
    }

    // Draw main star
    RenderUtil::fillCircle(s.pos, starColor, s.radius);

    // Draw glow based on star type
    if (s.type >= 1) { // Medium and larger stars have glow
        UIColor glowColor = starColor;
        glowColor.a = (unsigned char)(alpha * (0.3f + 0.1f * s.type));
        RenderUtil::fillCircle(s.pos, glowColor, s.glowRadius);

        // Extra bright center for large stars
        if (s.type >= 2) {
            UIColor centerColor = UIColor(255, 255, 255, alpha);
            RenderUtil::fillCircle(s.pos, centerColor, s.radius * 0.5f);
        }

        // Very bright stars have extra glow
        if (s.type == 3) {
            UIColor outerGlowColor = starColor;
            outerGlowColor.a = (unsigned char)(alpha * 0.15f);
            RenderUtil::fillCircle(s.pos, outerGlowColor, s.glowRadius * 2.0f);

            // Cross-shaped rays for very bright stars
            if (alphaFactor > 0.7f) {
                float rayLength = s.glowRadius * 2.5f;
                float rayAlpha = alpha * 0.2f * (alphaFactor - 0.7f) / 0.3f;
                UIColor rayColor = starColor;
                rayColor.a = (unsigned char)rayAlpha;

                // Horizontal ray
                RenderUtil::drawLine(
                    Vec2<float>(s.pos.x - rayLength, s.pos.y),
                    Vec2<float>(s.pos.x + rayLength, s.pos.y),
                    rayColor, s.radius * 0.3f
                );

                // Vertical ray
                RenderUtil::drawLine(
                    Vec2<float>(s.pos.x, s.pos.y - rayLength),
                    Vec2<float>(s.pos.x, s.pos.y + rayLength),
                    rayColor, s.radius * 0.3f
                );
            }
        }
    }
}


void BGAnimation::renderBackground() {
    if (!enableBackgroundColor) return;
    Vec2<float> winSize = RenderUtil::getWindowSize();
    UIColor bgColor = backgroundColor;
    bgColor.a = (unsigned char)(backgroundAlpha);
    RenderUtil::fillRect(Vec2<float>(0, 0), winSize, bgColor);
}




void BGAnimation::renderOverGUI() {
    updateEffects(RenderUtil::deltaTime);

    auto drawEffects = [&]() {

        renderBackground();
        // Spider web background lines should be drawn first (behind stars/raindrops)
        renderSpiderWeb();



        if (enableStars) {
            for (auto& s : stars) {
                renderStar(s, RenderUtil::deltaTime);
            }
        }

        if (enableRaindrops) {
            UIColor rainColor(0, 200, 255, 180);
            for (auto& d : raindrops) {
                UIColor c = rainColor;
                c.a = static_cast<unsigned char>(d.alpha * 0.7f);
                Vec2<float> endPos(d.pos.x, d.pos.y + d.length);
                RenderUtil::drawLine(d.pos, endPos, c, 1.5f);
                UIColor glow = c; glow.a = (unsigned char)(c.a * 0.3f);
                RenderUtil::drawLine(Vec2<float>(d.pos.x, d.pos.y - 2), Vec2<float>(d.pos.x, d.pos.y + d.length + 2), glow, 3.f);
            }
        }
        };
    drawEffects();
}


static Vec2<float> getMousePosScreen() {
    POINT p; GetCursorPos(&p);
    return Vec2<float>((float)p.x, (float)p.y);
}

