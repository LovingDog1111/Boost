#include "../../../ModuleManager.h"
#include "NameTagsV2.h"
#include "..\Utils\FriendUtil.h"
#include "../../../../../../SDK/Render/Matrix.h"
#include "..\Utils\Minecraft\TargetUtil.h"

bool showDistance = true;
float tagSize = 0.70f;
UIColor friendColor = (0, 255, 255, 255);
NameTags2::NameTags2() : Module("NameTagsV2", "OG Nametags", Category::RENDER) {
    registerSetting(new SliderSetting<float>("Size", "NULL", &tagSize, 0.5f, 0.f, 1.f));
    registerSetting(new SliderSetting<float>("Opacity", "NULL", &opacity, 1.f, 0.f, 1.f));
    registerSetting(new BoolSetting("Self", "Render urself", &showSelf, true));
    registerSetting(
        new BoolSetting("ShowDistance", "Show distance on nametags", &showDistance, showDistance));
    registerSetting(new BoolSetting("Underline", "Show a line under", &underline, false));
    registerSetting(new BoolSetting("Dynamic Size", "NULL", &dynamicSize, true));
}

static bool envaledchar(char c) {
    return !(c >= 0 && *reinterpret_cast<unsigned char*>(&c) < 128);
}

std::string sanitizexx(const std::string& text) {
    std::string out;
    bool wasValid = true;
    for(char c : text) {
        bool isValid = !envaledchar(c);
        if(wasValid) {
            if(!isValid) {
                wasValid = false;
            } else {
                out += c;
            }
        } else {
            wasValid = isValid;
        }
    }
    return out;
}

void NameTags2::onD2DRender() {
    LocalPlayer* lp = Game.getLocalPlayer();
    if(lp == nullptr)
        return;

    

    for(Actor* actor : lp->getlevel()->getRuntimeActorList()) {
        if(TargetUtil::isTargetValid(actor, false, 999.f) || (actor == lp && showSelf)) {
            Vec2<float> pos;
            if(!Matrix::WorldToScreen(actor->getEyePos().add2(0.f, 0.75f, 0.f), pos)) {
                std::string name = actor->getNameTag();
                name = sanitizexx(name);

                float distance = lp->getPos().dist(actor->getPos());
                std::string distanceText = " [" + std::to_string((int)distance) + "]";

                float textSize = 1.f * tagSize;
                float nameWidth = RenderUtil::getTextWidth(name, textSize);
                float distWidth =
                    showDistance ? RenderUtil::getTextWidth(distanceText, textSize) : 0.f;
                float totalWidth = nameWidth + distWidth;

                float textHeight = RenderUtil::getTextHeight(name, textSize);
                float textPadding = 1.f * textSize;
                Vec2<float> textPos =
                    Vec2<float>(pos.x - totalWidth / 2.f, pos.y - textHeight / 2.f);
                Vec4<float> rectPos =
                    Vec4<float>(textPos.x - textPadding * 2.f, textPos.y - textPadding,
                                textPos.x + totalWidth + textPadding * 2.f,
                                textPos.y + textHeight + textPadding);
                Vec4<float> underlineRect =
                    Vec4<float>(rectPos.x, rectPos.w - 1.f * textSize, rectPos.z, rectPos.w);

                // Draw background box
                RenderUtil::fillRectangle(rectPos, UIColor(0, 0, 0, (int)(255 * opacity)));

                // Optional underline
                if(underline) {
                    RenderUtil::fillRectangle(underlineRect, UIColor(255, 255, 255, 255));
                }

                // Name color (green if friend)
                UIColor nameColor = FriendManager::isFriend(name) ? UIColor(0, 255, 0, 255)
                                                             : UIColor(255, 255, 255, 255);
                RenderUtil::drawText(textPos, name, nameColor, textSize, true);

                // Distance color
                if(showDistance) {
                    UIColor distanceColor;
                    if(distance >= 100.0f) {
                        distanceColor = UIColor(255, 0, 0, 255);  // Red
                    } else if(distance >= 50.0f) {
                        distanceColor = UIColor(255, 255, 0, 255);  // Yellow
                    } else {
                        distanceColor = UIColor(0, 255, 0, 255);  // Green
                    }

                    Vec2<float> distPos = Vec2<float>(textPos.x + nameWidth, textPos.y);
                    RenderUtil::drawText(distPos, distanceText, distanceColor, textSize, true);
                }
            }
        }
    }
}

int waitForInit = 0;
void NameTags2::onMCRender(MinecraftUIRenderContext* renderCtx) {
    if(Game.getLocalPlayer() == nullptr)
        return;
    if(waitForInit >= 20) {
        for(Actor* actor : Game.getLocalPlayer()->getlevel()->getRuntimeActorList()) {
            if(TargetUtil::isTargetValid(actor, false, 999.f) || (actor == Game.getLocalPlayer() && showSelf)) {
                Vec2<float> pos;
                if(!Matrix::WorldToScreen(actor->getEyePos().add2(0.f, 0.85f, 0.f), pos)) {
                    std::string name = actor->getNameTag();
                    float textSize = 1.f * tagSize;
                    float textWidth = DrawUtil::getTextWidth(name, textSize);
                    float textHeight = DrawUtil::getTextHeight(textSize);
                    float textPadding = 1.f * textSize;
                    Vec2<float> textPos =
                        Vec2<float>(pos.x - textWidth / 2.f, pos.y - textHeight / 2.f);
                    Vec4<float> rectPos =
                        Vec4<float>(textPos.x - textPadding * 2.f, textPos.y - textPadding,
                                    textPos.x + textWidth + textPadding * 2.f,
                                    textPos.y + textHeight + textPadding);
                    if(showItems) {
                        float scale = tagSize * 0.75f;
                        float spacing = scale + 15.f;
                        int x = 0;
                        int y = rectPos.y - 5.f;
                        if(actor->getOffhandSlot() != nullptr && actor->getOffhandSlot()->isValid())
                            x += scale * spacing;
                        for(int i = 0; i < 4; i++) {
                            ItemStack* armor = actor->getArmor(i);
                            if(armor->isValid() && armor != nullptr) {
                                x += scale * spacing;
                            }
                        }
                        ItemStack* selected = actor->getCarriedItem();
                        if(selected->isValid() && selected != nullptr)
                            x += scale * spacing;
                        float xReal = (rectPos.x + rectPos.z - x) / 2;
                        if(actor->getOffhandSlot()->isValid() &&
                           actor->getOffhandSlot() != nullptr) {
                            ItemStack* offhand = actor->getOffhandSlot();
                            DrawUtil::drawItem(Vec2<float>(xReal, y), offhand, 1.f, scale);

                            if(offhand->mCount > 1) {
                                DrawUtil::drawText(Vec2<float>(xReal + 10.f, y + 1.5f),
                                              std::to_string((int)offhand->mCount),
                                              UIColor(1.f, 1.f, 1.f), scale);
                            }
                            xReal += scale * spacing;
                        }
                        for(int i = 0; i < 4; i++) {
                            ItemStack* armor = actor->getArmor(i);
                            if(armor->isValid() && armor != nullptr) {
                                DrawUtil::drawItem(Vec2<float>(xReal, y), armor, 1.f, scale);
              
                                if(armor->mCount > 1) {
                                    DrawUtil::drawText(Vec2<float>(xReal + 10.f, y + 1.5f),
                                                  std::to_string((int)armor->mCount),
                                                  UIColor(1.f, 1.f, 1.f), scale);
                                }
                                xReal += scale * spacing;
                            }
                        }
                        if(selected->isValid() && selected != nullptr) {
                            DrawUtil::drawItem(Vec2<float>(xReal, y), selected, 1.f, scale);

                            if(selected->mCount > 1) {
                                DrawUtil::drawText(Vec2<float>(xReal + 10.f, y + 1.5f),
                                              std::to_string((int)selected->mCount),
                                              UIColor(1.f, 1.f, 1.f), scale);
                            }
                        }
                    }
                }
            }
        }
    } else
        waitForInit++;
}