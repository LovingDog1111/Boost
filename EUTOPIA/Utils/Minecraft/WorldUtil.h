#pragma once

// Forward declarations / core includes
#include "../Maths.h"         // Vec3 & maths helpers (relative path: Utils/Maths.h)
#include "../../SDK/World/Actor/Actor.h"  // Actor definition

// Utility helpers for generic world-level maths / queries
namespace WorldUtil {

        static inline std::map<int, Vec3<int>> blockFaceOffsets = {
    {1,  Vec3<int>(0, -1, 0)}, {2,  Vec3<int>(0, 0, 1)},  {3,  Vec3<int>(0, 0, -1)},
    {4,  Vec3<int>(1, 0, 0)},  {5,  Vec3<int>(-1, 0, 0)}, {0,  Vec3<int>(0, 1, 0)}};

    // Distance between two positions (in blocks)
        float distanceToPoint(const Vec3<float>& pos1, const Vec3<float>& pos2);

        float getSeenPercent(const Vec3<float>& pos, const AABB& aabb);

        float getSeenPercent(const Vec3<float>& pos, Actor* actor);

    // Distance between a position and an entity's current position
    float distanceToEntity(const Vec3<float>& pos, Actor* entity);
    bool canBuildOn(const BlockPos& blockPos);
    float distanceToBlock(const Vec3<float>& pos, const BlockPos& blockPos);
    bool isValidPlacePos(Vec3<int> blockPos);
    int getBlockPlaceFace(Vec3<int> blockPos);
    void placeBlock(Vec3<int> pos, int side);
    bool isAirBlock(Vec3<int> blockPos);
    int getExposedFace(Vec3<int> blockPos, bool useFilter);
    void startDestroyBlock(Vec3<int> pos, int side);
    void destroyBlock(Vec3<int> pos, int side, bool useTransac);
    Block* getBlock(const BlockPos& blockPos);

    } 
