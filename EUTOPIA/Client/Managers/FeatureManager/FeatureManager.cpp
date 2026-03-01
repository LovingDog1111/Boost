

#include "FeatureManager.h"


void FeatureManager::init() {
    mDispatcher = std::make_unique<nes::event_dispatcher>();

}

void FeatureManager::shutdown() {
    mDispatcher.reset();
}