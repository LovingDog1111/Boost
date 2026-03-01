#pragma once
#include "../../ModuleBase/Module.h"

class NewChunks : public Module {
   public:
    std::unordered_map<uint64_t, std::optional<ChunkPos>> chunkMap;
    size_t chunkCount = 0;
    bool detectFlowing = false;

   private:
    std::unordered_map<size_t, bool> lineRenderList;

    void renderNewChunk(const ChunkPos& chunkPos);

   public:
    NewChunks();
    float yHeight = 30.f;
    std::string getModeText() override;
    void onMCRender(MinecraftUIRenderContext* renderCtx) override;
};