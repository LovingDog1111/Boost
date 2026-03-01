#include "DrawUtil.h"

#include "../Libs/glm/glm/glm.hpp"
#include "../Libs/glm/glm/gtc/matrix_transform.hpp"
#include "../Client/Managers/ModuleManager/ModuleManager.h"
#include "../SDK/Render/MeshHelpers.h"
#include <algorithm>
#include <cmath>
#include "../SDK/Render/Matrix.h"
#include "../SDK/Render/BaseActorRenderContext.h"
MinecraftUIRenderContext* DrawUtil::renderCtx = nullptr;
ScreenContext* DrawUtil::screenCtx = nullptr;
ScreenContext* DrawUtil::D3DCTX = nullptr;
ShaderColor* DrawUtil::currentColor = nullptr;
Tessellator* DrawUtil::tessellator = nullptr;
mce::MaterialPtr* DrawUtil::uiMaterial = nullptr;
mce::MaterialPtr* DrawUtil::blendMaterial = nullptr;
LevelRenderer* DrawUtil::levelrenderer = nullptr;
Font* DrawUtil::mcFont = nullptr;
Vec3<float> DrawUtil::origin;
float DrawUtil::deltaTime = 0.016f;
std::vector<DrawUtil::Draw3dData> DrawUtil::quad3dRenderList;
std::vector<DrawUtil::Draw3dData> DrawUtil::line3dRenderList;

void DrawUtil::onRenderScreen(MinecraftUIRenderContext* ctx) {
    renderCtx = ctx;
    screenCtx = ctx->screenContext;
    tessellator = screenCtx->tessellator;
    currentColor = ctx->screenContext->currentShaderColor;
   

	if(uiMaterial == nullptr)
        uiMaterial = mce::MaterialPtr::createMaterial(HashedString("ui_fill_color"));

    if(blendMaterial == nullptr)
        blendMaterial = mce::MaterialPtr::createMaterial(HashedString("fullscreen_cube_overlay_blend"));
    

    mcFont = ctx->clientInstance->minecraftGame->mcFont;

    ModuleManager::onMCRender(renderCtx);
    DrawUtil::renderCtx->flushText(0.f);
}

void DrawUtil::onRenderWorld(LevelRenderer* levelRenderer, ScreenContext* sct) {

    quad3dRenderList.clear();
    line3dRenderList.clear();
    origin = levelRenderer->renderplayer->getorigin();
    ModuleManager::onLevelRender();

  setColor(mce::Color(1.f, 1.f, 1.f, 1.f));
    if(!quad3dRenderList.empty()) {
        tessellator->begin(VertextFormat::QUAD);
        for(auto& quad3dRenderData : quad3dRenderList) {
            mce::Color mcColor = quad3dRenderData.color.toMCColor();
            tessellator->color(mcColor.r, mcColor.g, mcColor.b, mcColor.a);
            tessellator->vertex(quad3dRenderData.pos.x, quad3dRenderData.pos.y,
                                quad3dRenderData.pos.z);
        }
        MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
        quad3dRenderList.clear();
    }

    if(!line3dRenderList.empty()) {
        tessellator->begin(VertextFormat::LINE_LIST);
        for(auto& line3dRenderData : line3dRenderList) {
            mce::Color mcColor = line3dRenderData.color.toMCColor();
            tessellator->color(mcColor.r, mcColor.g, mcColor.b, mcColor.a);
            tessellator->vertex(line3dRenderData.pos.x, line3dRenderData.pos.y,
                                line3dRenderData.pos.z);
        }
        MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
        line3dRenderList.clear();
    }
}
void DrawUtil::renderNewChunkOutline(const ChunkPos& chunkPos, float yHeight, std::unordered_map<size_t, bool> lineRenderList) {
    Vec3<float> point3D[4];
    point3D[0] = Vec3<float>((float)chunkPos.x, yHeight, (float)chunkPos.z);
    point3D[1] = Vec3<float>((float)chunkPos.x + 16.f, yHeight, (float)chunkPos.z);
    point3D[2] = Vec3<float>((float)chunkPos.x + 16.f, yHeight, (float)chunkPos.z + 16.f);
    point3D[3] = Vec3<float>((float)chunkPos.x, yHeight, (float)chunkPos.z + 16.f);

    Vec2<float> point2D[4];
    for(int i = 0; i < 4; i++) {
        if(!Matrix::WorldToScreen(point3D[i], point2D[i]))
            return;
    }

    for(int i = 0; i < 4; i++) {
        Vec2<float> currentPoint = point2D[i];
        Vec2<float> nextPoint = point2D[(i + 1) % 4];
        size_t hash = std::hash<float>()(currentPoint.x) ^ std::hash<float>()(currentPoint.y) ^
                      std::hash<float>()(nextPoint.x) ^ std::hash<float>()(nextPoint.y);

        if(lineRenderList[hash]) {
            tessellator->vertex(currentPoint.x, currentPoint.y, 0.f);
            tessellator->vertex(nextPoint.x, nextPoint.y, 0.f);
        }
    }
}

void DrawUtil::setColor(const mce::Color& color) {
    currentColor->color = color;
    currentColor->dirty = true;
}

void DrawUtil::setColor(const UIColor& color) {
    currentColor->color = color.toMCColor();
    currentColor->dirty = true;
}

 void DrawUtil::drawText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
                   float textSize) {
    std::string text = textStr;
    mce::Color mcColor = color.toMCColor();

    RectangleArea rect = {};
    rect._x0 = textPos.x;
    rect._x1 = textPos.x;
    rect._y0 = textPos.y;
    rect._y1 = textPos.y;

    TextMeasureData textMeasureData = {};
    textMeasureData.fontSize = textSize;
    textMeasureData.linePadding = 0.f;
    textMeasureData.renderShadow = true;
    textMeasureData.showColorSymbol = false;
    textMeasureData.hideHyphen = false;

    static CaretMeasureData caretMeasureData = {};
    caretMeasureData.position = -1.f;
    caretMeasureData.shouldRender = false;

    renderCtx->drawText(mcFont, &rect, &text, &mcColor, mcColor.a, 0, &textMeasureData,
                        &caretMeasureData);
}


void DrawUtil::drawTextInWorld(const Vec3<float>& pos, const std::string& textStr,
                               const UIColor& color, float textSize) {
    Vec2<float> textPos;
    const float textWidth = getTextWidth(textStr, textSize);
    const float textHeight = getTextHeight(textSize);

    if(Matrix::WorldToScreen(pos.add(0.5f), textPos)) {
        textPos.y -= textHeight;
        textPos.x -= textWidth / 2;

        drawText(textPos, textStr, color, textSize);

        fillRectangle(Vec4<float>(textPos.x - 5, textPos.y - 5, textPos.x + 50, textPos.y + 20),
                      UIColor(255, 0, 0, 100)); 
    }
}
 float DrawUtil::getTextWidth(const std::string& textStr, float textSize) {
     if(renderCtx == nullptr || mcFont == nullptr) {
         return static_cast<float>(textStr.size()) * 6.f * textSize;
     }

     std::string text = textStr;
     if(!renderCtx || !mcFont) {
         return 0.0f;
     }
     return mcFont->getLineLength(&text, textSize, false);
 }

 float DrawUtil::getTextHeight(float textSize) {
     if(mcFont == nullptr)
         return 9.f * textSize;

     return 9.f * textSize;
 }
 void DrawUtil::drawQuad(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3,
                    const Vec2<float>& p4, const UIColor& color) {
     setColor(color);
     tessellator->begin(VertextFormat::QUAD, 4);

     tessellator->vertex(p1.x, p1.y, 0.f);
     tessellator->vertex(p2.x, p2.y, 0.f);
     tessellator->vertex(p3.x, p3.y, 0.f);
     tessellator->vertex(p4.x, p4.y, 0.f);

     MeshHelpers::renderMeshImmediately(screenCtx, tessellator, uiMaterial);
 }

 void DrawUtil::fillRectangle(const Vec4<float>& rect, const UIColor& color) {
     drawQuad(Vec2<float>(rect.x, rect.w), Vec2<float>(rect.z, rect.w), Vec2<float>(rect.z, rect.y),
              Vec2<float>(rect.x, rect.y), color);
 }
 void DrawUtil::drawImage(mce::TexturePtr& texture, Vec2<float> const& pos, Vec2<float> const& size,
                            mce::Color const& flushCol) {
     renderCtx->drawImage(&texture, pos, size, {0.f, 0.f},
                          {1.f, 1.f});
     renderCtx->flushImages(flushCol, 1.f, HashedString("ui_grayscale") /*random hashed string*/);
 }

void DrawUtil::drawItem(const Vec2<float>& pos, ItemStack* itemStack, float scale,
                   float opacity) {
    if (!itemStack || !renderCtx || !renderCtx->screenContext || !renderCtx->clientInstance) {
        return;
    }
    
    if (itemStack == ItemStack::getEmptyItem() || !itemStack->getItem()) {
        return;
    }

    auto baseCtx = std::make_unique<BaseActorRenderContext>(
        renderCtx->screenContext, 
        renderCtx->clientInstance, 
        renderCtx->clientInstance->minecraftGame
    );
    if (!baseCtx || !baseCtx->itemRenderer) {
        return;
    }

    auto it = itemStack->mItem.get();
    if (!it) {
        return;
    }
    baseCtx->itemRenderer->renderGuiItemNew(baseCtx.get(), itemStack, 0, pos.x, pos.y, opacity, scale, 0.f, false);
    if (it->isGlint(itemStack)) {
        baseCtx->itemRenderer->renderGuiItemNew(baseCtx.get(), itemStack, 0, pos.x, pos.y, opacity, scale, 0.f, true);
    }
}

void DrawUtil::drawItemDurability(ItemStack* itemStack, Vec2<float> itemPos, float scale, float opacity,
                             int mode) {
    if(Game.getLocalPlayer() == nullptr)
        return;
    Item* item = itemStack->mItem.get();
    float maxDamage = item->getMaxDamage();
    float damageValue = itemStack->getDamageValue();
    float currentDamage = maxDamage - damageValue;
    if(currentDamage == maxDamage)
        return;
    float percentage = currentDamage / maxDamage * 100;
    float fraction = percentage / 100;
    UIColor durabilityColor = ColorUtil::lerp(UIColor(255, 0, 0), UIColor(0, 255, 0), fraction);
    if(mode == 0 || mode == 2) {
        Vec4<float> barPos = Vec4<float>(itemPos.x + 1.5f * scale, itemPos.y + 14.f * scale,
                                         itemPos.x + 15.f * scale, itemPos.y + 16.f * scale);
        Vec4<float> rect = Vec4<float>(barPos.x, barPos.y,
                                       barPos.x + ((barPos.z - barPos.x) / 100 * (int)percentage),
                                       barPos.w - 1.f * scale);
        DrawUtil::fillRectangle(barPos, UIColor(0.f, 0.f, 0.f));
        DrawUtil::fillRectangle(rect, durabilityColor);
    }
    if(mode == 1 || mode == 2) {
        std::string duraText = std::to_string((int)percentage);
        DrawUtil::drawText(Vec2<float>(itemPos.x + 7.5f, itemPos.y - 6.5f), duraText,
                           durabilityColor,
                      scale);
    }
}

 void DrawUtil::drawBox3dFilled(const AABB& aabb, const UIColor& color, const UIColor& lineColor,
                           float scale) {
     Vec3<float> diff = aabb.upper.sub(aabb.lower);
     Vec3<float> newLower = aabb.lower.sub(DrawUtil::origin);

     Vec3<float> vertices[8] = {{newLower.x, newLower.y, newLower.z},
                                {newLower.x + diff.x, newLower.y, newLower.z},
                                {newLower.x, newLower.y, newLower.z + diff.z},
                                {newLower.x + diff.x, newLower.y, newLower.z + diff.z},

                                {newLower.x, newLower.y + diff.y, newLower.z},
                                {newLower.x + diff.x, newLower.y + diff.y, newLower.z},
                                {newLower.x, newLower.y + diff.y, newLower.z + diff.z},
                                {newLower.x + diff.x, newLower.y + diff.y, newLower.z + diff.z}};

     glm::mat4 rotationMatrix = glm::rotate(glm::mat4(scale), 0.f, glm::vec3(1.0f, 1.0f, 1.0f));
     Vec3<float> newLowerReal = newLower.add(Vec3<float>(0.5f, 0.5f, 0.5f));

     for(int i = 0; i < 8; i++) {
         glm::vec4 rotatedVertex = rotationMatrix * glm::vec4(vertices[i].x - newLowerReal.x,
                                                              vertices[i].y - newLowerReal.y,
                                                              vertices[i].z - newLowerReal.z, 0.0f);
         vertices[i] =
             Vec3<float>(rotatedVertex.x + newLowerReal.x, rotatedVertex.y + newLowerReal.y,
                         rotatedVertex.z + newLowerReal.z);
     }

     if(color.a > 0) {
         static uint8_t v[48] = {5, 7, 6, 4, 4, 6, 7, 5, 1, 3, 2, 0, 0, 2, 3, 1,
                                 4, 5, 1, 0, 0, 1, 5, 4, 6, 7, 3, 2, 2, 3, 7, 6,
                                 4, 6, 2, 0, 0, 2, 6, 4, 5, 7, 3, 1, 1, 3, 7, 5};
         for(int i = 0; i < 48; i++)
             quad3dRenderList.push_back(Draw3dData(vertices[v[i]], color));
     }

     if(lineColor.a > 0) {
         static auto drawLine = [](const Vec3<float>& start, const Vec3<float>& end,
                                   const UIColor& _color) {
             line3dRenderList.push_back(Draw3dData(start, _color));
             line3dRenderList.push_back(Draw3dData(end, _color));
         };

         // Top square
         drawLine(vertices[4], vertices[5], lineColor);
         drawLine(vertices[5], vertices[7], lineColor);
         drawLine(vertices[7], vertices[6], lineColor);
         drawLine(vertices[6], vertices[4], lineColor);

         // Bottom Square
         drawLine(vertices[0], vertices[1], lineColor);
         drawLine(vertices[1], vertices[3], lineColor);
         drawLine(vertices[3], vertices[2], lineColor);
         drawLine(vertices[2], vertices[0], lineColor);

         // Sides
         drawLine(vertices[0], vertices[4], lineColor);
         drawLine(vertices[1], vertices[5], lineColor);
         drawLine(vertices[2], vertices[6], lineColor);
         drawLine(vertices[3], vertices[7], lineColor);
     }
 }

 void DrawUtil::drawBoxFilledPre(const Vec3<float>& p1, const Vec3<float>& p2, const UIColor& color,
                            const UIColor& lineColor) {
     AABB aabb;
     aabb.lower =
         Vec3<float>((std::min)(p1.x, p2.x), (std::min)(p1.y, p2.y), (std::min)(p1.z, p2.z));
     aabb.upper =
         Vec3<float>((std::max)(p1.x, p2.x), (std::max)(p1.y, p2.y), (std::max)(p1.z, p2.z));
     drawBox3dFilled(aabb, color, lineColor, 1.f);
 }

 void DrawUtil::drawBoxFilled(const Vec3<float>& blockPos, const UIColor& color,
                           const UIColor& lineColor) {
     Vec3<float> lower(blockPos.x - 0.5f, blockPos.y-0.5f, blockPos.z-0.5f);
     Vec3<float> upper = lower.add(Vec3<float>(1.f, 1.f, 1.f));
     drawBoxFilledPre(lower, upper, color, lineColor);
 }

void DrawUtil::drawBox3dFilledRotation(const AABB& aabb, const UIColor& color, const UIColor& lineColor,
                                   float scale, BoxRotation rotation) {
     Vec3<float> diff = aabb.upper.sub(aabb.lower);
    Vec3<float> newLower = aabb.lower.sub(DrawUtil::origin);

     Vec3<float> vertices[8] = {{newLower.x, newLower.y, newLower.z},
                                {newLower.x + diff.x, newLower.y, newLower.z},
                                {newLower.x, newLower.y, newLower.z + diff.z},
                                {newLower.x + diff.x, newLower.y, newLower.z + diff.z},

                                {newLower.x, newLower.y + diff.y, newLower.z},
                                {newLower.x + diff.x, newLower.y + diff.y, newLower.z},
                                {newLower.x, newLower.y + diff.y, newLower.z + diff.z},
                                {newLower.x + diff.x, newLower.y + diff.y, newLower.z + diff.z}};

     Vec3<float> center = newLower.add(diff.div(Vec3<float>(2.f, 2.f, 2.f)));

     glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));

     glm::vec3 axis(rotation.rotX, rotation.rotY, rotation.rotZ);
     float angle = glm::length(axis);
     if(angle > 0.00001f) {
         axis = glm::normalize(axis);
         transform = glm::rotate(transform, angle, axis);
     }

     for(int i = 0; i < 8; i++) {
         glm::vec4 offset(vertices[i].x - center.x, vertices[i].y - center.y,
                          vertices[i].z - center.z, 1.0f);
         glm::vec4 res = transform * offset;
         vertices[i] = Vec3<float>(res.x + center.x, res.y + center.y, res.z + center.z);
     }

     if(color.a > 0) {
         static uint8_t v[48] = {5, 7, 6, 4, 4, 6, 7, 5, 1, 3, 2, 0, 0, 2, 3, 1,
                                 4, 5, 1, 0, 0, 1, 5, 4, 6, 7, 3, 2, 2, 3, 7, 6,
                                 4, 6, 2, 0, 0, 2, 6, 4, 5, 7, 3, 1, 1, 3, 7, 5};
         for(int i = 0; i < 48; i++)
             quad3dRenderList.push_back(Draw3dData(vertices[v[i]], color));
     }

     if(lineColor.a > 0) {
         static auto drawLine = [](const Vec3<float>& start, const Vec3<float>& end,
                                   const UIColor& _color) {
             line3dRenderList.push_back(Draw3dData(start, _color));
             line3dRenderList.push_back(Draw3dData(end, _color));
         };

         drawLine(vertices[4], vertices[5], lineColor);
         drawLine(vertices[5], vertices[7], lineColor);
         drawLine(vertices[7], vertices[6], lineColor);
         drawLine(vertices[6], vertices[4], lineColor);

         drawLine(vertices[0], vertices[1], lineColor);
         drawLine(vertices[1], vertices[3], lineColor);
         drawLine(vertices[3], vertices[2], lineColor);
         drawLine(vertices[2], vertices[0], lineColor);

         drawLine(vertices[0], vertices[4], lineColor);
         drawLine(vertices[1], vertices[5], lineColor);
         drawLine(vertices[2], vertices[6], lineColor);
         drawLine(vertices[3], vertices[7], lineColor);
     }



 }

void DrawUtil::drawBox3dFilledRotP(const Vec3<float>& center, const Vec3<float>& halfSize,
                               float pitchRad, float yawRad, const UIColor& color,
                               const UIColor& lineColor) {
     float cp = cosf(pitchRad), sp = sinf(pitchRad);
     float cy = cosf(yawRad), sy = sinf(yawRad);

     auto rotXPY = [&](const Vec3<float>& v) {
         // X-axis rotation
         float x1 = v.x;
         float y1 = v.y * cp - v.z * sp;
         float z1 = v.y * sp + v.z * cp;

         // Y-axis rotation
         float x2 = x1 * cy - z1 * sy;
         float z2 = x1 * sy + z1 * cy;
         return Vec3<float>(x2, y1, z2);
     };

     // ���ɾֲ� 8 ���㲢���任
     Vec3<float> world[8];
     int idx = 0;
     for(int dx = -1; dx <= 1; dx += 2)
         for(int dy = -1; dy <= 1; dy += 2)
             for(int dz = -1; dz <= 1; dz += 2)
                 world[idx++] =
                     rotXPY(Vec3<float>(halfSize.x * dx, halfSize.y * dy, halfSize.z * dz))
                         .add(center)
                         .sub(origin);

     // --- ��� ---
     if(color.a) {
         setColor(color);
         tessellator->begin(VertextFormat::TRIANGLE_LIST);
         mce::Color mc = color.toMCColor();
         auto quad = [&](int a, int b, int c, int d) {
             tessellator->color(mc.r, mc.g, mc.b, mc.a);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[b].x, world[b].y, world[b].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[d].x, world[d].y, world[d].z);
         };
         quad(0, 1, 3, 2);
         quad(4, 6, 7, 5);  // bottom & top
         quad(0, 2, 6, 4);
         quad(1, 5, 7, 3);  // -X & +X
         quad(0, 4, 5, 1);
         quad(2, 3, 7, 6);  // -Z & +Z
         MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
     }

     // --- ��� ---
     if(lineColor.a) {
         auto line = [&](int a, int b) {
             line3dRenderList.emplace_back(world[a], lineColor);
             line3dRenderList.emplace_back(world[b], lineColor);
         };
         line(0, 1);
         line(1, 3);
         line(3, 2);
         line(2, 0);
         line(4, 5);
         line(5, 7);
         line(7, 6);
         line(6, 4);
         line(0, 4);
         line(1, 5);
         line(2, 6);
         line(3, 7);
     }
 }

 void DrawUtil::drawBox3dFilledRotP_BottomCenter(const Vec3<float>& center, const Vec3<float>& halfSize,
                                            float pitchRad, float yawRad, const UIColor& color,
                                            const UIColor& lineColor) {
     float cp = cosf(pitchRad), sp = sinf(pitchRad);
     float cy = cosf(yawRad), sy = sinf(yawRad);

     Vec3<float> right(cy, 0.f, -sy);

     auto rotYawPitchBottom = [&](const Vec3<float>& v) {
         float x1 = v.x * cy - v.z * sy;
         float z1 = v.x * sy + v.z * cy;
         float y1 = v.y;

         y1 -= halfSize.y;  // ������ -halfSize.y���Ƶ� 0

         Vec3<float> k = right;
         Vec3<float> cross(k.y * z1 - k.z * y1, k.z * x1 - k.x * z1, k.x * y1 - k.y * x1);
         float dot = k.x * x1 + k.y * y1 + k.z * z1;

         Vec3<float> r;
         r.x = x1 * cp + cross.x * sp + k.x * dot * (1 - cp);
         r.y = y1 * cp + cross.y * sp + k.y * dot * (1 - cp);
         r.z = z1 * cp + cross.z * sp + k.z * dot * (1 - cp);

         r.y -= halfSize.y;
         return r;
     };

     Vec3<float> world[8];
     int idx = 0;
     for(int dx = -1; dx <= 1; dx += 2)
         for(int dy = -1; dy <= 1; dy += 2)
             for(int dz = -1; dz <= 1; dz += 2)
                 world[idx++] = rotYawPitchBottom(
                                    Vec3<float>(halfSize.x * dx, halfSize.y * dy, halfSize.z * dz))
                                    .add(center)
                                    .sub(origin);

     if(color.a) {
         setColor(color);
         tessellator->begin(VertextFormat::TRIANGLE_LIST);
         mce::Color mc = color.toMCColor();
         auto quad = [&](int a, int b, int c, int d) {
             tessellator->color(mc.r, mc.g, mc.b, mc.a);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[b].x, world[b].y, world[b].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[d].x, world[d].y, world[d].z);
         };
         quad(0, 1, 3, 2);
         quad(4, 6, 7, 5);
         quad(0, 2, 6, 4);
         quad(1, 5, 7, 3);
         quad(0, 4, 5, 1);
         quad(2, 3, 7, 6);
         MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
     }

     if(lineColor.a) {
         auto line = [&](int a, int b) {
             line3dRenderList.emplace_back(world[a], lineColor);
             line3dRenderList.emplace_back(world[b], lineColor);
         };
         line(0, 1);
         line(1, 3);
         line(3, 2);
         line(2, 0);
         line(4, 5);
         line(5, 7);
         line(7, 6);
         line(6, 4);
         line(0, 4);
         line(1, 5);
         line(2, 6);
         line(3, 7);
     }
 }

 void DrawUtil::drawBox3dFilledRotP_TopCenter(const Vec3<float>& center, const Vec3<float>& halfSize,
                                         float pitchRad, float yawRad, const UIColor& color,
                                         const UIColor& lineColor) {
     float cp = cosf(pitchRad), sp = sinf(pitchRad);
     float cy = cosf(yawRad), sy = sinf(yawRad);

     Vec3<float> right(cy, 0.f, -sy);

     auto rotYawPitchBottom = [&](const Vec3<float>& v) {
         float x1 = v.x * cy - v.z * sy;
         float z1 = v.x * sy + v.z * cy;
         float y1 = v.y;

         y1 += halfSize.y;  // ������ -halfSize.y���Ƶ� 0

         Vec3<float> k = right;
         Vec3<float> cross(k.y * z1 - k.z * y1, k.z * x1 - k.x * z1, k.x * y1 - k.y * x1);
         float dot = k.x * x1 + k.y * y1 + k.z * z1;

         Vec3<float> r;
         r.x = x1 * cp + cross.x * sp + k.x * dot * (1 - cp);
         r.y = y1 * cp + cross.y * sp + k.y * dot * (1 - cp);
         r.z = z1 * cp + cross.z * sp + k.z * dot * (1 - cp);

         r.y -= halfSize.y;
         return r;
     };

     Vec3<float> world[8];
     int idx = 0;
     for(int dx = -1; dx <= 1; dx += 2)
         for(int dy = -1; dy <= 1; dy += 2)
             for(int dz = -1; dz <= 1; dz += 2)
                 world[idx++] = rotYawPitchBottom(
                                    Vec3<float>(halfSize.x * dx, halfSize.y * dy, halfSize.z * dz))
                                    .add(center)
                                    .sub(origin);

     if(color.a) {
         setColor(color);
         tessellator->begin(VertextFormat::TRIANGLE_LIST);
         mce::Color mc = color.toMCColor();
         auto quad = [&](int a, int b, int c, int d) {
             tessellator->color(mc.r, mc.g, mc.b, mc.a);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[b].x, world[b].y, world[b].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[d].x, world[d].y, world[d].z);
         };
         quad(0, 1, 3, 2);
         quad(4, 6, 7, 5);
         quad(0, 2, 6, 4);
         quad(1, 5, 7, 3);
         quad(0, 4, 5, 1);
         quad(2, 3, 7, 6);
         MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
     }

     if(lineColor.a) {
         auto line = [&](int a, int b) {
             line3dRenderList.emplace_back(world[a], lineColor);
             line3dRenderList.emplace_back(world[b], lineColor);
         };
         line(0, 1);
         line(1, 3);
         line(3, 2);
         line(2, 0);
         line(4, 5);
         line(5, 7);
         line(7, 6);
         line(6, 4);
         line(0, 4);
         line(1, 5);
         line(2, 6);
         line(3, 7);
     }
 }


 void DrawUtil::drawLine3d(const Vec3<float>& start, const Vec3<float>& end, const UIColor& color) {
     line3dRenderList.push_back(Draw3dData(start.sub(origin), color));
     line3dRenderList.push_back(Draw3dData(end.sub(origin), color));
 }

 void DrawUtil::drawCircle3d(const Vec3<float>& center, float radius, const UIColor& color,
                        int segments) {
     if(segments < 3)
         segments = 3;
     float step = 2.f * PI / segments;
     Vec3<float> prev(center.x + radius, center.y, center.z);
     for(int i = 1; i <= segments; ++i) {
         float ang = step * i;
         Vec3<float> curr(center.x + radius * cosf(ang), center.y, center.z + radius * sinf(ang));
         drawLine3d(prev, curr, color);
         prev = curr;
     }
 }

 void DrawUtil::drawCylinder3d(const Vec3<float>& baseCenter, float radius, float height,
                          const UIColor& color, int segments) {
     if(segments < 3)
         segments = 3;
     // Bottom and top circles
     drawCircle3d(baseCenter, radius, color, segments);
     Vec3<float> topCenter = baseCenter.add(Vec3<float>(0.f, height, 0.f));
     drawCircle3d(topCenter, radius, color, segments);
     // Vertical lines
     float step = 2.f * PI / segments;
     for(int i = 0; i < segments; ++i) {
         float ang = step * i;
         Vec3<float> bottom(baseCenter.x + radius * cosf(ang), baseCenter.y,
                            baseCenter.z + radius * sinf(ang));
         Vec3<float> top = bottom.add(Vec3<float>(0.f, height, 0.f));
         drawLine3d(bottom, top, color);
     }
 }

 void DrawUtil::drawSphere3d(const Vec3<float>& center, float radius, const UIColor& color,
                        int segments) {
     if(segments < 6)
         segments = 6;
     // three great circles: XZ, XY, YZ planes
     drawCircle3d(center, radius, color, segments);  // XZ plane
     // XY plane
     float step = 2.f * PI / segments;
     Vec3<float> prev(center.x + radius, center.y, center.z);
     for(int i = 1; i <= segments; ++i) {
         float ang = step * i;
         Vec3<float> curr(center.x + radius * cosf(ang), center.y + radius * sinf(ang), center.z);
         drawLine3d(prev, curr, color);
         prev = curr;
     }
     // YZ plane
     prev = Vec3<float>(center.x, center.y + radius, center.z);
     for(int i = 1; i <= segments; ++i) {
         float ang = step * i;
         Vec3<float> curr(center.x, center.y + radius * cosf(ang), center.z + radius * sinf(ang));
         drawLine3d(prev, curr, color);
         prev = curr;
     }
 }

 void DrawUtil::drawSphere3dFilled(const Vec3<float>& center, float radius, const UIColor& color,
                              int stacks, int slices) {
     if(stacks < 2)
         stacks = 2;
     if(slices < 3)
         slices = 3;

     // Prepare tessellator
     setColor(color);
     tessellator->begin(VertextFormat::TRIANGLE_LIST);

     mce::Color mc = color.toMCColor();

     for(int lat = 0; lat < stacks; ++lat) {
         float phi1 = PI * ((float)lat / stacks) - PI / 2.0f;  // -90 to 90
         float phi2 = PI * ((float)(lat + 1) / stacks) - PI / 2.0f;

         float sinPhi1 = sinf(phi1);
         float cosPhi1 = cosf(phi1);
         float sinPhi2 = sinf(phi2);
         float cosPhi2 = cosf(phi2);

         for(int lon = 0; lon < slices; ++lon) {
             float theta1 = 2.0f * PI * (float)lon / slices;
             float theta2 = 2.0f * PI * (float)(lon + 1) / slices;

             float sinTheta1 = sinf(theta1);
             float cosTheta1 = cosf(theta1);
             float sinTheta2 = sinf(theta2);
             float cosTheta2 = cosf(theta2);

             // four vertices of quad on sphere surface
             Vec3<float> v1(center.x + radius * cosPhi1 * cosTheta1, center.y + radius * sinPhi1,
                            center.z + radius * cosPhi1 * sinTheta1);
             Vec3<float> v2(center.x + radius * cosPhi2 * cosTheta1, center.y + radius * sinPhi2,
                            center.z + radius * cosPhi2 * sinTheta1);
             Vec3<float> v3(center.x + radius * cosPhi2 * cosTheta2, center.y + radius * sinPhi2,
                            center.z + radius * cosPhi2 * sinTheta2);
             Vec3<float> v4(center.x + radius * cosPhi1 * cosTheta2, center.y + radius * sinPhi1,
                            center.z + radius * cosPhi1 * sinTheta2);

             // subtract camera origin
             v1 = v1.sub(origin);
             v2 = v2.sub(origin);
             v3 = v3.sub(origin);
             v4 = v4.sub(origin);

             tessellator->color(mc.r, mc.g, mc.b, mc.a);
             tessellator->vertex(v1.x, v1.y, v1.z);
             tessellator->vertex(v2.x, v2.y, v2.z);
             tessellator->vertex(v3.x, v3.y, v3.z);

             tessellator->vertex(v1.x, v1.y, v1.z);
             tessellator->vertex(v3.x, v3.y, v3.z);
             tessellator->vertex(v4.x, v4.y, v4.z);
         }
     }

     MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
 }

 void DrawUtil::drawCylinder3dFilled(const Vec3<float>& baseCenter, float radius, float height,
                                const UIColor& color, int slices) {
     if(slices < 3)
         slices = 3;
     setColor(color);
     tessellator->begin(VertextFormat::TRIANGLE_LIST);
     mce::Color mc = color.toMCColor();

     float yBottom = baseCenter.y;
     float yTop = baseCenter.y + height;

     for(int i = 0; i < slices; ++i) {
         float theta1 = 2.f * PI * (float)i / slices;
         float theta2 = 2.f * PI * (float)(i + 1) / slices;

         // Rim points
         Vec3<float> p1(baseCenter.x + radius * cosf(theta1), yBottom,
                        baseCenter.z + radius * sinf(theta1));
         Vec3<float> p2(baseCenter.x + radius * cosf(theta2), yBottom,
                        baseCenter.z + radius * sinf(theta2));
         Vec3<float> p3(baseCenter.x + radius * cosf(theta1), yTop,
                        baseCenter.z + radius * sinf(theta1));
         Vec3<float> p4(baseCenter.x + radius * cosf(theta2), yTop,
                        baseCenter.z + radius * sinf(theta2));

         // subtract origin
         p1 = p1.sub(origin);
         p2 = p2.sub(origin);
         p3 = p3.sub(origin);
         p4 = p4.sub(origin);

         // Side quad (two triangles)
         tessellator->color(mc.r, mc.g, mc.b, mc.a);
         tessellator->vertex(p1.x, p1.y, p1.z);
         tessellator->vertex(p3.x, p3.y, p3.z);
         tessellator->vertex(p4.x, p4.y, p4.z);

         tessellator->vertex(p1.x, p1.y, p1.z);
         tessellator->vertex(p4.x, p4.y, p4.z);
         tessellator->vertex(p2.x, p2.y, p2.z);

         // Top cap
         Vec3<float> topCenter = Vec3<float>(baseCenter.x, yTop, baseCenter.z).sub(origin);
         tessellator->vertex(topCenter.x, topCenter.y, topCenter.z);
         tessellator->vertex(p4.x, p4.y, p4.z);
         tessellator->vertex(p3.x, p3.y, p3.z);

         // Bottom cap
         Vec3<float> bottomCenter = Vec3<float>(baseCenter.x, yBottom, baseCenter.z).sub(origin);
         tessellator->vertex(bottomCenter.x, bottomCenter.y, bottomCenter.z);
         tessellator->vertex(p2.x, p2.y, p2.z);
         tessellator->vertex(p1.x, p1.y, p1.z);
     }

     MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
 }


 void DrawUtil::drawPlayerModel(const Vec3<float>& position, float pitch, float yaw,
                                      const UIColor& color, const UIColor& lineColor) {

  
     float yawRad = yaw * PI / 180.f;
     float pitchRad = pitch * PI / 180.f;


     // 模型尺寸定义（来自 PlayerCham.h）
     static const Vec3<float> headHalf = Vec3<float>(0.25f, 0.25f, 0.25f);
     static const Vec3<float> bodyHalf = Vec3<float>(0.3f, 0.35f, 0.15f);
     static const Vec3<float> armHalf = Vec3<float>(0.15f, 0.35f, 0.15f);
     static const Vec3<float> legHalf = Vec3<float>(0.15f, 0.35f, 0.15f);

     // 肢体相对位置偏移
     static const Vec3<float> localLeftArmOffset = Vec3<float>(-0.4f, 0.7f, 0.0f);
     static const Vec3<float> localRightArmOffset = Vec3<float>(0.4f, 0.7f, 0.0f);
     static const Vec3<float> localLeftLegOffset = Vec3<float>(-0.15f, 0.f, 0.0f);
     static const Vec3<float> localRightLegOffset = Vec3<float>(0.15f, 0.f, 0.0f);

     // 计算旋转值
     float cy = cosf(yawRad), sy = sinf(yawRad);

     // 世界坐标变换函数
     auto toWorld = [&](const Vec3<float>& local) {
         return Vec3<float>(local.x * cy - local.z * sy, local.y, local.x * sy + local.z * cy);
     };

     // 计算各部位中心点
     Vec3<float> headCenter = position;
     Vec3<float> bodyCenter = headCenter.add(Vec3<float>(0.f, -0.55f, 0.f));
     Vec3<float> leftArmCenter = bodyCenter.add(toWorld(localLeftArmOffset));
     Vec3<float> rightArmCenter = bodyCenter.add(toWorld(localRightArmOffset));
     Vec3<float> leftLegCenter = bodyCenter.add(toWorld(localLeftLegOffset));
     Vec3<float> rightLegCenter = bodyCenter.add(toWorld(localRightLegOffset));

     // 绘制头部
     drawBox3dFilledRotP(headCenter, headHalf, pitchRad, yawRad, color, lineColor);

     // 绘制身体（使用 TopCenter 锚点）
     drawBox3dFilledRotP_TopCenter(bodyCenter, bodyHalf, 0.f, yawRad, color, lineColor);

     // 绘制手臂（静态姿势，稍微向下）
     float staticArmPitch = 0.2f;  // 大约11.5度向下
     drawBox3dFilledRotP_BottomCenter(leftArmCenter, armHalf, staticArmPitch, yawRad, color,
                                      lineColor);
     drawBox3dFilledRotP_BottomCenter(rightArmCenter, armHalf, staticArmPitch, yawRad, color,
                                      lineColor);

     // 绘制腿部（静态姿势，垂直向下）
     drawBox3dFilledRotP_BottomCenter(leftLegCenter, legHalf, 0.f, yawRad, color, lineColor);
     drawBox3dFilledRotP_BottomCenter(rightLegCenter, legHalf, 0.f, yawRad, color, lineColor);
 }

void DrawUtil::drawWings(const Vec3<float>& center, float yaw, float wingSpan, float animationTime,
                          const UIColor& color, const UIColor& lineColor) {
     float yawRad = yaw * PI / 180.f;

     // 优化后的翅膀尺寸参数
     const float wingLength = wingSpan * 1.4f;  // 增加长度以更匹配目标效果
     const float wingHeight = wingSpan * 2.4f;  // 增加高度以获得更大的展开范围
     const float wingThickness = 0.12f;         // 稍微减少厚度让翅膀更轻盈
     const int featherSegments = 32;            // 减少到28以提高性能
     const int widthSegments =64;              // 减少到20以提高性能
     const int layerCount = 3;                  // 减少到4层以提高性能

     // 计算旋转
     float cy = cosf(yawRad), sy = sinf(yawRad);

     // 改进的翅膀摆动动画 - 更流畅更自然
     float flapAngle =
         AnimationUtil::easeInOutSine(sinf(animationTime * 1.5f) * 0.5f + 0.5f) * 0.3f - 0.15f;
     float wingTilt =
         0.1f + AnimationUtil::easeInOutCubic(sinf(animationTime * 1.2f) * 0.5f + 0.5f) * 0.08f;
     float featherRipple = AnimationUtil::getWaveOffset(animationTime, 3.0f, 0.05f, 0.0f);

     // 世界坐标变换函数 - 优化性能
     auto transformPoint = [&](const Vec3<float>& local) -> Vec3<float> {
         // 预计算sin/cos值以提高性能
         float cosTilt = cosf(wingTilt);
         float sinTilt = sinf(wingTilt);

         float localY = local.y;
         float localZ = local.z * cosTilt - local.x * sinTilt;
         float localX = local.z * sinTilt + local.x * cosTilt;

         float worldX = localX * cy - localZ * sy;
         float worldZ = localX * sy + localZ * cy;

         return Vec3<float>(worldX, localY, worldZ).add(center).sub(origin);
     };

     // 优化的羽毛层生成函数
     auto generateFeatherLayer = [&](bool isLeft, int layer) {
         float sideMultiplier = isLeft ? -1.0f : 1.0f;
         float currentFlapAngle = flapAngle * sideMultiplier;
         float layerOffset = layer * 0.06f;        // 减小层次间距
         float layerScale = 1.0f - layer * 0.15f;  // 增加层次缩放差异

         std::vector<Vec3<float>> vertices;
         std::vector<std::vector<int>> triangles;

         // 更高效的羽毛顶点生成
         for(int i = 0; i <= featherSegments; i++) {
             float t = (float)i / featherSegments;

             // 改进的翅膀轮廓曲线 - 更接近真实翅膀形状
             float wingProfile = 1.0f;
             if(t < 0.2f) {
                 // 根部区域 - 更平滑的过渡
                 float rootProgress = t / 0.2f;
                 wingProfile =
                     AnimationUtil::lerp(0.25f, 0.9f, AnimationUtil::easeInOutCubic(rootProgress));
             } else if(t < 0.7f) {
                 // 中部区域 - 主要展开区域
                 float midProgress = (t - 0.2f) / 0.5f;
                 float curveShape = sinf(midProgress * PI * 0.9f);
                 wingProfile = AnimationUtil::lerp(0.9f, 1.6f, curveShape);
             } else {
                 // 尖端区域 - 更自然的收窄
                 float tipProgress = (t - 0.7f) / 0.3f;
                 float exponentialCurve = 1.0f - pow(tipProgress, 1.8f);
                 wingProfile = AnimationUtil::lerp(1.6f, 0.02f, 1.0f - exponentialCurve);
             }

             // 计算基础高度
             float baseHeight = wingHeight * layerScale * wingProfile;

             // 改进的羽毛分叉逻辑
             int featherCount = 1;
             if(t > 0.25f && t < 0.85f) {
                 featherCount = (t > 0.4f && t < 0.75f) ? 4 : 2;  // 主要区域4根，其他2根
             }

             for(int feather = 0; feather < featherCount; feather++) {
                 float featherSpread = 0.15f * t * (1.0f + layer * 0.1f);  // 层次差异
                 float featherOffset = (feather - (featherCount - 1) / 2.0f) * featherSpread;

                 for(int j = 0; j <= widthSegments; j++) {
                     float s = (float)j / widthSegments;

                     // 优化的羽毛轮廓
                     float featherShape = sinf(s * PI);
                     float heightVariation = baseHeight * featherShape;

                     // 更精细的羽毛纹理
                     if(t > 0.3f) {
                         float detailFreq = 12.0f + layer * 2.0f;
                         float detailAmp = 0.03f * (t - 0.3f) * featherShape;
                         heightVariation += sinf(s * PI * detailFreq + animationTime * 2.0f) *
                                            detailAmp * baseHeight;
                     }

                     // 改进的羽毛锯齿效果
                     if(t > 0.6f && feather > 0) {
                         float jaggedness = sinf(s * PI * 8.0f + feather * 1.5f + animationTime) *
                                            0.06f * (t - 0.6f) * featherShape;
                         heightVariation += jaggedness * baseHeight;
                     }

                     // 计算3D位置 - 更自然的翅膀形状
                     float x = sideMultiplier * (0.15f + t * wingLength * layerScale +
                                                 cosf(currentFlapAngle + layerOffset) * t * 0.25f);

                     float y = (s - 0.5f) * heightVariation + featherOffset +
                               sinf(currentFlapAngle + layerOffset) * t * 0.2f +
                               featherRipple * sinf(t * PI * 2.5f + layer * 0.3f);

                     float z = -t * 0.4f * wingLength * layerScale + wingThickness * sinf(s * PI) +
                               layerOffset;

                     // 更自然的翅膀弯曲
                     if(t > 0.15f) {
                         float bendFactor = (t - 0.15f) / 0.85f;
                         float bendStrength = bendFactor * bendFactor;
                         float bendAngle = bendStrength * 0.4f * sideMultiplier;
                         float bendRadius = wingLength * 0.8f;

                         x += sinf(bendAngle) * bendRadius * 0.12f;
                         z -= cosf(bendAngle) * bendRadius * 0.06f;
                         y -= bendStrength * 0.25f;  // 添加垂直弯曲
                     }

                     // 尖端优雅下垂和内弯
                     if(t > 0.7f) {
                         float tipFactor = (t - 0.7f) / 0.3f;
                         float smoothTip = AnimationUtil::easeInOutCubic(tipFactor);

                         y -= smoothTip * 0.6f;                    // 下垂
                         z -= smoothTip * 0.3f;                    // 向后弯曲
                         x += sideMultiplier * smoothTip * 0.08f;  // 向内弯曲
                     }

                     // 层次间的细微差异
                     x += layer * 0.025f * sideMultiplier;
                     y += layer * 0.01f * sinf(t * PI + layer);
                     z += layer * 0.015f;

                     // 创建顶点（上下表面）
                     vertices.push_back(Vec3<float>(x, y, z));
                     vertices.push_back(Vec3<float>(x, y, z + wingThickness));
                 }
             }
         }

         // 优化的三角形网格生成
         int baseVerticesPerSegment = (widthSegments + 1) * 2;
         for(int i = 0; i < featherSegments; i++) {
             for(int j = 0; j < widthSegments; j++) {
                 int base = i * baseVerticesPerSegment + j * 2;
                 int next = (i + 1) * baseVerticesPerSegment + j * 2;

                 if(next + 3 >= vertices.size())
                     continue;

                 // 生成三角形（减少重复计算）
                 triangles.push_back({base, next, base + 2});
                 triangles.push_back({next, next + 2, base + 2});
                 triangles.push_back({base + 1, base + 3, next + 1});
                 triangles.push_back({next + 1, base + 3, next + 3});

                 // 边缘连接
                 if(j == 0 || j == widthSegments - 1) {
                     triangles.push_back({base, base + 1, next});
                     triangles.push_back({next, base + 1, next + 1});
                 }
             }
         }

         // 渲染当前层
         if(color.a > 0 && !vertices.empty() && !triangles.empty()) {
             setColor(color);
             tessellator->begin(VertextFormat::TRIANGLE_LIST);

             // 层次透明度变化
             mce::Color layerColor = color.toMCColor();
             float layerAlpha = layerColor.a * (1.0f - layer * 0.08f);
             layerColor.a = (std::max)(0.1f, layerAlpha);

             for(const auto& triangle : triangles) {
                 for(int idx : triangle) {
                     if(idx < vertices.size()) {
                         Vec3<float> worldPos = transformPoint(vertices[idx]);
                         tessellator->color(layerColor.r, layerColor.g, layerColor.b, layerColor.a);
                         tessellator->vertex(worldPos.x, worldPos.y, worldPos.z);
                     }
                 }
             }
             MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
         }
     };

     // 生成多层羽毛的左右翅膀（从后往前渲染保证正确的alpha混合）
     for(int layer = layerCount - 1; layer >= 0; layer--) {
         generateFeatherLayer(true, layer);   // 左翅膀
         generateFeatherLayer(false, layer);  // 右翅膀
     }

     // 可选：添加翅膀根部连接（如果需要）
     //if(lineColor.a > 0) {
     //    // 简单的翅膀根部连接
     //    Vec3<float> rootSize(0.2f, 0.5f, 0.15f);
     //    drawBox3dFilledRotP(center.add(Vec3<float>(0, 0.05f, -0.15f)), rootSize, 0.0f, yawRad,
     //                        UIColor(color.r, color.g, color.b, (std::min)(255, color.a + 50)),
     //                        lineColor);
     //}
 }

 /*     if(color.a > 0) {
         // 主要根部
         Vec3<float> mainRootSize(0.25f, 0.6f, 0.2f);
         drawBox3dFilledRotP(center.add(Vec3<float>(0, 0.1f, -0.2f)), mainRootSize, 0.0f, yawRad,
                             color, lineColor);

         // 辅助连接部件
         Vec3<float> auxRootSize(0.35f, 0.4f, 0.15f);
         drawBox3dFilledRotP(center.add(Vec3<float>(0, 0.3f, -0.1f)), auxRootSize, 0.0f, yawRad,
                             color, lineColor);

         // 上部连接
         Vec3<float> topRootSize(0.2f, 0.3f, 0.1f);
         drawBox3dFilledRotP(center.add(Vec3<float>(0, 0.5f, -0.05f)), topRootSize, 0.0f, yawRad,
                             color, lineColor);
     }*/