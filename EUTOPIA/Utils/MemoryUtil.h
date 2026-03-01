#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <intrin.h>
#include <memory>
#include <string_view>
#include "Logger.h"
#include <libhat.hpp>
#include <vector>
#include <future>
#include <thread>
#include <sstream>
#include <iomanip>

#define CLASS_MEMBER(type, name, offset)																			\
__declspec(property(get = get##name, put = set##name)) type name;													\
inline type& get##name() { return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); }			\
inline void set##name(type v) { *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset) = v; }

// New Patch/NOP related macros, use MemoryUtil and logF
#define DEFINE_PATCH_FUNC(name, addr, bytes) \
        void name(bool patch) { \
            static std::vector<unsigned char> ogBytes = { bytes }; \
            static bool wasPatched = false; \
            if ((addr) == 0) { \
                logF("Failed to patch %s at 0x%llX [Invalid address]", #name, static_cast<unsigned long long>(addr)); \
                return; \
            } \
            if (patch) { \
                if (!wasPatched) { \
                    ogBytes = MemoryUtil::readBytes(addr, ogBytes.size()); \
                    MemoryUtil::writeBytes(addr, std::vector<unsigned char>{ bytes }); \
                    logF("Patched %s at 0x%llX", #name, static_cast<unsigned long long>(addr)); \
                    wasPatched = true; \
                } \
            } else { \
                if (wasPatched) { \
                    MemoryUtil::writeBytes(addr, ogBytes); \
                    logF("Unpatched %s at 0x%llX", #name, static_cast<unsigned long long>(addr)); \
                    wasPatched = false; \
                } \
            } \
        }

#define DEFINE_NOP_PATCH_FUNC(name, addr, size) \
        void name(bool patch) { \
            static std::vector<unsigned char> ogBytes; \
            static bool wasPatched = false; \
            if ((addr) == 0) { \
                logF("Failed to patch %s at 0x%llX [Invalid address]", #name, static_cast<unsigned long long>(addr)); \
                return; \
            } \
            if (patch) { \
                if (!wasPatched) { \
                    ogBytes = MemoryUtil::readBytes(addr, size); \
                    std::vector<unsigned char> bytes((size), 0x90); \
                    MemoryUtil::writeBytes(addr, bytes); \
                    logF("Patched %s at 0x%llX", #name, static_cast<unsigned long long>(addr)); \
                    wasPatched = true; \
                } \
            } else { \
                if (wasPatched) { \
                    MemoryUtil::writeBytes(addr, ogBytes); \
                    logF("Unpatched %s at 0x%llX", #name, static_cast<unsigned long long>(addr)); \
                    wasPatched = false; \
                } \
            } \
        }

#define PATCH_BYTES(addr, bytes) MemoryUtil::writeBytes(addr, bytes)

namespace MemoryUtil {
	uintptr_t getGameAddress();
	uintptr_t findSignature(std::string_view sig);
	uintptr_t** getVtableFromSig(std::string_view sig);
	int resolveOffset(std::string_view sig, int relativeOffset = 3, bool isIndex = false);

	//template <unsigned int IIdx, typename TRet, typename... TArgs>
	//inline TRet CallVFunc(void* thisptr, TArgs... argList) {
	//	using Fn = TRet(__thiscall*)(void*, decltype(argList)...);
	//	return (*static_cast<Fn**>(thisptr))[IIdx](thisptr, argList...);
	//}

	inline uintptr_t getFuncFromCall(uintptr_t address) {
		if (address == 0) 
				return 0;
		return address + 1 + 4 + *(int*)(address + 1);
}



template <typename TRet, typename... TArgs>
static TRet callFastcallWithRel(uintptr_t address, TArgs... args) {
		uintptr_t funcAddr = getFuncFromCall(address);
		return callFastcall<TRet, TArgs...>(funcAddr, args...);
}



	template <typename R, typename... Args>
	R CallFunc(void* func, Args... args) {
		return ((R(*)(Args...))func)(args...);
	}

    template<typename Ret, typename... Args>
    inline Ret callVirtualFunc(int index, void* _this, Args... args) {
        using Fn = Ret(__thiscall*)(void*, Args...);
        auto vtable = *reinterpret_cast<uintptr_t**>(_this);
        return reinterpret_cast<Fn>(vtable[index])(_this, args...);
    }

	// util used to call functions by their uintptr_t address
	template <typename R, typename... Args>
	R CallFunc(uintptr_t func, Args... args) {
		return ((R(*)(Args...))func)(args...);
	}

	template <typename TRet, typename... TArgs>
	static TRet callFastcall(void* func, TArgs... args)
	{
		using Fn = TRet(__fastcall*)(TArgs...);
		Fn f = reinterpret_cast<Fn>(func);
		return f(args...);
	}

	template <typename TRet, typename... TArgs>
	static TRet callFastcall(uintptr_t func, TArgs... args)
	{
		using Fn = TRet(__fastcall*)(TArgs...);
		Fn f = reinterpret_cast<Fn>(func);
		return f(args...);
	}



	inline void nopBytes(void* dst, unsigned int size) {
		DWORD oldprotect;
		VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
		memset(dst, 0x90, size);
		VirtualProtect(dst, size, oldprotect, &oldprotect);
	}

	inline void copyBytes(void* src, void* dst, unsigned int size) {
		DWORD oldprotect;
		VirtualProtect(src, size, PAGE_EXECUTE_READWRITE, &oldprotect);
		memcpy(dst, src, size);
		VirtualProtect(src, size, oldprotect, &oldprotect);
	}

	inline void patchBytes(void* dst, void* src, unsigned int size) {
		DWORD oldprotect;
		VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
		memcpy(dst, src, size);
		VirtualProtect(dst, size, oldprotect, &oldprotect);
	}

    // -------- New function declarations --------
    // Write bytes (overloaded)
    void writeBytes(uintptr_t ptr, const std::vector<unsigned char>& bytes, size_t size);
    void writeBytes(uintptr_t ptr, const void* bytes, size_t size);
    void writeBytes(uintptr_t ptr, const std::vector<unsigned char>& bytes);

    // Read bytes
    std::vector<unsigned char> readBytes(uintptr_t ptr, size_t size);
    void ReadBytes(void* address, void* buffer, size_t size);

    // Modify memory protection
    void setProtection(uintptr_t ptr, size_t size, DWORD protection);

    // NOP and Patch
    void NopBytes(uintptr_t address, size_t size);

    // Utility tools
    std::string bytesToHex(char* bytes, int length);
    std::vector<uintptr_t> findPattern(const std::string& pattern);

    uintptr_t findString(const std::string& str);
    uintptr_t findReference(uintptr_t address);
    std::vector<uintptr_t> findReferences(uintptr_t address);
    uintptr_t getTopOfFunction(uintptr_t address);

    int32_t GetRelativeAddress(uintptr_t address, uintptr_t target);
    uintptr_t getAddressByIndex(uintptr_t _this, int index);
    uintptr_t GetVTableFunction(void* _this, int index);
}

static const char* const KeyNames[] = {
	"Unknown",
	"VK_LBUTTON",
	"VK_RBUTTON",
	"VK_CANCEL",
	"VK_MBUTTON",
	"VK_XBUTTON1",
	"VK_XBUTTON2",
	"Unknown",
	"Backspace",
	"Tab",
	"Unknown",
	"Unknown",
	"VK_CLEAR",
	"VK_RETURN",
	"Unknown",
	"Unknown",
	"Shift",
	"Ctrl",
	"Alt",
	"VK_PAUSE",
	"VK_CAPITAL",
	"VK_KANA",
	"Unknown",
	"VK_JUNJA",
	"VK_FINAL",
	"VK_KANJI",
	"Unknown",
	"VK_ESCAPE",
	"VK_CONVERT",
	"VK_NONCONVERT",
	"VK_ACCEPT",
	"VK_MODECHANGE",
	"VK_SPACE",
	"VK_PRIOR",
	"VK_NEXT",
	"VK_END",
	"VK_HOME",
	"VK_LEFT",
	"VK_UP",
	"VK_RIGHT",
	"VK_DOWN",
	"VK_SELECT",
	"VK_PRINT",
	"VK_EXECUTE",
	"VK_SNAPSHOT",
	"Insert",
	"Delete",
	"VK_HELP",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"VK_LWIN",
	"VK_RWIN",
	"VK_APPS",
	"Unknown",
	"VK_SLEEP",
	"NUMPAD0",
	"NUMPAD1",
	"NUMPAD2",
	"NUMPAD3",
	"NUMPAD4",
	"NUMPAD5",
	"NUMPAD6",
	"NUMPAD7",
	"NUMPAD8",
	"NUMPAD9",
	"VK_MULTIPLY",
	"VK_ADD",
	"VK_SEPARATOR",
	"VK_SUBTRACT",
	"VK_DECIMAL",
	"VK_DIVIDE",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"VK_F13",
	"VK_F14",
	"VK_F15",
	"VK_F16",
	"VK_F17",
	"VK_F18",
	"VK_F19",
	"VK_F20",
	"VK_F21",
	"VK_F22",
	"VK_F23",
	"VK_F24",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"VK_NUMLOCK",
	"VK_SCROLL",
	"VK_OEM_NEC_EQUAL",
	"VK_OEM_FJ_MASSHOU",
	"VK_OEM_FJ_TOUROKU",
	"VK_OEM_FJ_LOYA",
	"VK_OEM_FJ_ROYA",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"LeftShift",
	"RightShift",
	"VK_LCONTROL",
	"VK_RCONTROL",
	"VK_LMENU",
	"VK_RMENU",
	"Browser Back",
	"Browser Forward",
	"Browser Refresh",
	"Browser Stop",
	"Browser Search",
	"Browser Favourites",
	"Browser Home",
	"Volume Mute",
	"Volume Down",
	"Volume Up",
	"Media_Next",
	"Media_Prev",
	"Media_Stop",
	"Media_Pause",
	"Mail",
	"Media",
	"App1",
	"App2",
	"Unknown",
	"Unknown",
	"OEM_1",
	"PLUS",
	"COMMA",
	"MINUS",
	"DOT",
	"OEM_2",
	"OEM_3" 
};