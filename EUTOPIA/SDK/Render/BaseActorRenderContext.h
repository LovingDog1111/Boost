#pragma once
#include <array>
#include "../../Utils/Maths.h"
#include "../../Utils/MemoryUtil.h"
#include <optional>
#include "ItemRenderer.h"

// Ç°ÏòÉùÃ÷
class ScreenContext;
class ClientInstance;
class MinecraftGame;
class ItemInHandRenderer;
class ActorRenderDispatcher;

struct HistoricalFrameTimes {
    uint64_t mFrameIndex;                                // 0x0000
    std::array<float, 30> mHistoricalMinimumFrameTimes;  // 0x0008
    std::array<float, 30> mHistoricalMaximumFrameTimes;  // 0x0080
    std::array<float, 30> mHistoricalAverageFrameTimes;  // 0x00F8
    std::array<float, 30> mLastFrameTime;                // 0x0170
};
static_assert(sizeof(HistoricalFrameTimes) == 0x1E8);

class BaseActorRenderContext {
public:
    char pad[0x500];  // TODO: check actual size

    CLASS_MEMBER(ClientInstance*, clientInstance, 0x8);
    CLASS_MEMBER(ScreenContext*, screenContext, 0x28);
    CLASS_MEMBER(ActorRenderDispatcher*, actorRenderDispatcher, 0x38);
    CLASS_MEMBER(ItemInHandRenderer*, itemInHandRenderer, 0x50);
    CLASS_MEMBER(ItemRenderer*, itemRenderer, 0x58);

    BaseActorRenderContext(ScreenContext* ctx, ClientInstance* cInst, MinecraftGame* game);
    ~BaseActorRenderContext();
};  // Size: 0x02A0

