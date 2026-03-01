#include "Keystrokes.h"

#include "../../../ModuleManager.h"
#include "../Client\Managers\ModuleManager\Modules\Category\Client\ColorsMod.h"

Keystrokes::Keystrokes()
    : Module("Keystrokes", "Shows WASD + jump input on screen", Category::CLIENT) {
    registerSetting(new SliderSetting<float>("Scale", "Key size scale", &scale, 1.0f, 0.5f, 2.0f));
    registerSetting(new SliderSetting<int>("Opacity", "Background opacity", &opacity, 160, 0, 255));
}

Keystrokes::~Keystrokes() {}

void Keystrokes::onD2DRender() {
    Vec2<float> windowSize = GI::getClientInstance()->guiData->windowSizeReal;
    float keySize = 30.f * scale;
    float padding = 5.f;

    bool isForward = GI::isKeyDown('W');
    bool isLeft = GI::isKeyDown('A');
    bool isBackward = GI::isKeyDown('S');
    bool isRight = GI::isKeyDown('D');
    bool isJump = GI::isKeyDown(VK_SPACE);

    Colors* colorsModule = ModuleManager::getModule<Colors>();
    auto getColor = [&](int index) -> UIColor {
        if(colorsModule)
            return colorsModule->getColor(index);
        return UIColor(255, 255, 255, 255);
    };

    Vec2<float> startPos(windowSize.x - (keySize * 3 + padding * 4),
                         windowSize.y - (keySize * 3 + padding * 4));

    auto drawKey = [&](const std::string& label, Vec2<float> pos, bool pressed,
                       float width = -1.f) {
        if(width <= 0)
            width = keySize;
        UIColor bgColor = pressed ? UIColor(getColor(0).r, getColor(0).g, getColor(0).b, opacity)
                                  : UIColor(50, 50, 50, opacity);
        RenderUtil::fillRectangle(Vec4<float>(pos.x, pos.y, pos.x + width, pos.y + keySize),
                                  bgColor);
        RenderUtil::drawRectangle(Vec4<float>(pos.x, pos.y, pos.x + width, pos.y + keySize),
                                  UIColor(255, 255, 255, 200), 1.f);

        float textW = RenderUtil::getTextWidth(label, scale * 0.5f);
        float textH = RenderUtil::getTextHeight(label, scale * 0.5f);
        RenderUtil::drawText(
            Vec2<float>(pos.x + (width - textW) / 2.f, pos.y + (keySize - textH) / 2.f), label,
            UIColor(255, 255, 255), scale * 0.5f, true);
    };

    drawKey("W", startPos + Vec2<float>(keySize + padding, 0), isForward);
    drawKey("A", startPos + Vec2<float>(0, keySize + padding), isLeft);
    drawKey("S", startPos + Vec2<float>(keySize + padding, keySize + padding), isBackward);
    drawKey("D", startPos + Vec2<float>(keySize * 2 + padding * 2, keySize + padding), isRight);

    float spaceX = startPos.x;
    float spaceY = startPos.y + keySize * 2 + padding * 2;
    float spaceWidth = keySize * 3 + padding * 2; 
    drawKey("Space", Vec2<float>(spaceX, spaceY), isJump, spaceWidth);
}
