#pragma once
#include "../Utils/Maths.h"
#include <cmath>
#include <algorithm>

/**
 * AnimationUtil - 提供丝滑的动画插值函数
 * 支持 Vec2, Vec3 和基础数据类型的平滑动画
 */
class AnimationUtil {
public:
    // Vec2 动画函数
    /**
     * 对 Vec2 进行丝滑插值动画
     * @param current 当前值（会被修改）
     * @param target 目标值
     * @param speed 动画速度 (建议范围: 1.0f - 20.0f)
     * @param deltaTime 帧时间差
     * @param threshold 停止阈值，当接近目标值时直接设为目标值
     * @return 是否已到达目标值
     */
    static bool lerpVec2(Vec2<float>& current, const Vec2<float>& target, float speed, float deltaTime, float threshold = 0.01f);

    // Vec3 动画函数
    /**
     * 对 Vec3 进行丝滑插值动画
     * @param current 当前值（会被修改）
     * @param target 目标值
     * @param speed 动画速度 (建议范围: 1.0f - 20.0f)
     * @param deltaTime 帧时间差
     * @param threshold 停止阈值，当接近目标值时直接设为目标值
     * @return 是否已到达目标值
     */
    static bool lerpVec3(Vec3<float>& current, const Vec3<float>& target, float speed, float deltaTime, float threshold = 0.01f);

    // float 动画函数
    /**
     * 对 float 进行丝滑插值动画
     * @param current 当前值（会被修改）
     * @param target 目标值
     * @param speed 动画速度 (建议范围: 1.0f - 20.0f)
     * @param deltaTime 帧时间差
     * @param threshold 停止阈值，当接近目标值时直接设为目标值
     * @return 是否已到达目标值
     */
    static bool lerpFloat(float& current, float target, float speed, float deltaTime, float threshold = 0.01f);

    // 缓动动画函数 (更平滑的动画曲线)
    /**
     * 带缓动效果的 Vec2 动画 (EaseOut)
     * @param current 当前值（会被修改）
     * @param target 目标值
     * @param speed 动画速度 (建议范围: 5.0f - 30.0f)
     * @param deltaTime 帧时间差
     * @param threshold 停止阈值
     * @return 是否已到达目标值
     */
    static bool easeOutVec2(Vec2<float>& current, const Vec2<float>& target, float speed, float deltaTime, float threshold = 0.01f);

    /**
     * 带缓动效果的 Vec3 动画 (EaseOut)
     * @param current 当前值（会被修改）
     * @param target 目标值
     * @param speed 动画速度 (建议范围: 5.0f - 30.0f)
     * @param deltaTime 帧时间差
     * @param threshold 停止阈值
     * @return 是否已到达目标值
     */
    static bool easeOutVec3(Vec3<float>& current, const Vec3<float>& target, float speed, float deltaTime, float threshold = 0.01f);

    /**
     * 带缓动效果的 float 动画 (EaseOut)
     * @param current 当前值（会被修改）
     * @param target 目标值
     * @param speed 动画速度 (建议范围: 5.0f - 30.0f)
     * @param deltaTime 帧时间差
     * @param threshold 停止阈值
     * @return 是否已到达目标值
     */
    static bool easeOutFloat(float& current, float target, float speed, float deltaTime, float threshold = 0.01f);

    // 弹性动画函数 (带反弹效果)
    /**
     * 弹性动画效果的 Vec2 动画
     * @param current 当前值（会被修改）
     * @param target 目标值
     * @param speed 动画速度 (建议范围: 8.0f - 25.0f)
     * @param deltaTime 帧时间差
     * @param elasticity 弹性强度 (建议范围: 0.1f - 0.5f)
     * @param threshold 停止阈值
     * @return 是否已到达目标值
     */
    static bool elasticVec2(Vec2<float>& current, const Vec2<float>& target, float speed, float deltaTime, float elasticity = 0.1f, float threshold = 0.01f);

    /**
     * 弹性动画效果的 float 动画
     * @param current 当前值（会被修改）
     * @param target 目标值
     * @param speed 动画速度 (建议范围: 8.0f - 25.0f)
     * @param deltaTime 帧时间差
     * @param elasticity 弹性强度 (建议范围: 0.1f - 0.5f)
     * @param threshold 停止阈值
     * @return 是否已到达目标值
     */
    static bool elasticFloat(float& current, float target, float speed, float deltaTime, float elasticity = 0.1f, float threshold = 0.01f);

    // 新增：文字特效动画
    /**
     * 计算波浪偏移量（用于文字的波浪特效）
     * @param time 当前时间
     * @param frequency 波浪的频率
     * @param amplitude 波浪的幅度
     * @param phase 相位偏移
     */
    static float getWaveOffset(float time, float frequency, float amplitude, float phase = 0.0f);

    /**
     * 计算扫过效果的进度（用于文字的扫描特效）
     * @param time 当前时间
     * @param duration 扫过效果持续时间
     * @param delay 延迟时间
     */
    static float getSweepProgress(float time, float duration, float delay = 0.0f);

    /**
     * 计算闪光效果（用于文字的闪烁特效）
     * @param time 当前时间
     * @param speed 闪烁速度
     */
    static float getShimmerEffect(float time, float speed = 2.0f);

    /**
     * 计算发光脉冲效果（用于文字的脉动特效）
     * @param time 当前时间
     * @param frequency 脉冲频率
     */
    static float getGlowPulse(float time, float frequency = 3.0f);

    /**
     * 计算弹跳效果（用于文字的弹跳特效）
     * @param progress 动画进度
     * @param bounceHeight 弹跳高度
     */
    static float getBounceEffect(float progress, float bounceHeight = 0.3f);

    /**
     * 计算弹性效果（用于文字的弹性特效）
     * @param progress 动画进度
     * @param elasticity 弹性系数
     */
    static float getElasticEffect(float progress, float elasticity = 0.5f);

    /**
     * 计算水波纹效果（用于文字的波纹特效）
     * @param time 当前时间
     * @param distance 波纹扩散的距离
     * @param speed 波纹传播速度
     */
    static float getRippleEffect(float time, float distance, float speed = 2.0f);

    // 高级缓动函数
    /**
     * 三次方缓动函数（用于高级动画插值）
     */
    static float easeInOutCubic(float t);

    /**
     * 四次方缓动函数（用于高级动画插值）
     */
    static float easeInOutQuart(float t);

    /**
     * 正弦曲线缓动函数（用于高级动画插值）
     */
    static float easeInOutSine(float t);

    /**
     * 反弹缓动函数（用于高级动画插值）
     */
    static float easeOutBounce(float t);

    /**
     * 弹性缓动函数（用于高级动画插值）
     */
    static float easeInElastic(float t);
    static float easeOutElastic(float t);

    // 工具函数
    /**
     * 计算两个 Vec2 之间的距离
     */
    static float distance(const Vec2<float>& a, const Vec2<float>& b);

    /**
     * 计算两个 Vec3 之间的距离
     */
    static float distance(const Vec3<float>& a, const Vec3<float>& b);

    /**
     * 限制值在指定范围内
     */
    static float clamp(float value, float min, float max);

    /**
     * 基础线性插值函数
     */
    static float lerp(float a, float b, float t);

    /**
     * 平滑插值函数（用于平滑化动画过程）
     */
    static float smoothstep(float edge0, float edge1, float x);
};
