// 在 PopCounter.h 中添加这些包含和结构体定义
#pragma once
#include <AnimationUtil.h>
#include <ColorUtil.h>
#include <DrawUtil.h>

#include <chrono>
#include <vector>

#include "../../ModuleBase/Module.h"

// 爆图腾动画数据结构
struct PopAnimationData {
    Vec3<float> startPos;    // 起始位置
    Vec3<float> currentPos;  // 当前位置
    float startTime;         // 开始时间
    float duration;          // 动画持续时间
    UIColor color;           // 随机颜色
    UIColor lineColor;       // 边框颜色
    float pitch;             // 玩家俯仰角
    float yaw;               // 玩家偏航角
    bool isFinished;         // 是否完成

    PopAnimationData(Vec3<float> pos, float p, float y) {
        startPos = pos;
        currentPos = pos;
        startTime = GetTickCount64() / 1000.0f;
        duration = 3.0f;  // 3秒动画
        pitch = p;
        yaw = y;
        isFinished = false;

        // 生成随机颜色
        generateRandomColor();
    }

   private:
    void generateRandomColor() {
        // 生成鲜艳的随机颜色
        int colorType = rand() % 6;
        switch(colorType) {
            case 0:
                color = UIColor(255, rand() % 128 + 127, rand() % 128, 200);
                break;  // 红色系
            case 1:
                color = UIColor(rand() % 128, 255, rand() % 128 + 127, 200);
                break;  // 绿色系
            case 2:
                color = UIColor(rand() % 128, rand() % 128 + 127, 255, 200);
                break;  // 蓝色系
            case 3:
                color = UIColor(255, 255, rand() % 128, 200);
                break;  // 黄色系
            case 4:
                color = UIColor(255, rand() % 128, 255, 200);
                break;  // 品红系
            case 5:
                color = UIColor(rand() % 128 + 127, 255, 255, 200);
                break;  // 青色系
        }
        lineColor = UIColor(color.r, color.g, color.b, 255);  // 边框完全不透明
    }
};

class PopCounter : public Module {
   private:
    bool sendchat = false;
    bool popcham = true;
    std::string names = "PopCounter";

    // 动画相关
    std::vector<PopAnimationData> popAnimations;

   public:
    PopCounter();
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onLevelRender() override;
};