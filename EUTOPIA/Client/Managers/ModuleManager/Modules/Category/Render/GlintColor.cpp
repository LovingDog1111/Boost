#include "GlintColor.h"

// Glint 48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 55 41 56 41 57 48 8B EC 48 83 EC 40 4C

// GLINT_COLOR  48 89 5C 24 10 48 89 7C 24 18 55 48 8D AC 24 10 FF FF FF 48 81 EC F0 01 00 00 48 8B
// 05 ?? ?? ?? ?? 48 33 C4 48 89 85 E8ool isGlint = false; 00 00 00 0F 57 C0 0F 11 85 B8 00 00 00 0F
// 11 85 C8 00 00 00 0F 11 85 D8 00 00 00 48 8D 05 ?? ?? ?? ?? 48 89 44 24 30 48 C7 44 24 38 0F

GlintColor::GlintColor() : Module("GlintColor", "NULL", Category::RENDER) {
    registerSetting(new BoolSetting("Glint", "Enable glint effect", &isGlint, isGlint));
    registerSetting(
        new ColorSetting("Glint Color", "Color for glint effect", &glintColor, glintColor, true));
    ClientInstance* client = Game.getClientInstance();
    if(client)
        glintColorPtr = reinterpret_cast<Vec3<float>*>((uintptr_t)client + 0x8C);
}

void GlintColor::onDisable() {
    if(glintColorPtr != nullptr)
        *glintColorPtr = Vec3<float>(0.38f, 0.19f, 0.60799998f);
}