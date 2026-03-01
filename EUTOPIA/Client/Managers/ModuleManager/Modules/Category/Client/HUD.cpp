#include "HUD.h"

#include <Windows.h>
#include <wininet.h>
#include <wincrypt.h>
#include <string>
#include <vector>
#include <regex>

#include "../../../ModuleManager.h"
bool showBackground;
bool showBPS;
bool showFPS;
bool watermark;

HUD::HUD() : Module("HUD", "Displays HUD Elements", Category::CLIENT) {
    registerSetting(new BoolSetting("Watermark", "NULL", &watermark, true));
    registerSetting(new EnumSetting("Mode", "Font mode", {"Custom", "Mojangles"}, &Mode, 1));
    registerSetting(new BoolSetting("Background", "Watermark background (d2d)", &showBackground, false));
    registerSetting(new BoolSetting("Position", "NULL", &showPosition, true));
    registerSetting(new BoolSetting("Direction", "NULL", &showDirection, false));
    registerSetting(new BoolSetting("BPS", "NULL", &showBPS, true));
    registerSetting(new BoolSetting("FPS", "NULL", &showFPS, true));

    registerSetting(new SliderSetting<int>("Opacity", "NULL", &opacity, 130, 0, 255));
    registerSetting(new SliderSetting<int>("Spacing", "NULL", &spacing, -2, -3, 3));
    registerSetting(new SliderSetting<int>("Offset", "NULL", &offset, 10, 0, 30));
}

static auto lastFrameTime = std::chrono::high_resolution_clock::now();
static float fps = 0.f;
static float smoothedFPS = 0.f;



HUD::~HUD() {}

static bool hwidChecked = false;   
static bool hwidVerified = false;  


std::string gethwidyo() {
    HW_PROFILE_INFO hwProfileInfo;
    if(!GetCurrentHwProfile(&hwProfileInfo))
        return "error";

    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    BYTE pbHash[16];
    DWORD dwHashLen = 16;
    char rgbDigits[] = "0123456789abcdef";

    if(!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return "error";
    if(!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return "error";
    }

    DWORD guidLen = lstrlenW(hwProfileInfo.szHwProfileGuid) * sizeof(WCHAR);
    if(!CryptHashData(hHash, (BYTE*)hwProfileInfo.szHwProfileGuid, guidLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "error";
    }
    if(!CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "error";
    }

    char hashStr[33];
    for(DWORD i = 0; i < dwHashLen; i++) {
        hashStr[i * 2] = rgbDigits[pbHash[i] >> 4];
        hashStr[i * 2 + 1] = rgbDigits[pbHash[i] & 0xf];
    }
    hashStr[32] = 0;

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return std::string(hashStr);
}

std::string downloaddalist(const std::string& url) {
    HINTERNET hInternet = InternetOpenA("HWIDFetcher", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if(!hInternet)
        return "";
    HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if(!hFile) {
        InternetCloseHandle(hInternet);
        return "";
    }

    char buffer[1024];
    DWORD bytesRead;
    std::string content;
    while(InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
        content.append(buffer, bytesRead);
    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
    return content;
}

bool verifydahwidboi2(const std::string& url) {
    std::string hwid = gethwidyo();
    std::string list = downloaddalist(url);
    if(list.empty())
        return false;

    std::vector<std::string> hwids;
    std::regex hwidRegex("\"([a-fA-F0-9]+)\"");
    std::smatch match;
    std::string::const_iterator searchStart(list.cbegin());
    while(std::regex_search(searchStart, list.cend(), match, hwidRegex)) {
        hwids.push_back(match[1]);
        searchStart = match.suffix().first;
    }

    return std::find(hwids.begin(), hwids.end(), hwid) != hwids.end();
}

void HUD::onEnable() {

}


void HUD::onD2DRender() {
    if(Mode != 0)
        return;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(localPlayer == nullptr)
        return;

    auto now = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastFrameTime).count();
    lastFrameTime = now;

    if(deltaTime > 0.0f) {
        float currentFPS = 1.0f / deltaTime;
        smoothedFPS = smoothedFPS * 0.95f + currentFPS * 0.1f;

    }



    Vec2<float> windowSize = GI::getClientInstance()->guiData->windowSizeReal;
    Vec2<float> windowSizeScaled = GI::getClientInstance()->guiData->windowSizeScaled;

    float textSize = 1.f;
    float textPaddingX = std::max(1.f, ((float)spacing + 3.f)) * textSize;
    float textPaddingY = (float)spacing * textSize;
    float textHeight = RenderUtil::getTextHeight("", textSize);
    float posX = 0.f + (float)offset;
    float posY = windowSize.y - (textHeight + textPaddingY * 2.f) - (float)offset;

    if(Mode == 0 && watermark) {

        Colors* colorsModule = ModuleManager::getModule<Colors>();
        UIColor mainColor = colorsModule->getColor();

        if(!hwidChecked) {
            hwidVerified = true;
            hwidChecked = true;
        }

        std::string part1 = "Boost";
        std::string part2 = hwidVerified ? "+" : " v4"; //lol?
         
        float x = offset;
        float y = offset;

        float width = 0.f;
        for(char c : part1)
            width += RenderUtil::getTextWidth(std::string(1, c), 1.2f);
        for(char c : part2)
            width += RenderUtil::getTextWidth(std::string(1, c), 1.2f);

        float height = RenderUtil::getTextHeight("", 1.2f);
        if(showBackground) {
            RenderUtil::fillRectangle(Vec4<float>(x - 1, y, x + width + 1, y + height),
                                      UIColor(0, 0, 0, opacity));
        }

        static float shimmerTime = 0.f;
        shimmerTime += 0.02f;
        if(shimmerTime > 1.f)
            shimmerTime = 0.f;

        for(int i = 0; i < (int)part1.size(); i++) {
            std::string letter(1, part1[i]);
            float charWidth = RenderUtil::getTextWidth(letter, 1.2f);
            RenderUtil::drawText(Vec2<float>(x, y), letter, mainColor, 1.2f);

            float shimmerBandWidth = charWidth * 0.5f;
            float shimmerStartX = x + (width + shimmerBandWidth) * shimmerTime - shimmerBandWidth;
            if(shimmerStartX + shimmerBandWidth > x && shimmerStartX < x + charWidth) {
                float overlapStart = std::max(shimmerStartX, x);
                float overlapEnd = std::min(shimmerStartX + shimmerBandWidth, x + charWidth);
                float overlapWidth = overlapEnd - overlapStart;
                float alpha = overlapWidth / shimmerBandWidth;
                UIColor shimmerColor(std::min(255, mainColor.r + 100),
                                     std::min(255, mainColor.g + 100),
                                     std::min(255, mainColor.b + 100), int(255 * alpha));
                RenderUtil::drawText(Vec2<float>(x, y), letter, shimmerColor, 1.2f);
            }

            x += charWidth;
        }

        for(int i = 0; i < (int)part2.size(); i++) {
            std::string letter(1, part2[i]);
            float charWidth = RenderUtil::getTextWidth(letter, 1.2f);
            RenderUtil::drawText(Vec2<float>(x, y), letter, mainColor, 1.2f);

            float shimmerBandWidth = charWidth * 0.5f;
            float shimmerStartX = x + (width + shimmerBandWidth) * shimmerTime - shimmerBandWidth;
            if(shimmerStartX + shimmerBandWidth > x && shimmerStartX < x + charWidth) {
                float overlapStart = std::max(shimmerStartX, x);
                float overlapEnd = std::min(shimmerStartX + shimmerBandWidth, x + charWidth);
                float overlapWidth = overlapEnd - overlapStart;
                float alpha = overlapWidth / shimmerBandWidth;
                UIColor shimmerColor(std::min(255, mainColor.r + 100),
                                     std::min(255, mainColor.g + 100),
                                     std::min(255, mainColor.b + 100), int(255 * alpha));
                RenderUtil::drawText(Vec2<float>(x, y), letter, shimmerColor, 1.2f);
            }

            x += charWidth;
        }
    }

    if(showPosition) {
        Vec3<float> lpPos = localPlayer->getPos();
        int dimensionId = localPlayer->getDimensionTypeComponent()->type;

        if(dimensionId == 0) {
            char netherPosText[50];
            sprintf_s(netherPosText, 50, "Nether: %.1f, %.1f, %.1f", lpPos.x / 8.f, lpPos.y - 1.6f,
                      lpPos.z / 8.f);
            RenderUtil::fillRectangle(
                Vec4<float>(
                    posX, posY,
                    posX + RenderUtil::getTextWidth(std::string(netherPosText), textSize, false) +
                        textPaddingX * 2.f,
                    posY + textHeight + textPaddingY * 2.f),
                UIColor(0, 0, 0, opacity));
            RenderUtil::drawText(Vec2<float>(posX + textPaddingX, posY + textPaddingY),
                                 std::string(netherPosText), UIColor(255, 255, 255), textSize,
                                 false);
            posY -= textHeight + textPaddingY * 2.f;
        } else if(dimensionId == 1) {
            char overworldPosText[50];
            sprintf_s(overworldPosText, 50, "Overworld: %.1f, %.1f, %.1f", lpPos.x * 8.f,
                      lpPos.y - 1.6f, lpPos.z * 8.f);
            RenderUtil::fillRectangle(
                Vec4<float>(
                    posX, posY,
                    posX +
                        RenderUtil::getTextWidth(std::string(overworldPosText), textSize, false) +
                        textPaddingX * 2.f,
                    posY + textHeight + textPaddingY * 2.f),
                UIColor(0, 0, 0, opacity));
            RenderUtil::drawText(Vec2<float>(posX + textPaddingX, posY + textPaddingY),
                                 std::string(overworldPosText), UIColor(255, 255, 255), textSize,
                                 false);
            posY -= textHeight + textPaddingY * 2.f;
        }

        char posText[50];
        sprintf_s(posText, 50, "Position: %.1f, %.1f, %.1f", lpPos.x, lpPos.y - 1.6f, lpPos.z);
        RenderUtil::fillRectangle(
            Vec4<float>(posX, posY,
                        posX + RenderUtil::getTextWidth(std::string(posText), textSize, false) +
                            textPaddingX * 2.f,
                        posY + textHeight + textPaddingY * 2.f),
            UIColor(0, 0, 0, opacity));
        RenderUtil::drawText(Vec2<float>(posX + textPaddingX, posY + textPaddingY),
                             std::string(posText), UIColor(255, 255, 255), textSize, false);
        posY -= textHeight + textPaddingY * 2.f;
    }

    if(showFPS) {
        char fpsText[20];
        sprintf_s(fpsText, 20, "FPS: %d", (int)(smoothedFPS + 0.5f));


        RenderUtil::fillRectangle(
            Vec4<float>(posX, posY,
                        posX + RenderUtil::getTextWidth(fpsText, textSize) + textPaddingX * 2.f,
                        posY + textHeight + textPaddingY * 2.f),
            UIColor(0, 0, 0, opacity));

        RenderUtil::drawText(Vec2<float>(posX + textPaddingX, posY + textPaddingY),
                             std::string(fpsText), UIColor(255, 255, 255), textSize);

        posY -= textHeight + textPaddingY * 2.f;

    }

    if(showDirection) {
        float rotation = localPlayer->getRotation().x;
        if(rotation < 0)
            rotation += 360.0f;

        std::string directionText;
        if((0 <= rotation && rotation < 45) || (315 <= rotation && rotation < 360))
            directionText = "South (+Z)";
        else if(45 <= rotation && rotation < 135)
            directionText = "West (-X)";
        else if(135 <= rotation && rotation < 225)
            directionText = "North (-Z)";
        else if(225 <= rotation && rotation < 315)
            directionText = "East (+X)";
        else
            directionText = "NULL";

        char finalText[25];
        sprintf_s(finalText, 25, "Direction: %s", directionText.c_str());
        RenderUtil::fillRectangle(
            Vec4<float>(posX, posY,
                        posX + RenderUtil::getTextWidth(std::string(finalText), textSize) +
                            textPaddingX * 2.f,
                        posY + textHeight + textPaddingY * 2.f),
            UIColor(0, 0, 0, opacity));
        RenderUtil::drawText(Vec2<float>(posX + textPaddingX, posY + textPaddingY),
                             std::string(finalText), UIColor(255, 255, 255), textSize);
        posY -= textHeight + textPaddingY * 2.f;
    }

    if(showBPS) {
        static Vec3<float> lastPos = Vec3<float>(0.f, 0.f, 0.f);
        static float bps = 0.f;
        static auto lastTime = std::chrono::high_resolution_clock::now();

        Vec3<float> currentPos = localPlayer->getPos();
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastTime).count();

        if(deltaTime > 0.05f) {
            float dx = currentPos.x - lastPos.x;
            float dz = currentPos.z - lastPos.z;
            bps = std::sqrt(dx * dx + dz * dz) / deltaTime;
            lastPos = currentPos;
            lastTime = now;
        }

        char bpsText[30];
        sprintf_s(bpsText, 30, "BPS: %.2f", bps);
        RenderUtil::fillRectangle(
            Vec4<float>(posX, posY,
                        posX + RenderUtil::getTextWidth(std::string(bpsText), textSize) +
                            textPaddingX * 2.f,
                        posY + textHeight + textPaddingY * 2.f),
            UIColor(0, 0, 0, opacity));
        RenderUtil::drawText(Vec2<float>(posX + textPaddingX, posY + textPaddingY),
                             std::string(bpsText), UIColor(255, 255, 255), textSize);
        posY -= textHeight + textPaddingY * 2.f;
    }
}

void HUD::onMCRender(MinecraftUIRenderContext* ctx) {
    if(Mode != 1)
        return;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    auto now = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastFrameTime).count();
    lastFrameTime = now;

    if(deltaTime > 0.0f) {
        float currentFPS = 1.0f / deltaTime;
        smoothedFPS = smoothedFPS * 0.95f + currentFPS * 0.1f;
    }



    Colors* colorsModule = ModuleManager::getModule<Colors>();
    UIColor mainColor = colorsModule->getColor();

    if(!hwidChecked) {
        hwidVerified = verifydahwidboi2("http://43.226.0.155:5000/get_hwids2");
        hwidChecked = true;
    }
    
    if(watermark) {
        std::string part1 = "Boost";
        std::string part2 = hwidVerified ? "+" : " v4";

        float x = offset;
        float y = offset;

        float width = 0.f;
        for(char c : part1)
            width += DrawUtil::getTextWidth(std::string(1, c), 1.2f);
        for(char c : part2)
            width += DrawUtil::getTextWidth(std::string(1, c), 1.2f);

        static float shimmerTime = 0.f;
        shimmerTime += 0.02f;
        if(shimmerTime > 1.f)
            shimmerTime = 0.f;

        for(int i = 0; i < (int)part1.size(); i++) {
            std::string letter(1, part1[i]);
            float charWidth = DrawUtil::getTextWidth(letter, 1.2f);
            DrawUtil::drawText({x, y}, letter, mainColor, 1.2f);
            float shimmerBandWidth = charWidth * 0.5f;
            float shimmerStartX = x + (width + shimmerBandWidth) * shimmerTime - shimmerBandWidth;
            if(shimmerStartX + shimmerBandWidth > x && shimmerStartX < x + charWidth) {
                float overlapStart = std::max(shimmerStartX, x);
                float overlapEnd = std::min(shimmerStartX + shimmerBandWidth, x + charWidth);
                float overlapWidth = overlapEnd - overlapStart;
                float alpha = overlapWidth / shimmerBandWidth;
                UIColor shimmerColor(std::min(255, mainColor.r + 100),
                                     std::min(255, mainColor.g + 100),
                                     std::min(255, mainColor.b + 100), int(255 * alpha));
                DrawUtil::drawText({x, y}, letter, shimmerColor, 1.2f);
            }
            x += charWidth;
        }

        for(int i = 0; i < (int)part2.size(); i++) {
            std::string letter(1, part2[i]);
            float charWidth = DrawUtil::getTextWidth(letter, 1.2f);
            DrawUtil::drawText({x, y}, letter, mainColor, 1.2f);
            float shimmerBandWidth = charWidth * 0.5f;
            float shimmerStartX = x + (width + shimmerBandWidth) * shimmerTime - shimmerBandWidth;
            if(shimmerStartX + shimmerBandWidth > x && shimmerStartX < x + charWidth) {
                float overlapStart = std::max(shimmerStartX, x);
                float overlapEnd = std::min(shimmerStartX + shimmerBandWidth, x + charWidth);
                float overlapWidth = overlapEnd - overlapStart;
                float alpha = overlapWidth / shimmerBandWidth;
                UIColor shimmerColor(std::min(255, mainColor.r + 100),
                                     std::min(255, mainColor.g + 100),
                                     std::min(255, mainColor.b + 100), int(255 * alpha));
                DrawUtil::drawText({x, y}, letter, shimmerColor, 1.2f);
            }
            x += charWidth;
        }
    }

    Vec2<float> screen = GI::getClientInstance()->guiData->windowSizeScaled;
    float textSize = 0.8f;
    float spacingY = 2.f;
    float posX = 5.f;
    float posY = screen.y - 5.f;

    if(showPosition) {
        Vec3<float> lpPos = localPlayer->getPos();
        int dimId = localPlayer->getDimensionTypeComponent()->type;

        if(dimId == 0) {  // Overworld
            char netherPos[50];
            sprintf_s(netherPos, 50, "Nether: %.1f %.1f %.1f", lpPos.x / 8.f, lpPos.y,
                      lpPos.z / 8.f);
            DrawUtil::drawText({posX, posY - textSize * 8}, netherPos, mainColor, textSize);
            posY -= textSize * 8 + spacingY;
        } else if(dimId == 1) {  // Nether
            char overworldPos[50];
            sprintf_s(overworldPos, 50, "Overworld: %.1f %.1f %.1f", lpPos.x * 8.f, lpPos.y,
                      lpPos.z * 8.f);
            DrawUtil::drawText({posX, posY - textSize * 8}, overworldPos, mainColor, textSize);
            posY -= textSize * 8 + spacingY;
        }

        char posText[50];
        sprintf_s(posText, 50, "Pos: %.1f %.1f %.1f", lpPos.x, lpPos.y, lpPos.z);
        DrawUtil::drawText({posX, posY - textSize * 8}, posText, mainColor, textSize);
        posY -= textSize * 8 + spacingY;
    }

    if(showBPS) {
        static Vec3<float> lastPos = localPlayer->getPos();
        static float bps = 0.f;
        static auto lastTime = std::chrono::high_resolution_clock::now();
        Vec3<float> currentPos = localPlayer->getPos();
        auto now = std::chrono::high_resolution_clock::now();
        float delta = std::chrono::duration<float>(now - lastTime).count();
        if(delta > 0.05f) {
            float dx = currentPos.x - lastPos.x;
            float dz = currentPos.z - lastPos.z;
            bps = std::sqrt(dx * dx + dz * dz) / delta;
            lastPos = currentPos;
            lastTime = now;
        }
        char buffer[30];
        sprintf_s(buffer, 30, "BPS: %.2f", bps);
        DrawUtil::drawText({posX, posY - textSize * 8}, buffer, mainColor, textSize);
        posY -= textSize * 8 + spacingY;
    }

    if(showDirection) {
        float rot = localPlayer->getRotation().x;
        if(rot < 0)
            rot += 360.f;
        const char* dir = "NULL";
        if((0 <= rot && rot < 45) || (315 <= rot && rot < 360))
            dir = "South";
        else if(45 <= rot && rot < 135)
            dir = "West";
        else if(135 <= rot && rot < 225)
            dir = "North";
        else if(225 <= rot && rot < 315)
            dir = "East";
        char buffer[25];
        sprintf_s(buffer, 25, "Dir: %s", dir);
        DrawUtil::drawText({posX, posY - textSize * 8}, buffer, mainColor, textSize);
        posY -= textSize * 8 + spacingY;
    }

    if(showFPS) {
        char fpsText[20];
        sprintf_s(fpsText, 20, "FPS: %d", (int)(smoothedFPS + 0.5f));
        DrawUtil::drawText({posX, posY - textSize * 8}, fpsText, mainColor, textSize);
        posY -= textSize * 8 + spacingY;
    }
}
