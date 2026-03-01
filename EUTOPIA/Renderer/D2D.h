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

namespace D2D {
extern float deltaTime;

extern Vec2<float> mpos;
void NewFrame(IDXGISwapChain3* swapChain, ID3D11Device* d3d11Device, float fxdpi);
void EndFrame();
void Render();
void Clean();
void Flush();

Vec2<float> getWindowSize();
void drawText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
              float textSize = 1.f, bool storeTextLayout = false);
float getTextWidth(const std::string& textStr, float textSize = 1.f, bool storeTextLayout = true);
float getTextHeight(const std::string& textStr, float textSize = 1.f, bool storeTextLayout = true);
void drawLine(const Vec2<float>& startPos, const Vec2<float>& endPos, const UIColor& color,
              float width = 1.f);
void drawRectangle(const Vec4<float>& rect, const UIColor& color, float width = 1.f);
void fillRectangle(const Vec4<float>& rect, const UIColor& color);
void drawCircle(const Vec2<float>& centerPos, const UIColor& color, float radius,
                float width = 1.f);
void fillCircle(const Vec2<float>& centerPos, const UIColor& color, float radius);
void addBlur(const Vec4<float>& rect, float strength, bool flush = true);
void fillCircleGradient(const Vec2<float>& centerPos, float radius, const UIColor& innerColor,
                        const UIColor& outerColor);
void drawTriangle(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3,
                  const UIColor& color, float width);
Vec2<float> WorldToScreen(const Vec3<float>& worldPos, const Vec2<float>& screenSize);
void fillTriangle(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3,
                  const UIColor& color);
void drawGlossBlurEffect(const Vec4<float>& rect, float blurStrength, const UIColor& topLeftColor,
                         const UIColor& topRightColor, const UIColor& bottomLeftColor,
                         const UIColor& bottomRightColor);
void drawDropDownTriangle(const Vec2<float>& center, float size, float rotationDegrees,
                          const UIColor& color);
void drawBlurOutline(const Vec4<float>& rect, float blurStrength, float thickness);
void drawWrench(const Vec2<float>& center, float size, const UIColor& color, float width);
void drawVerticalGradient(const Vec4<float>& rect, const UIColor& startColor,
                          const UIColor& endColor);
void drawGear(const Vec2<float>& center, float radius, int teethCount, float toothDepth,
              const UIColor& color, float width);
void drawGlowingText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
                     float textSize, float glowSize);
void PushAxisAlignedClip(const Vec4<float>& rect, bool aliased);
void PopAxisAlignedClip();
void drawGradientText(const Vec2<float>& textPos, const std::string& textStr,
                      const UIColor& startColor, const UIColor& endColor, float textSize);
};  // namespace D2D
