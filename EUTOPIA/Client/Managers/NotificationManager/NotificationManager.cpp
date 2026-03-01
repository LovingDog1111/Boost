#include "NotificationManager.h"
#include "../../SDK/GlobalInstance.h"
#include "../../Utils/RenderUtil.h"
#include "../../Utils/TimerUtil.h"

void NotificationManager::Render() {

	Vec2<float> windowSize = GI::getGuiData()->windowSizeReal;
	float textSize = 1.0f;
	float textPaddingX = 4.f;
	float textPaddingY = 2.f;
    float textHeight = RenderUtil::getTextHeight("", textSize);
	float cornerRadius = 6.f;
	float shadowOffset = 2.f;
	float offsetX = 12.f;
	float offsetY = 35.f;
	float posX = offsetX;
	float posY = windowSize.y - (textHeight + textPaddingY * 2.f) - offsetY;

	for (int i = 0; i < notifList.size(); i++) {
		auto& notif = notifList[i];
		std::string message = notif->message;
        float textWidth = RenderUtil::getTextWidth(message, textSize);

		if (notif->duration == notif->maxDuration) {
			notif->pos = Vec2<float>(windowSize.x + 50.f, posY);  // Start further off-screen for slide-in
		}

		if (notif->duration > 0.f) {
            notif->duration -= RenderUtil::deltaTime;
			float t = 1.f - (notif->duration / notif->maxDuration);
			float easedT = 1.f - powf(1.f - t, 3.f);  // Cubic ease-out for smooth animation
			Vec2<float> posTo = Vec2<float>(windowSize.x - offsetX - textWidth - (textPaddingX * 2.f), posY);
			notif->pos.x = Math::lerp(windowSize.x + 50.f, posTo.x, easedT);
            notif->pos.y = Math::lerp(notif->pos.y, posTo.y, RenderUtil::deltaTime * 8.f);
		} else {
			// Fade out animation
			notif->pos.x += notif->vel.x;
            notif->pos.y = Math::lerp(notif->pos.y, posY, RenderUtil::deltaTime * 8.f);
            notif->vel.x += RenderUtil::deltaTime * 30.f;
			if (notif->pos.x > windowSize.x + 50.f) {
				delete notif;
				notifList.erase(notifList.begin() + i);
				i--;
				continue;
			}
		}

		float anim = (windowSize.x - notif->pos.x) / (windowSize.x - (windowSize.x - offsetX - textWidth - (textPaddingX * 2.f)));
		if (anim > 1.f) anim = 1.f;
		if (anim < 0.f) anim = 0.f;
		float alpha = (notif->duration > 0.f) ? anim : (1.f - (notif->pos.x - (windowSize.x - offsetX - textWidth - (textPaddingX * 2.f))) / 50.f);
		if (alpha < 0.f) alpha = 0.f;

		Vec4<float> rectPos = Vec4<float>(notif->pos.x,
			notif->pos.y,
			notif->pos.x + textWidth + (textPaddingX * 2.f),
			notif->pos.y + textHeight + (textPaddingY * 2.f));

		// Draw shadow
		Vec4<float> shadowRect = Vec4<float>(rectPos.x + shadowOffset, rectPos.y + shadowOffset, rectPos.z + shadowOffset, rectPos.w + shadowOffset);
        RenderUtil::fillRoundedRectangle(shadowRect, UIColor(0, 0, 0, (int)(50 * alpha)),
                                         cornerRadius);

		// Draw background with subtle gradient
        RenderUtil::fillRoundedRectangle(rectPos, UIColor(30, 30, 35, (int)(220 * alpha)),
                                         cornerRadius);  // Dark base
		Vec4<float> gradientRect = Vec4<float>(rectPos.x, rectPos.y, rectPos.z, rectPos.y + 2.f);
        RenderUtil::fillRoundedRectangle(gradientRect, UIColor(50, 50, 55, (int)(100 * alpha)),
                                         cornerRadius);  // Top highlight

		// Draw progress bar
		float progress = notif->duration / notif->maxDuration;
		if (progress > 0.f) {
			Vec4<float> progressRect = Vec4<float>(rectPos.x, rectPos.w - 2.f, rectPos.x + (rectPos.z - rectPos.x) * progress, rectPos.w);
            RenderUtil::fillRoundedRectangle(
                progressRect, UIColor(74, 144, 226, (int)(180 * alpha)), 1.f);  // Blue progress
		}

		// Draw text
		Vec2<float> textPos = Vec2<float>(rectPos.x + textPaddingX, rectPos.y + textPaddingY);
        RenderUtil::drawText(textPos, message, UIColor(255, 255, 255, (int)(255 * alpha)),
                             textSize);

		posY -= ((textHeight + (textPaddingY * 2.f)) + (12.f * textSize)) * anim;
	}
}