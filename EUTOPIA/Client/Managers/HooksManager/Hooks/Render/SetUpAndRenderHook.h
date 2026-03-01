#pragma once
#include "../FuncHook.h"
#include "../DirectX/PresentHook.h"
#include "../../../../../Utils/TimerUtil.h"
#include "../../../../../Utils/ConsoleUtil.h"
#include "../../../../../SDK/Render/ScreenView.h"
#include <DrawUtil.h>

class SetUpAndRenderHook : public FuncHook {
public:	
	static inline bool initlized = false;
	  static inline uintptr_t* ctxVTable;
private:
	using func_t = void(__thiscall*)(ScreenView*, MinecraftUIRenderContext*);
	static inline func_t oFunc;
    
	static void ScreenView_SetUpAndRenderCallback(ScreenView* _this, MinecraftUIRenderContext* renderCtx) {

		if(!initlized) {
			ctxVTable = *(uintptr_t**)renderCtx;
			initlized = true;
		}

		auto ctxInstance = renderCtx;
		auto ci = renderCtx->clientInstance;
        
		static ClickGUI* clickGuiMod = ModuleManager::getModule<ClickGUI>();
        static Editor* EditorMod = ModuleManager::getModule<Editor>();
		if (clickGuiMod->isEnabled()) {
			ci->releaseVMouse();
		}
		 else if( EditorMod->isEnabled()) {
			ci->releaseVMouse();
		} 
		oFunc(_this, renderCtx);

		uintptr_t* visualTree = *(uintptr_t**)((uintptr_t)(_this)+0x48);
		std::string rootControlName = *(std::string*)((uintptr_t)(visualTree)+0x28);//also works

		static std::string debugScreen = "debug_screen.debug_screen";
		if (strcmp(rootControlName.c_str(), debugScreen.c_str()) == 0) {
            DrawUtil::onRenderScreen(renderCtx);
            DrawUtil::deltaTime = _this->deltaTime;
		}
	}
public:
	SetUpAndRenderHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&ScreenView_SetUpAndRenderCallback;
	}
};
