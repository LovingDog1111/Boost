#include "Editor.h"
#include "../../../../../../Libs/json.hpp"

#include "ClickGUI.h"
#include "../../../ModuleManager.h"
                                                                                    
Editor::Editor() : Module("Editor", "Edit UI component positions and sizes", Category::CLIENT, 88) {
    defaultWidgetConfigs["potion_effects"] = {Vec2<float>(1, 200), 1.0f};
    defaultWidgetConfigs["render_pos"] = {Vec2<float>(700, 1480), 0.88f};
    defaultWidgetConfigs["armor_hud"] = {Vec2<float>(400, 400), 1.0f};//1313 1296
    defaultWidgetConfigs["watermark"] = {Vec2<float>(14, 1386), 2.7f};
    defaultWidgetConfigs["player_info"] = {Vec2<float>(1556, 1328), 0.85f};
    defaultWidgetConfigs["player_list"] = {Vec2<float>(2374, 602), 0.8f};
}

void Editor::onEnable() {
    GI::getClientInstance()->releaseVMouse();

    selectedWidget = nullptr;
    draggingWidget = nullptr;
    resizingWidget = nullptr;

    static ClickGUI* clickGuiMod = ModuleManager::getModule<ClickGUI>();
  
    if(clickGuiMod->isEnabled()) {
        clickGuiMod->setEnabled(false);
    }
}

void Editor::onDisable() {
    GI::getClientInstance()->grabVMouse();
    selectedWidget = nullptr;
    draggingWidget = nullptr;
    resizingWidget = nullptr;
}

void Editor::onD2DRender() {
    if (!this->isEnabled()) return;
    if(GI::canUseMoveKeys()) {
        GI::getClientInstance()->releaseVMouse();
    }
    renderWidgetMarkers();
    renderSizeSliders();
    renderWidgetInfo();
}

void Editor::onKeyUpdate(int key, bool isDown) {
    if(!isEnabled()) {
        if(key == getKeybind() && isDown) {
            setEnabled(true);
        }
        return;
    }

    if(isDown) {
        if(key == getKeybind() || key == VK_ESCAPE) {
            setEnabled(false);
        }
    }
}

void Editor::onMCRender(MinecraftUIRenderContext* renderCtx) {}

void Editor::onMouseUpdate(Vec2<float> mousePosA, char mouseButton, char isDown) {
    if (!this->isEnabled()) return;
    
    static Vec2<float> oldMousePos = mousePos;

    switch(mouseButton) {
        case 0: 
            mouseDelta = mousePosA.sub(oldMousePos);
            mousePos = mousePosA;
            oldMousePos = mousePosA;
            break;
            
        case 1: 
            isLeftClickDown = isDown;
            isHoldingLeftClick = isDown;

            if (!isDown) {
                if (draggingWidget) {
                    updateWidgetCallback(draggingWidget);
                    draggingWidget->isDragging = false;
                    draggingWidget = nullptr;
                }
                if (resizingWidget) {
                    updateWidgetCallback(resizingWidget);
                    resizingWidget->isResizing = false;
                    resizingWidget = nullptr;
                }
                selectedWidget = nullptr;
            }
            break;
            
        case 2: 
            isRightClickDown = isDown;
            isHoldingRightClick = isDown;

            if (isDown) {
                selectedWidget = nullptr;
                if (draggingWidget) {
                    draggingWidget->isDragging = false;
                    draggingWidget = nullptr;
                }
                if (resizingWidget) {
                    resizingWidget->isResizing = false;
                    resizingWidget = nullptr;
                }
            }
            break;
    }

    if (draggingWidget != nullptr) {
        if (isHoldingLeftClick) {
            draggingWidget->position = mousePos.sub(draggingWidget->dragOffset);
            constrainWidgetPosition(draggingWidget);
            if (draggingWidget->onUpdate) {
                draggingWidget->onUpdate(draggingWidget->position, draggingWidget->size);
            }
        } else {
            updateWidgetCallback(draggingWidget);
            draggingWidget->isDragging = false;
            draggingWidget = nullptr;
        }
        return; 
    }

    if (resizingWidget != nullptr) {
        if (isHoldingLeftClick) {
            float deltaX = mousePos.x - resizingWidget->resizeStart.x;
            resizingWidget->size = std::max(0.1f, std::min(3.0f, resizingWidget->originalSize + deltaX * 0.01f));
            if (resizingWidget->onUpdate) {
                resizingWidget->onUpdate(resizingWidget->position, resizingWidget->size);
            }
        } else {
            updateWidgetCallback(resizingWidget);
            resizingWidget->isResizing = false;
            resizingWidget = nullptr;
        }
        return; 
    }

    if (mouseButton == 1 && isDown) {
        selectedWidget = nullptr;
        
        for (auto& widget : widgets) {
            if (isMouseNearSizeSlider(widget)) {
                selectedWidget = &widget;
                resizingWidget = &widget;
                widget.isResizing = true;
                widget.originalSize = widget.size;
                widget.resizeStart = mousePos;
                return;
            }
        }
        
        for (auto& widget : widgets) {
            if (isMouseNearWidgetStart(widget)) {
                selectedWidget = &widget;
                draggingWidget = &widget;
                widget.isDragging = true;
                widget.dragOffset = Vec2<float>(
                    mousePos.x - widget.position.x,
                    mousePos.y - widget.position.y
                );
                return;
            }
        }
    }
}

void Editor::constrainWidgetPosition(WidgetInfo* widget) {
    if (!widget) return;
    
    Vec2<float> screenSize = GI::getGuiData()->windowSizeReal; 
    
    widget->position.x = std::max(0.0f, std::min(screenSize.x, widget->position.x));
    widget->position.y = std::max(0.0f, std::min(screenSize.y, widget->position.y));
}

void Editor::updateWidgetCallback(WidgetInfo* widget) {
    if (widget && widget->onUpdate) {
        widget->onUpdate(widget->position, widget->size);
    }
}

void Editor::registerWidget(const std::string& id, std::function<void(Vec2<float>, float)> callback) {
    for (auto& widget : widgets) {
        if (widget.id == id) {
            widget.onUpdate = callback;
            return;
        }
    }
    
    auto defaultConfig = getDefaultWidgetConfig(id);
    widgets.emplace_back(id, defaultConfig.first, defaultConfig.second, callback);
}

void Editor::unregisterWidget(const std::string& id) {
    widgets.erase(
        std::remove_if(widgets.begin(), widgets.end(),
            [&id](const WidgetInfo& widget) { return widget.id == id; }),
        widgets.end()
    );
}

WidgetInfo* Editor::getWidget(const std::string& id) {
    for (auto& widget : widgets) {
        if (widget.id == id) {
            return &widget;
        }
    }
    return nullptr;
}

Vec2<float> Editor::getWidgetPosition(const std::string& id) {
    WidgetInfo* widget = getWidget(id);
    return widget ? widget->position : getDefaultWidgetConfig(id).first;
}

float Editor::getWidgetSize(const std::string& id) {
    WidgetInfo* widget = getWidget(id);
    return widget ? widget->size : getDefaultWidgetConfig(id).second;
}

std::pair<Vec2<float>, float> Editor::getDefaultWidgetConfig(const std::string& id) {
    auto it = defaultWidgetConfigs.find(id);
    if (it != defaultWidgetConfigs.end()) {
        return it->second;
    }
    return {Vec2<float>(0, 50), 1.0f};
}

bool Editor::isMouseNearWidgetStart(const WidgetInfo& widget, float radius) {
    float dx = mousePos.x - widget.position.x;
    float dy = mousePos.y - widget.position.y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

bool Editor::isMouseNearSizeSlider(const WidgetInfo& widget, float radius) {
    Vec2<float> sliderPos = Vec2<float>(
        widget.position.x,     
        widget.position.y - 25  
    );
    
    float dx = mousePos.x - sliderPos.x;
    float dy = mousePos.y - sliderPos.y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

void Editor::renderWidgetMarkers() {
    for(const auto& widget : widgets) {
        UIColor markerColor;
        if(selectedWidget && selectedWidget->id == widget.id) {
            if(widget.isDragging) {
                markerColor = UIColor(255, 255, 0, 255);  
            } else {
                markerColor = UIColor(0, 255, 255, 255);  
            }
        } else {
            markerColor = UIColor(255, 100, 100, 200);
        }

        RenderUtil::fillCircle(widget.position, markerColor, 8.0f);
        RenderUtil::drawCircle(widget.position, UIColor(255, 255, 255, 255), 10.0f, 1.5f);
    }
}

void Editor::renderSizeSliders() {
    for (const auto& widget : widgets) {
        Vec2<float> sliderPos = Vec2<float>(
            widget.position.x,       
            widget.position.y - 25 
        );
        
        UIColor sliderColor;
        if (selectedWidget && selectedWidget->id == widget.id && widget.isResizing) {
            sliderColor = UIColor(255, 0, 255, 255); 
        } else if (selectedWidget && selectedWidget->id == widget.id) {
            sliderColor = UIColor(0, 255, 255, 255);  
        } else {
            sliderColor = UIColor(100, 255, 100, 200);  
        }
            
        RenderUtil::fillCircle(sliderPos, sliderColor, 5.0f);
        RenderUtil::drawCircle(sliderPos, UIColor(255, 255, 255, 255), 7.0f, 1.0f);
        
        Vec2<float> sizeTextPos = Vec2<float>(sliderPos.x + 12, sliderPos.y - 6);
        std::string sizeText = std::to_string(widget.size).substr(0, 4);
        RenderUtil::drawText(sizeTextPos, sizeText, UIColor(255, 255, 255, 180), 0.7f);
    }
}

void Editor::renderWidgetInfo() {
    if (selectedWidget) {
        Vec2<float> winsize = GI::getGuiData()->windowSizeReal;
        Vec2<float> infoPos = Vec2<float>(winsize.x/2, 10);
        std::string status = "";
        if (selectedWidget->isDragging) {
            status = " [DRAGGING]";
        } else if (selectedWidget->isResizing) {
            status = " [RESIZING]";
        }
        
        std::string info = "Selected: " + selectedWidget->id + status +
                          " | Pos: (" + std::to_string((int)selectedWidget->position.x) + 
                          ", " + std::to_string((int)selectedWidget->position.y) + 
                          ") | Size: " + std::to_string(selectedWidget->size).substr(0, 4);
        
        float textWidth = RenderUtil::getTextWidth(info, 0.9f);
        Vec2<float> realInfoPos = Vec2<float>(
          infoPos.x-textWidth/2,infoPos.y);

        Vec4<float> bgRect = Vec4<float>(realInfoPos.x - 5, realInfoPos.y - 5, 
                                        realInfoPos.x + textWidth + 5, realInfoPos.y + 30);
        RenderUtil::fillRoundedRectangle(bgRect, UIColor(0, 0, 0, 150), 3.0f);
        
        RenderUtil::drawText(realInfoPos, info, UIColor(255, 255, 255, 255), 0.9f);
        
        Vec2<float> hintPos = Vec2<float>(10, 35);
        std::string hint = "Left Click: Drag | Right Click: Cancel | Drag Green Circle: Resize";
        RenderUtil::drawText(hintPos, hint, UIColor(200, 200, 200, 180), 0.7f);
    }
}

void Editor::onLoadConfig(void* conf) {
    using json = nlohmann::json;
    json* config = reinterpret_cast<json*>(conf);
    std::string modName = this->getModuleName();

    try {
        if(config->contains(modName)) {
            json obj = config->at(modName);
            if(obj.is_null())
                return;


            for(Setting* setting : this->getSettingList()) {
                if(obj.contains(setting->name)) {
                    if(setting->type == SettingType::KEYBIND_S) {
                        KeybindSetting* keySetting = static_cast<KeybindSetting*>(setting);
                        *keySetting->value = obj[keySetting->name].get<int>();
                    } else if(setting->type == SettingType::BOOL_S) {
                        BoolSetting* boolSetting = static_cast<BoolSetting*>(setting);
                        *boolSetting->value = obj[boolSetting->name].get<bool>();
                    }
                }
            }

            if(obj.contains("Widgets")) {
                json widgetsObj = obj["Widgets"];
                
                for(auto& [widgetId, widgetConfig] : widgetsObj.items()) {
                    bool found = false;
                    
                    for(auto& widget : widgets) {
                        if(widget.id == widgetId) {
                            if(widgetConfig.contains("Position")) {
                                widget.position.x = widgetConfig["Position"]["X"].get<float>();
                                widget.position.y = widgetConfig["Position"]["Y"].get<float>();
                            }
                            if(widgetConfig.contains("Size")) {
                                widget.size = widgetConfig["Size"].get<float>();
                            }
                            if(widget.onUpdate) {
                                widget.onUpdate(widget.position, widget.size);
                            }
                            found = true;
                            break;
                        }
                    }
                    
                    if(!found) {
                        Vec2<float> pos = getDefaultWidgetConfig(widgetId).first;
                        float size = getDefaultWidgetConfig(widgetId).second;
                        
                        if(widgetConfig.contains("Position")) {
                            pos.x = widgetConfig["Position"]["X"].get<float>();
                            pos.y = widgetConfig["Position"]["Y"].get<float>();
                        }
                        if(widgetConfig.contains("Size")) {
                            size = widgetConfig["Size"].get<float>();
                        }
                        
                        widgets.emplace_back(widgetId, pos, size, nullptr);
                    }
                }
            }
        }
    } catch(const std::exception& e) {
    }
}

void Editor::onSaveConfig(void* conf) {
    using json = nlohmann::json;
    json* currentConfig = reinterpret_cast<json*>(conf);

    try {
        json obj;

        for(Setting* setting : this->getSettingList()) {
            if(setting->type == SettingType::KEYBIND_S) {
                KeybindSetting* keySetting = static_cast<KeybindSetting*>(setting);
                obj[keySetting->name] = *keySetting->value;
            } else if(setting->type == SettingType::BOOL_S) {
                BoolSetting* boolSetting = static_cast<BoolSetting*>(setting);
                obj[boolSetting->name] = *boolSetting->value;
            }
        }
        json widgetsObj;
        for(const auto& widget : widgets) {
            json widgetObj;
            widgetObj["Position"]["X"] = widget.position.x;
            widgetObj["Position"]["Y"] = widget.position.y;
            widgetObj["Size"] = widget.size;
            widgetsObj[widget.id] = widgetObj;
        }
        obj["Widgets"] = widgetsObj;

        (*currentConfig)[this->getModuleName()] = obj;
    } catch(const std::exception& e) {
    }
}