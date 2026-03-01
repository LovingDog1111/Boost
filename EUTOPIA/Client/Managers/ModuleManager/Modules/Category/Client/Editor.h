#pragma once
#include "../../ModuleBase/Module.h"
#include <functional>
#include <vector>
#include <map>

struct WidgetInfo {
    std::string id;
    Vec2<float> position;
    float size;
    bool isDragging = false;
    bool isResizing = false;
    Vec2<float> dragOffset;
    Vec2<float> resizeStart;
    float originalSize;
    
    std::function<void(Vec2<float>, float)> onUpdate;
    
    WidgetInfo(const std::string& id, Vec2<float> pos, float size, std::function<void(Vec2<float>, float)> callback = nullptr)
        : id(id), position(pos), size(size), onUpdate(callback) {}
};

class Editor : public Module {
private:
    std::vector<WidgetInfo> widgets;
    WidgetInfo* selectedWidget;
    
    // 内置的默认widget配置
    std::map<std::string, std::pair<Vec2<float>, float>> defaultWidgetConfigs;

    Vec2<float> mousePos;
    Vec2<float> oldMousePos;
    Vec2<float> mouseDelta;
    bool isLeftClickDown = false;
    bool isRightClickDown = false;
    bool isHoldingLeftClick = false;
    bool isHoldingRightClick = false;

    WidgetInfo* draggingWidget = nullptr;
    WidgetInfo* resizingWidget = nullptr;

public:
    Editor();
    
    void onEnable() override;
    void onDisable() override;
    void onD2DRender() override;
    void onMCRender(MinecraftUIRenderContext* renderCtx) override;
    
    void onKeyUpdate(int key, bool isDown) override;
    void onMouseUpdate(Vec2<float> mousePosA, char mouseButton, char isDown);
    
    void registerWidget(const std::string& id, std::function<void(Vec2<float>, float)> callback = nullptr);
    void unregisterWidget(const std::string& id);
    WidgetInfo* getWidget(const std::string& id);
    Vec2<float> getWidgetPosition(const std::string& id);
    float getWidgetSize(const std::string& id);
    
    void onLoadConfig(void* conf) override;
    void onSaveConfig(void* conf) override;
    
private:
    bool isMouseNearWidgetStart(const WidgetInfo& widget, float radius = 8.0f);
    bool isMouseNearSizeSlider(const WidgetInfo& widget, float radius = 6.0f);
    void renderWidgetMarkers();
    void renderSizeSliders();
    void renderWidgetInfo();
    
    void constrainWidgetPosition(WidgetInfo* widget);
    void updateWidgetCallback(WidgetInfo* widget);
    

    std::pair<Vec2<float>, float> getDefaultWidgetConfig(const std::string& id);
};