#include "KillAuraH.h"

#include <DrawUtil.h>
#include <Minecraft/InvUtil.h>
#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include "../../../../../../SDK/Render/MeshHelpers.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

#include <Windows.h>
#include <wininet.h>
#include <wincrypt.h>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include "json.hpp"

#include "../../../ModuleManager.h"

KillAuraH2::KillAuraH2()
    : Module("KillAuraH", "Advanced KillAura with Hook Speed", Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Range", "Attack range", &range, 5.f, 3.f, 150.f));
    registerSetting(new SliderSetting<float>("APS", "Attacks per second", &aps, 10.f, 1.f, 1000.f));
    registerSetting(
        new BoolSetting("EnableHookSpeed", "Enable hook speed system", &enableHookSpeed, true));
    registerSetting(
        new BoolSetting("AutoHookSpeed", "Auto hook speed patterns", &autoHookSpeed, false));
    registerSetting(
        new SliderSetting<int>("HookAmount", "Number of hook attacks", &hookAmount, 5, 1, 20));
    registerSetting(
        new SliderSetting<int>("Multiplier", "Extra attacks per hook", &multiplier, 1, 1, 10));
    registerSetting(new EnumSetting("SpeedType", "Hook speed pattern",
                                    {"Fast", "Slow", "Medium", "Random"}, &speedType, 0));
    registerSetting(new EnumSetting("Rotation", "Rotation mode",
                                    {"None", "Normal", "Strafe", "Predict"}, &rotMode, 1));
    registerSetting(new BoolSetting("Target Strafe", "Strafe around target", &strafe, false));
    registerSetting(
        new SliderSetting<float>("Test-Strafe", "Test-Strafe distance", &test, 0.f, 0.f, 15.f));
    registerSetting(new SliderSetting<float>("Test", "Test value", &headspeed, 30.f, 0.f, 360.f));
    registerSetting(new BoolSetting("Mobs", "Attack Mobs", &includeMobs, false));
    registerSetting(new BoolSetting("VisualTarget", "Show target highlight", &visualTarget, false));
    registerSetting(new BoolSetting("VisualRange", "Show attack range", &visualRange, false));
    registerSetting(
        new ColorSetting("TargetColor", "Target color", &targetColor, UIColor(255, 0, 0, 255)));
}

std::string getuserhwid() {
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

std::string downloadlist(const std::string& url) {
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

const std::string baseUrl = "http://43.226.0.155:5000";




bool verifydahwidboi(const std::string& baseUrl) {
    std::string hwid = getuserhwid();
    std::string url = baseUrl + "/check_hwid/" + hwid;

    HINTERNET hInternet = InternetOpenA("HWIDCheck", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if(!hInternet)
        return false;

    HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if(!hFile) {
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[1024];
    DWORD bytesRead;
    std::string response;

    while(InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
        response.append(buffer, bytesRead);

    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);

    try {
        auto json = nlohmann::json::parse(response);
        return json.value("authorized", false);  
    } catch(...) {
        return false; 
    }
}


void KillAuraH2::onEnable() {
    if(!verifydahwidboi(baseUrl)) {
        GI::DisplayClientMessage("Unauthorized HWID! This is a Boost+ feature. Please upgrade your plan to use it.");
        this->setEnabled(false);
    }
    targetList.clear();
    shouldRotate = false;
    attackCounter = 0;
    hookCounter = 0;
    autoSpeedCounter = 0;
    start = GetTickCount64();
    lastAttackTime = 0;
}

void KillAuraH2::onDisable() {
    targetList.clear();
    shouldRotate = false;
}

bool KillAuraH2::sortByDist(Actor* a1, Actor* a2) {
    Vec3<float> lpPos = GI::getLocalPlayer()->getPos();
    return a1->getPos().dist(lpPos) < a2->getPos().dist(lpPos);
}

void KillAuraH2::Attack(Actor* target) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer || !localPlayer->gamemode)
        return;
    localPlayer->gamemode->attack(target);
    localPlayer->swing();
    attackCounter++;
}

bool KillAuraH2::Counter(double a1) {
    if((GetTickCount64() - start) >= (a1 / aps)) {
        start = GetTickCount64();
        return true;
    }
    return false;
}

void KillAuraH2::updateAutoHookSpeed() {
    if(!autoHookSpeed)
        return;

    autoSpeedCounter++;

    switch(speedType) {
        case 0:
            if(autoSpeedCounter % 5 == 0)
                hookAmount = 8;
            else if(autoSpeedCounter % 5 == 1)
                hookAmount = 10;
            else if(autoSpeedCounter % 5 == 2)
                hookAmount = 6;
            else if(autoSpeedCounter % 5 == 3)
                hookAmount = 12;
            else
                hookAmount = 9;
            break;

        case 1:
            if(autoSpeedCounter % 8 == 0)
                hookAmount = 3;
            else if(autoSpeedCounter % 8 == 1)
                hookAmount = 5;
            else if(autoSpeedCounter % 8 == 2)
                hookAmount = 4;
            else if(autoSpeedCounter % 8 == 3)
                hookAmount = 2;
            else if(autoSpeedCounter % 8 == 4)
                hookAmount = 6;
            else if(autoSpeedCounter % 8 == 5)
                hookAmount = 3;
            else if(autoSpeedCounter % 8 == 6)
                hookAmount = 4;
            else
                hookAmount = 5;
            break;

        case 2:
            if(autoSpeedCounter % 6 == 0)
                hookAmount = 6;
            else if(autoSpeedCounter % 6 == 1)
                hookAmount = 8;
            else if(autoSpeedCounter % 6 == 2)
                hookAmount = 5;
            else if(autoSpeedCounter % 6 == 3)
                hookAmount = 7;
            else if(autoSpeedCounter % 6 == 4)
                hookAmount = 6;
            else
                hookAmount = 7;
            break;

        case 3:
            hookAmount = rand() % 15 + 1;
            break;
    }

    if(autoSpeedCounter > 1000)
        autoSpeedCounter = 0;
}

void KillAuraH2::onNormalTick(LocalPlayer* player) {
    if(!player)
        return;

    Level* level = player->level;
    if(!level)
        return;

    targetList.clear();
    std::vector<Actor*> actors = player->level->getRuntimeActorList();
    for(auto* entity : actors) {

        if(!TargetUtil::isTargetValid(entity, includeMobs))
            continue;
        if(!entity->isAlive()) 
            continue;
        if(player->getPos().dist(entity->getPos()) <= range)
            targetList.push_back(entity);

    }

    if(targetList.empty()) {
        shouldRotate = false;
        return;
    }

    std::sort(targetList.begin(), targetList.end(),
              [this](Actor* a1, Actor* a2) { return sortByDist(a1, a2); });

    if(rotMode != 0 || strafe) {
        Vec3<float> targetPos = targetList[0]->getEyePos();
        rotAngle = player->getEyePos().CalcAngle(targetPos);
        rotAngle5 = player->getEyePos().CalcAngle(targetPos);

        if(strafe) {
            Vec3<float> localPos = player->getPos();
            float angle = atan2f(targetPos.z - localPos.z, targetPos.x - localPos.x);
            float strafeDistance = 2.5f;
            targetPos.x += cosf(angle + (M_PI / 2.0f)) * strafeDistance;
            targetPos.z += sinf(angle + (M_PI / 2.0f)) * strafeDistance;
            rotAngle = player->getEyePos().CalcAngle(targetPos);
        }

        shouldRotate = true;
    } else {
        shouldRotate = false;
    }

    if(Counter(1000.0)) {
        Attack(targetList[0]);

        if(enableHookSpeed) {
            for(int i = 0; i < hookAmount; i++) {
                LocalPlayer* localPlayer = GI::getLocalPlayer();
                if(localPlayer && localPlayer->gamemode) {
                    localPlayer->gamemode->attack(targetList[0]);
                    localPlayer->swing();
                    attackCounter++;
                }
            }

            if(multiplier > 1) {
                for(int i = 0; i < multiplier; i++) {
                    LocalPlayer* localPlayer = GI::getLocalPlayer();
                    if(localPlayer && localPlayer->gamemode) {
                        localPlayer->gamemode->attack(targetList[0]);
                        localPlayer->swing();
                        attackCounter++;
                    }
                }
            }

            if(autoHookSpeed) {
                updateAutoHookSpeed();
            }
        }
    }
}

void KillAuraH2::onUpdateRotation(LocalPlayer* player) {
    if(!shouldRotate || targetList.empty() || !targetList[0]->isAlive() ||
       (rotMode == 0 && !strafe))
        return;
    Actor* target = targetList[0];



    auto* rot = player->getActorRotationComponent();
    auto* head = player->getActorHeadRotationComponent();
    auto* body = player->getMobBodyRotationComponent();

    if(!rot)
        return;

    switch(rotMode) {
        case 0:
            return;

        case 1:
            if(rot) {
                rot->mPitch = rotAngle5.x;
                rot->mYaw = rotAngle5.y;
            }
            if(head)
                head->mHeadRot = rotAngle5.y;
            if(body)
                body->yBodyRot = rotAngle5.y;
            break;

        case 2:
            if(rot) {
                rot->mPitch = rotAngle5.x;
                rot->mYaw = rotAngle5.y;
            }
            break;

        case 3:
            float distanceXZ1 = player->getPos().dist(Vec3<float>(
                targetList[0]->getPos().x, player->getPos().y, targetList[0]->getPos().z));

            float targetYaw = target->getActorRotationComponent()->mYaw;
            float predictYaw = targetYaw + (headspeed - 90.0f);

            float predictRad = predictYaw * (M_PI / 180.0f);
            Vec3<float> targetPos = target->getPos();
            Vec3<float> backwardOffset(-cos(predictRad) * test, 0, -sin(predictRad) * test);
            Vec3<float> backwardPos =
                Vec3<float>(targetPos.x + backwardOffset.x, targetPos.y + backwardOffset.y,
                            targetPos.z + backwardOffset.z);

            rotAngle = player->getPos().CalcAngle(backwardPos).normAngles();

            if(rot) {
                rot->mYaw = rotAngle.y;
                rot->mPitch = rotAngle.x;
            }
            break;
    }

    if(head)
        head->mHeadRot = rotAngle.y;
    if(body)
        body->yBodyRot = rotAngle.y;
}

void KillAuraH2::onLevelRender() {
    LocalPlayer* player = GI::getLocalPlayer();
    if(!player)
        return;

    auto* levelRenderer = GI::getClientInstance()->getLevelRenderer();
    if(!levelRenderer)
        return;

    Vec3<float> origin = levelRenderer->getrenderplayer()->origin;

    if(visualRange) {
        Vec3<float> pos = player->getPos() - origin;
        std::vector<Vec3<float>> circlePoints;
        const int segments = 90;

        for(int i = 0; i < segments; i++) {
            float angle = (i * 2.0f * M_PI) / segments;
            float x = pos.x + range * cosf(angle);
            float z = pos.z + range * sinf(angle);
            circlePoints.emplace_back(x, pos.y + 0.1f, z);
        }

        DrawUtil::setColor(UIColor(255, 0, 255, 255));
        DrawUtil::tessellator->begin(VertextFormat::LINE_STRIP, 2);

        for(const auto& point : circlePoints) {
            DrawUtil::tessellator->vertex(point.x, point.y, point.z);
        }

        MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, DrawUtil::tessellator,
                                           DrawUtil::blendMaterial);
    }

    if(visualTarget && !targetList.empty()) {
        drawTargetHighlight(targetList[0], origin, targetColor);
    }
}

void KillAuraH2::drawTargetHighlight(Actor* target, Vec3<float> origin, UIColor color) {
    auto now = std::chrono::steady_clock::now();
    auto timeMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    float animation = sin(timeMs * 0.008f) * 0.2f;

    Vec3<float> targetPos = target->getPos() - origin;
    Vec3<float> circleCenter = {targetPos.x, targetPos.y - 0.8f + animation, targetPos.z};

    float radius = 0.7f;
    const int segments = 36;
    std::vector<Vec3<float>> circlePoints;
    circlePoints.reserve(segments + 1);

    for(int i = 0; i <= segments; i++) {
        float angle = (i * 2.0f * M_PI) / segments;
        float x = circleCenter.x + radius * cosf(angle);
        float z = circleCenter.z + radius * sinf(angle);
        circlePoints.emplace_back(x, circleCenter.y, z);
    }

    DrawUtil::setColor(color);
    DrawUtil::tessellator->begin(VertextFormat::LINE_STRIP, 2);

    for(const auto& point : circlePoints) {
        DrawUtil::tessellator->vertex(point.x, point.y, point.z);
    }

    MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, DrawUtil::tessellator,
                                       DrawUtil::blendMaterial);
}