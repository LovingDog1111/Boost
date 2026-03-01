#pragma once
#include "DamageUtil.h"

// 添加护甲值计算函数
int getArmorProtectionValue(ItemStack* armor) {
    if(!armor || !armor->valid || !armor->mItem) {
        return 0;
    }

    Item* item = armor->mItem.get();
    if(!item) {
        return 0;
    }
    if(item->mProtection > 0) {
        return item->mProtection;
    }

    std::string itemName = item->mName;

    int armorValue = 0;

    // 确定护甲类型
    bool isHelmet = itemName.find("_helmet") != std::string::npos;
    bool isChestplate = itemName.find("_chestplate") != std::string::npos;
    bool isLeggings = itemName.find("_leggings") != std::string::npos;
    bool isBoots = itemName.find("_boots") != std::string::npos;

    // 确定材质并计算护甲值
    if(itemName.find("leather_") != std::string::npos) {
        if(isHelmet || isBoots)
            armorValue = 1;
        else if(isLeggings)
            armorValue = 2;
        else if(isChestplate)
            armorValue = 3;
    } else if(itemName.find("golden_") != std::string::npos) {
        if(isHelmet || isBoots)
            armorValue = 1;
        else if(isLeggings)
            armorValue = 3;
        else if(isChestplate)
            armorValue = 5;
    } else if(itemName.find("chainmail_") != std::string::npos) {
        if(isHelmet)
            armorValue = 2;
        else if(isBoots)
            armorValue = 1;
        else if(isLeggings)
            armorValue = 4;
        else if(isChestplate)
            armorValue = 5;
    } else if(itemName.find("iron_") != std::string::npos) {
        if(isHelmet)
            armorValue = 2;
        else if(isBoots)
            armorValue = 2;
        else if(isLeggings)
            armorValue = 5;
        else if(isChestplate)
            armorValue = 6;
    } else if(itemName.find("diamond_") != std::string::npos) {
        if(isHelmet || isBoots)
            armorValue = 3;
        else if(isLeggings)
            armorValue = 6;
        else if(isChestplate)
            armorValue = 8;
    } else if(itemName.find("netherite_") != std::string::npos) {
        if(isHelmet || isBoots)
            armorValue = 3;
        else if(isLeggings)
            armorValue = 6;
        else if(isChestplate)
            armorValue = 8;
    }

    return armorValue;
}

float DamageUtils::getExplosionDamage(const Vec3<float>& position, Actor* actor,
                                      float extrapolation, int ignoredBlockId) {
    static constexpr float explosionRadius = 12.f;
    static constexpr float armorReductionF = 0.035f;
    static constexpr float maxEpf = 25.f;
    static constexpr float maxPR = 20.f;
    Vec3<float> targetPosition = actor->getHumanPos();
    Vec3<float> predictVal = actor->getStateVectorComponent()->mVelocity.mul(extrapolation);
    Vec3<float> finalPos = targetPosition.add(predictVal);
    const float distanceToExplosion = finalPos.dist(position) / explosionRadius;
    if(distanceToExplosion > 1.f)
        return 0.f;
    float exposure =
        GI::getRegion()->getSeenPercent(position, actor->getAABBShapeComponent()->getAABB());
    if(ignoredBlockId != 0)
        exposure = GI::getRegion()
                               ->getBlock(Vec3<int>(position.x, position.y, position.z))
                               ->getblockLegcy()
                               ->blockid == ignoredBlockId
                       ? 1.f
                       : GI::getRegion()->getSeenPercent(position,
                                                         actor->getAABBShapeComponent()->getAABB());
    else
        exposure =
            GI::getRegion()->getSeenPercent(position, actor->getAABBShapeComponent()->getAABB());
    const float explosionImpact = (1 - distanceToExplosion) * exposure;
    /*
     * PEACEFUL: no damage increase
     * NORMAL: damage / 2 + 1
     * HARD: damage * 3 / 2
     */
    float dmg = ((explosionImpact * explosionImpact * 3.5f + explosionImpact * 0.5f * 7.f) *
                     explosionRadius +
                 1.f);
    int totalArmorPoints = 0;
    float epf = 0.f;
    for(int i = 0; i < 4; i++) {
        ItemStack* armor = actor->getArmor(i);
        if(armor->valid) {
            totalArmorPoints += getArmorProtectionValue(armor);
            epf += 5.f;  // you cant really get their enchants in bedrock only in java so we
                         // assuming ig
        }
    }
    dmg -= dmg * totalArmorPoints * armorReductionF;
    return std::max(
        0.0f, dmg -= dmg * std::min(ceilf(std::min(epf, maxEpf) * 0.75), maxPR) * armorReductionF);
}