#pragma once
#include "../../Utils/MemoryUtil.h"
#include "ScreenContext.h"
#include "Tessellator.h"
#include "../Core/mce.h"

class MeshHelpers {
public:

	static void renderMeshImmediately(ScreenContext* screenContext, Tessellator* tessellator, mce::MaterialPtr* material) {
		char pad[0x58]{};//idk what the fuck is this in latest version there are perhaps 
		static auto address = MemoryUtil::getFuncFromCall(MemoryUtil::findSignature("E8 ? ? ? ? 90 48 0F BE 44 24 ? 48 FF C0 48 83 F8 ? 77 ? 48 8D 0D ? ? ? ? 8B 84 81 ? ? ? ? 48 03 C1 FF E0 48 8B 7C 24 ? 48 85 FF 74 ? BB ? ? ? ? 8B C3 F0 0F C1 47 ? 83 F8 ? 75 ? ? ? ? 48 8B CF ? ? ? FF 15 ? ? ? ? F0 0F C1 5F ? 83 FB ? 75 ? ? ? ? 48 8B CF 48 8B 40 ? FF 15 ? ? ? ? 48 8B 9C 24 ? ? ? ? 0F 28 7C 24 ? 44 0F 28 44 24 ? 48 83 C4 ? 5F C3 B9 ? ? ? ? CD ? 66 90"));
		using meshHelper_renderImm_t = void(__fastcall*)(ScreenContext*, Tessellator*, mce::MaterialPtr*, char*);
		static meshHelper_renderImm_t meshHelper_renderImm = reinterpret_cast<meshHelper_renderImm_t>(address);
		meshHelper_renderImm(screenContext, tessellator, material,pad);
	}
};