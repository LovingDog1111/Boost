#pragma once
#include <DrawUtil.h>

#include <../xorstr.hpp>
#include <chrono>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "../Client\Managers\HooksManager\Hooks\DirectX\PresentHook.h"
#include "../SDK\Render\ScreenView.h"
#include "../Utils\ConsoleUtil.h"
#include "../Utils\TimerUtil.h"
#include "FuncHook.h"
#include "HWIDUtil.h"

const std::unordered_set<std::string> debuggerwindows = {xorstr_("x64dbg"),
                                                         xorstr_("OllyDbg"),
                                                         xorstr_("Cheat Engine"),
                                                         xorstr_("Ghidra"),
                                                         xorstr_("dnSpy"),
                                                         xorstr_("Wireshark"),
                                                         xorstr_("IDA "),
                                                         xorstr_("HxD"),
                                                         xorstr_("Process Hacker"),
                                                         xorstr_("Process Explorer"),
                                                         xorstr_("Resource Hacker"),
                                                         xorstr_("Binary Ninja"),
                                                         xorstr_("Radare2"),
                                                         xorstr_("Immunity Debugger"),
                                                         xorstr_("PE-bear"),
                                                         xorstr_("LordPE"),
                                                         xorstr_("Scylla"),
                                                         xorstr_("Detect It Easy"),
                                                         xorstr_("ReClass"),
                                                         xorstr_("xAnalyzer"),
                                                         xorstr_("GH Injector"),
                                                         xorstr_("Extreme Injector"),
                                                         xorstr_("Sandboxie"),
                                                         xorstr_("Fiddler"),
                                                         xorstr_("HTTP Debugger"),
                                                         xorstr_("WinDbg"),
                                                         xorstr_("MegaDumper"),
                                                         xorstr_("CFF Explorer"),
                                                         xorstr_("PCHunter"),
                                                         xorstr_("Sysinternals"),
                                                         xorstr_("NetLimiter"),
                                                         xorstr_("Charles"),
                                                         xorstr_("Burp Suite"),
                                                         xorstr_("Frida"),
                                                         xorstr_("Procmon"),
                                                         xorstr_("Regshot"),
                                                         xorstr_("OllyICE"),
                                                         xorstr_("DbgView"),
                                                         xorstr_("SoftICE"),
                                                         xorstr_("HookExplorer"),
                                                         xorstr_("ImportREC"),
                                                         xorstr_("Olly Advanced"),
                                                         xorstr_("TitanHide"),
                                                         xorstr_("Driver Loader"),
                                                         xorstr_("Proxifier"),
                                                         xorstr_("mitmproxy"),
                                                         xorstr_("IDA Pro"),
                                                         xorstr_("GDB"),
                                                         xorstr_("Packet Sniffer"),
                                                         xorstr_("sn0int"),
                                                         xorstr_("Maltego"),
                                                         xorstr_("r2ghidra-dec"),
                                                         xorstr_("Relyze"),
                                                         xorstr_("ILSpy"),
                                                         xorstr_("JustDecompile"),
                                                         xorstr_("dotPeek"),
                                                         xorstr_("JEB Decompiler"),
                                                         xorstr_("JD-GUI"),
                                                         xorstr_("APKTool"),
                                                         xorstr_("Androguard"),
                                                         xorstr_("Hopper"),
                                                         xorstr_("Capstone"),
                                                         xorstr_("Disassembler"),
                                                         xorstr_("Debugger")};


static unsigned __int64 timeOfLastToolCheck = -6000;

class SetUpAndRenderHook : public FuncHook {
   public:
    static inline bool initlized = false;
    static inline uintptr_t* ctxVTable;

   private:
    using func_t = void(__thiscall*)(ScreenView*, MinecraftUIRenderContext*);
    static inline func_t oFunc;
    static void ScreenView_SetUpAndRenderCallback(ScreenView* _this,
                                                  MinecraftUIRenderContext* renderCtx) {
        if(!initlized) {
            ctxVTable = *(uintptr_t**)renderCtx;
            initlized = true;
        }

        auto ci = renderCtx->clientInstance;
        static ClickGUI* clickGuiMod = ModuleManager::getModule<ClickGUI>();
        static Editor* EditorMod = ModuleManager::getModule<Editor>();
        if(clickGuiMod && clickGuiMod->isEnabled()) {
            ci->releaseVMouse();
        } else if(EditorMod && EditorMod->isEnabled()) {
            ci->releaseVMouse();
        }
        oFunc(_this, renderCtx);

        uintptr_t* visualTree = *(uintptr_t**)((uintptr_t)(_this) + 0x48);
        std::string rootControlName = *(std::string*)((uintptr_t)(visualTree) + 0x28);

        static std::string debugScreen = "debug_screen.debug_screen";
        if(strcmp(rootControlName.c_str(), debugScreen.c_str()) == 0) {
            DrawUtil::onRenderScreen(renderCtx);
            DrawUtil::deltaTime = _this->deltaTime;

            unsigned __int64 current = std::chrono::duration_cast<std::chrono::milliseconds>(
                                           std::chrono::system_clock::now().time_since_epoch())
                                           .count();
            if(timeOfLastToolCheck == -6000)
                timeOfLastToolCheck = current;
            if(current - timeOfLastToolCheck > 983) {
                timeOfLastToolCheck = current;
                std::thread toolCheckThread([]() {
                    std::unordered_set<std::string> runningTitles =
                        ConsoleUtil::getRunningWindowTitles();
                    for(const std::string& title : runningTitles) {
                        for(const std::string& suspicious : debuggerwindows) {
                            if(title.find(suspicious) != std::string::npos) {
                                std::thread shutdownThread([]() { Client::shutdown(); });
                                shutdownThread.detach();

                                return;
                            }
                        }
                    }
                });
                toolCheckThread.detach();
            }
        }
    }

   public:
    SetUpAndRenderHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&ScreenView_SetUpAndRenderCallback;
    }
};
