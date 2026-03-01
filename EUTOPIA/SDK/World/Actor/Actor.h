#pragma once
#include "../../NetWork/MinecraftPacket.h"
#include "../../NetWork/PacketSender.h"
#include "../../NetWork/Packets/MovePlayerPacket.h"
#include "../../NetWork/Packets/PacketID.h"
#include "../../NetWork/Packets/PlayerAuthInputPacket.h"
#include "../../NetWork/Packets/SetPlayerGameTypePacket.h"
#include "../../Utils/MemoryUtil.h"
#include "../Effect/MobEffectInstance.h"
#include "../Inventory/ContainerManagerModel.h"
#include "../Inventory/PlayerInventory.h"
#include "../Inventory/SimpleContainer.h"
#include "../Level/Level.h"
#include "ActorFlags.h"
#include "Components/AABBShapeComponent.h"
#include "Components/ActorDataFlagComponent.h"
#include "Components/ActorEquipmentComponent.h"
#include "Components/ActorGameTypeComponent.h"
#include "Components/ActorHeadRotationComponent.h"
#include "Components/ActorRotationComponent.h"
#include "Components/ActorTypeComponent.h"
#include "Components/AttributesComponent.h"
#include "Components/DimensionTypeComponent.h"
#include "Components/FallDistanceComponent.h"
#include "Components/FlagComponent.h"
#include "Components/JumpControlComponent.h"
#include "Components/MobBodyRotationComponent.h"
#include "Components/MobHurtTimeComponent.h"
#include "Components/MoveInputComponent.h"
#include "Components/RenderPositionComponent.h"
#include "Components/RuntimeIDComponent.h"
#include "Components/StateVectorComponent.h"
#include "GameMode.h"
#include "../Utils\Minecraft\ActorUtils.h"
#include "../Utils\Minecraft\ActorCache.h"
#include "..\Utils\Minecraft\Utils.h"
#include "..\Utils\PlusUtils.h"
#include "..\Utils\Minecraft\PacketUtil.h"

class Actor {
   public:
    void setGameType(int gameType);
    void setPos(Vec3<float> pos);
    void sendToSelf(const std::shared_ptr<Packet>& packet);
    void SetPosition(StateVectorComponent* Component,  Vec3<float> Position);
    void setPos2(Vec3<float> pos);
    void teleport(Actor* actor, const Vec3<float>& pos);

   public:
    CLASS_MEMBER(Level*, level, 0x1d8)
    CLASS_MEMBER(GameMode*, gamemode, 0xA88);
    CLASS_MEMBER(bool, destroying, 0xAED);
    CLASS_MEMBER(StateVectorComponent*, stateVector, 0x290);
    CLASS_MEMBER(bool, swinging, 0x42C)
    CLASS_MEMBER(PlayerInventory*, supplies, 0x5c8);
    //CLASS_MEMBER(ActorRotationComponent*, rotation, 0x2A0);

    Vec3<float> getHumanPos() {
        Vec3<float> targetPos = this->getEyePos();

        if(this->getActorTypeComponent()->id != ActorType::Player)
            return targetPos.sub(Vec3<float>(0.f, 1.6f, 0.f));
        return targetPos;
    }
    void setSwinging(bool swinging) {
        this->swinging = swinging;
    }

    int getSwingProgress() {
        if(this == nullptr)
            return 0;  // 防止空指针崩溃
        return hat::member_at<int>(this, 0x42C + 0x4);
    }

    void teleportTo(const Vec3<float>& pos, bool shouldStopRiding, int a, int b,
                    bool keepVelocity) {
        MemoryUtil::callVirtualFunc<void, const Vec3<float>&, int, int, bool>(
            21, this, pos, shouldStopRiding, a, b, keepVelocity);
    }




    int getOldSwingProgress() {
        if(this == nullptr)
            return 0;  // 防止空指针崩溃
        return hat::member_at<int>(this, 0x42C + 0x8);
    }

    void setSwingProgress(int progress) {
        if(this == nullptr)
            return;  // 防止空指针崩溃
        hat::member_at<int>(this, 0x42C + 0x4) = progress;
    }

    EntityContext* getEntityContext() {
        uintptr_t address = reinterpret_cast<uintptr_t>(this);
        return reinterpret_cast<EntityContext*>((uintptr_t)this + 0x8);
    }

    bool getStatusFlag(ActorFlags flag) {
        return getEntityContext()->tryGetComponent<ActorDataFlagComponent>()->getStatusFlag(flag);
    }

    void setStatusFlag(ActorFlags flag, bool value) {
        return getEntityContext()->tryGetComponent<ActorDataFlagComponent>()->setStatusFlag(flag,
                                                                                            value);
    }

    AABBShapeComponent* getAABBShapeComponent() {
        return getEntityContext()->tryGetComponent<AABBShapeComponent>();
    }

    AttributesComponent* getAttributesComponent() {
        return getEntityContext()->tryGetComponent<AttributesComponent>();
    }

    MoveInputComponent* getMoveInputComponent() {
        return getEntityContext()->tryGetComponent<MoveInputComponent>();
    }

    StateVectorComponent* getStateVectorComponent() {
        return getEntityContext()->tryGetComponent<StateVectorComponent>();
    }

    DimensionTypeComponent* getDimensionTypeComponent() {
        return getEntityContext()->tryGetComponent<DimensionTypeComponent>();
    }

    JumpControlComponent* getJumpControlComponent() {
        return getEntityContext()->tryGetComponent<JumpControlComponent>();
    }

    RenderPositionComponent* getRenderPositionComponent() {
        return getEntityContext()->tryGetComponent<RenderPositionComponent>();
    }

    ActorTypeComponent* getActorTypeComponent() {
        return getEntityContext()->tryGetComponent<ActorTypeComponent>();
    }

    ActorEquipmentComponent* getActorEquipmentComponent() {
        return getEntityContext()->tryGetComponent<ActorEquipmentComponent>();
    }

    ActorRotationComponent* getActorRotationComponent() {
        return getEntityContext()->tryGetComponent<ActorRotationComponent>();
    }

    ActorHeadRotationComponent* getActorHeadRotationComponent() {
        return getEntityContext()->tryGetComponent<ActorHeadRotationComponent>();
    }

    MobBodyRotationComponent* getMobBodyRotationComponent() {
        return getEntityContext()->tryGetComponent<MobBodyRotationComponent>();
    }

    JumpControlComponent* getJumpControl() {
        return getEntityContext()->tryGetComponent<JumpControlComponent>();
    }

    RuntimeIDComponent* getRuntimeIDComponent() {
        return getEntityContext()->tryGetComponent<RuntimeIDComponent>();
    }

    MobHurtTimeComponent* getMobHurtTimeComponent() {
        return getEntityContext()->tryGetComponent<MobHurtTimeComponent>();
    }

    int32_t getHurtTime() {
        return (getMobHurtTimeComponent()->mHurtTime);
    }

    Vec2<float> getRotation() {
        return Vec2<float>(getActorRotationComponent()->mYaw, getActorRotationComponent()->mPitch);
    }

    float getBodyRot() {
        return (getMobBodyRotationComponent()->yBodyRot);
    }

    float getHeadRot() {
        return getActorHeadRotationComponent()->mHeadRot;
    }

    Vec2<float> getOldRotation() {
        return Vec2<float>(getActorRotationComponent()->mOldYaw,
                           getActorRotationComponent()->mOldPitch);
    }

    Vec2<float> SetRotaion(float yaw, float pitch) {
        getActorRotationComponent()->mYaw = yaw;
        getActorRotationComponent()->mPitch = pitch;
        return 0;
    }

    Vec3<float> getEyePos() {
        auto comp = getRenderPositionComponent();
        if(!comp) {
            return Vec3<float>(0.f, 0.f, 0.f);
        }
        return comp->mPosition;
    }

   



    void swing() {
        MemoryUtil::callVirtualFunc<void>(111, this);
    }

    int getGameType() {
        return getEntityContext()->tryGetComponent<ActorGameTypeComponent>()->mGameType;
    }

    AABB getAABB(bool normal) {
        if(normal) {
            return AABB(this->getAABBShapeComponent()->mMin, this->getAABBShapeComponent()->mMax);
        }
    }

    void setAABB(AABB& aabb) {
        AABBShapeComponent* aabbShapeComponent = getAABBShapeComponent();
        if(!aabbShapeComponent)
            return;
        aabbShapeComponent->mMin = aabb.lower;
        aabbShapeComponent->mMax = aabb.upper;
    }

    bool isPlayer() {
        auto actorType = getActorTypeComponent();
        if(!actorType)
            return false;
        return actorType->id == ActorType::Player;
    }

    Vec3<float> getPos() {
        return getStateVectorComponent()->mPos;
    }

    ItemStack* getCarriedItem() {
        return MemoryUtil::callVirtualFunc<ItemStack*>(83, this);
    }

    Vec3<float> getPosition() {
        if(this->stateVector) {
            return stateVector->mPos;
        } else {
            return {0.f, 0.f, 0.f};
        }
    }

    Vec3<float> getPosPrev() {
        return getStateVectorComponent()->mPosOld;
    }

    SimpleContainer* getArmorContainer() {
        return getActorEquipmentComponent()->mArmorContainer;
    }

    bool isAlive() {
        return !isDead();
    }

    ItemStack* getArmor(int slot) {
        if(getActorEquipmentComponent() == nullptr)
            return nullptr;
        return getArmorContainer()->getItem(slot);
    }

    ItemStack* getOffhandSlot() {
        ActorEquipmentComponent* actorEquipmentComponent = getActorEquipmentComponent();
        if(actorEquipmentComponent == nullptr)
            return nullptr;

        SimpleContainer* offhandContainer = actorEquipmentComponent->mOffhandContainer;

        return (ItemStack*)(*(__int64(__fastcall**)(SimpleContainer*, __int64))(
            *(uintptr_t*)offhandContainer + 56i64))(offhandContainer, 1i64);
    }

    int64_t getRuntimeID() {
        auto runtimeIDComponent = getEntityContext()->tryGetComponent<RuntimeIDComponent>();
        return runtimeIDComponent ? runtimeIDComponent->mRuntimeID : -1;
    }

    float calculateDamage(Actor* target) {
        static auto address = MemoryUtil::getFuncFromCall(
            MemoryUtil::findSignature("E8 ? ? ? ? 44 0F 28 C0 0F 57 FF 44 0F 2F C7 0F 86"));
        using calDamage = float(__thiscall*)(Actor*, Actor*);
        static calDamage func = (calDamage)address;
        return func(this, target);
    }

    MobEffectInstance* hasEffect(MobEffect* effect) {
        if(this == nullptr || !effect)
            return nullptr;

        uint32_t effectId = hat::member_at<uint32_t>(effect, 0x8);

        void* effectContainer = hat::member_at<void*>(this, 0x10);  // Actor+0x10
        int actorValue = hat::member_at<int>(this, 0x18);           // Actor+0x18

        using geteffect_t = void*(__fastcall*)(void*, int*);
        static auto address = MemoryUtil::getFuncFromCall(MemoryUtil::findSignature(
            "E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? ? ? ? 49 BE ? ? ? ? ? ? ? ? 48 8B 40 ? 8B D5 48 2B "
            "C1 48 C1 F8 ? 49 0F AF C6 48 3B C5 76 ? 48 69 C2 ? ? ? ? 48 03 C8 8B 05 ? ? ? ? ? ? "
            "74 ? 48 85 C9 74 ? 48 8B 4F"));
        static geteffect_t geteffect_func = (geteffect_t)address;

        void* container = geteffect_func(effectContainer, &actorValue);
        if(!container)
            return nullptr;

        uint64_t* containerPtr = (uint64_t*)container;
        uint64_t containerStart = containerPtr[0];
        uint64_t containerEnd = containerPtr[1];

        uint64_t containerSize = containerEnd - containerStart;
        uint64_t instanceCount = containerSize / 136;

        if(effectId >= instanceCount)
            return nullptr;

        uint64_t instanceAddress = containerStart + (effectId * 136);
        uint32_t* instance = (uint32_t*)instanceAddress;

        static uint32_t invalidMarker = 0;
        if(*instance == invalidMarker)
            return nullptr;

        return (MobEffectInstance*)instance;
    }

    bool onGround() {
        return getEntityContext()->hasComponent<OnGroundFlagComponent>();
    }
    void jumpFromGround() {
        return getEntityContext()->getOrAddComponent<JumpFromGroundRequestComponent>();
    }
    void setOnGround(bool state) {
        if(state) {
            return getEntityContext()->getOrAddComponent<OnGroundFlagComponent>();
        } else {
            getEntityContext()->removeComponent<OnGroundFlagComponent>();
        }
    };
    bool isInWater() {
        return getEntityContext()->hasComponent<WasInWaterFlagComponent>();
    }
    bool isInLava() {
        return getEntityContext()->hasComponent<WasInLavaFlagComponent>();
    }
    bool isInLiquid() {
        return this->isInWater() || this->isInLava();
    }

    bool isDead() {
        return getEntityContext()->hasComponent<IsDeadFlagComponent>();
    }

    bool isCollidingHorizontal() {
        return getEntityContext()->hasComponent<HorizontalCollisionFlagComponent>();
    }

    float getFallDisntance() {
        return getEntityContext()->tryGetComponent<FallDistanceComponent>()->mFallDistance;
    }

    void setFallDisntance(float vaule) {
        getEntityContext()->tryGetComponent<FallDistanceComponent>()->mFallDistance = vaule;
    }

    const std::string& getNameTag() {
        static auto func = MemoryUtil::findSignature(
            "48 83 EC ? 48 8B 81 ? ? ? ? 48 85 C0 74 ? 48 89 5C 24 ? B9 ? ? ? ? 48 8B 18 48 8B 40 "
            "? 48 2B C3 48 C1 F8 ? 66 3B C8 73 ? 48 8B 5B ? 48 85 DB 74 ? 48 8B 03 48 8B CB 48 8B "
            "40 ? FF 15 ? ? ? ? 3C ? 48 8D 43");
        ;
        return *MemoryUtil::callFastcall<std::string*>(func, this);
    }

    void setNametag(const std::string& name) {
        static auto func = MemoryUtil::getFuncFromCall(
            MemoryUtil::findSignature("E8 ? ? ? ? 4C 8D ? ? ? ? ? 8B D3 48 8B ? E8 ? ? ? ? E9"));
        MemoryUtil::callFastcall<void, void*, std::string>(func, this, name);
    }

    float getMaxHealth() {
        auto health = getAttribute(Health);
        if(!health)
            return 0;
        return health->mCurrentMaxValue;
    }

    float getHealth() {
        auto health = getAttribute(Health);
        if(!health)
            return 0;
        return health->mCurrentValue;
    }

    float getAbsorption() {
        auto absorption = getAttribute(Absorption);
        if(!absorption)
            return 0;
        return absorption->mCurrentValue;
    }

    float getMaxAbsorption() {
        auto absorption = getAttribute(Absorption);
        if(!absorption)
            return 0;
        return absorption->mCurrentMaxValue;
    }

    AttributeInstance* getAttribute(AttributeId id) {
        return getAttribute(static_cast<int>(id));
    }

    AttributeInstance* getAttribute(int id) {
        auto component = getAttributesComponent();
        if(!component) {
            return nullptr;
        }
        return component->mBaseAttributeMap.getInstance(id);
    }

    void lerpMotion(const Vec3<float>& delta) {
        MemoryUtil::callVirtualFunc<void, const Vec3<float>&>(22, this, delta);
    }

    bool isAliveCheck() {
        return MemoryUtil::callVirtualFunc<bool>(47, this);
    }

    // ItemStack* getCarriedItem() {
    //	return MemoryUtil::callVirtualFunc<ItemStack*>(77, this);
    // }//这个虚表有问题

    void setOffhandSlot(ItemStack* itemStack) {
        MemoryUtil::callVirtualFunc<void, ItemStack*>(79, this, itemStack);
    }
    // bool canSee(Actor* actor)
    // {
    // 	return MemoryUtil::callFastcallWithRel<bool>(Addresses::Actor_canSee, this, actor);
    // }
    bool IsinWaterOrRain() {
        static auto func = MemoryUtil::getFuncFromCall(
            MemoryUtil::findSignature("E8 ? ? ? ? 84 C0 74 ? 66 C7 87"));
        return MemoryUtil::callFastcall<bool, void*>(func, this);
    }
    ContainerManagerModel* getContainerManagerModel() {
        return hat::member_at<ContainerManagerModel*>(this, 0x5B0);
    }
};