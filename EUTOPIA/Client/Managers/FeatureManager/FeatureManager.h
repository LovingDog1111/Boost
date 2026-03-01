#pragma once
#include <memory>

#include "event_dispatcher.h" 

class FeatureManager {
   public:
    std::unique_ptr<nes::event_dispatcher> mDispatcher;

    void init();
    void shutdown();
};


inline std::shared_ptr<FeatureManager> gFeatureManager = std::make_shared<FeatureManager>();
