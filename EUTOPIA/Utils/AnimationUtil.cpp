#include "AnimationUtil.h"

// 新特效动画实现
float AnimationUtil::getWaveOffset(float time, float frequency, float amplitude, float phase) {
    return sin(time * frequency + phase) * amplitude;
}

float AnimationUtil::getSweepProgress(float time, float duration, float delay) {
    float adjustedTime = time - delay;
    if (adjustedTime < 0.0f) return 0.0f;
    
    float progress = fmod(adjustedTime, duration) / duration;
    return smoothstep(0.0f, 1.0f, progress);
}

float AnimationUtil::getShimmerEffect(float time, float speed) {
    return (sin(time * speed) + 1.0f) * 0.5f;
}

float AnimationUtil::getGlowPulse(float time, float frequency) {
    float pulse = sin(time * frequency) * 0.5f + 0.5f;
    return pulse * pulse; // 平方使效果更尖锐
}

float AnimationUtil::getBounceEffect(float progress, float bounceHeight) {
    if (progress >= 1.0f) return 1.0f;
    
    float scaledProgress = progress * 4.0f;
    if (scaledProgress < 1.0f) {
        return 1.0f - (1.0f - scaledProgress) * (1.0f - scaledProgress);
    } else {
        float bounceProgress = (scaledProgress - 1.0f) / 3.0f;
        return 1.0f + sin(bounceProgress * 3.14159f) * bounceHeight * (1.0f - bounceProgress);
    }
}

float AnimationUtil::getElasticEffect(float progress, float elasticity) {
    if (progress <= 0.0f) return 0.0f;
    if (progress >= 1.0f) return 1.0f;
    
    float p = 0.3f;
    float s = p / 4.0f;
    return pow(2.0f, -10.0f * progress) * sin((progress - s) * (2.0f * 3.14159f) / p) + 1.0f;
}

float AnimationUtil::getRippleEffect(float time, float distance, float speed) {
    float wave = sin(time * speed - distance * 0.1f);
    float fadeOut = exp(-distance * 0.02f);
    return wave * fadeOut;
}

// 高级缓动函数
float AnimationUtil::easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

float AnimationUtil::easeInOutQuart(float t) {
    return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 4.0f) / 2.0f;
}

float AnimationUtil::easeInOutSine(float t) {
    return -(cos(3.14159f * t) - 1.0f) / 2.0f;
}

float AnimationUtil::easeOutBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;

    if (t < 1.0f / d1) {
        return n1 * t * t;
    } else if (t < 2.0f / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    } else if (t < 2.5f / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    } else {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}

float AnimationUtil::easeInElastic(float t) {
    const float c4 = (2.0f * 3.14159f) / 3.0f;
    return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : -pow(2.0f, 10.0f * t - 10.0f) * sin((t * 10.0f - 10.75f) * c4);
}

float AnimationUtil::easeOutElastic(float t) {
    const float c4 = (2.0f * 3.14159f) / 3.0f;
    return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : pow(2.0f, -10.0f * t) * sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

float AnimationUtil::smoothstep(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// 现有实现保持不变...
bool AnimationUtil::lerpVec2(Vec2<float>& current, const Vec2<float>& target, float speed, float deltaTime, float threshold) {
    Vec2<float> diff = target.sub(current);
    float dist = distance(current, target);
    
    if (dist < threshold) {
        current = target;
        return true;
    }
    
    float factor = speed * deltaTime;
    factor = clamp(factor, 0.0f, 1.0f);
    
    current.x += diff.x * factor;
    current.y += diff.y * factor;
    
    return false;
}

bool AnimationUtil::lerpVec3(Vec3<float>& current, const Vec3<float>& target, float speed, float deltaTime, float threshold) {
    Vec3<float> diff = target.sub(current);
    float dist = distance(current, target);
    
    if (dist < threshold) {
        current = target;
        return true;
    }
    
    float factor = speed * deltaTime;
    factor = clamp(factor, 0.0f, 1.0f);
    
    current.x += diff.x * factor;
    current.y += diff.y * factor;
    current.z += diff.z * factor;
    
    return false;
}

bool AnimationUtil::lerpFloat(float& current, float target, float speed, float deltaTime, float threshold) {
    float diff = target - current;

    if (std::abs(diff) < threshold) {
        current = target;
        return true;
    }
    
    float factor = speed * deltaTime;
    factor = clamp(factor, 0.0f, 1.0f);
    
    current += diff * factor;
    
    return false;
}

bool AnimationUtil::easeOutVec2(Vec2<float>& current, const Vec2<float>& target, float speed, float deltaTime, float threshold) {
    Vec2<float> diff = target.sub(current);
    float dist = distance(current, target);
    
    if (dist < threshold) {
        current = target;
        return true;
    }
    
    float factor = 1.0f - std::pow(1.0f - clamp(speed * deltaTime, 0.0f, 1.0f), 2.0f);
    
    current.x += diff.x * factor;
    current.y += diff.y * factor;
    
    return false;
}

bool AnimationUtil::easeOutVec3(Vec3<float>& current, const Vec3<float>& target, float speed, float deltaTime, float threshold) {
    Vec3<float> diff = target.sub(current);
    float dist = distance(current, target);
    
    if (dist < threshold) {
        current = target;
        return true;
    }
    
    float factor = 1.0f - std::pow(1.0f - clamp(speed * deltaTime, 0.0f, 1.0f), 2.0f);
    
    current.x += diff.x * factor;
    current.y += diff.y * factor;
    current.z += diff.z * factor;
    
    return false;
}

bool AnimationUtil::easeOutFloat(float& current, float target, float speed, float deltaTime, float threshold) {
    float diff = target - current;
    
    if (std::abs(diff) < threshold) {
        current = target;
        return true;
    }
    
    float factor = 1.0f - std::pow(1.0f - clamp(speed * deltaTime, 0.0f, 1.0f), 2.0f);
    
    current += diff * factor;
    
    return false;
}

bool AnimationUtil::elasticVec2(Vec2<float>& current, const Vec2<float>& target, float speed, float deltaTime, float elasticity, float threshold) {
    Vec2<float> diff = target.sub(current);
    float dist = distance(current, target);
    
    if (dist < threshold) {
        current = target;
        return true;
    }

    float baseSpeed = speed * deltaTime;
    float elasticFactor = 1.0f + elasticity * std::sin(dist * 10.0f);
    float factor = clamp(baseSpeed * elasticFactor, 0.0f, 1.0f);
    
    current.x += diff.x * factor;
    current.y += diff.y * factor;
    
    return false;
}

bool AnimationUtil::elasticFloat(float& current, float target, float speed, float deltaTime, float elasticity, float threshold) {
    float diff = target - current;
    
    if (std::abs(diff) < threshold) {
        current = target;
        return true;
    }
    
    float baseSpeed = speed * deltaTime;
    float elasticFactor = 1.0f + elasticity * std::sin(std::abs(diff) * 10.0f);
    float factor = clamp(baseSpeed * elasticFactor, 0.0f, 1.0f);
    
    current += diff * factor;
    
    return false;
}

float AnimationUtil::distance(const Vec2<float>& a, const Vec2<float>& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

float AnimationUtil::distance(const Vec3<float>& a, const Vec3<float>& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

float AnimationUtil::clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
                                        
float AnimationUtil::lerp(float a, float b, float t) {
    return a + (b - a) * t;
}