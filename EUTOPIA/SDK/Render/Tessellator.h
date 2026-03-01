#pragma once
#include "../../Utils/MemoryUtil.h"
#include "../../Utils/Maths.h"

enum class VertextFormat {
	QUAD = 1,
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	LINE_LIST,
	LINE_STRIP
};

class Tessellator {
public:
	inline void begin(VertextFormat format, int maxVertices = 0) {
		static auto  addresses = MemoryUtil::findSignature("40 53 55 48 83 EC ? 80 B9 ? ? ? ? ? 44 0F B6 D2");
		using tess_begin_t = void(__thiscall*)(Tessellator*, VertextFormat, int);
		static tess_begin_t tess_begin = reinterpret_cast<tess_begin_t>(addresses);
		tess_begin(this, format, maxVertices);
	}

	inline void vertex(float x, float y, float z) {
				static auto  addresses = MemoryUtil::getFuncFromCall(MemoryUtil::findSignature("e8 ? ? ? ? f3 0f 10 5b ? f3 0f 10 13"));
		using tess_vertex_t = void(__thiscall*)(Tessellator*, float, float, float);
		static tess_vertex_t tess_vertex = reinterpret_cast<tess_vertex_t>(addresses);
		tess_vertex(this, x, y, z);
	}

	inline void color(float r, float g, float b, float a) {
				static auto  addresses = MemoryUtil::findSignature("80 B9 ? ? ? ? ? 4C 8B C1 75");
		using tess_color_t = void(__thiscall*)(Tessellator*, float, float, float, float);
		static tess_color_t tess_color = reinterpret_cast<tess_color_t>(addresses);
		tess_color(this, r, g, b, a);
	}
};