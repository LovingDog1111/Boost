#pragma once

#include <d2d1_3.h>
#include <d3d11.h>
#include <d3d11on12.h>
#include <d3d12.h>
#include <dwrite_3.h>
#include <dxgi1_4.h>

#include <string>

#include "../Utils/ColorUtil.h"
#include "../Utils/Maths.h"
#include "Render3dData.h"

namespace RenderUtil {

extern Vec2<float> mpos;

extern float deltaTime;

void NewFrame(IDXGISwapChain3* swapChain, ID3D11Device* d3d11Device, float fxdpi);
void EndFrame();
void Render();
void Clean();
void Flush();

Vec2<float> getWindowSize();
void drawText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
              float textSize = 1.f, bool storeTextLayout = true);
float getTextWidth(const std::string& textStr, float textSize = 1.f, bool storeTextLayout = true);
float getTextHeight(const std::string& textStr, float textSize = 1.f, bool storeTextLayout = true);
void drawLine(const Vec2<float>& startPos, const Vec2<float>& endPos, const UIColor& color,
              float width = 1.f);
void drawHSVBox(const Vec4<float>& rect, const Vec4<float>& hueColor);
void drawHueBar(const Vec4<float>& rect);
void drawAlphaBar(const Vec4<float>& rect, const UIColor& baseColor);
void drawRectangle(const Vec4<float>& rect, const UIColor& color, float width = 1.f);
void fillRectangle(const Vec4<float>& rect, const UIColor& color);
void drawCircle(const Vec2<float>& centerPos, const UIColor& color, float radius,
                float width = 1.f);
void fillCircle(const Vec2<float>& centerPos, const UIColor& color, float radius);
void addBlur(const Vec4<float>& rect, float strength, bool flush = true);
void drawRoundedRectangle(const Vec4<float>& rect, const UIColor& color, float radius = 5.f,
                          float width = 1.f);
void fillRoundedRectangle(const Vec4<float>& rect, const UIColor& color, float radius = 5.f);
void fillRect(const Vec2<float>& pos, const Vec2<float>& size, const UIColor& color);
float getTime();
void drawImage(const Vec4<float>& rect, const std::string& url);
// UI icon helpers
void drawTriangle(const Vec4<float>& rect, const UIColor& color, bool leftDirection, bool filled);
void drawPause(const Vec4<float>& rect, const UIColor& color);
Vec2<float> WorldToScreen(const Vec3<float>& worldPos, const Vec2<float>& screenSize);
void drawTextInWorld(const Vec3<float>& worldPos, const std::string& textStr, const UIColor& color,
                     float textSize = 1.f);
void drawImageInWorld(const Vec3<float>& worldPos, const std::string& imageUrl,
                      const Vec2<float>& imageSize = Vec2<float>(32.0f, 32.0f),
                      float opacity = 1.0f);
void drawNamePng(float x, float y, float width, float height);

// 皮肤头部渲染相关函数
void drawSkinHead(const Vec2<float>& position, const Vec2<float>& size,
                  const std::vector<unsigned char>& skinData, float opacity = 1.0f);

void drawSkinHeadFromPlayer(const Vec2<float>& position, const Vec2<float>& size,
                            const std::string& playerName, float opacity = 1.0f);

// 从原始皮肤数据创建位图
ID2D1Bitmap1* createBitmapFromSkinData(const std::vector<unsigned char>& skinData, int width,
                                       int height);

// 完整皮肤渲染相关函数
void drawFullSkin(const Vec2<float>& position, const Vec2<float>& size,
                  const std::vector<unsigned char>& skinData, int skinWidth, int skinHeight,
                  float opacity = 1.0f);

void drawFullSkinFromPlayer(const Vec2<float>& position, const Vec2<float>& size,
                            const std::string& playerName, float opacity = 1.0f);

// 从完整皮肤数据创建位图
ID2D1Bitmap1* createBitmapFromFullSkinData(const std::vector<unsigned char>& skinData, int width,
                                           int height);

void drawGlowEffect(const Vec4<float>& rect, const UIColor& color, float glowRadius = 10.0f,
                    float intensity = 1.0f);

void drawTitle(float size, const std::string& font, const Vec2<float>& position,
               float opacity = 1.f);
bool ScreenChange(std::string& from, std::string& to);

void drawNamePng(float x, float y, float width, float height);
void drawAdvancedText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color1,
                      const UIColor& color2, float textSize = 1.0f, bool enableDynamicColor = true,
                      bool enableSweep = true, bool enableShadow = true, float time = 0.0f,
                      float colorSpeed = 1.0f, float sweepSpeed = 1.0f, float sweepWidth = 0.3f,
                      float sweepIntensity = 1.0f, float shadowSize = 1.5f,
                      float shadowIntensity = 0.8f);

void drawGlowEffect(const Vec2<float>& centerPos, float radius, const UIColor& glowColor,
                    float glowIntensity = 1.0f, int glowLayers = 8);

void drawRectGlowEffect(const Vec4<float>& rect, const UIColor& glowColor, float glowRadius = 10.0f,
                        float glowIntensity = 1.0f);

void drawCircleGlowEffect(const Vec2<float>& centerPos, float radius, const UIColor& glowColor,
                          float glowRadius = 5.0f, float glowIntensity = 1.0f);

void drawAnimatedGlow(const Vec2<float>& centerPos, float radius, const UIColor& glowColor,
                      float time = 0.0f, bool pulse = true, bool rotate = false);

void drawAdvancedGlowEffect(const Vec2<float>& centerPos, float radius, const UIColor& innerColor,
                            const UIColor& outerColor, float glowRadius, float intensity,
                            bool animated);
// 在 RenderUtil.h 中更新声明
void drawGradientText(const Vec2<float>& textPos, const std::string& textStr,
                      const std::vector<ColorUtil::GradientStop>& stops, float textSize = 1.0f,
                      bool enableSweep = false, float sweepSpeed = 1.0f, float sweepWidth = 0.3f,
                      float sweepIntensity = 1.0f,
                      float sweepCycleDuration = 3.0f);  // 新增：扫光周期时长

void drawGradientTextWithSweep(const Vec2<float>& textPos, const std::string& textStr,
                               const std::vector<ColorUtil::GradientStop>& stops,
                               float textSize = 1.0f, bool enableSweep = true, float time = 0.0f,
                               float sweepSpeed = 1.0f, float sweepWidth = 0.3f,
                               float sweepIntensity = 1.0f, float sweepCycleDuration = 3.0f);

void fillRectangleWithLinearGradient(const Vec4<float>& rect,
                                     const std::vector<ColorUtil::GradientStop>& stops,
                                     const Vec2<float>& startPoint, const Vec2<float>& endPoint);

void fillRectangleWithRadialGradient(const Vec4<float>& rect,
                                     const std::vector<ColorUtil::GradientStop>& stops,
                                     const Vec2<float>& center, float radius);

// 图片URL加载系统
static std::string generateImageFileName(const std::string& url, const std::string& customName);
static bool isLocalImageExists(const std::string& fileName);
static bool isValidCachedImage(const std::string& imagePath);
static bool validateImageData(const std::string& imageData, const std::string& url);
static bool saveImageData(const std::string& imageData, const std::string& imagePath);
static std::string downloadAndCacheImageFromUrl(const std::string& url,
                                                const std::string& customName);
static bool isValidImageUrl(const std::string& url);
static void drawPlaceholder(const Vec4<float>& rect, const std::string& text);

void drawImageFromUrl(const Vec4<float>& rect, const std::string& url,
                      const std::string& customName, float opacity);

static void cleanCorruptedImageCache();
static void clearImageCache();
static std::string getImageCacheStats();
static std::string getFailureStats();
static void clearFailureRecords();

void drawImageFromCache(const Vec4<float>& rect, const std::string& imageName, float opacity);

void addShadow(const Vec4<float>& rect, float strength, const UIColor& shadowColor, float rounding,
               const std::vector<Vec4<float>>& excludeRects);

void addDropShadow(const Vec4<float>& rect, float blurRadius, const UIColor& shadowColor,
                   const Vec2<float>& offset, float rounding);

}  // namespace RenderUtil
