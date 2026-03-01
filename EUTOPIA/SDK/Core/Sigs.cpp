#include "MemoryUtil.h"
#include "Sigs.h"
uintptr_t Sigs::Actor_setPos = 0;
uintptr_t Sigs::Actor_setPos1 = 0;


void Sigs::initializee() {
    // 1.21.90
    Actor_setPos = MemoryUtil::findSignature("48 89 5C 24 10 48 89 6C 24 20 56 57 41 56 48 83 EC 30 48 8B DA");
    Actor_setPos1 = MemoryUtil::findSignature("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D 68 B9");

}