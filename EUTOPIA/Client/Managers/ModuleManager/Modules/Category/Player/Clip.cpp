#include "Clip.h"
#include <DrawUtil.h>

Clip::Clip() : Module("Clip", "Smart clipping with directional rendering", Category::MOVEMENT) {
    registerSetting(new SliderSetting<float>("Reduce", "Reduce hitbox size (0.0-0.29)", &offset, offset, 0.0f, 0.29f));
    registerSetting(new BoolSetting("Show Hitbox", "Show modified hitbox", &showHitbox, showHitbox));
}

void Clip::onEnable() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if (!localPlayer) return;
    
    // 保存原始AABB
    originalAABB = localPlayer->getAABB(true);
    hasOriginalAABB = true;
}

void Clip::onDisable() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if (!localPlayer || !hasOriginalAABB) return;
    
    // 恢复原始AABB
    localPlayer->setAABB(originalAABB);
    hasOriginalAABB = false;
}

bool Clip::isBlockSolid(BlockPos pos) {
    BlockSource* region = GI::getRegion();
    if (!region) return false;
    
    Block* block = region->getBlock(pos);
    if (!block || !block->blockLegcy) return false;
    
    uint16_t blockId = block->blockLegcy->blockid;
    // 检查是否为固体方块（非空气、非液体）
    return blockId != 0 && !(blockId >= 8 && blockId <= 11); // 排除水和岩浆
}

Vec3<float> Clip::getPlayerDirection(LocalPlayer* player) {
    // 获取玩家朝向
    Vec2<float> rotation = player->getRotation();
    float yaw = rotation.y * (3.14159f / 180.0f); // 转换为弧度
    
    return Vec3<float>(-sinf(yaw), 0.0f, cosf(yaw));
}

void Clip::onNormalTick(LocalPlayer* localPlayer) {
    if (!localPlayer) return;
    
    // 获取当前AABB
    AABB currentAABB = localPlayer->getAABB(true);
    Vec3<float> playerPos = localPlayer->getPos();
    
    // 计算新的AABB尺寸
    float standardWidth = 0.6f;
    float newWidth = standardWidth * (1.0f - offset);
    if (newWidth < 0.01f) newWidth = 0.01f;
    
    float halfWidth = newWidth * 0.5f;
    
    // 设置新的AABB
    AABB newAABB;
    newAABB.lower.x = playerPos.x - halfWidth;
    newAABB.lower.y = currentAABB.lower.y;
    newAABB.lower.z = playerPos.z - halfWidth;
    
    newAABB.upper.x = playerPos.x + halfWidth;
    newAABB.upper.y = currentAABB.upper.y;
    newAABB.upper.z = playerPos.z + halfWidth;
    
    localPlayer->setAABB(newAABB);
}

void Clip::onLevelRender() {
    if (!showHitbox) return;
    
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if (!localPlayer) return;
    
    Vec3<float> playerPos = localPlayer->getPos();
    AABB playerAABB = localPlayer->getAABB(true);
    
    // 获取玩家脚部位置的方块坐标
    BlockPos playerBlockPos = Vec3<float>(playerPos.x, playerAABB.lower.y, playerPos.z).floor().CastTo<int>();
    
    // 检查玩家AABB与周围方块的碰撞
    struct Direction {
        Vec3<int> offset;
        std::string name;
        UIColor color;
    };
    
    Direction directions[] = {
        {{1, 0, 0}, "East", UIColor(255, 0, 0, 120)},    // 红色 - 东
        {{-1, 0, 0}, "West", UIColor(0, 255, 0, 120)},   // 绿色 - 西
        {{0, 0, 1}, "South", UIColor(0, 0, 255, 120)},   // 蓝色 - 南
        {{0, 0, -1}, "North", UIColor(255, 255, 0, 120)} // 黄色 - 北
    };
    
    for (const auto& dir : directions) {
        // 检查玩家高度范围内的方块（脚部和头部）
        for (int y = 0; y < 2; y++) {
            BlockPos checkPos = playerBlockPos.add(dir.offset).add(Vec3<int>(0, y, 0));
            
            if (isBlockSolid(checkPos)) {
                // 创建方块的AABB
                AABB blockAABB;
                blockAABB.lower = checkPos.CastTo<float>();
                blockAABB.upper = blockAABB.lower.add(Vec3<float>(1.0f, 1.0f, 1.0f));
                
                // 检查玩家AABB是否与该方块AABB相交或非常接近
                bool intersects = false;
                
                // 扩展检查范围，考虑玩家可能靠着方块的情况
                AABB expandedPlayerAABB = playerAABB;
                float checkDistance = 0.1f; // 检查距离
                expandedPlayerAABB.lower = expandedPlayerAABB.lower.sub(Vec3<float>(checkDistance, 0, checkDistance));
                expandedPlayerAABB.upper = expandedPlayerAABB.upper.add(Vec3<float>(checkDistance, 0, checkDistance));
                
                // 检查X轴重叠
                bool xOverlap = expandedPlayerAABB.lower.x < blockAABB.upper.x && expandedPlayerAABB.upper.x > blockAABB.lower.x;
                // 检查Y轴重叠
                bool yOverlap = expandedPlayerAABB.lower.y < blockAABB.upper.y && expandedPlayerAABB.upper.y > blockAABB.lower.y;
                // 检查Z轴重叠
                bool zOverlap = expandedPlayerAABB.lower.z < blockAABB.upper.z && expandedPlayerAABB.upper.z > blockAABB.lower.z;
                
                intersects = xOverlap && yOverlap && zOverlap;
                
                if (intersects) {
                    // 使用不同颜色渲染不同方向的方块
                    UIColor lineColor = dir.color;
                    lineColor.a = 255; // 边框不透明
                    
                    DrawUtil::drawBox3dFilled(blockAABB, dir.color, lineColor, 1.0f);
                    break; // 找到一个就够了，避免重复渲染
                }
            }
        }
    }
    
    // 移除玩家hitbox渲染（按用户要求）
    // 不再渲染玩家自身的ESP
}