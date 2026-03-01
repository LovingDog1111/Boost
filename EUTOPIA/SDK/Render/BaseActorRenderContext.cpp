#include "BaseActorRenderContext.h"

#include "../../Utils/MemoryUtil.h"
#include "../Core/ClientInstance.h"
#include "../Core/MinecraftGame.h"
#include "ScreenContext.h"

BaseActorRenderContext::BaseActorRenderContext(ScreenContext *ctx, ClientInstance *cInst,
                                               MinecraftGame *game) {
    using oFunc_t = BaseActorRenderContext *(*)(BaseActorRenderContext *, ScreenContext *,
                                                ClientInstance *, MinecraftGame *);
    memset(this, 0, sizeof(BaseActorRenderContext));
    static auto oFunc = reinterpret_cast<oFunc_t>(MemoryUtil::findSignature(
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 4C 24 ? 57 48 83 EC ? 49 8B F8 48 "
        "8B DA 48 8B F1 48 8D 05 ? ? ? ? ? ? ? 33 ED"));
    oFunc(this, ctx, cInst, game);
}

BaseActorRenderContext::~BaseActorRenderContext() {}