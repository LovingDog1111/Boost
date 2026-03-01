#include "TargetInfo.h"
bool onlywhilehitting;
int position;
bool showhealth;
bool showname;
bool showdistance;
bool showhitdifficulty;
bool showposition;
bool showfriend;
bool showwinchance;
#include "..\Utils\FriendUtil.h"


int lastHurtTime = 0;
float targetScale = 0.f;  
float scaleSpeed = 0.15f;  


TargetInfo::TargetInfo()
    : Module("TargetInfo", "Displays target info like health and hit difficulty",
             Category::PLAYER) {
    registerSetting(new EnumSetting("Mode", "Render mode", {"Mojangles", "Custom"}, &rendermode, 0));
    registerSetting(new EnumSetting("Position", "Position of the Target Info",
                                    {"Top-Left", "Top-Right", "Bottom-Left", "Bottom-Right", "Center"},
                                    &position, 0));
    registerSetting(new BoolSetting("Only While Hitting", "Only show when hitting target",
                                    &onlywhilehitting, false));
    registerSetting(new BoolSetting("Show health", "Show target health",
                                    &showhealth, true));
    registerSetting(
        new BoolSetting("Show hit count", "Display hits on target", &showhitcount, true));
    registerSetting(new BoolSetting("Show win chance", "Show win chance", &showwinchance, true));

    registerSetting(new BoolSetting("Show name", "Show target name", &showname, true));
    registerSetting(new BoolSetting("Show distance", "Show target distance", &showdistance, true));
    registerSetting(new BoolSetting("Show hit difficulty", "Show hit difficulty", &showhitdifficulty, true));
    registerSetting(
        new BoolSetting("Show pos", "Show target pos", &showposition, false));
    registerSetting(new BoolSetting("Show friend", "Show if target is friend", &showfriend, false));
}

static std::string stripFormatting(const std::string& text) {
    std::string out;
    out.reserve(text.size());

    for(size_t i = 0; i < text.size(); i++) {
        if(text[i] == '§' && i + 1 < text.size()) {
            i++;
            continue;
        }
        out.push_back(text[i]);
    }
    return out;
}


bool TargetInfo::isTargetValid() {
    std::vector<Actor*> actors = ActorUtils::getActorList(true);
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return false;

    Actor* closestTarget = nullptr;
    float closestDistance = FLT_MAX;
    for(Actor* actor : actors) {
        if(!actor || actor == localPlayer)
            continue;

        Vec3<float> playerPos = localPlayer->getPos();
        Vec3<float> actorPos = actor->getPos();

        float dx = actorPos.x - playerPos.x;
        float dy = actorPos.y - playerPos.y;
        float dz = actorPos.z - playerPos.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if(dist < closestDistance) {
            closestTarget = actor;
            closestDistance = dist;
        }
    }

    if(closestTarget) {
        target = closestTarget;
        distance = closestDistance;
        return true;
    }

    return false;
}

float calculateWinChance(Actor* targetActor, int hitCount) {
    if(!targetActor)
        return 0.f;


    float baseChance = 1.f;


    float winChance = baseChance + hitCount * 10.f;

  
    if(winChance > 100.f)
        winChance = 100.f;
    if(winChance < 0.f)
        winChance = 0.f;

    return winChance;
}



void TargetInfo::onD2DRender() {
    if(rendermode != 1)
        return;

    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    if(!isTargetValid())
        return;

    Actor* targetActor = target;

    if(lastTarget != targetActor) {
        lastTarget = targetActor;
        hitCount = 0;
        lastHurtTime = 0;
        targetScale = 0.f;
    }

    if(targetScale < 1.f)
        targetScale += scaleSpeed;
    if(targetScale > 1.f)
        targetScale = 1.f;

    int currentHurtTime = targetActor->getHurtTime();
    if(currentHurtTime > 0 && lastHurtTime == 0) {
        hitCount++;
    }
    lastHurtTime = currentHurtTime;

    if(onlywhilehitting) {
        if(currentHurtTime <= 0 || distance > 10.f)
            return;
    }

    Colors* colorsModule = ModuleManager::getModule<Colors>();
    UIColor accent = colorsModule->getColor();
    UIColor textColor(255, 255, 255, 255);

    Vec2<float> screen = GI::getClientInstance()->guiData->windowSizeReal;

    float baseScale = 0.8f;
    float lineHeight = RenderUtil::getTextHeight("A", baseScale) + 3.f;
    float padding = 6.f;
    float panelWidth = 0.f;
    int lineCount = 0;

    float targetHealth = targetActor->getHealth();
    float targetDistance = distance;
    Vec3<float> pos3 = targetActor->getPos();
    bool targetisFriend = FriendManager::isFriend(targetActor->getNameTag());

    std::string nameTag = stripFormatting(targetActor->getNameTag());
    std::string hitCounterText = "Hits: " + std::to_string(hitCount);
    float winChance = calculateWinChance(targetActor, hitCount);
    std::string winChanceText = "Win Chance: " + std::to_string((int)winChance) + "%";


    std::string healthText = "Health: " + std::to_string((int)targetHealth);
    std::string distanceText = "Distance: " + std::to_string((int)targetDistance) + " m";
    std::string positionText = "Pos: " + std::to_string((int)pos3.x) + " " +
                               std::to_string((int)pos3.y) + " " + std::to_string((int)pos3.z);
    std::string friendText = "Friend: " + std::string(targetisFriend ? "Yes" : "No");

    std::string hitDifficulty = "Good";
    UIColor hitColor(0, 255, 0);
    if(targetDistance > 20.f) {
        hitDifficulty = "Impossible";
        hitColor = UIColor(255, 0, 0);
    } else if(targetDistance > 10.f) {
        hitDifficulty = "Maybe";
        hitColor = UIColor(255, 255, 0);
    }
    std::string hitText = "Hit Difficulty: " + hitDifficulty;

    auto calc = [&](const std::string& s) {
        panelWidth = std::max(panelWidth, RenderUtil::getTextWidth(s, baseScale));
        lineCount++;
    };

    if(showname)
        calc(nameTag);
    if(showhitcount)
        calc(hitCounterText);
    if(showwinchance) {
        calc(winChanceText);  
    }
    if(showhealth)
        calc(healthText);
    if(showdistance)
        calc(distanceText);
    if(showposition)
        calc(positionText);
    if(showfriend)
        calc(friendText);
    if(showhitdifficulty)
        calc(hitText);

    float panelHeight = lineCount * lineHeight + padding * 2.f;
    panelWidth += padding * 2.f;

    float x = 0.f;
    float y = 0.f;

    if(position == 0) {
        x = screen.x * 0.02f;
        y = screen.y * 0.05f;
    } else if(position == 1) {
        x = screen.x * 0.98f - panelWidth;
        y = screen.y * 0.05f;
    } else if(position == 2) {
        x = screen.x * 0.02f;
        y = screen.y * 0.95f - panelHeight;
    } else if(position == 3) {
        x = screen.x * 0.98f - panelWidth;
        y = screen.y * 0.95f - panelHeight;
    } else if(position == 4) {
        x = (screen.x - panelWidth) * 0.5f;
        y = screen.y * 0.55f;
    }

    Vec4<float> bg(x, y, x + panelWidth * targetScale, y + panelHeight * targetScale);
    RenderUtil::fillRoundedRectangle(bg, UIColor(0, 0, 0, 170), 8.f * targetScale);
    RenderUtil::drawRoundedRectangle(bg, accent, 8.f * targetScale, 1.f);

    float textY = y + padding * targetScale;

    auto drawLine = [&](const std::string& s, const UIColor& c) {
        RenderUtil::drawText({x + padding, textY}, s, c, baseScale * targetScale);
        textY += lineHeight * targetScale;
    };

    if(showname)
        drawLine(nameTag, textColor);
    if(showhitcount)
        drawLine(hitCounterText, textColor);
    if(showwinchance)
        drawLine(winChanceText, textColor);
    if(showhealth)
        drawLine(healthText, textColor);
    if(showdistance)
        drawLine(distanceText, textColor);
    if(showposition)
        drawLine(positionText, textColor);
    if(showfriend)
        drawLine(friendText, targetisFriend ? UIColor(0, 191, 255) : textColor);
    if(showhitdifficulty)
        drawLine(hitText, hitColor);
}

void TargetInfo::onMCRender(MinecraftUIRenderContext* ctx) {
    if(rendermode != 0)
        return;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;


    Vec2<float> screen = GI::getClientInstance()->guiData->windowSizeScaled;
    Colors* colorsModule = ModuleManager::getModule<Colors>();
    UIColor color = colorsModule->getColor();

    if(!isTargetValid())
        return;

    Actor* targetActor = target;

    
    if(lastTarget != targetActor) {
        lastTarget = targetActor;
        hitCount = 0;
        lastHurtTime = 0;
    }

    int currentHurtTime = targetActor->getHurtTime();
    if(currentHurtTime > 0 && lastHurtTime == 0) {
        hitCount++;
    }
    lastHurtTime = currentHurtTime;



    float targetHealth = targetActor->getHealth();
    float targetHitCounter = hitCount;
    float targetWinChance = calculateWinChance(targetActor, hitCount);
    float targetHurtTime = targetActor->getHurtTime();
    float targetDistance = distance;
    float targetPosX = targetActor->getPos().x;
    float targetPosY = targetActor->getPos().y;
    float targetPosZ = targetActor->getPos().z;
    bool targetisFriend = FriendManager::isFriend(targetActor->getNameTag());


    if(onlywhilehitting) {
        if(targetActor->getHurtTime() <= 0 || targetDistance > 10.f)
            return;
    }

    std::string nameTag = targetActor->getNameTag();
    std::string hitCounterText = "Hits: " + std::to_string(hitCount);
    std::string winChanceText = "Win Chance: " + std::to_string((int)targetWinChance) + "%";
    std::string healthText = "Health: " + std::to_string(targetHealth);
    std::string distanceText = "Distance: " + std::to_string(targetDistance) + " m";
    std::string positionText = "Position: X:" + std::to_string((int)targetPosX) + " Y:" +
                               std::to_string((int)targetPosY) + " Z:" + std::to_string((int)targetPosZ);
    std::string friendText = "Friend: " + std::string(targetisFriend ? "Yes" : "No");

    float textHeight = DrawUtil::getTextHeight(0.8f);
    float x = 10.f;
    float y = 20.f;

    switch(position) {
        case 0:  
            x = 10.f;
            y = 20.f;
            break;
        case 1:  
            x = screen.x -
                std::max(DrawUtil::getTextWidth(nameTag, 0.8f),
                         std::max(DrawUtil::getTextWidth(healthText, 0.8f),
                                  std::max(DrawUtil::getTextWidth(distanceText, 0.8f),
                                           DrawUtil::getTextWidth("Hit Difficulty: Good", 0.8f)))) -
                10.f;
            y = 20.f;
            break;
        case 2:  
            x = 10.f;
            y = screen.y - (textHeight + 2.f) * 4 - 10.f;
            break;
        case 3:  
            x = screen.x -
                std::max(DrawUtil::getTextWidth(nameTag, 0.8f),
                         std::max(DrawUtil::getTextWidth(healthText, 0.8f),
                                  std::max(DrawUtil::getTextWidth(distanceText, 0.8f),
                                           DrawUtil::getTextWidth("Hit Difficulty: Good", 0.8f)))) -
                10.f;
            y = screen.y - (textHeight + 2.f) * 4 - 10.f;
            break;
        case 4: // below crosshair 
            x = screen.x / 2.f -
                std::max(DrawUtil::getTextWidth(nameTag, 0.8f),
                         std::max(DrawUtil::getTextWidth(healthText, 0.8f),
                                  std::max(DrawUtil::getTextWidth(distanceText, 0.8f),
                                           DrawUtil::getTextWidth("Hit Difficulty: Good", 0.8f)))) /
                    2.f;
            y = screen.y / 2.f + 20.f;  
            break;

    }

    if(showname) {
        DrawUtil::drawText({x, y}, nameTag, color, 0.8f);
        y += textHeight + 2.f;
    }
    
    if(showhitcount) {
        DrawUtil::drawText({x, y}, hitCounterText, color, 0.8f);
        y += textHeight + 2.f;
    }
    if(showwinchance) {
        DrawUtil::drawText({x, y}, winChanceText, color, 0.8f);
        y += textHeight + 2.f;
    }
    if(showhealth) {
        DrawUtil::drawText({x, y}, healthText, color, 0.8f);
        y += textHeight + 2.f;
    }
    if(showdistance) {
        DrawUtil::drawText({x, y}, distanceText, color, 0.8f);
        y += textHeight + 2.f;
    }

    if(showposition) {
        DrawUtil::drawText({x, y}, positionText, color, 0.8f);
        y += textHeight + 2.f;
    }
    
    if(showfriend) {
        DrawUtil::drawText({x, y}, friendText, UIColor(0, 191, 255), 0.8f);
        y += textHeight + 2.f;
    }

    if(!showhitdifficulty)
        return;
    std::string hitDifficulty = "Good";
    UIColor hitColor = UIColor(0, 255, 0);

    if(targetDistance > 20.f) {
        hitDifficulty = "Impossible";
        hitColor = UIColor(255, 0, 0);
    } else if(targetDistance > 10.f) {
        hitDifficulty = "Maybe";
        hitColor = UIColor(255, 255, 0);
    }

    std::string hitText = "Hit Difficulty: " + hitDifficulty;
    DrawUtil::drawText({x, y}, hitText, hitColor, 0.8f);
}
