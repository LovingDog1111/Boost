#pragma once
#include <Maths.h>
#include "ColorUtil.h"
#include "../SDK/Render/ScreenContext.h"
#include "../SDK/Render/LevelRender.h"
#include "../SDK/Render/MinecraftUIRenderContext.h"
#include "../SDK/Render/BaseActorRenderContext.h"
#include "..\SDK\World\Item\ItemStack.h"
namespace DrawUtil {
    struct Draw3dData {
        Vec3<float> pos;
        UIColor color;

        Draw3dData(const Vec3<float>& _pos, const UIColor& _color) {
            pos = _pos;
            color = _color;
        }
    };



    struct BoxRotation {
        float rotX;
        float rotY;
        float rotZ;
        BoxRotation(float _x = 0.f, float _y = 0.f, float _z = 0.f)
            : rotX(_x), rotY(_y), rotZ(_z) {}
    };
    extern MinecraftUIRenderContext* renderCtx;
    extern ShaderColor* currentColor;
    extern ScreenContext* screenCtx;
    extern ScreenContext* D3DCTX;
    extern Tessellator* tessellator;
    extern mce::MaterialPtr* uiMaterial;
    extern mce::MaterialPtr* blendMaterial;
    extern Font* mcFont;
    extern Vec3<float> origin;
    extern float deltaTime;
    extern std::vector<Draw3dData> quad3dRenderList;
    extern std::vector<Draw3dData> line3dRenderList;
    extern LevelRenderer* levelrenderer;
    void onRenderScreen(MinecraftUIRenderContext* ctx);
    void onRenderWorld(LevelRenderer* levelRenderer, ScreenContext* sct);
    void renderNewChunkOutline(const ChunkPos& chunkPos, float yHeight,
                               std::unordered_map<size_t, bool> lineRenderList);
    void setColor(const mce::Color& color);
    void setColor(const UIColor& color);
    void drawText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
                  float textSize = 1.f);
    void drawTextInWorld(const Vec3<float>& pos, const std::string& textStr, const UIColor& color,
                         float textSize);
    float getTextWidth(const std::string& textStr, float textSize = 1.f);
    float getTextHeight(float textSize = 1.f);
    void drawQuad(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3,
                  const Vec2<float>& p4, const UIColor& color);
    void fillRectangle(const Vec4<float>& rect, const UIColor& color);
    void drawImage(mce::TexturePtr& texture, Vec2<float> const& pos, Vec2<float> const& size,
                   mce::Color const& flushCol);
    void drawItem(const Vec2<float>& pos, ItemStack* itemStack, float scale, float opacity);
    //void drawItem(const Vec2<float>& pos, ItemStack* itemStack, float scale,
    //              bool showDurabilityBar);
    void drawBox3dFilled(const AABB& aabb, const UIColor& color, const UIColor& lineColor,
                         float scale = 1.f);
    void drawBoxFilledPre(const Vec3<float>& p1, const Vec3<float>& p2, const UIColor& color,
                       const UIColor& lineColor);
    void drawBoxFilled(const Vec3<float>& blockPos, const UIColor& color, const UIColor& lineColor);
    void drawBox3dFilledRotation(const AABB& aabb, const UIColor& color, const UIColor& lineColor,
                                 float scale, BoxRotation rotation);
    // New: arbitrary axis-angle rotation version
    void drawBox3dFilledRotP(const Vec3<float>& center, const Vec3<float>& halfSize, float pitchRad,
                             float yawRad, const UIColor& color, const UIColor& lineColor);
    void drawBox3dFilledRotP_BottomCenter(const Vec3<float>& center, const Vec3<float>& halfSize,
                                          float pitchRad, float yawRad, const UIColor& color,
                                          const UIColor& lineColor);
    void drawBox3dFilledRotP_TopCenter(const Vec3<float>& center, const Vec3<float>& halfSize,
                                       float pitchRad, float yawRad, const UIColor& color,
                                       const UIColor& lineColor);
    void drawLine3d(const Vec3<float>& start, const Vec3<float>& end, const UIColor& color);
    void drawCircle3d(const Vec3<float>& center, float radius, const UIColor& color, int segments);
    void drawCylinder3d(const Vec3<float>& baseCenter, float radius, float height,
                        const UIColor& color, int segments);
    void drawSphere3d(const Vec3<float>& center, float radius, const UIColor& color, int segments);
    void drawSphere3dFilled(const Vec3<float>& center, float radius, const UIColor& color,
                            int stacks, int slices);
    void drawCylinder3dFilled(const Vec3<float>& baseCenter, float radius, float height,
                              const UIColor& color, int slices);
    void drawPlayerModel(const Vec3<float>& position, float pitchRad, float yawRad,
                         const UIColor& color, const UIColor& lineColor);
    // 在 DrawUtil.h 中添加函数声明
     void drawWings(const Vec3<float>& center, float yaw, float wingSpan,
                          float animationTime, const UIColor& color, const UIColor& lineColor);
    void drawItemDurability(ItemStack* itemStack, Vec2<float> itemPos, float scale, float opacity,
                            int mode);
    }  // namespace DrawUtil