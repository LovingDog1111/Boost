//
// Created by vastrakai on 7/22/2024.
//

#pragma once
#include "glm/detail/type_quat.hpp"
#include "../../../Core/HashedString.h"
#include "../../../../Utils/Maths.h"
#include "../../../../Utils/MemoryUtil.h"
// Credit: disabledmallis on discord

enum class CameraMode : int {
    FirstPerson = 0,
    ThirdPerson = 1,
    ThirdPersonFront = 2,
    DeathCamera = 3,
    Free = 4,
    FollowOrbit = 5,
};

class CameraComponent {
public:
    HashedString mViewName{};
    glm::quat mQuat{};
   Vec3<float> mOrigin{};
    Vec3<float> mFov{}; // mViewport? (Fov X, Fov Y, Near, Far)?
    glm::mat4 mWorld{};
    glm::mat4 mView{};
    glm::mat4 mProjection{};
    int8_t padding[4]{0};

    void copyFrom(const CameraComponent* other) {
        mQuat = other->mQuat;
        mOrigin = other->mOrigin;
        mFov = other->mFov;
        mWorld = other->mWorld;
        mView = other->mView;
        mProjection = other->mProjection;
    }

    [[nodiscard]] CameraMode getMode() const {
        if (mViewName.str == "minecraft:first_person") return CameraMode::FirstPerson;
        if (mViewName.str == "minecraft:third_person") return CameraMode::ThirdPerson;
        if (mViewName.str == "minecraft:third_person_front") return CameraMode::ThirdPersonFront;
        if (mViewName.str == "minecraft:death_camera") return CameraMode::DeathCamera;
        if (mViewName.str == "minecraft:free") return CameraMode::Free;
        if (mViewName.str == "minecraft:follow_orbit") return CameraMode::FollowOrbit;

        return CameraMode::FirstPerson;
    }
};
static_assert(sizeof(CameraComponent) == 0x120);

class StationaryCameraComponent
{
public:
    glm::vec3 mPosition;
    glm::vec3 mRots; // In degrees, not sure if the z is used

    StationaryCameraComponent() = default;
    StationaryCameraComponent(glm::vec3 pos, glm::vec3 rots, float idk) : mPosition(pos), mRots(rots) {}
};

class CameraDirectLookComponent {
public:
    glm::vec2 mRotRads{};
    float mDelta = 0.0f;
    glm::vec2 mWrap{};

    void operator=(const CameraDirectLookComponent& other) {
        mRotRads = other.mRotRads;
        mDelta = other.mDelta;
        mWrap = other.mWrap;
    }
};

struct CameraWorldSpaceRotationComponent
{
public:
    char sb1[0x10];
};

class CameraFlyMoveComponent
{
public:
    char sb11[0x8];
};

class CameraOrbitComponent
{
public:
    char sb111[0x44]; // Padding to match size

    CLASS_MEMBER(float, mRadius, 0x28);
    CLASS_MEMBER(Vec2<float>, mRotRads, 0x2C);
};

class DebugCameraComponent
{
public:
    char sb1111[0x128];

    CLASS_MEMBER(Vec3<float>, mRotRads, 0x30); // i don't think this is correct :/
    CLASS_MEMBER(Vec3<float>, mOrigin, 0x40);
    CLASS_MEMBER(Vec2<float>, mFov, 0x4C);
    CLASS_MEMBER(float, mNearClippingPlane, 0x54);
    CLASS_MEMBER(float, mFarClippingPlane, 0x58);
};

struct CameraLookAtPositionComponent
{
public:
    char sb11111[0x12];
};

struct CameraOffsetComponent
{
public:
    Vec3<float> mPosOffset;
    Vec2<float> mRotOffset;
};

class UpdatePlayerFromCameraComponent
{
public:
    int mUpdateMode = 0;

    UpdatePlayerFromCameraComponent() = default;
    UpdatePlayerFromCameraComponent(int mode) : mUpdateMode(mode) {}
};

struct CameraAvoidanceComponent
{
    char jb[8];
};

struct ShadowOffsetComponent
{
    float mOffset = 0.0f;
};

struct SubBBsComponent
{
    std::vector<AABB> mSubBBs;
};