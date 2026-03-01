#pragma once
// 48 89 5C 24 ? 57 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 48 81 C1

#include "../FuncHook.h"
#include <chrono>

class doBobHurt : public FuncHook {
   private:
    using func_t = void*(__thiscall*)(void*,glm::mat4*);
    static inline func_t oFunc;
    
    // 动画状态变量
    static inline float animationTime = 0.0f;
    static inline float swingStartTime = 0.0f;
    static inline float eatStartTime = 0.0f;
    static inline bool isSwinging = false;
    static inline bool wasEating = false;
    static inline bool lastSwingingState = false;
    static inline bool lastEatingState = false;

    // 获取当前时间（秒）
    static float getCurrentTime() {
        static auto start = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        return duration.count() / 1000000.0f;
    }

    // 多种挥舞动画风格
    static Vec3<float> getSwingAnimation(ViewModel* viewMod) {
        if (!isSwinging || !viewMod->AttackAnim) return Vec3<float>(0.f, 0.f, 0.f);
        
        float progress = (animationTime - swingStartTime) * viewMod->swingSpeed;
        float swingDuration = 0.6f;
        
        if (progress > swingDuration) {
            isSwinging = false;
            return Vec3<float>(0.f, 0.f, 0.f);
        }
        
        float t = progress / swingDuration;
        
        switch(viewMod->swingAnimMode) {
            case 0: // Fancy - 原来的复杂动画
                return getFancySwingAnimation(t);
            case 1: // Smooth - 修复后的平滑弧线
                return getSmoothSwingAnimation(t);
            case 2: // Spin - 修复后的旋转式
                return getSpinSwingAnimation(t);
            case 3: // Wobble - 修复后的摆动式
                return getWobbleSwingAnimation(t);
            case 4: // Exhibition - 炫技式
                return getExhibitionSwingAnimation(t);
            default:
                return getFancySwingAnimation(t);
        }
    }

    // Fancy 风格 - 复杂摆动
        static Vec3<float> getFancySwingAnimation(float t) {
        float easeOut = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
        float easeIn = t * t * t;
        
        float rotX = sin(t * 3.14159f * 2.0f) * 45.0f * easeOut;
        float rotY = cos(t * 3.14159f * 1.5f) * 30.0f * easeIn;
        float rotZ = sin(t * 3.14159f * 4.0f) * 15.0f * (1.0f - easeOut);
        
        return Vec3<float>(rotX, rotY, rotZ);
    }

    // Smooth 风格 - 大幅减小幅度的平滑弧线
    static Vec3<float> getSmoothSwingAnimation(float t) {
        float smooth = sin(t * 3.14159f); // 平滑的正弦曲线
        float ease = t * (2.0f - t); // 平滑缓动
        
        float rotX = -12.0f * smooth * ease;  // 从-25减少到-12
        float rotY = 8.0f * sin(t * 3.14159f * 0.8f) * ease;  // 从15减少到8
        float rotZ = 4.0f * cos(t * 3.14159f * 1.2f) * ease;  // 从8减少到4
        
        return Vec3<float>(rotX, rotY, rotZ);
    }

    // Spin 风格 - 大幅减小旋转强度
    static Vec3<float> getSpinSwingAnimation(float t) {
        float spin = t * 3.14159f * 0.8f; // 从1.5减少到0.8，减少旋转
        float intensity = sin(t * 3.14159f); // 强度控制
        
        float rotX = 6.0f * sin(spin * 1.2f) * intensity;  // 从12减少到6
        float rotY = 18.0f * sin(spin) * intensity;        // 从35减少到18
        float rotZ = 22.0f * cos(spin * 0.8f) * intensity; // 从45减少到22
        
        return Vec3<float>(rotX, rotY, rotZ);
    }

    // Wobble 风格 - 大幅减小摆动强度
    static Vec3<float> getWobbleSwingAnimation(float t) {
        float baseSwing = sin(t * 3.14159f);
        float wobble = sin(t * 3.14159f * 10.0f) * (1.0f - t) * 0.15f; // 从0.3减少到0.15
        
        float rotX = 10.0f * baseSwing + wobble * 4.0f;  // 从20减少到10，wobble从8减少到4
        float rotY = 12.0f * cos(t * 3.14159f * 0.6f) + wobble * 3.0f; // 从25减少到12，wobble从5减少到3
        float rotZ = 5.0f * sin(t * 3.14159f * 1.5f) + wobble * 6.0f;  // 从10减少到5，wobble从12减少到6
        
        return Vec3<float>(rotX, rotY, rotZ);
    }

    // Exhibition 风格 - 减小防守动作的幅度
    static Vec3<float> getExhibitionSwingAnimation(float t) {
        // 分为三个阶段的防守动作
        float phase1 = 0.25f;  // 快速转向防守位置
        float phase2 = 0.65f;  // 持续防守姿势
        float phase3 = 1.0f;   // 恢复正常位置
        
        float rotX, rotY, rotZ;
        
        if (t < phase1) {
            // 阶段1: 快速转向防守位置 (模仿右键防守)
            float localT = t / phase1;
            float easeOut = 1.0f - (1.0f - localT) * (1.0f - localT) * (1.0f - localT);
            
            // 减小防守姿势的幅度
            rotX = -22.0f * easeOut;  // 从-45减少到-22
            rotY = -30.0f * easeOut;  // 从-60减少到-30
            rotZ = 15.0f * easeOut;   // 从30减少到15
            
        } else if (t < phase2) {
            // 阶段2: 防守持续阶段，减小抖动
            float localT = (t - phase1) / (phase2 - phase1);
            float stabilize = sin(localT * 3.14159f * 8.0f) * 1.5f; // 从3.0减少到1.5
            
            rotX = -22.0f + stabilize;
            rotY = -30.0f + stabilize * 0.5f;
            rotZ = 15.0f + stabilize * 0.3f;
            
        } else {
            // 阶段3: 快速恢复到攻击位置，减小攻击动作幅度
            float localT = (t - phase2) / (phase3 - phase2);
            float easeIn = localT * localT * localT;
            
            // 减小攻击动作的幅度
            float attackRotX = 12.0f * sin(localT * 3.14159f * 2.0f);    // 从25减少到12
            float attackRotY = 18.0f * cos(localT * 3.14159f * 1.5f);    // 从35减少到18
            float attackRotZ = -10.0f * sin(localT * 3.14159f);          // 从-20减少到-10
            
            // 从防守姿势过渡到攻击动作
            rotX = -22.0f * (1.0f - easeIn) + attackRotX * easeIn;
            rotY = -30.0f * (1.0f - easeIn) + attackRotY * easeIn;
            rotZ = 15.0f * (1.0f - easeIn) + attackRotZ * easeIn;
        }
        
        return Vec3<float>(rotX, rotY, rotZ);
    }

    // 多种吃东西动画风格
    static Vec3<float> getEatAnimation(ViewModel* viewMod) {
        if (!wasEating || !viewMod->EatAnim) return Vec3<float>(0.f, 0.f, 0.f);
        
        float progress = (animationTime - eatStartTime) * viewMod->eatSpeed;
        float eatCycleDuration = 1.2f;
        float t = fmod(progress, eatCycleDuration) / eatCycleDuration;
        
        switch(viewMod->eatAnimMode) {
            case 0: // Fancy - 原来的分阶段动画
                return getFancyEatAnimation(t);
            case 1: // Circular - 圆形运动
                return getCircularEatAnimation(t);
            case 2: // Shake - 震动式
                return getShakeEatAnimation(t);
            case 3: // Pull - 拉拽式
                return getPullEatAnimation(t);
            case 4: // Twist - 扭转式
                return getTwistEatAnimation(t);
            default:
                return getFancyEatAnimation(t);
        }
    }

    // Fancy 风格 - 分阶段动画
    static Vec3<float> getFancyEatAnimation(float t) {
        float phase1 = 0.3f;
        float phase2 = 0.7f;
        
        float rotX, rotY, rotZ;
        
        if (t < phase1) {
            float localT = t / phase1;
            rotX = -20.0f * sin(localT * 3.14159f);
            rotY = 10.0f * cos(localT * 3.14159f * 2.0f);
            rotZ = 5.0f * sin(localT * 3.14159f * 3.0f);
        } else if (t < phase2) {
            float localT = (t - phase1) / (phase2 - phase1);
            rotX = -15.0f + 10.0f * sin(localT * 3.14159f * 6.0f);
            rotY = 15.0f * cos(localT * 3.14159f * 4.0f);
            rotZ = 8.0f * sin(localT * 3.14159f * 8.0f);
        } else {
            float localT = (t - phase2) / (1.0f - phase2);
            float ease = 1.0f - localT * localT;
            rotX = -5.0f * ease;
            rotY = 5.0f * cos(localT * 3.14159f) * ease;
            rotZ = 3.0f * sin(localT * 3.14159f * 2.0f) * ease;
        }
        
        return Vec3<float>(rotX, rotY, rotZ);
    }

    // Circular 风格 - 修复圆形运动
    static Vec3<float> getCircularEatAnimation(float t) {
        float angle = t * 3.14159f * 3.0f; // 减少圈数
        float intensity = sin(t * 3.14159f); // 强度控制
        
        float rotX = 12.0f * sin(angle) * intensity;
        float rotY = 12.0f * cos(angle) * intensity;
        float rotZ = 6.0f * sin(angle * 1.5f) * intensity;
        
        return Vec3<float>(rotX, rotY, rotZ);
    }

    // Shake 风格 - 修复震动效果
    static Vec3<float> getShakeEatAnimation(float t) {
        float eatCycle = sin(t * 3.14159f); // 基础吃东西周期
        float shake = sin(t * 3.14159f * 15.0f) * eatCycle * 0.5f; // 减少抖动强度
        
        float rotX = -8.0f * eatCycle + shake * 4.0f;
        float rotY = shake * 6.0f;
        float rotZ = shake * 3.0f;
        
        return Vec3<float>(rotX, rotY, rotZ);
    }

    // Pull 风格 - 修复拉拽动作
    static Vec3<float> getPullEatAnimation(float t) {
        float pullCycle = sin(t * 3.14159f * 2.5f); // 拉拽周期
        float intensity = sin(t * 3.14159f); // 整体强度
        
        float rotX = -18.0f * intensity + pullCycle * 4.0f;
        float rotY = 6.0f * pullCycle * intensity;
        float rotZ = 2.0f * sin(t * 3.14159f * 4.0f) * intensity;
        
        return Vec3<float>(rotX, rotY, rotZ);
    }

    // Twist 风格 - 修复扭转动作
    static Vec3<float> getTwistEatAnimation(float t) {
        float twist = t * 3.14159f * 4.0f; // 减少扭转强度
        float fade = sin(t * 3.14159f);
        
        float rotX = 8.0f * sin(twist * 0.4f) * fade;
        float rotY = 15.0f * cos(twist * 0.6f) * fade;
        float rotZ = 20.0f * sin(twist * 0.8f) * fade;
        
        return Vec3<float>(rotX, rotY, rotZ);
    }

    // 修复平移动画的偏差
    static Vec3<float> getEatTranslation(ViewModel* viewMod) {
        if (!wasEating || !viewMod->EatAnim) return Vec3<float>(0.f, 0.f, 0.f);
        
        float progress = (animationTime - eatStartTime) * viewMod->eatSpeed;
        float eatCycleDuration = 1.2f;
        float t = fmod(progress, eatCycleDuration) / eatCycleDuration;
        
        switch(viewMod->eatAnimMode) {
            case 0: // Fancy - 减少位移强度
                return Vec3<float>(
                    0.03f * sin(t * 3.14159f * 4.0f), 
                    -0.06f * sin(t * 3.14159f * 2.0f), 
                    0.12f * sin(t * 3.14159f)
                );
            case 1: // Circular - 修复圆形位移
                return Vec3<float>(
                    0.04f * cos(t * 3.14159f * 3.0f), 
                    0.04f * sin(t * 3.14159f * 3.0f), 
                    0.08f * sin(t * 3.14159f * 1.5f)
                );
            case 2: // Shake - 减少震动位移
                return Vec3<float>(
                    0.02f * sin(t * 3.14159f * 20.0f), 
                    0.02f * cos(t * 3.14159f * 20.0f), 
                    0.06f * sin(t * 3.14159f)
                );
            case 3: // Pull - 修复拉拽位移
                return Vec3<float>(
                    0.015f * sin(t * 3.14159f * 5.0f), 
                    -0.08f * sin(t * 3.14159f * 2.5f), 
                    0.15f * sin(t * 3.14159f)
                );
            case 4: // Twist - 减少扭转位移
                return Vec3<float>(
                    0.03f * sin(t * 3.14159f * 6.0f), 
                    0.025f * cos(t * 3.14159f * 4.0f), 
                    0.08f * sin(t * 3.14159f * 1.5f)
                );
            default:
                return Vec3<float>(0.f, 0.f, 0.f);
        }
    }

    static void* doBobHurtCallback(void* _this, glm::mat4* matrix) {
        auto result = oFunc(_this, matrix);
        static ViewModel* viewMod = ModuleManager::getModule<ViewModel>();
        auto player = GI::getLocalPlayer();
        
        if(!player || !viewMod || !viewMod->isEnabled())
            return result;

        // 更新动画时间
        animationTime = getCurrentTime();

        // 检测攻击动画
        bool currentSwinging = player->swinging;
        if (viewMod->AttackAnim && currentSwinging && !lastSwingingState) {
            isSwinging = true;
            swingStartTime = animationTime;
        }
        lastSwingingState = currentSwinging;

        // 检测吃东西动画
        bool currentEating = player->getItemUseDuration() > 0;
        if (viewMod->EatAnim && currentEating && !lastEatingState) {
            wasEating = true;
            eatStartTime = animationTime;
        }
        if (!currentEating) {
            wasEating = false;
        }
        lastEatingState = currentEating;

        // 应用变换
        {
            // 基础缩放
            float scaleX = viewMod->mhScale.x;
            float scaleY = viewMod->mhScale.y;
            float scaleZ = viewMod->mhScale.z;
            *matrix = glm::scale(*matrix, glm::vec3(scaleX, scaleY, scaleZ));

            // 基础旋转
            float rotX = glm::radians(viewMod->mhRot.x);
            float rotY = glm::radians(viewMod->mhRot.y);
            float rotZ = glm::radians(viewMod->mhRot.z);

            // 添加动画旋转
            Vec3<float> swingAnim = getSwingAnimation(viewMod);
            Vec3<float> eatAnim = getEatAnimation(viewMod);
            
            rotX += glm::radians(swingAnim.x + eatAnim.x);
            rotY += glm::radians(swingAnim.y + eatAnim.y);
            rotZ += glm::radians(swingAnim.z + eatAnim.z);

            if(rotX != 0.0f)
                *matrix = glm::rotate(*matrix, rotX, glm::vec3(1.0f, 0.0f, 0.0f));
            if(rotY != 0.0f)
                *matrix = glm::rotate(*matrix, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
            if(rotZ != 0.0f)
                *matrix = glm::rotate(*matrix, rotZ, glm::vec3(0.0f, 0.0f, 1.0f));

            // 基础平移
            float x = viewMod->mhTrans.x;
            float y = viewMod->mhTrans.y;
            float z = viewMod->mhTrans.z;
            
            // 添加动画平移
            Vec3<float> eatTrans = getEatTranslation(viewMod);
            x += eatTrans.x;
            y += eatTrans.y;
            z += eatTrans.z;
            
            *matrix = glm::translate(*matrix, glm::vec3(x, y, z));
        }
        
        return result;
    }

   public:
    doBobHurt() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&doBobHurtCallback;
    }
};