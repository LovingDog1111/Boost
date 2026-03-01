
#include "RenderUtil.h"

#include <AnimationUtil.h>
#include <Windows.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "../Client/Client.h"
#include "../Client/Managers/HooksManager/Hooks/Network/GetAvgPingHook.h"
#include "../Client/Managers/ModuleManager/ModuleManager.h"
#include "../Client/Managers/ModuleManager/Modules/Category/Client/BGAnimation.h"
#include "../Client/Managers/ModuleManager/Modules/Category/Client/CustomFont.h"
#include "../Client/Managers/NotificationManager/NotificationManager.h"
#include "../SDK/GlobalInstance.h"
#include "../SDK/Render/Matrix.h"
#include "../Utils/FileUtil.h"
#include "../Utils/Logger.h"
#include "../Utils/NetworkUtil.h"
#include "../Utils/TimerUtil.h"
// 添加缺失的头文件
#include <algorithm>
#include <set>
#include <thread>

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;


// 只在此处定义float RenderUtil::deltaTime = 0.016f;
float RenderUtil::deltaTime = 0.016f;
Vec2<float> RenderUtil::mpos = Vec2<float>(0.f, 0.f);

// d2d stuff
static ID2D1Factory3* d2dFactory = nullptr;
static IDWriteFactory* d2dWriteFactory = nullptr;
static ID2D1Device2* d2dDevice = nullptr;
static ID2D1DeviceContext2* d2dDeviceContext = nullptr;
static ID2D1Bitmap1* sourceBitmap = nullptr;
static ID2D1Effect* blurEffect = nullptr;
static ID2D1Bitmap1* shadowBitmap = nullptr;
static ID2D1Effect* shadowEffect = nullptr;
static ID2D1Effect* stencilEffect = nullptr;
// cache
static std::unordered_map<float, winrt::com_ptr<IDWriteTextFormat>> textFormatCache;
static std::unordered_map<uint64_t, winrt::com_ptr<IDWriteTextLayout>> textLayoutCache;
static std::unordered_map<uint32_t, winrt::com_ptr<ID2D1SolidColorBrush>> colorBrushCache;
static std::unordered_map<std::string, winrt::com_ptr<ID2D1Bitmap1>> skinBitmapCache;
static std::unordered_map<std::string, std::string> imageCacheMap;

// 添加失败URL追踪
static std::unordered_set<std::string> failedUrls;
static std::unordered_map<std::string, int> failureCount;
// temporary cache
static std::unordered_map<uint64_t, winrt::com_ptr<IDWriteTextLayout>> textLayoutTemporary;

static int currentD2DFontSize = 25;
static std::string currentD2DFont = "Microsoft YaHei UI";
static bool isFontItalic = false;

static bool initD2D = false;

static IWICImagingFactory* wicFactory = nullptr;
static std::unordered_map<std::string, winrt::com_ptr<ID2D1Bitmap1>> urlBitmapCache;

// Helper: ensure WIC factory
static void EnsureWICFactory() {
    if(!wicFactory) {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&wicFactory));
    }
}

namespace {
winrt::com_ptr<ID2D1Bitmap1> loadBitmapFromUrl(const std::string& url) {
    auto it = urlBitmapCache.find(url);
    if(it != urlBitmapCache.end())
        return it->second;

    // If not http/https, treat as local file path
    bool isRemote = false;
    if(url.rfind("http://", 0) == 0 || url.rfind("https://", 0) == 0)
        isRemote = true;

    winrt::com_ptr<ID2D1Bitmap1> bitmap;
    EnsureWICFactory();

    if(isRemote) {
        std::string imgData;
        if(!NetworkUtil::httpGet(url, imgData) || imgData.empty()) {
            return nullptr;
        }

        // 验证图片数据大小和基本格式
        if(imgData.size() < 100) {  // 太小的文件可能无效
            return nullptr;
        }

        winrt::com_ptr<IWICStream> stream;
        HRESULT hr = wicFactory->CreateStream(stream.put());
        if(FAILED(hr)) {
            return nullptr;
        }

        hr = stream->InitializeFromMemory(reinterpret_cast<BYTE*>(imgData.data()),
                                          static_cast<DWORD>(imgData.size()));
        if(FAILED(hr)) {
            return nullptr;
        }

        winrt::com_ptr<IWICBitmapDecoder> decoder;
        hr = wicFactory->CreateDecoderFromStream(stream.get(), nullptr,
                                                 WICDecodeMetadataCacheOnDemand, decoder.put());
        if(FAILED(hr) || !decoder) {
            return nullptr;
        }

        // 验证decoder是否有效并获取帧数
        UINT frameCount = 0;
        hr = decoder->GetFrameCount(&frameCount);
        if(FAILED(hr) || frameCount == 0) {
            return nullptr;
        }

        winrt::com_ptr<IWICBitmapFrameDecode> frame;
        hr = decoder->GetFrame(0, frame.put());
        if(FAILED(hr) || !frame) {
            return nullptr;
        }

        winrt::com_ptr<IWICFormatConverter> converter;
        hr = wicFactory->CreateFormatConverter(converter.put());
        if(FAILED(hr)) {
            return nullptr;
        }

        hr = converter->Initialize(frame.get(), GUID_WICPixelFormat32bppPBGRA,
                                   WICBitmapDitherTypeNone, nullptr, 0.f,
                                   WICBitmapPaletteTypeCustom);
        if(FAILED(hr)) {
            return nullptr;
        }

        hr = d2dDeviceContext->CreateBitmapFromWicBitmap(converter.get(), nullptr, bitmap.put());
        if(FAILED(hr)) {
            return nullptr;
        }

    } else {
        // 本地文件处理保持不变，但也要加错误检查
        std::wstring wpath(url.begin(), url.end());

        // 检查文件是否存在
        if(!std::filesystem::exists(url)) {
            return nullptr;
        }

        winrt::com_ptr<IWICBitmapDecoder> decoder;
        HRESULT hr = wicFactory->CreateDecoderFromFilename(
            wpath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.put());
        if(FAILED(hr)) {
            return nullptr;
        }

        winrt::com_ptr<IWICBitmapFrameDecode> frame;
        hr = decoder->GetFrame(0, frame.put());
        if(FAILED(hr)) {
            return nullptr;
        }

        winrt::com_ptr<IWICFormatConverter> converter;
        hr = wicFactory->CreateFormatConverter(converter.put());
        if(FAILED(hr)) {
            return nullptr;
        }

        hr = converter->Initialize(frame.get(), GUID_WICPixelFormat32bppPBGRA,
                                   WICBitmapDitherTypeNone, nullptr, 0.f,
                                   WICBitmapPaletteTypeCustom);
        if(FAILED(hr)) {
            return nullptr;
        }

        hr = d2dDeviceContext->CreateBitmapFromWicBitmap(converter.get(), nullptr, bitmap.put());
        if(FAILED(hr)) {
            return nullptr;
        }
    }

    if(bitmap) {
        urlBitmapCache[url] = bitmap;
    }

    return bitmap;
}
}  // namespace

template <typename T>
void SafeRelease(T*& ptr) {
    if(ptr != nullptr) {
        ptr->Release();
        ptr = nullptr;
    }
}

std::wstring to_wide(const std::string& str);
uint64_t getTextLayoutKey(const std::string& textStr, float textSize);
IDWriteTextFormat* getTextFormat(float textSize);
IDWriteTextLayout* getTextLayout(const std::string& textStr, float textSize,
                                 bool storeTextLayout = true);
ID2D1SolidColorBrush* getSolidColorBrush(const UIColor& color);

// Guard macro to early-return when D2D context is missing
#define D2D_CTX_GUARD()   \
    if(!d2dDeviceContext) \
        return;
#define D2D_CTX_GUARD_RET(defaultVal) \
    if(!d2dDeviceContext)             \
        return defaultVal;

void RenderUtil::NewFrame(IDXGISwapChain3* swapChain, ID3D11Device* d3d11Device, float fxdpi) {
    if(!initD2D) {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &d2dFactory);

        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(d2dWriteFactory),
                            reinterpret_cast<IUnknown**>(&d2dWriteFactory));

        IDXGIDevice* dxgiDevice;
        d3d11Device->QueryInterface<IDXGIDevice>(&dxgiDevice);
        d2dFactory->CreateDevice(dxgiDevice, &d2dDevice);
        dxgiDevice->Release();

        d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dDeviceContext);
        // d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
        // &d2dDeviceContext);

        d2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &blurEffect);
        blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
        blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION,
                             D2D1_GAUSSIANBLUR_OPTIMIZATION_QUALITY);

        d2dDeviceContext->CreateEffect(CLSID_D2D1Shadow, &shadowEffect);

        d2dDeviceContext->CreateEffect(CLSID_D2D1AlphaMask, &stencilEffect);

        IDXGISurface* dxgiBackBuffer = nullptr;
        swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), fxdpi, fxdpi);
        d2dDeviceContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer, &bitmapProperties,
                                                      &sourceBitmap);
        dxgiBackBuffer->Release();

        d2dDeviceContext->SetTarget(sourceBitmap);

        initD2D = true;
    }

    d2dDeviceContext->BeginDraw();
}

void RenderUtil::EndFrame() {
    if(!initD2D)
        return;

    d2dDeviceContext->EndDraw();

    static CustomFont* customFontMod = ModuleManager::getModule<CustomFont>();
    if((currentD2DFont != customFontMod->getSelectedFont()) ||
       (currentD2DFontSize != customFontMod->fontSize) || (isFontItalic != customFontMod->italic)) {
        currentD2DFont = customFontMod->getSelectedFont();
        currentD2DFontSize = customFontMod->fontSize;
        isFontItalic = customFontMod->italic;
        textFormatCache.clear();
        textLayoutCache.clear();
        // textLayoutTemporary.clear();
    }

    static float timeCounter = 0.0f;
    timeCounter += RenderUtil::deltaTime;
    if(timeCounter > 90.f) {
        if(textFormatCache.size() > 1000)
            textFormatCache.clear();

        if(textLayoutCache.size() > 500)
            textLayoutCache.clear();

        if(colorBrushCache.size() > 2000)
            colorBrushCache.clear();

        timeCounter = 0.0f;
    }

    textLayoutTemporary.clear();
}

void RenderUtil::drawTitle(float size, const std::string& font, const Vec2<float>& position,
                           float opacity) {
    // 添加动画时间变量
    static float titleAnimTime = 0.f;
    titleAnimTime += RenderUtil::deltaTime;

    // 保存原始字体
    std::string originalFont = currentD2DFont;
    currentD2DFont = font;  // 使用现代字体
    textFormatCache.clear();
    textLayoutCache.clear();

    // 标题参数
    float mainTitleSize = size;  // 调整到14字体大小
    std::string mainTitle = "";

    // 动画效果
    float fadeIn = std::min(titleAnimTime * 2.0f, 1.0f);               // 淡入效果
    float floatOffset = sinf(titleAnimTime * 1.5f) * 3.0f;             // 浮动效果
    float glowIntensity = (sinf(titleAnimTime * 2.0f) + 1.0f) * 0.5f;  // 发光强度

    // 计算文本尺寸
    float mainWidth = RenderUtil::getTextWidth(mainTitle, mainTitleSize);
    float mainHeight = RenderUtil::getTextHeight(mainTitle, mainTitleSize);

    // 居中位置
    Vec2<float> mainTitlePos = Vec2<float>(position.x - mainWidth / 2.0f,
                                           position.y - mainHeight / 2.0f + floatOffset - 50.0f);

    // 主标题发光效果
    for(int i = 0; i < 3; i++) {
        float glowAlpha = (glowIntensity * 40 - i * 12) * fadeIn;
        if(glowAlpha > 0) {
            UIColor glowColor = UIColor(100, 150, 255, (int)glowAlpha);
            RenderUtil::drawText(Vec2<float>(mainTitlePos.x - i, mainTitlePos.y - i), mainTitle,
                                 glowColor, mainTitleSize + i * 0.5f);
        }
    }

    // 主标题//UIColor glowColor = UIColor(127, 255, 212, (int)glowAlpha)
    auto titleColor = ColorUtil::getNeonGradientStops(titleAnimTime * 0.1f);  // 慢10倍的颜色变化

    RenderUtil::drawGradientText(mainTitlePos, mainTitle, titleColor, mainTitleSize, true, 2.f,
                                 0.15, 1.f, 10.f);

    // 装饰性元素
    if(fadeIn > 0.8f) {
        // 顶部装饰线
        float lineY = mainTitlePos.y - 10.0f;
        float lineWidth = mainWidth * 0.8f;
        Vec4<float> topLine = Vec4<float>(position.x - lineWidth / 2.0f, lineY,
                                          position.x + lineWidth / 2.0f, lineY + 1.5f);

        UIColor lineColor = UIColor(100, 150, 255, (int)(120 * glowIntensity));
        RenderUtil::fillRoundedRectangle(topLine, lineColor, 1.0f);

        // 底部装饰线
        float bottomLineY = mainTitlePos.y + mainHeight + 8.0f;
        Vec4<float> bottomLine = Vec4<float>(position.x - lineWidth / 2.0f, bottomLineY,
                                             position.x + lineWidth / 2.0f, bottomLineY + 1.5f);

        RenderUtil::fillRoundedRectangle(bottomLine, lineColor, 1.0f);

        // 装饰性粒子点
        for(int i = 0; i < 5; i++) {
            float particleX = position.x - mainWidth * 1.2f / 2.0f + (i * mainWidth * 1.2f / 4.0f);
            float particleY = mainTitlePos.y - 15.0f;
            float particleAlpha = (sinf(titleAnimTime * 3.0f + i * 1.2f) + 1.0f) * 0.5f;

            Vec2<float> particleCenter = Vec2<float>(particleX, particleY);
            UIColor particleColor = UIColor(150, 200, 255, (int)(100 * particleAlpha));
            RenderUtil::fillCircle(particleCenter, particleColor, 1.5f);
        }

        // 底部粒子点
        for(int i = 0; i < 5; i++) {
            float particleX = position.x - mainWidth * 1.2f / 2.0f + (i * mainWidth * 1.2f / 4.0f);
            float particleY = mainTitlePos.y + mainHeight + 15.0f;
            float particleAlpha = (sinf(titleAnimTime * 2.5f + i * 1.5f) + 1.0f) * 0.5f;

            Vec2<float> particleCenter = Vec2<float>(particleX, particleY);
            UIColor particleColor = UIColor(150, 200, 255, (int)(80 * particleAlpha));
            RenderUtil::fillCircle(particleCenter, particleColor, 1.2f);
        }
    }

    // 恢复原始字体
    currentD2DFont = originalFont;
    textFormatCache.clear();
    textLayoutCache.clear();
}

bool RenderUtil::ScreenChange(std::string& from, std::string& to) {
    std::string currentScreen = GI::getClientInstance()->getScreenName();
    if(currentScreen != from && currentScreen == to) {
        // Screen changed from 'from' to 'to'
        from = currentScreen;  // Update 'from' to the new screen
        return true;           // Indicate that the screen has changed
    }
    return false;
}

void RenderUI() {
    static Vec2<float> smoothMousePos = Vec2<float>(0.f, 0.f);

    std::string screenName = GI::getClientInstance()->getScreenName();
    Vec2<float> windowsSize = GI::getGuiData()->windowSizeReal;

    float lerpFactor = 0.01f;
    Vec2<float> targetMousePos(RenderUtil::mpos.x, RenderUtil::mpos.y);
    // smoothMousePos = SecretMethodAwA(smoothMousePos, targetMousePos, lerpFactor);

    Vec4<float> StartbuttonSize = Vec4<float>(
        windowsSize.x / 2 - windowsSize.x * 0.0859, windowsSize.y * 0.540 - windowsSize.y * 0.031,
        windowsSize.x / 2 + windowsSize.x * 0.0859, windowsSize.y * 0.540 + windowsSize.y * 0.031);

    Vec4<float> SettingsButtonSize = Vec4<float>(
        windowsSize.x / 2 - windowsSize.x * 0.0859, windowsSize.y * 0.611 - windowsSize.y * 0.031,
        windowsSize.x / 2 + windowsSize.x * 0.0859, windowsSize.y * 0.611 + windowsSize.y * 0.031);

    Vec4<float> RealmsButtonSize = Vec4<float>(
        windowsSize.x / 2 - windowsSize.x * 0.0859, windowsSize.y * 0.682 - windowsSize.y * 0.031,
        windowsSize.x / 2 + windowsSize.x * 0.0859, windowsSize.y * 0.682 + windowsSize.y * 0.031);

    Vec4<float> ScamplaceButtonSize = Vec4<float>(
        windowsSize.x / 2 - windowsSize.x * 0.0859, windowsSize.y * 0.753 - windowsSize.y * 0.031,
        windowsSize.x / 2 + windowsSize.x * 0.0859, windowsSize.y * 0.753 + windowsSize.y * 0.031);

    bool isInStartButtonAera =
        (RenderUtil::mpos.x >= StartbuttonSize.x && RenderUtil::mpos.x <= StartbuttonSize.z &&
         RenderUtil::mpos.y >= StartbuttonSize.y && RenderUtil::mpos.y <= StartbuttonSize.w);

    bool isInSettingsButtonAera =
        (RenderUtil::mpos.x >= SettingsButtonSize.x && RenderUtil::mpos.x <= SettingsButtonSize.z &&
         RenderUtil::mpos.y >= SettingsButtonSize.y && RenderUtil::mpos.y <= SettingsButtonSize.w);

    bool isInRealmsButtonAera =
        (RenderUtil::mpos.x >= RealmsButtonSize.x && RenderUtil::mpos.x <= RealmsButtonSize.z &&
         RenderUtil::mpos.y >= RealmsButtonSize.y && RenderUtil::mpos.y <= RealmsButtonSize.w);

    bool isInScamplaceButtonAera = (RenderUtil::mpos.x >= ScamplaceButtonSize.x &&
                                    RenderUtil::mpos.x <= ScamplaceButtonSize.z &&
                                    RenderUtil::mpos.y >= ScamplaceButtonSize.y &&
                                    RenderUtil::mpos.y <= ScamplaceButtonSize.w);

    auto buttonColor = UIColor(0, 0, 0, 120);



    if(screenName == "start_screen") {
        float overflowX = windowsSize.x * 0.5f;
        float overflowY = windowsSize.y * 0.5f;

        float maxOffsetX = overflowX;
        float maxOffsetY = overflowY;

        float offsetX = ((smoothMousePos.x / windowsSize.x) - 0.5f) * 2.f * maxOffsetX * -1.f;
        float offsetY = ((smoothMousePos.y / windowsSize.y) - 0.5f) * 2.f * maxOffsetY * -1.f;

        Vec4<float> bigRect =
            Vec4<float>(-overflowX + offsetX, -overflowY + offsetY,
                        windowsSize.x + overflowX + offsetX, windowsSize.y + overflowY + offsetY);

        std::string image = "https://wallpapercave.com/wp/wp12199523.jpg";

        RenderUtil::drawImageFromUrl(bigRect, image, "background", 1.f);

        float time = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                            std::chrono::steady_clock::now().time_since_epoch())
                                            .count()) /
                     1000.f;

 
        RenderUtil::fillRectangle(StartbuttonSize, buttonColor);
        RenderUtil::drawText(
            Vec2<float>(windowsSize.x / 2 - RenderUtil::getTextWidth("Start") / 2,
                        windowsSize.y * 0.540 - RenderUtil::getTextHeight("Start") / 2),
            "Start", UIColor(255, 255, 255, 255), 1.f);
        if(isInStartButtonAera) {
            RenderUtil::fillRectangle(StartbuttonSize, UIColor(0, 0, 0, 120));
        }

        RenderUtil::fillRectangle(SettingsButtonSize, buttonColor);
        RenderUtil::drawText(
            Vec2<float>(windowsSize.x / 2 - RenderUtil::getTextWidth("Settings") / 2,
                        windowsSize.y * 0.611 - RenderUtil::getTextHeight("Settings") / 2),
            "Settings", UIColor(255, 255, 255, 255), 1.f);
        if(isInSettingsButtonAera) {
            RenderUtil::fillRectangle(SettingsButtonSize, UIColor(0, 0, 0, 120));
        }

        RenderUtil::fillRectangle(RealmsButtonSize, buttonColor);
        RenderUtil::drawText(
            Vec2<float>(windowsSize.x / 2 - RenderUtil::getTextWidth("Realms") / 2,
                        windowsSize.y * 0.682 - RenderUtil::getTextHeight("Realms") / 2),
            "Realms", UIColor(255, 255, 255, 255), 1.f);
        if(isInRealmsButtonAera) {
            RenderUtil::fillRectangle(RealmsButtonSize, UIColor(0, 0, 0, 120));
        }

        RenderUtil::fillRectangle(ScamplaceButtonSize, buttonColor);
        RenderUtil::drawText(
            Vec2<float>(windowsSize.x / 2 - RenderUtil::getTextWidth("Marketplace") / 2,
                        windowsSize.y * 0.753 - RenderUtil::getTextHeight("Marketplace") / 2),
            "Marketplace", UIColor(255, 255, 255, 255), 1.f);
        if(isInScamplaceButtonAera) {
            RenderUtil::fillRectangle(ScamplaceButtonSize, UIColor(0, 0, 0, 120));
        }

        RenderUtil::drawText(
            Vec2<float>(5.f, windowsSize.y - RenderUtil::getTextHeight("1.21.94") - 5.f), "1.21.94",
            UIColor(255, 255, 255, 255), 1.f);
    } else if(screenName.find("world_loading") != std::string::npos) {
        std::string image =
            "https://cdn.wallpapersafari.com/33/79/5upM3e.jpg";
        RenderUtil::drawImageFromUrl(Vec4<float>(windowsSize.x, windowsSize.y), image, "backsex",
                                     1.f);
        Vec4<float> CancelButtonSize = Vec4<float>(windowsSize.x / 2 - windowsSize.x * 0.0859,
                                                     windowsSize.y * 0.611 - windowsSize.y * 0.031,
                                                     windowsSize.x / 2 + windowsSize.x * 0.0859,
                                                     windowsSize.y * 0.611 + windowsSize.y * 0.031);
        Vec2<float> BoostText = Vec2<float>(windowsSize.x / 2 - RenderUtil::getTextWidth("Boost") / 2 - 100,
                        windowsSize.y * 0.611 - RenderUtil::getTextHeight("Boost") / 2 - 250);
        RenderUtil::fillRectangle(CancelButtonSize, buttonColor);
        RenderUtil::drawText(
            Vec2<float>(windowsSize.x / 2 - RenderUtil::getTextWidth("Cancel") / 2,
                        windowsSize.y * 0.611 - RenderUtil::getTextHeight("Cancel") / 2),
            "Cancel", UIColor(255, 255, 255, 255), 1.f);
        if(isInSettingsButtonAera) {
            RenderUtil::fillRectangle(CancelButtonSize, UIColor(0, 0, 0, 120));
        }
        float time = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                            std::chrono::steady_clock::now().time_since_epoch())
                                            .count()) /

                     1000.f;
        float colorPulse = 0.5f + 0.5f * std::sin(time * 5.f);
        UIColor animatedColor(static_cast<int>(255 * colorPulse),
                              static_cast<int>(128 + 127 * colorPulse), 255, 255);

        RenderUtil::drawText(Vec2<float>(BoostText.x, BoostText.y), "Boost", animatedColor, 4.f);
        using clock = std::chrono::steady_clock;
        static auto lastTick = clock::now();
        static int dotPhase = 3;  
        const int stepMs = 300;
        auto now = clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick).count() >=
           stepMs) {
            dotPhase = (dotPhase + 1) % 4;  
            lastTick = now;
        }
        std::string baseText = "Loading";
        std::string dots(dotPhase, '.');

        const float scale = 2.f;
        const UIColor color(128, 128, 128, 255);

        float loadingX = BoostText.x  + 30;
        float loadingY = BoostText.y + RenderUtil::getTextHeight("Boost") * 4.f + 20.0f;

        float dotsX = loadingX + RenderUtil::getTextWidth(baseText) * scale + 5.0f;
        float dotsY = loadingY;

        RenderUtil::drawText(Vec2<float>(loadingX, loadingY), baseText.c_str(), color, scale);
        RenderUtil::drawText(Vec2<float>(dotsX, dotsY), dots.c_str(), color, scale);

    }

      if(screenName == "start_screen") {
        float baseX = windowsSize.x / 2.f;
         
        float baseY;
        float pornSize = 3.f;
        float time = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                            std::chrono::steady_clock::now().time_since_epoch())
                                            .count()) /
                     1000.f;

            baseY  = windowsSize.y * 0.15f;
        float scaleAnim = pornSize + 0.3f * std::sin(time * 3.f);
        float colorPulse = 0.5f + 0.5f * std::sin(time * 5.f);

        UIColor animatedColor(static_cast<int>(255 * colorPulse),
                              static_cast<int>(128 + 127 * colorPulse), 255, 255);

        float textWidth = RenderUtil::getTextWidth("Boost") * scaleAnim;
        float textHeight = RenderUtil::getTextHeight("Boost") * scaleAnim;

        RenderUtil::drawText(Vec2<float>(baseX - textWidth / 2.f, baseY - textHeight / 2.f),
                             "Boost", animatedColor, scaleAnim);
    }


    }
void RenderUtil::Render() {
    Vec2<float> windowsSize = GI::getGuiData()->windowSizeReal;
    ModuleManager::onD2DRender();
    NotificationManager::Render();
    // std::string winX = std::to_string(mpos.x);
    // std::string winY = std::to_string(mpos.y);
    // std::string MySize = winX + "x" + winY;

    RenderUI();
    static Editor* editor = ModuleManager::getModule<Editor>();
    Vec2<float> winSize = GI::getGuiData()->windowSizeReal;
    static ClickGUI* clickGuiMod = ModuleManager::getModule<ClickGUI>();
    // BlurBackGround

    // ClickGUI
    {
        static BGAnimation* bg = ModuleManager::getModule<BGAnimation>();
        static ClickGUI* clickGuiMod = ModuleManager::getModule<ClickGUI>();
        if(bg->isEnabled() && clickGuiMod->isEnabled()) {
            bg->renderOverGUI();
        }

        if(!clickGuiMod)  // nullptr check
            return;

        if(clickGuiMod->isEnabled()) {
            clickGuiMod->Render();
        }

        // BGAnimation (render above ClickGUI)
    }
    // NeteaseMusicGUI (always on top)
    {
        static NeteaseMusicGUI* musicGuiMod = ModuleManager::getModule<NeteaseMusicGUI>();
        if(musicGuiMod->isEnabled())
            musicGuiMod->Render();
    }

    // Modern Eject Interface
    {
        Vec2<float> windowSize = RenderUtil::getWindowSize();
        static float holdTime = 0.f;
        static float holdAnim = 0.f;
        static float showDuration = 0.f;
        static float exitDuration = 0.f;
        static float exitVelocity = 0.f;
        static float pulseAnim = 0.f;

        // Update pulse animation
        pulseAnim += deltaTime * 4.f;

        if(showDuration > 0.1f) {
            // Modern styling
            float scale = showDuration;
            float textSize = 0.9f * scale;
            float padding = 20.f * scale;
            float borderRadius = 12.f * scale;

            // Enhanced text with modern styling
            static std::string mainText = "Hold Ctrl + L to eject";
            static std::string subText = "Release to cancel";

            float mainTextWidth = getTextWidth(mainText, textSize);
            float subTextWidth = getTextWidth(subText, textSize * 0.7f);
            float maxTextWidth = std::max(mainTextWidth, subTextWidth);
            float mainTextHeight = getTextHeight(mainText, textSize);
            float subTextHeight = getTextHeight(subText, textSize * 0.7f);
            float totalHeight = mainTextHeight + subTextHeight + 8.f * scale;

            // Center positioning with smooth slide-in animation
            float slideOffset = (1.f - showDuration) * 50.f;
            Vec2<float> panelPos =
                Vec2<float>((windowSize.x - maxTextWidth - padding * 2) / 2.f, 50.f + slideOffset);

            // Panel dimensions
            Vec4<float> panelRect = Vec4<float>(panelPos.x - padding, panelPos.y - padding,
                                                panelPos.x + maxTextWidth + padding,
                                                panelPos.y + totalHeight + padding);

            // Modern gradient background with glassmorphism effect
            float alpha = showDuration * 0.9f;
            UIColor bgColor1 = UIColor(30, 30, 40, (int)(220 * alpha));
            UIColor bgColor2 = UIColor(45, 45, 60, (int)(180 * alpha));

            // Background with subtle gradient effect (simulated with multiple layers)
            fillRoundedRectangle(panelRect, bgColor1, borderRadius);

            // Add subtle inner glow
            Vec4<float> innerGlow =
                Vec4<float>(panelRect.x + 1, panelRect.y + 1, panelRect.z - 1, panelRect.w - 1);
            UIColor glowColor = UIColor(80, 120, 200, (int)(40 * alpha));
            fillRoundedRectangle(innerGlow, glowColor, borderRadius - 1);

            // Animated border with pulse effect
            float pulseIntensity = (sinf(pulseAnim) + 1.f) * 0.5f;
            UIColor borderColor =
                UIColor(100 + (int)(50 * pulseIntensity), 150 + (int)(50 * pulseIntensity), 255,
                        (int)(150 * alpha));
            drawRoundedRectangle(panelRect, borderColor, borderRadius, 2.f);

            // Progress bar background
            float progressBarY = panelRect.w - padding * 0.7f;
            Vec4<float> progressBg =
                Vec4<float>(panelRect.x + padding * 0.5f, progressBarY - 3.f * scale,
                            panelRect.z - padding * 0.5f, progressBarY + 3.f * scale);

            UIColor progressBgColor = UIColor(60, 60, 80, (int)(180 * alpha));
            fillRoundedRectangle(progressBg, progressBgColor, 3.f * scale);

            // Animated progress bar with gradient effect
            float progressWidth = (progressBg.z - progressBg.x) * holdAnim;
            if(progressWidth > 0) {
                Vec4<float> progressFill = Vec4<float>(progressBg.x, progressBg.y,
                                                       progressBg.x + progressWidth, progressBg.w);

                // Create gradient effect for progress bar
                UIColor progressColor1 = UIColor(100, 200, 255, (int)(255 * alpha));
                UIColor progressColor2 = UIColor(50, 150, 255, (int)(255 * alpha));

                fillRoundedRectangle(progressFill, progressColor1, 3.f * scale);

                // Add highlight on top
                Vec4<float> highlight = Vec4<float>(progressFill.x, progressFill.y, progressFill.z,
                                                    progressFill.y + 2.f * scale);
                UIColor highlightColor = UIColor(150, 220, 255, (int)(100 * alpha));
                fillRoundedRectangle(highlight, highlightColor, 3.f * scale);
            }

            // Main text with enhanced styling
            Vec2<float> mainTextPos =
                Vec2<float>(panelPos.x + (maxTextWidth - mainTextWidth) / 2.f, panelPos.y);

            // Text shadow for depth
            drawText(Vec2<float>(mainTextPos.x + 1, mainTextPos.y + 1), mainText,
                     UIColor(0, 0, 0, (int)(120 * alpha)), textSize);

            // Main text with subtle color animation
            float textColorCycle = (sinf(pulseAnim * 0.8f) + 1.f) * 0.5f;
            UIColor mainTextColor =
                UIColor(220 + (int)(35 * textColorCycle), 230 + (int)(25 * textColorCycle), 255,
                        (int)(255 * alpha));
            drawText(mainTextPos, mainText, mainTextColor, textSize);

            // Subtitle text
            Vec2<float> subTextPos = Vec2<float>(panelPos.x + (maxTextWidth - subTextWidth) / 2.f,
                                                 mainTextPos.y + mainTextHeight + 8.f * scale);

            UIColor subTextColor = UIColor(160, 170, 190, (int)(200 * alpha));
            drawText(subTextPos, subText, subTextColor, textSize * 0.7f);

            // Animated particles effect around the panel
            if(holdAnim > 0.1f) {
                for(int i = 0; i < 6; i++) {
                    float angle = (pulseAnim + i * 1.047f) * 0.5f;  // 60 degrees apart
                    float distance = 30.f + sinf(pulseAnim + i) * 8.f;
                    float particleX = (panelRect.x + panelRect.z) / 2.f + cosf(angle) * distance;
                    float particleY = (panelRect.y + panelRect.w) / 2.f + sinf(angle) * distance;

                    float particleAlpha = holdAnim * (sinf(pulseAnim * 2.f + i) + 1.f) * 0.5f;
                    UIColor particleColor = UIColor(100, 150, 255, (int)(80 * particleAlpha));
                    fillCircle(Vec2<float>(particleX, particleY), particleColor, 2.f * scale);
                }
            }
        }

        // Input handling
        if(GI::isKeyDown(VK_CONTROL) && GI::isKeyDown('L')) {
            holdTime += RenderUtil::deltaTime;
            if(holdTime > 1.f)
                holdTime = 1.f;
            exitDuration = 2.f;  // Extended duration for better UX
        } else {
            holdTime = 0.f;
            exitDuration -= RenderUtil::deltaTime;
        }

        // Smooth animations
        holdAnim += (holdTime - holdAnim) * (RenderUtil::deltaTime * 8.f);
        if(holdAnim > 1.f)
            holdAnim = 1.f;
        if(holdAnim < 0.f)
            holdAnim = 0.f;

        if(exitDuration > 0.f) {
            showDuration += (1.f - showDuration) * (RenderUtil::deltaTime * 6.f);
            exitVelocity = 0.f;
        } else {
            showDuration -= exitVelocity;
            exitVelocity += RenderUtil::deltaTime / 3.f;
        }

        if(showDuration < 0.f)
            showDuration = 0.f;
        if(showDuration > 1.f)
            showDuration = 1.f;

        // Eject when progress is complete
        if(holdAnim > 0.99f) {
            // if (auto chatMod = ModuleManager::getModule<ChatModule>()) {
            //	if (chatMod->isEnabled()) chatMod->forceDisable();
            // }
            if(auto musicMod = ModuleManager::getModule<NeteaseMusicGUI>()) {
                if(musicMod->isEnabled())
                    musicMod->setEnabled(false);
            }
            Client::shutdown();
        }
    }
}

void RenderUtil::Clean() {
    if(!initD2D)
        return;

    // 先设置标志防止新的渲染调用
    initD2D = false;

    // 等待可能正在进行的渲染操作完成
    if(d2dDeviceContext) {
        d2dDeviceContext->Flush();
        Sleep(16);  // 等待一帧时间
    }

    // 释放所有 D2D 资源（按照依赖关系逆序释放）
    SafeRelease(shadowBitmap);   // 新增的阴影位图
    SafeRelease(stencilEffect);  // 新增的模板效果
    SafeRelease(shadowEffect);   // 新增的阴影效果
    SafeRelease(blurEffect);
    SafeRelease(sourceBitmap);
    SafeRelease(d2dDeviceContext);
    SafeRelease(d2dDevice);
    SafeRelease(d2dWriteFactory);
    SafeRelease(d2dFactory);

    // 释放 WIC 工厂
    SafeRelease(wicFactory);

    // 清理 COM 智能指针缓存
    textFormatCache.clear();
    textLayoutCache.clear();
    colorBrushCache.clear();
    textLayoutTemporary.clear();
    urlBitmapCache.clear();
    skinBitmapCache.clear();

    // 清理图片缓存
    imageCacheMap.clear();
    failedUrls.clear();
    failureCount.clear();

    // 确保标志位正确设置
    initD2D = false;
}

void RenderUtil::Flush() {
    d2dDeviceContext->Flush();
}

Vec2<float> RenderUtil::getWindowSize() {
    if(sourceBitmap == nullptr) {
        return Vec2<float>(1920.f, 1080.f);
    }
    D2D1_SIZE_U size = sourceBitmap->GetPixelSize();
    return Vec2<float>((float)size.width, (float)size.height);
}

void RenderUtil::drawText(const Vec2<float>& textPos, const std::string& textStr,
                          const UIColor& color, float textSize, bool storeTextLayout) {
    D2D_CTX_GUARD();
    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);

    static CustomFont* customFontMod = ModuleManager::getModule<CustomFont>();
    if(customFontMod->shadow) {
        ID2D1SolidColorBrush* shadowColorBrush = getSolidColorBrush(UIColor(0, 0, 0, color.a));
        d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x + 1.f, textPos.y + 1.f),
                                         textLayout, shadowColorBrush);
    }

    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x, textPos.y), textLayout, colorBrush);
}

float RenderUtil::getTextWidth(const std::string& textStr, float textSize, bool storeTextLayout) {
    D2D_CTX_GUARD_RET(0.f);
    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);
    DWRITE_TEXT_METRICS textMetrics;
    textLayout->GetMetrics(&textMetrics);

    return textMetrics.widthIncludingTrailingWhitespace;
}

float RenderUtil::getTextHeight(const std::string& textStr, float textSize, bool storeTextLayout) {
    D2D_CTX_GUARD_RET(0.f);
    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);
    if(!textLayout)  // stupid fix 
        return 0.f;

    DWRITE_TEXT_METRICS textMetrics;
    textLayout->GetMetrics(&textMetrics);

    return std::ceilf(textMetrics.height);
}


void RenderUtil::drawLine(const Vec2<float>& startPos, const Vec2<float>& endPos,
                          const UIColor& color, float width) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawLine(D2D1::Point2F(startPos.x, startPos.y),
                               D2D1::Point2F(endPos.x, endPos.y), colorBrush, width);
}
void RenderUtil::drawHSVBox(const Vec4<float>& rect, const Vec4<float>& hueColor) {
    ID2D1GradientStopCollection* stops = nullptr;
    D2D1_GRADIENT_STOP hStops[2] = {{0.0f, D2D1::ColorF(D2D1::ColorF::White)},
                                    {1.0f, D2D1::ColorF(hueColor.x, hueColor.y, hueColor.z)}};
    d2dDeviceContext->CreateGradientStopCollection(hStops, 2, &stops);
    ID2D1LinearGradientBrush* hBrush = nullptr;
    d2dDeviceContext->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(D2D1::Point2F(rect.x, rect.y),
                                            D2D1::Point2F(rect.z, rect.y)),
        stops, &hBrush);

    d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), hBrush);
    stops->Release();
    hBrush->Release();
    D2D1_GRADIENT_STOP vStops[2] = {{0.0f, D2D1::ColorF(0, 0, 0, 0)},
                                    {1.0f, D2D1::ColorF(0, 0, 0, 1)}};
    d2dDeviceContext->CreateGradientStopCollection(vStops, 2, &stops);
    ID2D1LinearGradientBrush* vBrush = nullptr;
    d2dDeviceContext->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(D2D1::Point2F(rect.x, rect.y),
                                            D2D1::Point2F(rect.x, rect.w)),
        stops, &vBrush);

    d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), vBrush);
    stops->Release();
    vBrush->Release();
}
void RenderUtil::drawHueBar(const Vec4<float>& rect) {
    D2D1_GRADIENT_STOP hueStops[7] = {{0.0f, D2D1::ColorF(D2D1::ColorF::Red)},
                                      {1.0f / 6.0f, D2D1::ColorF(D2D1::ColorF::Yellow)},
                                      {2.0f / 6.0f, D2D1::ColorF(D2D1::ColorF::Green)},
                                      {3.0f / 6.0f, D2D1::ColorF(D2D1::ColorF::Cyan)},
                                      {4.0f / 6.0f, D2D1::ColorF(D2D1::ColorF::Blue)},
                                      {5.0f / 6.0f, D2D1::ColorF(D2D1::ColorF::Magenta)},
                                      {1.0f, D2D1::ColorF(D2D1::ColorF::Red)}};

    ID2D1GradientStopCollection* stops = nullptr;
    d2dDeviceContext->CreateGradientStopCollection(hueStops, 7, &stops);

    ID2D1LinearGradientBrush* hueBrush = nullptr;
    d2dDeviceContext->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(D2D1::Point2F(rect.x, rect.y),
                                            D2D1::Point2F(rect.x, rect.w)),
        stops, &hueBrush);

    d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), hueBrush);
    stops->Release();
    hueBrush->Release();
}
void RenderUtil::drawAlphaBar(const Vec4<float>& rect, const UIColor& baseColor) {
    D2D1_GRADIENT_STOP alphaStops[2] = {
        {0.0f, D2D1::ColorF(baseColor.r / 255.f, baseColor.g / 255.f, baseColor.b / 255.f, 1.f)},
        {1.0f, D2D1::ColorF(baseColor.r / 255.f, baseColor.g / 255.f, baseColor.b / 255.f, 0.f)}};

    ID2D1GradientStopCollection* stops = nullptr;
    d2dDeviceContext->CreateGradientStopCollection(alphaStops, 2, &stops);

    ID2D1LinearGradientBrush* alphaBrush = nullptr;
    d2dDeviceContext->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(D2D1::Point2F(rect.x, rect.y),
                                            D2D1::Point2F(rect.x, rect.w)),
        stops, &alphaBrush);

    d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), alphaBrush);
    stops->Release();
    alphaBrush->Release();
}
void RenderUtil::drawRectangle(const Vec4<float>& rect, const UIColor& color, float width) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush, width);
}

void RenderUtil::fillRectangle(const Vec4<float>& rect, const UIColor& color) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush);
}

void RenderUtil::drawRoundedRectangle(const Vec4<float>& rect, const UIColor& color, float radius,
                                      float width) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    D2D1_ROUNDED_RECT roundedRect =
        D2D1::RoundedRect(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), radius, radius);
    d2dDeviceContext->DrawRoundedRectangle(&roundedRect, colorBrush, width);
}

void RenderUtil::fillRoundedRectangle(const Vec4<float>& rect, const UIColor& color, float radius) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    D2D1_ROUNDED_RECT roundedRect =
        D2D1::RoundedRect(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), radius, radius);
    d2dDeviceContext->FillRoundedRectangle(&roundedRect, colorBrush);
}

void RenderUtil::drawCircle(const Vec2<float>& centerPos, const UIColor& color, float radius,
                            float width) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawEllipse(
        D2D1::Ellipse(D2D1::Point2F(centerPos.x, centerPos.y), radius, radius), colorBrush, width);
}

void RenderUtil::fillCircle(const Vec2<float>& centerPos, const UIColor& color, float radius) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->FillEllipse(
        D2D1::Ellipse(D2D1::Point2F(centerPos.x, centerPos.y), radius, radius), colorBrush);
}

void RenderUtil::addBlur(const Vec4<float>& rect, float strength, bool flush) {
    D2D_CTX_GUARD();
    if(flush) {
        d2dDeviceContext->Flush();
    }
    ID2D1Bitmap* targetBitmap = nullptr;
    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(sourceBitmap->GetPixelFormat());
    d2dDeviceContext->CreateBitmap(sourceBitmap->GetPixelSize(), props, &targetBitmap);
    D2D1_POINT_2U destPoint = D2D1::Point2U(0, 0);
    D2D1_SIZE_U size = sourceBitmap->GetPixelSize();
    D2D1_RECT_U Rect = D2D1::RectU(0, 0, size.width, size.height);
    targetBitmap->CopyFromBitmap(&destPoint, sourceBitmap, &Rect);

    D2D1_RECT_F screenRectF = D2D1::RectF(0.f, 0.f, (float)sourceBitmap->GetPixelSize().width,
                                          (float)sourceBitmap->GetPixelSize().height);
    D2D1_RECT_F clipRectD2D = D2D1::RectF(rect.x, rect.y, rect.z, rect.w);

    blurEffect->SetInput(0, targetBitmap);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, strength);

    ID2D1Image* outImage = nullptr;
    blurEffect->GetOutput(&outImage);

    ID2D1ImageBrush* outImageBrush = nullptr;
    D2D1_IMAGE_BRUSH_PROPERTIES outImage_props = D2D1::ImageBrushProperties(screenRectF);
    d2dDeviceContext->CreateImageBrush(outImage, outImage_props, &outImageBrush);

    ID2D1RectangleGeometry* clipRectGeo = nullptr;
    d2dFactory->CreateRectangleGeometry(clipRectD2D, &clipRectGeo);
    d2dDeviceContext->FillGeometry(clipRectGeo, outImageBrush);

    targetBitmap->Release();
    outImage->Release();
    outImageBrush->Release();
    clipRectGeo->Release();
}

std::wstring to_wide(const std::string& str) {
    if(str.empty())
        return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if(len <= 0)
        return L"";
    std::wstring wstr(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    return wstr;
}

uint64_t getTextLayoutKey(const std::string& textStr, float textSize) {
    std::hash<std::string> textHash;
    std::hash<float> textSizeHash;
    uint64_t combinedHash = textHash(textStr) ^ textSizeHash(textSize);
    return combinedHash;
}

IDWriteTextFormat* getTextFormat(float textSize) {
    if(textFormatCache[textSize].get() == nullptr) {
        std::wstring fontNameWide = to_wide(currentD2DFont);
        if(fontNameWide.empty())
            fontNameWide = L"Microsoft YaHei";  // Ensure Chinese font fallback

        d2dWriteFactory->CreateTextFormat(
            fontNameWide.c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL,
            isFontItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, (float)currentD2DFontSize * textSize,
            L"zh-cn",  // Use Chinese locale
            textFormatCache[textSize].put());
    }
    return textFormatCache[textSize].get();
}

IDWriteTextLayout* getTextLayout(const std::string& textStr, float textSize, bool storeTextLayout) {
    std::wstring wideText = to_wide(textStr);
    const WCHAR* text = wideText.c_str();
    IDWriteTextFormat* textFormat = getTextFormat(textSize);
    uint64_t textLayoutKey = getTextLayoutKey(textStr, textSize);

    if(storeTextLayout) {
        if(textLayoutCache[textLayoutKey].get() == nullptr) {
            d2dWriteFactory->CreateTextLayout(text, (UINT32)wcslen(text), textFormat, FLT_MAX, 0.f,
                                              textLayoutCache[textLayoutKey].put());
        }
        return textLayoutCache[textLayoutKey].get();
    } else {
        if(textLayoutTemporary[textLayoutKey].get() == nullptr) {
            d2dWriteFactory->CreateTextLayout(text, (UINT32)wcslen(text), textFormat, FLT_MAX, 0.f,
                                              textLayoutTemporary[textLayoutKey].put());
        }
        return textLayoutTemporary[textLayoutKey].get();
    }
}




ID2D1SolidColorBrush* getSolidColorBrush(const UIColor& color) {
    if(!d2dDeviceContext)
        return nullptr;  // Context not ready �C avoid crash
    uint32_t colorBrushKey = ColorUtil::ColorToUInt(color);
    if(colorBrushCache[colorBrushKey].get() == nullptr) {
        d2dDeviceContext->CreateSolidColorBrush(color.toD2D1Color(),
                                                colorBrushCache[colorBrushKey].put());
    }
    return colorBrushCache[colorBrushKey].get();
}

void RenderUtil::drawImage(const Vec4<float>& rect, const std::string& url) {
    D2D_CTX_GUARD();
    if(!d2dDeviceContext)
        return;

    auto bmp = loadBitmapFromUrl(url);
    if(!bmp)
        return;

    D2D1_RECT_F dest = D2D1::RectF(rect.x, rect.y, rect.z, rect.w);
    D2D1_SIZE_F size = bmp->GetSize();
    D2D1_RECT_F src = D2D1::RectF(0, 0, size.width, size.height);
    float opacity = 1.0f;
    d2dDeviceContext->DrawBitmap(bmp.get(), dest, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                                 src);
}

// Icon helpers
void RenderUtil::drawTriangle(const Vec4<float>& rect, const UIColor& color, bool leftDirection,
                              bool filled) {
    Vec2<float> p1, p2, p3;
    float midY = (rect.y + rect.w) / 2.f;
    if(leftDirection) {
        p1 = {rect.z, rect.y};
        p2 = {rect.x, midY};
        p3 = {rect.z, rect.w};
    } else {
        p1 = {rect.x, rect.y};
        p2 = {rect.z, midY};
        p3 = {rect.x, rect.w};
    }
    if(filled) {
        ID2D1PathGeometry* geometry = nullptr;
        d2dFactory->CreatePathGeometry(&geometry);
        ID2D1GeometrySink* sink = nullptr;
        geometry->Open(&sink);
        sink->BeginFigure(D2D1::Point2F(p1.x, p1.y), D2D1_FIGURE_BEGIN_FILLED);
        D2D1_POINT_2F points[2] = {D2D1::Point2F(p2.x, p2.y), D2D1::Point2F(p3.x, p3.y)};
        sink->AddLines(points, 2);
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
        sink->Release();
        ID2D1SolidColorBrush* brush = getSolidColorBrush(color);
        d2dDeviceContext->FillGeometry(geometry, brush);
        geometry->Release();
    } else {
        drawLine(p1, p2, color, 2.f);
        drawLine(p2, p3, color, 2.f);
        drawLine(p3, p1, color, 2.f);
    }
}

void RenderUtil::drawPause(const Vec4<float>& rect, const UIColor& color) {
    float barWidth = (rect.z - rect.x) * 0.3f;
    Vec4<float> leftBar{rect.x, rect.y, rect.x + barWidth, rect.w};
    Vec4<float> rightBar{rect.z - barWidth, rect.y, rect.z, rect.w};
    fillRectangle(leftBar, color);
    fillRectangle(rightBar, color);
}

void RenderUtil::fillRect(const Vec2<float>& pos, const Vec2<float>& size, const UIColor& color) {
    Vec4<float> rect(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
    fillRectangle(rect, color);
}

float RenderUtil::getTime() {
    return TimerUtil::getTime();
}

Vec2<float> RenderUtil::WorldToScreen(const Vec3<float>& worldPos, const Vec2<float>& screenSize) {
    Vec2<float> screenCoords;

    if(Matrix::WorldToScreen(worldPos, screenCoords)) {
        return screenCoords;
    }

    return Vec2<float>(-1, -1);  // 转换失败
}

// 新的 drawTextInWorld 函数
void RenderUtil::drawTextInWorld(const Vec3<float>& worldPos, const std::string& textStr,
                                 const UIColor& color, float textSize) {
    D2D_CTX_GUARD();

    Vec2<float> screenSize = getWindowSize();
    Vec2<float> screenPos = WorldToScreen(worldPos, screenSize);

    // 检查是否转换成功
    if(screenPos.x < 0 || screenPos.y < 0)
        return;

    // 检查是否在屏幕范围内
    if(screenPos.x > screenSize.x || screenPos.y > screenSize.y)
        return;

    // 调整文字位置（居中）
    float textWidth = getTextWidth(textStr, textSize);
    float textHeight = getTextHeight(textStr, textSize);

    screenPos.x -= textWidth * 0.5f;
    screenPos.y -= textHeight * 0.5f;

    // 使用 D2D 渲染文字
    drawText(screenPos, textStr, color, textSize);
}

void RenderUtil::drawAdvancedText(const Vec2<float>& textPos, const std::string& textStr,
                                  const UIColor& color1, const UIColor& color2, float textSize,
                                  bool enableDynamicColor, bool enableSweep, bool enableShadow,
                                  float time, float colorSpeed, float sweepSpeed, float sweepWidth,
                                  float sweepIntensity, float shadowSize, float shadowIntensity) {
    D2D_CTX_GUARD();

    if(time == 0.0f) {
        time = getTime();
    }

    // 1. 计算文字尺寸
    float textWidth = getTextWidth(textStr, textSize);
    float textHeight = getTextHeight(textStr, textSize);

    // 2. 绘制阴影（如果启用）- 现在可以自定义大小和强度
    if(enableShadow) {
        int shadowLayers =
            std::clamp(static_cast<int>(shadowIntensity * 5), 1, 8);  // 根据强度调整层数

        for(int i = shadowLayers; i >= 1; i--) {
            float layerOffset = (shadowSize * i * 1.5f) / shadowLayers;  // 使用自定义阴影大小
            float layerAlpha =
                (80.0f * shadowIntensity / i) * (color1.a / 255.0f);  // 使用自定义强度

            UIColor shadowColor = UIColor(0, 0, 0, static_cast<uint8_t>(layerAlpha));
            Vec2<float> shadowPos = Vec2<float>(textPos.x + layerOffset, textPos.y + layerOffset);

            drawText(shadowPos, textStr, shadowColor, textSize, false);
        }
    }

    // 3. 计算最终颜色
    UIColor finalColor = color1;

    // 4. 双色动态变化（如果启用）
    if(enableDynamicColor) {
        // 在两个颜色之间来回变化
        float wave = (sinf(time * colorSpeed) + 1.0f) * 0.5f;  // 0-1 范围
        finalColor = ColorUtil::lerp(color1, color2, wave);
    }

    // 5. 绘制主文字
    drawText(textPos, textStr, finalColor, textSize);

    // 6. 扫光效果（如果启用）- 现在可以自定义宽度和强度
    if(enableSweep) {
        float sweepCycle = fmodf(time * sweepSpeed, 3.0f);
        float sweepProgress = sweepCycle / 3.0f;

        // 使用自定义扫光宽度（相对于文字宽度的比例）
        float actualSweepWidth = textWidth * std::clamp(sweepWidth, 0.1f, 2.0f);
        float sweepCenter = -actualSweepWidth + (textWidth + actualSweepWidth * 2) * sweepProgress;

        if(sweepCenter > -actualSweepWidth && sweepCenter < textWidth + actualSweepWidth) {
            Vec4<float> sweepRect =
                Vec4<float>(textPos.x + sweepCenter - actualSweepWidth / 2, textPos.y,
                            textPos.x + sweepCenter + actualSweepWidth / 2, textPos.y + textHeight);

            sweepRect.x = std::max(sweepRect.x, textPos.x);
            sweepRect.z = std::min(sweepRect.z, textPos.x + textWidth);

            if(sweepRect.z > sweepRect.x) {
                ID2D1RectangleGeometry* sweepGeometry = nullptr;
                d2dFactory->CreateRectangleGeometry(
                    D2D1::RectF(sweepRect.x, sweepRect.y, sweepRect.z, sweepRect.w),
                    &sweepGeometry);

                if(sweepGeometry) {
                    d2dDeviceContext->PushLayer(
                        D2D1::LayerParameters(D2D1::InfiniteRect(), sweepGeometry), nullptr);

                    // 使用自定义扫光强度
                    int brightnessBoost =
                        static_cast<int>(100 * std::clamp(sweepIntensity, 0.1f, 3.0f));
                    UIColor sweepColor =
                        UIColor(std::min(255, finalColor.r + brightnessBoost),
                                std::min(255, finalColor.g + brightnessBoost),
                                std::min(255, finalColor.b + brightnessBoost), finalColor.a);

                    drawText(textPos, textStr, sweepColor, textSize, false);

                    d2dDeviceContext->PopLayer();
                    sweepGeometry->Release();
                }
            }
        }
    }
}

// 基础圆形光晕效果
void RenderUtil::drawGlowEffect(const Vec2<float>& centerPos, float radius,
                                const UIColor& glowColor, float glowIntensity, int glowLayers) {
    D2D_CTX_GUARD();

    // 确保参数合理
    glowLayers = std::max(1, std::min(glowLayers, 20));
    glowIntensity = std::max(0.1f, std::min(glowIntensity, 5.0f));

    // 从外到内绘制多层圆形
    for(int i = glowLayers; i >= 1; i--) {
        float layerRadius = radius + (radius * 0.5f * i / glowLayers);
        float layerAlpha = (glowColor.a * glowIntensity) / (i * i);  // 二次衰减

        UIColor layerColor =
            UIColor(glowColor.r, glowColor.g, glowColor.b, std::max(0, (int)layerAlpha));

        fillCircle(centerPos, layerColor, layerRadius);
    }
}

// 矩形光晕效果
void RenderUtil::drawRectGlowEffect(const Vec4<float>& rect, const UIColor& glowColor,
                                    float glowRadius, float glowIntensity) {
    D2D_CTX_GUARD();

    int layers = std::max(1, (int)(glowRadius / 2.0f));

    for(int i = layers; i >= 1; i--) {
        float layerRadius = (glowRadius * i) / layers;
        float layerAlpha = (glowColor.a * glowIntensity) / (i * 1.5f);

        UIColor layerColor =
            UIColor(glowColor.r, glowColor.g, glowColor.b, std::max(0, (int)layerAlpha));

        Vec4<float> expandedRect = Vec4<float>(rect.x - layerRadius, rect.y - layerRadius,
                                               rect.z + layerRadius, rect.w + layerRadius);

        fillRoundedRectangle(expandedRect, layerColor, layerRadius * 0.5f);
    }
}

// 圆形边框光晕效果
void RenderUtil::drawCircleGlowEffect(const Vec2<float>& centerPos, float radius,
                                      const UIColor& glowColor, float glowRadius,
                                      float glowIntensity) {
    D2D_CTX_GUARD();

    int layers = std::max(3, (int)(glowRadius / 1.5f));

    for(int i = layers; i >= 1; i--) {
        float layerGlowRadius = (glowRadius * i) / layers;
        float layerAlpha = (glowColor.a * glowIntensity * 0.8f) / i;

        UIColor layerColor =
            UIColor(glowColor.r, glowColor.g, glowColor.b, std::max(0, (int)layerAlpha));

        drawCircle(centerPos, layerColor, radius + layerGlowRadius, layerGlowRadius * 0.3f);
    }
}

// 动画光晕效果
void RenderUtil::drawAnimatedGlow(const Vec2<float>& centerPos, float radius,
                                  const UIColor& glowColor, float time, bool pulse, bool rotate) {
    D2D_CTX_GUARD();

    if(time == 0.0f) {
        time = getTime();
    }

    // 脉冲效果
    float pulseMultiplier = pulse ? (sinf(time * 3.0f) * 0.4f + 0.8f) : 1.0f;

    // 旋转效果（通过多个偏移光源模拟）
    if(rotate) {
        int numGlows = 6;
        for(int i = 0; i < numGlows; i++) {
            float angle = (time * 2.0f + i * (360.0f / numGlows)) * (3.14159f / 180.0f);
            float offsetRadius = radius * 0.3f;

            Vec2<float> glowPos = Vec2<float>(centerPos.x + cosf(angle) * offsetRadius,
                                              centerPos.y + sinf(angle) * offsetRadius);

            float glowAlpha = (sinf(time * 4.0f + i) + 1.0f) * 0.5f;
            UIColor animColor = UIColor(glowColor.r, glowColor.g, glowColor.b,
                                        (int)(glowColor.a * glowAlpha * 0.6f));

            drawGlowEffect(glowPos, radius * 0.6f * pulseMultiplier, animColor, 0.8f, 6);
        }
    } else {
        // 简单脉冲光晕
        drawGlowEffect(centerPos, radius * pulseMultiplier, glowColor, 1.0f, 8);
    }
}

// 更新现有的 drawGradientText 函数
void RenderUtil::drawGradientText(const Vec2<float>& textPos, const std::string& textStr,
                                  const std::vector<ColorUtil::GradientStop>& stops, float textSize,
                                  bool enableSweep, float sweepSpeed, float sweepWidth,
                                  float sweepIntensity, float sweepCycleDuration) {
    if(enableSweep) {
        static float time = 0.0f;
        time += deltaTime;
        drawGradientTextWithSweep(textPos, textStr, stops, textSize, true, time, sweepSpeed,
                                  sweepWidth, sweepIntensity, sweepCycleDuration);
    } else {
        D2D_CTX_GUARD();

        IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, true);
        DWRITE_TEXT_METRICS metrics;
        textLayout->GetMetrics(&metrics);

        auto brush = ColorUtil::createLinearGradientBrush(
            d2dDeviceContext, stops, D2D1::Point2F(textPos.x, textPos.y),
            D2D1::Point2F(textPos.x + metrics.width, textPos.y));

        if(brush) {
            d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x, textPos.y), textLayout,
                                             brush);
            brush->Release();
        }
    }
}

// 更新 drawGradientTextWithSweep 函数
void RenderUtil::drawGradientTextWithSweep(const Vec2<float>& textPos, const std::string& textStr,
                                           const std::vector<ColorUtil::GradientStop>& stops,
                                           float textSize, bool enableSweep, float time,
                                           float sweepSpeed, float sweepWidth, float sweepIntensity,
                                           float sweepCycleDuration) {
    D2D_CTX_GUARD();

    if(time == 0.0f) {
        time = getTime();
    }

    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, true);
    DWRITE_TEXT_METRICS metrics;
    textLayout->GetMetrics(&metrics);

    // 1. 首先绘制基础渐变文字
    auto gradientBrush = ColorUtil::createLinearGradientBrush(
        d2dDeviceContext, stops, D2D1::Point2F(textPos.x, textPos.y),
        D2D1::Point2F(textPos.x + metrics.width, textPos.y));

    if(gradientBrush) {
        d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x, textPos.y), textLayout,
                                         gradientBrush);
        gradientBrush->Release();
    }

    // 2. 如果启用扫光效果，则添加扫光层
    if(enableSweep) {
        // 分离扫光周期和速度控制
        // sweepSpeed 控制整体动画的播放速度（时间流逝的快慢）
        // sweepCycleDuration 控制一个完整扫光周期的时长（秒）
        float adjustedTime = time * sweepSpeed;                      // 速度调整
        float sweepCycle = fmodf(adjustedTime, sweepCycleDuration);  // 周期调整
        float sweepProgress = sweepCycle / sweepCycleDuration;       // 0-1 进度

        // 计算扫光宽度和位置
        float actualSweepWidth = metrics.width * std::clamp(sweepWidth, 0.1f, 2.0f);
        float sweepCenter =
            -actualSweepWidth + (metrics.width + actualSweepWidth * 2) * sweepProgress;

        if(sweepCenter > -actualSweepWidth && sweepCenter < metrics.width + actualSweepWidth) {
            Vec4<float> sweepRect = Vec4<float>(
                textPos.x + sweepCenter - actualSweepWidth / 2, textPos.y,
                textPos.x + sweepCenter + actualSweepWidth / 2, textPos.y + metrics.height);

            sweepRect.x = std::max(sweepRect.x, textPos.x);
            sweepRect.z = std::min(sweepRect.z, textPos.x + metrics.width);

            if(sweepRect.z > sweepRect.x) {
                ID2D1RectangleGeometry* sweepGeometry = nullptr;
                d2dFactory->CreateRectangleGeometry(
                    D2D1::RectF(sweepRect.x, sweepRect.y, sweepRect.z, sweepRect.w),
                    &sweepGeometry);

                if(sweepGeometry) {
                    d2dDeviceContext->PushLayer(
                        D2D1::LayerParameters(D2D1::InfiniteRect(), sweepGeometry), nullptr);

                    // 创建增强版的渐变停止点（更亮的扫光效果）
                    std::vector<ColorUtil::GradientStop> brightStops;
                    for(const auto& stop : stops) {
                        int brightnessBoost =
                            static_cast<int>(100 * std::clamp(sweepIntensity, 0.1f, 3.0f));
                        UIColor brightColor =
                            UIColor(std::min(255, stop.color.r + brightnessBoost),
                                    std::min(255, stop.color.g + brightnessBoost),
                                    std::min(255, stop.color.b + brightnessBoost), stop.color.a);
                        brightStops.push_back({stop.position, brightColor});
                    }

                    // 绘制增亮的渐变文字作为扫光效果
                    auto sweepBrush = ColorUtil::createLinearGradientBrush(
                        d2dDeviceContext, brightStops, D2D1::Point2F(textPos.x, textPos.y),
                        D2D1::Point2F(textPos.x + metrics.width, textPos.y));

                    if(sweepBrush) {
                        d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x, textPos.y),
                                                         textLayout, sweepBrush);
                        sweepBrush->Release();
                    }

                    d2dDeviceContext->PopLayer();
                    sweepGeometry->Release();
                }
            }
        }
    }
}
void RenderUtil::fillRectangleWithLinearGradient(const Vec4<float>& rect,
                                                 const std::vector<ColorUtil::GradientStop>& stops,
                                                 const Vec2<float>& startPoint,
                                                 const Vec2<float>& endPoint) {
    D2D_CTX_GUARD();

    auto brush = ColorUtil::createLinearGradientBrush(d2dDeviceContext, stops,
                                                      D2D1::Point2F(startPoint.x, startPoint.y),
                                                      D2D1::Point2F(endPoint.x, endPoint.y));

    if(brush) {
        d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), brush);
        brush->Release();
    }
}

void RenderUtil::fillRectangleWithRadialGradient(const Vec4<float>& rect,
                                                 const std::vector<ColorUtil::GradientStop>& stops,
                                                 const Vec2<float>& center, float radius) {
    D2D_CTX_GUARD();

    auto brush = ColorUtil::createRadialGradientBrush(
        d2dDeviceContext, stops, D2D1::Point2F(center.x, center.y), radius, radius);

    if(brush) {
        d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), brush);
        brush->Release();
    }
}
// 高级光晕效果（带渐变和噪声）
void RenderUtil::drawAdvancedGlowEffect(const Vec2<float>& centerPos, float radius,
                                        const UIColor& innerColor, const UIColor& outerColor,
                                        float glowRadius, float intensity, bool animated) {
    D2D_CTX_GUARD();

    static float time = 0.0f;
    time += deltaTime;

    float animMultiplier = animated ? (sinf(time * 2.5f) * 0.3f + 0.7f) : 1.0f;

    int layers = std::max(5, (int)(glowRadius / 2.0f));

    for(int i = layers; i >= 1; i--) {
        float progress = (float)i / layers;
        float layerRadius = radius + (glowRadius * progress * animMultiplier);

        // 颜色插值
        UIColor layerColor = ColorUtil::lerp(innerColor, outerColor, progress);
        layerColor.a = (int)(layerColor.a * intensity / (i * 0.8f));

        // 添加一些随机噪声（可选）
        if(animated) {
            float noise = sinf(time * 4.0f + i * 1.2f) * 0.1f + 1.0f;
            layerRadius *= noise;
        }

        fillCircle(centerPos, layerColor, layerRadius);
    }
}

// 生成图片文件名
std::string RenderUtil::generateImageFileName(const std::string& url,
                                              const std::string& customName) {
    if(!customName.empty()) {
        return customName + ".png";
    } else {
        std::hash<std::string> hasher;
        size_t hashValue = hasher(url);
        return "img_" + std::to_string(hashValue) + ".png";
    }
}

// 检查本地图片是否存在
bool RenderUtil::isLocalImageExists(const std::string& fileName) {
    std::string imagePath = FileUtil::getClientPath() + "Images\\" + fileName;
    return std::filesystem::exists(imagePath);
}

// 验证缓存的图片文件
bool RenderUtil::isValidCachedImage(const std::string& imagePath) {
    try {
        if(!std::filesystem::exists(imagePath)) {
            return false;
        }

        auto fileSize = std::filesystem::file_size(imagePath);
        if(fileSize < 1000) {
            return false;
        }

        // 读取文件头部进行验证
        std::ifstream file(imagePath, std::ios::binary);
        if(!file)
            return false;

        char header[16];
        file.read(header, 16);
        if(file.gcount() < 8)
            return false;

        // 检查魔数
        if((header[0] == (char)0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G') ||
           (header[0] == (char)0xFF && header[1] == (char)0xD8) ||
           (header[0] == 'G' && header[1] == 'I' && header[2] == 'F') ||
           (std::string(header, 4) == "RIFF" && std::string(header + 8, 4) == "WEBP")) {
            return true;
        }

        return false;
    } catch(...) {
        return false;
    }
}

// 验证下载的图片数据
bool RenderUtil::validateImageData(const std::string& imageData, const std::string& url) {
    // 1. 检查大小
    if(imageData.size() < 1000) {
        return false;
    }

    // 2. 检查是否是HTML错误页面
    if(imageData.size() < 10000) {
        std::string lowerData = imageData;
        std::transform(lowerData.begin(), lowerData.end(), lowerData.begin(), ::tolower);

        if(lowerData.find("<html") != std::string::npos ||
           lowerData.find("<!doctype") != std::string::npos ||
           lowerData.find("<body") != std::string::npos ||
           lowerData.find("404 not found") != std::string::npos ||
           lowerData.find("400 bad request") != std::string::npos ||
           lowerData.find("403 forbidden") != std::string::npos ||
           lowerData.find("the plain http request was sent to https port") != std::string::npos) {
            return false;
        }
    }

    // 3. 验证图片魔数
    if(imageData.size() >= 8) {
        bool validFormat = false;

        // PNG: 89 50 4E 47
        if(imageData[0] == (char)0x89 && imageData[1] == 'P' && imageData[2] == 'N' &&
           imageData[3] == 'G') {
            validFormat = true;
        }
        // JPEG: FF D8
        else if(imageData[0] == (char)0xFF && imageData[1] == (char)0xD8) {
            validFormat = true;
        }
        // GIF: GIF8
        else if(imageData.substr(0, 3) == "GIF") {
            validFormat = true;
        }
        // WebP: RIFF...WEBP
        else if(imageData.size() >= 12 && imageData.substr(0, 4) == "RIFF" &&
                imageData.substr(8, 4) == "WEBP") {
            validFormat = true;
        }

        if(!validFormat) {
            std::string hexOutput = "Header bytes: ";
            for(int i = 0; i < std::min(16, (int)imageData.size()); i++) {
                char hex[4];
                sprintf_s(hex, "%02X ", (unsigned char)imageData[i]);
                hexOutput += hex;
            }

            return false;
        }
    }

    return true;
}

// 保存图片数据
bool RenderUtil::saveImageData(const std::string& imageData, const std::string& imagePath) {
    try {
        std::ofstream file(imagePath, std::ios::binary);
        if(!file) {
            return false;
        }

        file.write(imageData.data(), imageData.size());
        file.close();

        if(!file.good()) {
            std::filesystem::remove(imagePath);
            return false;
        }

        return true;
    } catch(const std::exception& e) {
        return false;
    }
}

// 下载并缓存图片到本地
std::string RenderUtil::downloadAndCacheImageFromUrl(const std::string& url,
                                                     const std::string& customName) {
    // 检查失败次数
    if(failureCount[url] >= 3) {
        return "";
    }

    std::string fileName = generateImageFileName(url, customName);
    std::string imageDir = FileUtil::getClientPath() + "Images\\";
    std::string imagePath = imageDir + fileName;

    // 检查是否已经缓存且有效
    if(isLocalImageExists(fileName) && isValidCachedImage(imagePath)) {
        failedUrls.erase(url);
        failureCount.erase(url);
        return imagePath;
    }

    // 删除无效的缓存文件
    if(std::filesystem::exists(imagePath)) {
        std::filesystem::remove(imagePath);
    }

    // 创建Images目录
    try {
        if(!std::filesystem::exists(imageDir)) {
            std::filesystem::create_directories(imageDir);
        }
    } catch(const std::exception& e) {
        failureCount[url]++;
        return "";
    }

    // 使用NetworkUtil下载
    std::string imageData;
    if(!NetworkUtil::httpGet(url, imageData) || imageData.empty()) {
        failedUrls.insert(url);
        failureCount[url]++;
        return "";
    }

    // 验证下载内容
    if(!validateImageData(imageData, url)) {
        failedUrls.insert(url);
        failureCount[url]++;
        return "";
    }

    // 保存文件
    if(!saveImageData(imageData, imagePath)) {
        failureCount[url]++;
        return "";
    }

    // 验证保存的文件
    if(!isValidCachedImage(imagePath)) {
        std::filesystem::remove(imagePath);
        failureCount[url]++;
        return "";
    }

    imageCacheMap[url] = imagePath;

    // 清除失败记录
    failedUrls.erase(url);
    failureCount.erase(url);

    return imagePath;
}

// 检查URL是否有效
bool RenderUtil::isValidImageUrl(const std::string& url) {
    if(url.empty() || (url.find("http://") != 0 && url.find("https://") != 0)) {
        return false;
    }

    std::string lowerUrl = url;
    std::transform(lowerUrl.begin(), lowerUrl.end(), lowerUrl.begin(), ::tolower);

    return lowerUrl.find(".png") != std::string::npos ||
           lowerUrl.find(".jpg") != std::string::npos ||
           lowerUrl.find(".jpeg") != std::string::npos ||
           lowerUrl.find(".gif") != std::string::npos ||
           lowerUrl.find(".webp") != std::string::npos ||
           lowerUrl.find(".bmp") != std::string::npos;
}

// 绘制占位符
void RenderUtil::drawPlaceholder(const Vec4<float>& rect, const std::string& text) {
    fillRectangle(rect, UIColor(64, 64, 64, 100));
    drawRectangle(rect, UIColor(128, 128, 128, 150), 1.0f);

    float textSize = 1.0f;
    float textWidth = getTextWidth(text, textSize);
    float textHeight = getTextHeight(text, textSize);

    Vec2<float> textPos(rect.x + (rect.z - rect.x - textWidth) / 2.0f,
                        rect.y + (rect.w - rect.y - textHeight) / 2.0f);

    drawText(textPos, text, UIColor(200, 200, 200, 200), textSize);
}

void RenderUtil::drawImageFromUrl(const Vec4<float>& rect, const std::string& url,
                                  const std::string& customName, float opacity) {
    D2D_CTX_GUARD();

    if(url.empty()) {
        return;
    }

    // 添加URL验证
    if(!isValidImageUrl(url)) {
        drawPlaceholder(rect, "Invalid URL");
        return;
    }

    std::string localImagePath;

    // 1. 检查内存缓存
    auto cacheIt = imageCacheMap.find(url);
    if(cacheIt != imageCacheMap.end()) {
        localImagePath = cacheIt->second;
        if(std::filesystem::exists(localImagePath) && isValidCachedImage(localImagePath)) {
            drawImage(rect, localImagePath);
            return;
        } else {
            imageCacheMap.erase(cacheIt);
        }
    }

    // 2. 检查本地文件
    std::string fileName = generateImageFileName(url, customName);
    std::string fullPath = FileUtil::getClientPath() + "Images\\" + fileName;
    if(isValidCachedImage(fullPath)) {
        imageCacheMap[url] = fullPath;
        drawImage(rect, fullPath);
        return;
    }

    // 3. 尝试下载
    static std::set<std::string> downloadingUrls;

    if(failedUrls.find(url) != failedUrls.end()) {
        drawPlaceholder(rect, "Load Failed");
        return;
    }

    if(downloadingUrls.find(url) == downloadingUrls.end()) {
        downloadingUrls.insert(url);

        std::thread([url, customName]() {
            std::string result = downloadAndCacheImageFromUrl(url, customName);
            if(result.empty()) {
                failedUrls.insert(url);
            }
            downloadingUrls.erase(url);
        }).detach();
    }

    drawPlaceholder(rect, "Loading...");
}

// 清理损坏的图片缓存
void RenderUtil::cleanCorruptedImageCache() {
    std::string imageDir = FileUtil::getClientPath() + "Images\\";
    if(!std::filesystem::exists(imageDir))
        return;

    int cleanedCount = 0;
    try {
        for(const auto& entry : std::filesystem::directory_iterator(imageDir)) {
            if(entry.is_regular_file()) {
                std::string filePath = entry.path().string();

                if(!isValidCachedImage(filePath)) {
                    std::filesystem::remove(entry.path());

                    cleanedCount++;
                }
            }
        }

        if(cleanedCount > 0) {
        }
    } catch(...) {
        // 忽略错误
    }
}

// 清理缓存
void RenderUtil::clearImageCache() {
    imageCacheMap.clear();
    failedUrls.clear();
    failureCount.clear();
}

// 获取缓存统计信息
std::string RenderUtil::getImageCacheStats() {
    size_t cacheSize = imageCacheMap.size();
    std::string imageDir = FileUtil::getClientPath() + "Images\\";
    size_t fileCount = 0;

    try {
        if(std::filesystem::exists(imageDir)) {
            for(const auto& entry : std::filesystem::directory_iterator(imageDir)) {
                if(entry.is_regular_file()) {
                    fileCount++;
                }
            }
        }
    } catch(...) {
        // 忽略错误
    }

    return "Memory Cache: " + std::to_string(cacheSize) +
           " | Local Files: " + std::to_string(fileCount) +
           " | Failed URLs: " + std::to_string(failedUrls.size());
}

// 获取失败统计信息
std::string RenderUtil::getFailureStats() {
    return "Failed URLs: " + std::to_string(failedUrls.size()) +
           " | Failure records: " + std::to_string(failureCount.size());
}

// 清理失败记录
void RenderUtil::clearFailureRecords() {
    failedUrls.clear();
    failureCount.clear();
}

void RenderUtil::drawImageFromCache(const Vec4<float>& rect, const std::string& imageName,
                                    float opacity) {
    D2D_CTX_GUARD();

    // 尝试从文件名直接加载
    std::string fullPath = FileUtil::getClientPath() + "Images\\" + imageName;

    // 如果没有扩展名，尝试常见的图片格式
    if(fullPath.find('.') == std::string::npos) {
        std::vector<std::string> extensions = {".png", ".jpg", ".jpeg", ".gif", ".webp"};
        bool found = false;
        for(const auto& ext : extensions) {
            std::string testPath = fullPath + ext;
            if(std::filesystem::exists(testPath)) {
                fullPath = testPath;
                found = true;
                break;
            }
        }
        if(!found) {
            drawPlaceholder(rect, "Not Found");
            return;
        }
    }

    if(std::filesystem::exists(fullPath) && isValidCachedImage(fullPath)) {
        drawImage(rect, fullPath);
    } else {
        drawPlaceholder(rect, "Invalid Cache");
    }
}

// 添加新的阴影函数
void RenderUtil::addShadow(const Vec4<float>& rect, float strength, const UIColor& shadowColor,
                           float rounding, const std::vector<Vec4<float>>& excludeRects) {
    D2D_CTX_GUARD();

    if(!d2dDeviceContext)
        return;

    Vec2<float> windowSize = getWindowSize();

    // 创建阴影位图（如果还没有）
    if(shadowBitmap == nullptr) {
        D2D1_BITMAP_PROPERTIES1 newLayerProps =
            D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET, sourceBitmap->GetPixelFormat());
        d2dDeviceContext->CreateBitmap(sourceBitmap->GetPixelSize(), nullptr, 0, newLayerProps,
                                       &shadowBitmap);
    }

    // 在阴影位图上绘制形状
    d2dDeviceContext->SetTarget(shadowBitmap);
    d2dDeviceContext->Clear(D2D1::ColorF(0, 0, 0, 0));

    // 创建颜色画刷
    ID2D1SolidColorBrush* colorBrush =
        getSolidColorBrush(UIColor(shadowColor.r, shadowColor.g, shadowColor.b, 255));

    // 绘制圆角矩形
    D2D1_ROUNDED_RECT roundedRect =
        D2D1::RoundedRect(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), rounding, rounding);
    d2dDeviceContext->FillRoundedRectangle(roundedRect, colorBrush);

    // 应用高斯模糊
    blurEffect->SetInput(0, shadowBitmap);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, strength);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION,
                         D2D1_GAUSSIANBLUR_OPTIMIZATION_QUALITY);

    // 如果有排除区域，创建遮罩
    ID2D1Bitmap1* holeMaskBitmap = nullptr;
    if(!excludeRects.empty()) {
        D2D1_BITMAP_PROPERTIES1 maskProps =
            D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET, sourceBitmap->GetPixelFormat());
        d2dDeviceContext->CreateBitmap(sourceBitmap->GetPixelSize(), nullptr, 0, maskProps,
                                       &holeMaskBitmap);

        d2dDeviceContext->SetTarget(holeMaskBitmap);
        d2dDeviceContext->Clear(D2D1::ColorF(1, 1, 1, 1));  // 白色背景

        // 创建透明画刷来"挖洞"
        ID2D1SolidColorBrush* transparentBrush = nullptr;
        d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f),
                                                &transparentBrush);

        // 保存原始混合模式
        D2D1_PRIMITIVE_BLEND originalBlend = d2dDeviceContext->GetPrimitiveBlend();
        d2dDeviceContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);

        // 在排除区域绘制透明矩形
        for(const auto& excludeRect : excludeRects) {
            D2D1_RECT_F rectF =
                D2D1::RectF(excludeRect.x, excludeRect.y, excludeRect.z, excludeRect.w);
            ID2D1RectangleGeometry* rectGeo = nullptr;
            d2dFactory->CreateRectangleGeometry(rectF, &rectGeo);
            d2dDeviceContext->FillGeometry(rectGeo, transparentBrush);
            rectGeo->Release();
        }

        // 恢复混合模式
        d2dDeviceContext->SetPrimitiveBlend(originalBlend);
        transparentBrush->Release();
    }

    // 切换回源位图
    d2dDeviceContext->SetTarget(sourceBitmap);

    // 获取模糊输出
    ID2D1Image* blurOutput = nullptr;
    blurEffect->GetOutput(&blurOutput);

    ID2D1Image* finalOutput = blurOutput;

    // 如果有遮罩，应用 Alpha 遮罩效果
    if(holeMaskBitmap) {
        stencilEffect->SetInput(0, blurOutput);
        stencilEffect->SetInput(1, holeMaskBitmap);
        stencilEffect->GetOutput(&finalOutput);
    }

    // 创建图像画刷
    ID2D1ImageBrush* imageBrush = nullptr;
    D2D1_IMAGE_BRUSH_PROPERTIES brushProps =
        D2D1::ImageBrushProperties(D2D1::RectF(0, 0, windowSize.x, windowSize.y));
    d2dDeviceContext->CreateImageBrush(finalOutput, brushProps, &imageBrush);

    // 设置画刷透明度
    imageBrush->SetOpacity(shadowColor.a / 255.0f);

    // 绘制到整个屏幕
    ID2D1RectangleGeometry* screenGeo = nullptr;
    d2dFactory->CreateRectangleGeometry(D2D1::RectF(0, 0, windowSize.x, windowSize.y), &screenGeo);
    d2dDeviceContext->FillGeometry(screenGeo, imageBrush);

    // 清理资源
    screenGeo->Release();
    imageBrush->Release();
    blurOutput->Release();

    if(holeMaskBitmap) {
        if(finalOutput != blurOutput) {
            finalOutput->Release();
        }
        holeMaskBitmap->Release();
    }
}

void RenderUtil::addDropShadow(const Vec4<float>& rect, float blurRadius,
                               const UIColor& shadowColor, const Vec2<float>& offset,
                               float rounding) {
    D2D_CTX_GUARD();

    // 计算阴影矩形（加上偏移）
    Vec4<float> shadowRect =
        Vec4<float>(rect.x + offset.x, rect.y + offset.y, rect.z + offset.x, rect.w + offset.y);

    // 创建排除原始矩形的列表（避免阴影覆盖原始内容）
    std::vector<Vec4<float>> excludeRects = {rect};

    addShadow(shadowRect, blurRadius, shadowColor, rounding, excludeRects);
}
