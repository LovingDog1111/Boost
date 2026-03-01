#include "D2D.h"

#include <winrt/base.h>

#include <codecvt>
#include <locale>
#include <unordered_map>

#include "../Client/Client.h"
#include "../Libs/Fonts/Arial.h"
#include "../Libs/Fonts/BahnSchrift.h"
#include "../Libs/Fonts/Mojangles.h"
#include "../Libs/Fonts/NotoSans.h"
#include "../Libs/Fonts/ProductSansRegular.h"
#include "../Libs/Fonts/Verdana.h"
#include "../SDK/GlobalInstance.h"
#include "../Utils/Logger.h"
#include "../Utils/TimerUtil.h"
#include "../SDK/Render/Matrix.h"

float D2D::deltaTime = 0.016f;
Vec2<float> D2D::mpos = Vec2<float>(0.f, 0.f);
// d2d stuff
static ID2D1Factory3* d2dFactory = nullptr;
static IDWriteFactory* d2dWriteFactory = nullptr;
static ID2D1Device2* d2dDevice = nullptr;
static ID2D1DeviceContext2* d2dDeviceContext = nullptr;
static ID2D1Bitmap1* sourceBitmap = nullptr;
static ID2D1Effect* blurEffect = nullptr;
ID2D1BitmapRenderTarget* glowRenderTarget = nullptr;
ID2D1SolidColorBrush* glowBrush = nullptr;
ID2D1Effect* blurEffect2 = nullptr;
ID2D1Effect* transformEffect = nullptr;

// cache
static std::unordered_map<float, winrt::com_ptr<IDWriteTextFormat>> textFormatCache;
static std::unordered_map<uint64_t, winrt::com_ptr<IDWriteTextLayout>> textLayoutCache;
static std::unordered_map<uint32_t, winrt::com_ptr<ID2D1SolidColorBrush>> colorBrushCache;

// temporary cache
static std::unordered_map<uint64_t, winrt::com_ptr<IDWriteTextLayout>> textLayoutTemporary;

static int currentD2DFontSize = 25;
static std::string currentD2DFont = "Arial";
static bool isFontItalic = false;
static bool compactMojanglesSpace = false;

static bool initD2D = false;
static winrt::com_ptr<IDWriteFactory5> d2dWriteFactory5;
static winrt::com_ptr<IDWriteInMemoryFontFileLoader> d2dInMemoryFontLoader;
static winrt::com_ptr<IDWriteFontCollection1> d2dEmbeddedFontCollection;
static bool d2dInMemoryLoaderRegistered = false;

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
std::wstring resolveSelectedFontFamily(const std::string& selectedFont);
HRESULT ensureEmbeddedFontCollection();
bool embeddedCollectionHasFamily(const std::wstring& familyName);
void applyMojanglesSpaceCompaction(IDWriteTextLayout* textLayout, const std::wstring& text,
                                   float textSize);

Vec2<float> D2D::WorldToScreen(const Vec3<float>& worldPos, const Vec2<float>& screenSize) {
    Vec2<float> screenCoords;

    if(Matrix::WorldToScreen(worldPos, screenCoords)) {
        return screenCoords;
    }

    return Vec2<float>(-1, -1);  // 转换失败
}

void D2D::NewFrame(IDXGISwapChain3* swapChain, ID3D11Device* d3d11Device, float fxdpi) {
    if(!initD2D) {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &d2dFactory);

        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(d2dWriteFactory),
                            reinterpret_cast<IUnknown**>(&d2dWriteFactory));
        ensureEmbeddedFontCollection();
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

#include "../Client/Managers/ModuleManager/ModuleManager.h"

void D2D::EndFrame() {
    if(!initD2D)
        return;

    d2dDeviceContext->EndDraw();

    static CustomFont* customFontMod = ModuleManager::getModule<CustomFont>();
    if((currentD2DFont != customFontMod->getSelectedFont()) ||
       (currentD2DFontSize != customFontMod->fontSize) || (isFontItalic != customFontMod->italic) ||
       (compactMojanglesSpace != customFontMod->compactMojanglesSpace)) {
        currentD2DFont = customFontMod->getSelectedFont();
        currentD2DFontSize = customFontMod->fontSize;
        isFontItalic = customFontMod->italic;
        compactMojanglesSpace = customFontMod->compactMojanglesSpace;
        textFormatCache.clear();
        textLayoutCache.clear();
        // textLayoutTemporary.clear();
    }

    static float timeCounter = 0.0f;
    timeCounter += D2D::deltaTime;
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

void D2D::Render() {
    ModuleManager::onD2DRender();
    NotificationManager::Render();

    {  // Yuh
        static Colors* colorMod = ModuleManager::getModule<Colors>();
        for(Module* mod : ModuleManager::moduleList) {
            for(Setting* setting : mod->getSettingList()) {
                if(setting->type != SettingType::COLOR_S)
                    continue;
                ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
                if(colorSetting->colorSynced && colorSetting->showSynced) {
                    colorSetting->colorPtr->r = colorMod->getColor().r;
                    colorSetting->colorPtr->g = colorMod->getColor().g;
                    colorSetting->colorPtr->b = colorMod->getColor().b;
                }
            }
        }
    }
    // ClickGUI
    {
        static ClickGUI* clickGuiMod = ModuleManager::getModule<ClickGUI>();
        if(clickGuiMod->isEnabled())
            clickGuiMod->Render();
    }
    // Eject
    {
        Vec2<float> windowSize = D2D::getWindowSize();
        static float holdTime = 0.f;
        static float holdAnim = 0.f;
        static float showDuration = 0.f;
        static float exitDuration = 0.f;
        static float exitVelocity = 0.f;

        if(showDuration > 0.1f) {
            static std::string text = "Hold Ctrl and L to eject";
            float textSize = 1.f * showDuration;
            float textPaddingX = 4.f * textSize;
            float textPaddingY = 1.f * textSize;
            float textWidth = getTextWidth(text, textSize);
            float textHeight = getTextHeight(text, textSize);

            Vec2<float> textPos =
                Vec2<float>((windowSize.x - textWidth) / 2.f, -30.f + (100.f * showDuration));
            Vec4<float> rectPos = Vec4<float>(textPos.x - textPaddingX, textPos.y - textPaddingY,
                                              textPos.x + textWidth + textPaddingX,
                                              textPos.y + textHeight + textPaddingY);
            Vec4<float> underlineRect =
                Vec4<float>(rectPos.x, rectPos.w, rectPos.z, rectPos.w + 2.f * textSize);
            Vec4<float> underlineAnim = Vec4<float>(
                underlineRect.x, underlineRect.y,
                underlineRect.x + (underlineRect.z - underlineRect.x) * holdAnim, underlineRect.w);

            fillRectangle(rectPos, UIColor(0, 0, 0, 125));
            fillRectangle(underlineRect, UIColor(0, 0, 0, 165));
            fillRectangle(underlineAnim, UIColor(255, 255, 255));
            drawText(textPos, text, UIColor(255, 255, 255), textSize);
        }

        if(GI::isKeyDown(VK_CONTROL) && GI::isKeyDown('L')) {
            holdTime += D2D::deltaTime / 2.f;
            if(holdTime > 1.f)
                holdTime = 1.f;
            exitDuration = 1.5f;
        } else {
            holdTime = 0.f;
            exitDuration -= D2D::deltaTime;
        }

        holdAnim += (holdTime - holdAnim) * (D2D::deltaTime * 10.f);
        if(holdAnim > 1.f)
            holdAnim = 1.f;
        if(holdAnim < 0.f)
            holdAnim = 0.f;

        if(exitDuration > 0.f) {
            showDuration += (1.f - showDuration) * (D2D::deltaTime * 8.f);
            exitVelocity = 0.f;
        } else {
            showDuration -= exitVelocity;
            exitVelocity += D2D::deltaTime / 4.f;
        }

        if(showDuration < 0.f)
            showDuration = 0.f;
        if(showDuration > 1.f)
            showDuration = 1.f;

        if(holdAnim > 0.99f)
            Client::shutdown();
    }
}

void D2D::Flush() {
    d2dDeviceContext->Flush();
}

Vec2<float> D2D::getWindowSize() {
    D2D1_SIZE_U size = sourceBitmap->GetPixelSize();
    return Vec2<float>((float)size.width, (float)size.height);
}

void D2D::drawText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
                   float textSize, bool storeTextLayout) {
    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);

    static CustomFont* customFontMod = ModuleManager::getModule<CustomFont>();
    D2D1_TEXT_ANTIALIAS_MODE textAntialiasMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
    switch(customFontMod->textAntialiasMode) {
        case 1:
            textAntialiasMode = D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE;
            break;
        case 2:
            textAntialiasMode = D2D1_TEXT_ANTIALIAS_MODE_ALIASED;
            break;
        default:
            textAntialiasMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
            break;
    }
    d2dDeviceContext->SetTextAntialiasMode(textAntialiasMode);
    if(customFontMod->shadow) {
        const float shadowOffset = 2.f;
        UIColor shadowColor(static_cast<uint8_t>(color.r / 5), static_cast<uint8_t>(color.g / 5),
                            static_cast<uint8_t>(color.b / 5), color.a);
        ID2D1SolidColorBrush* shadowColorBrush = getSolidColorBrush(shadowColor);
        d2dDeviceContext->DrawTextLayout(
            D2D1::Point2F(textPos.x + shadowOffset, textPos.y + shadowOffset), textLayout,
            shadowColorBrush);
    }

    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x, textPos.y), textLayout, colorBrush);
}

float D2D::getTextWidth(const std::string& textStr, float textSize, bool storeTextLayout) {
    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);
    DWRITE_TEXT_METRICS textMetrics;
    textLayout->GetMetrics(&textMetrics);

    return textMetrics.widthIncludingTrailingWhitespace;
}

float D2D::getTextHeight(const std::string& textStr, float textSize, bool storeTextLayout) {
    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);
    DWRITE_TEXT_METRICS textMetrics;
    textLayout->GetMetrics(&textMetrics);

    return std::ceilf(textMetrics.height);
}

void D2D::drawLine(const Vec2<float>& startPos, const Vec2<float>& endPos, const UIColor& color,
                   float width) {
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawLine(D2D1::Point2F(startPos.x, startPos.y),
                               D2D1::Point2F(endPos.x, endPos.y), colorBrush, width);
}

void D2D::drawRectangle(const Vec4<float>& rect, const UIColor& color, float width) {
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush, width);
}

void D2D::fillRectangle(const Vec4<float>& rect, const UIColor& color) {
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush);
}

void D2D::drawCircle(const Vec2<float>& centerPos, const UIColor& color, float radius,
                     float width) {
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawEllipse(
        D2D1::Ellipse(D2D1::Point2F(centerPos.x, centerPos.y), radius, radius), colorBrush, width);
}

void D2D::fillCircle(const Vec2<float>& centerPos, const UIColor& color, float radius) {
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->FillEllipse(
        D2D1::Ellipse(D2D1::Point2F(centerPos.x, centerPos.y), radius, radius), colorBrush);
}

void D2D::addBlur(const Vec4<float>& rect, float strength, bool flush) {
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

#include <Windows.h>

std::wstring to_wide(const std::string& str) {
    if(str.empty())
        return std::wstring();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);

    return wstr;
}

uint64_t getTextLayoutKey(const std::string& textStr, float textSize) {
    std::hash<std::string> textHash;
    std::hash<float> textSizeHash;
    uint64_t combinedHash = textHash(textStr) ^ textSizeHash(textSize);
    return combinedHash;
}
struct EmbeddedFontData {
    const void* data;
    UINT32 size;
};

static const EmbeddedFontData gEmbeddedFonts[] = {
    {Arial, static_cast<UINT32>(sizeof(Arial))},
    {BahnSchrift, static_cast<UINT32>(sizeof(BahnSchrift))},
    {Mojangles, static_cast<UINT32>(sizeof(Mojangles))},
    {NotoSans, static_cast<UINT32>(sizeof(NotoSans))},
    {ProductSansRegular, static_cast<UINT32>(sizeof(ProductSansRegular))},
    {Verdana, static_cast<UINT32>(sizeof(Verdana))}};

HRESULT ensureEmbeddedFontCollection() {
    if(d2dEmbeddedFontCollection)
        return S_OK;

    if(!d2dWriteFactory)
        return E_POINTER;

    HRESULT hr = d2dWriteFactory->QueryInterface(
        __uuidof(IDWriteFactory5), reinterpret_cast<void**>(d2dWriteFactory5.put_void()));
    if(FAILED(hr))
        return hr;

    hr = d2dWriteFactory5->CreateInMemoryFontFileLoader(d2dInMemoryFontLoader.put());
    if(FAILED(hr))
        return hr;

    hr = d2dWriteFactory->RegisterFontFileLoader(d2dInMemoryFontLoader.get());
    if(SUCCEEDED(hr)) {
        d2dInMemoryLoaderRegistered = true;
    } else if(hr != DWRITE_E_ALREADYREGISTERED) {
        return hr;
    }

    winrt::com_ptr<IDWriteFontSetBuilder1> fontSetBuilder;
    hr = d2dWriteFactory5->CreateFontSetBuilder(fontSetBuilder.put());
    if(FAILED(hr))
        return hr;

    bool hasAtLeastOneFont = false;
    for(const EmbeddedFontData& font : gEmbeddedFonts) {
        winrt::com_ptr<IDWriteFontFile> fontFile;
        hr = d2dInMemoryFontLoader->CreateInMemoryFontFileReference(
            d2dWriteFactory, font.data, font.size, nullptr, fontFile.put());
        if(FAILED(hr))
            continue;

        hr = fontSetBuilder->AddFontFile(fontFile.get());
        if(SUCCEEDED(hr))
            hasAtLeastOneFont = true;
    }

    if(!hasAtLeastOneFont)
        return E_FAIL;

    winrt::com_ptr<IDWriteFontSet> fontSet;
    hr = fontSetBuilder->CreateFontSet(fontSet.put());
    if(FAILED(hr))
        return hr;

    return d2dWriteFactory5->CreateFontCollectionFromFontSet(fontSet.get(),
                                                             d2dEmbeddedFontCollection.put());
}

std::wstring resolveSelectedFontFamily(const std::string& selectedFont) {
    if(selectedFont == "Bahnschrift")
        return L"Bahnschrift";
    if(selectedFont == "Mojangles" || selectedFont == "Mojang")
        return L"Mojangles";
    if(selectedFont == "Product Sans")
        return L"Product Sans";
    if(selectedFont == "Noto Sans")
        return L"Noto Sans";
    if(selectedFont == "Verdana")
        return L"Verdana";
    if(selectedFont == "Arial")
        return L"Arial";

    return L"Arial";
}

bool embeddedCollectionHasFamily(const std::wstring& familyName) {
    if(!d2dEmbeddedFontCollection)
        return false;

    UINT32 familyIndex = 0;
    BOOL familyExists = FALSE;
    d2dEmbeddedFontCollection->FindFamilyName(familyName.c_str(), &familyIndex, &familyExists);
    return familyExists == TRUE;
}

IDWriteTextFormat* getTextFormat(float textSize) {
    if(textFormatCache[textSize].get() == nullptr) {
        ensureEmbeddedFontCollection();
        std::wstring fontNameWide = resolveSelectedFontFamily(currentD2DFont);
        IDWriteFontCollection* fontCollection = nullptr;
        if(embeddedCollectionHasFamily(fontNameWide)) {
            fontCollection = d2dEmbeddedFontCollection.get();
        } else if(embeddedCollectionHasFamily(L"Arial")) {
            fontNameWide = L"Arial";
            fontCollection = d2dEmbeddedFontCollection.get();
        }

        HRESULT hr = d2dWriteFactory->CreateTextFormat(
            fontNameWide.c_str(), fontCollection, DWRITE_FONT_WEIGHT_NORMAL,
            isFontItalic ? DWRITE_FONT_STYLE_ITALIC
                         : DWRITE_FONT_STYLE_NORMAL,  // DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, (float)currentD2DFontSize * textSize,
            L"en-us",  // locale
            textFormatCache[textSize].put());
        if(FAILED(hr)) {
            d2dWriteFactory->CreateTextFormat(
                L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
                isFontItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL, (float)currentD2DFontSize * textSize, L"en-us",
                textFormatCache[textSize].put());
        }
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
            applyMojanglesSpaceCompaction(textLayoutCache[textLayoutKey].get(), wideText, textSize);
        }
        return textLayoutCache[textLayoutKey].get();
    } else {
        if(textLayoutTemporary[textLayoutKey].get() == nullptr) {
            d2dWriteFactory->CreateTextLayout(text, (UINT32)wcslen(text), textFormat, FLT_MAX, 0.f,
                                              textLayoutTemporary[textLayoutKey].put());
            applyMojanglesSpaceCompaction(textLayoutTemporary[textLayoutKey].get(), wideText,
                                          textSize);
        }
        return textLayoutTemporary[textLayoutKey].get();
    }
}

void applyMojanglesSpaceCompaction(IDWriteTextLayout* textLayout, const std::wstring& text,
                                   float textSize) {
    if(textLayout == nullptr)
        return;

    if(!compactMojanglesSpace)
        return;

    if(currentD2DFont != "Mojangles" && currentD2DFont != "Mojang")
        return;

    winrt::com_ptr<IDWriteTextLayout1> textLayout1;
    if(FAILED(textLayout->QueryInterface(__uuidof(IDWriteTextLayout1),
                                         reinterpret_cast<void**>(textLayout1.put_void()))))
        return;

    const float spaceTightenAmount = -((float)currentD2DFontSize * textSize * 0.12f);
    for(UINT32 i = 0; i < static_cast<UINT32>(text.size()); i++) {
        if(text[i] != L' ')
            continue;

        DWRITE_TEXT_RANGE range = {i, 1};
        textLayout1->SetCharacterSpacing(0.f, spaceTightenAmount, 0.f, range);
    }
}

ID2D1SolidColorBrush* getSolidColorBrush(const UIColor& color) {
    uint32_t colorBrushKey = ColorUtil::ColorToUInt(color);
    if(colorBrushCache[colorBrushKey].get() == nullptr) {
        d2dDeviceContext->CreateSolidColorBrush(color.toD2D1Color(),
                                                colorBrushCache[colorBrushKey].put());
    }
    return colorBrushCache[colorBrushKey].get();
}

void D2D::drawTriangle(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3,
                       const UIColor& color, float width) {
    ID2D1SolidColorBrush* brush = getSolidColorBrush(color);

    D2D1_POINT_2F points[4] = {D2D1::Point2F(p1.x, p1.y), D2D1::Point2F(p2.x, p2.y),
                               D2D1::Point2F(p3.x, p3.y), D2D1::Point2F(p1.x, p1.y)};

    d2dDeviceContext->DrawGeometry(nullptr, brush, width);

    ID2D1PathGeometry* triangleGeometry = nullptr;
    d2dFactory->CreatePathGeometry(&triangleGeometry);

    ID2D1GeometrySink* sink = nullptr;
    triangleGeometry->Open(&sink);

    sink->BeginFigure(points[0], D2D1_FIGURE_BEGIN_HOLLOW);
    sink->AddLines(points + 1, 3);
    sink->EndFigure(D2D1_FIGURE_END_OPEN);
    sink->Close();
    sink->Release();

    d2dDeviceContext->DrawGeometry(triangleGeometry, brush, width);

    triangleGeometry->Release();
}

void D2D::fillTriangle(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3,
                       const UIColor& color) {
    ID2D1SolidColorBrush* brush = getSolidColorBrush(color);

    D2D1_POINT_2F points[3] = {D2D1::Point2F(p1.x, p1.y), D2D1::Point2F(p2.x, p2.y),
                               D2D1::Point2F(p3.x, p3.y)};

    ID2D1PathGeometry* triangleGeometry = nullptr;
    d2dFactory->CreatePathGeometry(&triangleGeometry);

    ID2D1GeometrySink* sink = nullptr;
    triangleGeometry->Open(&sink);

    sink->BeginFigure(points[0], D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLines(points + 1, 2);
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();
    sink->Release();

    d2dDeviceContext->FillGeometry(triangleGeometry, brush);

    triangleGeometry->Release();
}

void D2D::drawDropDownTriangle(const Vec2<float>& center, float size, float rotationDegrees,
                               const UIColor& color) {
    float halfSize = size / 2.f;

    std::vector<Vec2<float>> points = {
        {-halfSize, -halfSize}, {halfSize, 0}, {-halfSize, halfSize}};

    float rad = rotationDegrees * (3.14159265f / 180.f);
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);

    for(auto& p : points) {
        float x = p.x;
        float y = p.y;
        p.x = x * cosA - y * sinA + center.x;
        p.y = x * sinA + y * cosA + center.y;
    }

    fillTriangle(points[0], points[1], points[2], color);
}

void D2D::drawWrench(const Vec2<float>& center, float size, const UIColor& color, float width) {
    ID2D1SolidColorBrush* brush = getSolidColorBrush(color);

    ID2D1PathGeometry* wrenchGeometry = nullptr;
    d2dFactory->CreatePathGeometry(&wrenchGeometry);

    ID2D1GeometrySink* sink = nullptr;
    wrenchGeometry->Open(&sink);

    float halfSize = size / 2.f;

    Vec2<float> h1 = {center.x - halfSize * 0.3f, center.y + halfSize};
    Vec2<float> h2 = {center.x + halfSize * 0.3f, center.y + halfSize};
    Vec2<float> h3 = {center.x + halfSize * 0.3f, center.y - halfSize * 0.2f};
    Vec2<float> h4 = {center.x - halfSize * 0.3f, center.y - halfSize * 0.2f};

    sink->BeginFigure(D2D1::Point2F(h1.x, h1.y), D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLine(D2D1::Point2F(h2.x, h2.y));
    sink->AddLine(D2D1::Point2F(h3.x, h3.y));
    sink->AddLine(D2D1::Point2F(h4.x, h4.y));
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);

    Vec2<float> o1 = {center.x - halfSize * 0.7f, center.y - halfSize * 0.2f};
    Vec2<float> o2 = {center.x + halfSize * 0.5f, center.y - halfSize * 0.8f};
    Vec2<float> o3 = {center.x + halfSize * 0.8f, center.y - halfSize * 0.4f};
    Vec2<float> o4 = {center.x - halfSize * 0.5f, center.y + halfSize * 0.2f};

    sink->BeginFigure(D2D1::Point2F(o1.x, o1.y), D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLine(D2D1::Point2F(o2.x, o2.y));
    sink->AddLine(D2D1::Point2F(o3.x, o3.y));
    sink->AddLine(D2D1::Point2F(o4.x, o4.y));
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);

    sink->Close();
    sink->Release();

    d2dDeviceContext->DrawGeometry(wrenchGeometry, brush, width);

    wrenchGeometry->Release();
}

void D2D::drawGear(const Vec2<float>& center, float radius, int teethCount, float toothDepth,
                   const UIColor& color, float width) {
    ID2D1SolidColorBrush* brush = getSolidColorBrush(color);

    if(teethCount < 3)
        teethCount = 3;

    ID2D1PathGeometry* gearGeometry = nullptr;
    d2dFactory->CreatePathGeometry(&gearGeometry);

    ID2D1GeometrySink* sink = nullptr;
    gearGeometry->Open(&sink);

    float angleStep = (2.f * 3.14159265f) / (float)(teethCount * 2);
    bool isTooth = true;

    for(int i = 0; i < teethCount * 2; ++i) {
        float angle = i * angleStep;
        float r = isTooth ? radius + toothDepth : radius;

        float x = center.x + r * cosf(angle);
        float y = center.y + r * sinf(angle);

        if(i == 0) {
            sink->BeginFigure(D2D1::Point2F(x, y), D2D1_FIGURE_BEGIN_FILLED);
        } else {
            sink->AddLine(D2D1::Point2F(x, y));
        }
        isTooth = !isTooth;
    }

    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();
    sink->Release();

    d2dDeviceContext->DrawGeometry(gearGeometry, brush, width);

    gearGeometry->Release();
}

#include <d2d1_1.h>

void D2D::drawVerticalGradient(const Vec4<float>& rect, const UIColor& startColor,
                               const UIColor& endColor) {
    D2D1_GRADIENT_STOP gradientStops[2];
    gradientStops[0].color = startColor.toD2D1Color();
    gradientStops[0].position = 0.0f;
    gradientStops[1].color = endColor.toD2D1Color();
    gradientStops[1].position = 1.0f;

    ID2D1GradientStopCollection* pGradientStops = nullptr;

    HRESULT hr = d2dDeviceContext->CreateGradientStopCollection(
        gradientStops, 2, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &pGradientStops);
    if(FAILED(hr) || !pGradientStops)
        return;

    ID2D1LinearGradientBrush* pLinearGradientBrush = nullptr;
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES brushProperties = {};
    brushProperties.startPoint = D2D1::Point2F(rect.x, rect.y);
    brushProperties.endPoint = D2D1::Point2F(rect.x, rect.w);

    hr = d2dDeviceContext->CreateLinearGradientBrush(brushProperties, pGradientStops,
                                                     &pLinearGradientBrush);
    if(FAILED(hr) || !pLinearGradientBrush) {
        pGradientStops->Release();
        return;
    }

    d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w),
                                    pLinearGradientBrush);

    pLinearGradientBrush->Release();
    pGradientStops->Release();
}

void D2D::drawBlurOutline(const Vec4<float>& rect, float blurStrength, float thickness) {
    // Make a snapshot of current render target for blurring
    ID2D1Bitmap* snapshot = nullptr;
    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(sourceBitmap->GetPixelFormat());
    d2dDeviceContext->CreateBitmap(sourceBitmap->GetPixelSize(), props, &snapshot);
    snapshot->CopyFromBitmap(nullptr, sourceBitmap, nullptr);

    // Set the input and blur strength
    blurEffect->SetInput(0, snapshot);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, blurStrength);

    ID2D1Image* blurredImage = nullptr;
    blurEffect->GetOutput(&blurredImage);

    // Create image brush from blurred image
    ID2D1ImageBrush* brush = nullptr;
    D2D1_RECT_F imageRect = D2D1::RectF(0.f, 0.f, (float)sourceBitmap->GetPixelSize().width,
                                        (float)sourceBitmap->GetPixelSize().height);
    d2dDeviceContext->CreateImageBrush(blurredImage, D2D1::ImageBrushProperties(imageRect), &brush);

    // Create path geometry for hollow ring (outline)
    ID2D1PathGeometry* path = nullptr;
    d2dFactory->CreatePathGeometry(&path);

    ID2D1GeometrySink* sink = nullptr;
    path->Open(&sink);

    // Outer rectangle (clockwise)
    sink->BeginFigure(D2D1::Point2F(rect.x - thickness, rect.y - thickness),
                      D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLine(D2D1::Point2F(rect.z + thickness, rect.y - thickness));
    sink->AddLine(D2D1::Point2F(rect.z + thickness, rect.w + thickness));
    sink->AddLine(D2D1::Point2F(rect.x - thickness, rect.w + thickness));
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);

    // Inner rectangle (counter-clockwise) — makes a hollow ring
    sink->BeginFigure(D2D1::Point2F(rect.x + thickness, rect.y + thickness),
                      D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLine(D2D1::Point2F(rect.x + thickness, rect.w - thickness));
    sink->AddLine(D2D1::Point2F(rect.z - thickness, rect.w - thickness));
    sink->AddLine(D2D1::Point2F(rect.z - thickness, rect.y + thickness));
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);

    sink->Close();
    sink->Release();

    // Fill the hollow ring with the blurred brush
    d2dDeviceContext->FillGeometry(path, brush);

    // Cleanup
    path->Release();
    brush->Release();
    blurredImage->Release();
    snapshot->Release();
}

void D2D::drawGlossBlurEffect(const Vec4<float>& rect, float blurStrength,
                              const UIColor& topLeftColor, const UIColor& topRightColor,
                              const UIColor& bottomLeftColor, const UIColor& bottomRightColor) {
    ID2D1Bitmap* snapshot = nullptr;
    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(sourceBitmap->GetPixelFormat());
    d2dDeviceContext->CreateBitmap(sourceBitmap->GetPixelSize(), props, &snapshot);
    snapshot->CopyFromBitmap(nullptr, sourceBitmap, nullptr);

    blurEffect->SetInput(0, snapshot);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, blurStrength);

    ID2D1Image* blurredImage = nullptr;
    blurEffect->GetOutput(&blurredImage);

    ID2D1PathGeometry* glossMaskGeometry = nullptr;
    d2dFactory->CreatePathGeometry(&glossMaskGeometry);
    ID2D1GeometrySink* sink = nullptr;
    glossMaskGeometry->Open(&sink);
    sink->BeginFigure(D2D1::Point2F(rect.x, rect.y), D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLine(D2D1::Point2F(rect.z, rect.y));
    sink->AddLine(D2D1::Point2F(rect.z, rect.w));
    sink->AddLine(D2D1::Point2F(rect.x, rect.w));
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();
    sink->Release();

    D2D1_GRADIENT_STOP stops[] = {{0.0f, topLeftColor.toD2D1Color()},
                                  {0.33f, topRightColor.toD2D1Color()},
                                  {0.66f, bottomLeftColor.toD2D1Color()},
                                  {1.0f, bottomRightColor.toD2D1Color()}};
    ID2D1GradientStopCollection* stopCollection = nullptr;
    d2dDeviceContext->CreateGradientStopCollection(stops, ARRAYSIZE(stops), D2D1_GAMMA_2_2,
                                                   D2D1_EXTEND_MODE_CLAMP, &stopCollection);

    ID2D1LinearGradientBrush* gradientBrush = nullptr;
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES propsG;
    propsG.startPoint = D2D1::Point2F(rect.x, rect.y);
    propsG.endPoint = D2D1::Point2F(rect.z, rect.w);
    d2dDeviceContext->CreateLinearGradientBrush(propsG, stopCollection, &gradientBrush);

    ID2D1Layer* maskLayer = nullptr;
    d2dDeviceContext->CreateLayer(&maskLayer);
    d2dDeviceContext->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), glossMaskGeometry),
                                maskLayer);

    ID2D1ImageBrush* blurBrush = nullptr;
    D2D1_IMAGE_BRUSH_PROPERTIES brushProps =
        D2D1::ImageBrushProperties(D2D1::RectF(0, 0, (float)sourceBitmap->GetPixelSize().width,
                                               (float)sourceBitmap->GetPixelSize().height));
    d2dDeviceContext->CreateImageBrush(blurredImage, brushProps, &blurBrush);

    d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), blurBrush);
    d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), gradientBrush);

    d2dDeviceContext->PopLayer();
    maskLayer->Release();
    stopCollection->Release();
    gradientBrush->Release();
    blurBrush->Release();
    glossMaskGeometry->Release();
    blurredImage->Release();
    snapshot->Release();
}

void D2D::drawGlowingText(const Vec2<float>& textPos, const std::string& textStr,
                          const UIColor& color, float textSize, float glowSize) {
    if(glowSize != 0.f) {
        IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, false);
        ID2D1SolidColorBrush* shadowColorBrush = getSolidColorBrush(UIColor(0, 0, 0, color.a));

        DWRITE_TEXT_METRICS textMetrics;
        textLayout->GetMetrics(&textMetrics);

        float width = textMetrics.width + glowSize * 4;
        float height = textMetrics.height + glowSize * 4;

        if(!glowRenderTarget || glowRenderTarget->GetSize().width < width ||
           glowRenderTarget->GetSize().height < height) {
            if(glowRenderTarget)
                glowRenderTarget->Release();
            d2dDeviceContext->CreateCompatibleRenderTarget(D2D1::SizeF(width, height),
                                                           &glowRenderTarget);
        }

        if(!glowBrush) {
            d2dDeviceContext->CreateSolidColorBrush(
                D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, 1.0f), &glowBrush);
        } else {
            glowBrush->SetColor(
                D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, 1.0f));
        }

        glowRenderTarget->BeginDraw();
        glowRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));
        glowRenderTarget->DrawTextLayout(D2D1::Point2F(glowSize * 2, glowSize * 2), textLayout,
                                         glowBrush);
        glowRenderTarget->EndDraw();

        ID2D1Bitmap* glowBitmap = nullptr;
        glowRenderTarget->GetBitmap(&glowBitmap);

        if(!blurEffect2) {
            d2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &blurEffect2);
        }

        blurEffect2->SetInput(0, glowBitmap);
        blurEffect2->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, glowSize);

        if(!transformEffect) {
            d2dDeviceContext->CreateEffect(CLSID_D2D12DAffineTransform, &transformEffect);
        }

        transformEffect->SetInputEffect(0, blurEffect2);

        D2D1_MATRIX_3X2_F matrix =
            D2D1::Matrix3x2F::Translation(textPos.x - glowSize * 2, textPos.y - glowSize * 2);
        transformEffect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX, matrix);

        d2dDeviceContext->DrawImage(transformEffect);

        ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
        d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x, textPos.y), textLayout,
                                         colorBrush);

        glowBitmap->Release();
    } else {
        drawText(textPos, textStr, color, textSize);
    }
}

void D2D::Clean() {
    if(!initD2D)
        return;

    SafeRelease(glowRenderTarget);
    SafeRelease(glowBrush);
    SafeRelease(blurEffect2);
    SafeRelease(transformEffect);

    SafeRelease(d2dFactory);
    if(d2dWriteFactory && d2dInMemoryLoaderRegistered && d2dInMemoryFontLoader) {
        d2dWriteFactory->UnregisterFontFileLoader(d2dInMemoryFontLoader.get());
    }
    d2dEmbeddedFontCollection = nullptr;
    d2dWriteFactory5 = nullptr;
    d2dInMemoryFontLoader = nullptr;
    d2dInMemoryLoaderRegistered = false;
    SafeRelease(d2dWriteFactory);
    SafeRelease(d2dDevice);
    SafeRelease(d2dDeviceContext);
    SafeRelease(sourceBitmap);
    SafeRelease(blurEffect);

    textFormatCache.clear();
    textLayoutCache.clear();
    colorBrushCache.clear();
    textLayoutTemporary.clear();

    initD2D = false;
}

void D2D::PushAxisAlignedClip(const Vec4<float>& rect, bool aliased) {
    if(!initD2D || !d2dDeviceContext)
        return;

    D2D1_RECT_F clipRect = D2D1::RectF(rect.x, rect.y, rect.z, rect.w);
    d2dDeviceContext->PushAxisAlignedClip(
        clipRect, aliased ? D2D1_ANTIALIAS_MODE_ALIASED : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

void D2D::PopAxisAlignedClip() {
    if(!initD2D || !d2dDeviceContext)
        return;
    d2dDeviceContext->PopAxisAlignedClip();
}

void D2D::drawGradientText(const Vec2<float>& textPos, const std::string& textStr,
                           const UIColor& startColor, const UIColor& endColor, float textSize) {
    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, false);
    if(!textLayout)
        return;

    DWRITE_TEXT_METRICS metrics;
    textLayout->GetMetrics(&metrics);

    float width = metrics.widthIncludingTrailingWhitespace;
    float height = metrics.height;

    D2D1_GRADIENT_STOP gradientStops[2];
    gradientStops[0].color = startColor.toD2D1Color();
    gradientStops[0].position = 0.0f;
    gradientStops[1].color = endColor.toD2D1Color();
    gradientStops[1].position = 1.0f;

    ID2D1GradientStopCollection* stopCollection = nullptr;
    d2dDeviceContext->CreateGradientStopCollection(gradientStops, 2, D2D1_GAMMA_2_2,
                                                   D2D1_EXTEND_MODE_CLAMP, &stopCollection);

    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES brushProps = {};
    brushProps.startPoint = D2D1::Point2F(textPos.x, textPos.y);
    brushProps.endPoint = D2D1::Point2F(textPos.x, textPos.y + height);

    ID2D1LinearGradientBrush* gradientBrush = nullptr;
    d2dDeviceContext->CreateLinearGradientBrush(brushProps, stopCollection, &gradientBrush);

    d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x, textPos.y), textLayout,
                                     gradientBrush);

    gradientBrush->Release();
    stopCollection->Release();
}

void D2D::fillCircleGradient(const Vec2<float>& centerPos, float radius, const UIColor& innerColor,
                             const UIColor& outerColor) {
    if(!initD2D || !d2dDeviceContext)
        return;

    D2D1_GRADIENT_STOP stops[2];
    stops[0].color = innerColor.toD2D1Color();
    stops[0].position = 0.0f;
    stops[1].color = outerColor.toD2D1Color();
    stops[1].position = 1.0f;

    ID2D1GradientStopCollection* stopCollection = nullptr;
    HRESULT hr = d2dDeviceContext->CreateGradientStopCollection(
        stops, 2, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &stopCollection);
    if(FAILED(hr) || !stopCollection)
        return;

    D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES brushProps = {};
    brushProps.center = D2D1::Point2F(centerPos.x, centerPos.y);
    brushProps.gradientOriginOffset = D2D1::Point2F(0.f, 0.f);
    brushProps.radiusX = radius;
    brushProps.radiusY = radius;

    ID2D1RadialGradientBrush* radialBrush = nullptr;
    hr = d2dDeviceContext->CreateRadialGradientBrush(brushProps, stopCollection, &radialBrush);
    if(FAILED(hr) || !radialBrush) {
        stopCollection->Release();
        return;
    }

    d2dDeviceContext->FillEllipse(
        D2D1::Ellipse(D2D1::Point2F(centerPos.x, centerPos.y), radius, radius), radialBrush);

    radialBrush->Release();
    stopCollection->Release();
}