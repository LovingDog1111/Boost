#include "Step.h"
#include "../../../../../../SDK/World/Actor/Components/MaxAutoStepComponent.h"

Step::Step() : Module("Step", "Stepping on blocks", Category::MOVEMENT) {
    registerSetting(new SliderSetting<int>("Step value", "NULL", &value, 2, 2, 5));
}

void Step::onNormalTick(LocalPlayer* localPlayer) {
    auto stepComponent = localPlayer->getEntityContext()->tryGetComponent<MaxAutoStepComponent>();
    if(stepComponent) {
        stepComponent->mMaxStepHeight = static_cast<float>(value);
    }
}

void Step::onDisable() {
    auto localPlayer = GI::getLocalPlayer();
    if(localPlayer != nullptr) {
        auto stepComponent =
            localPlayer->getEntityContext()->tryGetComponent<MaxAutoStepComponent>();
        if(stepComponent) {
            stepComponent->mMaxStepHeight = 0.5625f;
        }
    }
}