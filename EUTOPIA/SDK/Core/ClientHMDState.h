#pragma once
#include "../Render/MinecraftMatrix.h"
#include <MemoryUtil.h>
class ClientHMDState {
   public:
    CLASS_MEMBER(glm::mat4, lastviewMatrix, 0xc8);
    CLASS_MEMBER(MinecraftMatrix*, lastViewMatrixAbsolute, 0x108);
    CLASS_MEMBER(glm::mat4, lastProjectionMatrix, 0x148);
    CLASS_MEMBER(MinecraftMatrix*, lastWorldMatrix, 0x188);
    CLASS_MEMBER(MinecraftMatrix*, hudMatrixPatch, 0x1C8);
    CLASS_MEMBER(MinecraftMatrix*, vrTransitionMatrixPatch, 0x208);
    // mLastLevelViewMatrix | Offset: 0x0C8 (200)
    //MatrixStack lastViewMatrix;

    //// mLastLevelViewMatrixAbsolute | Offset: 0x108 (264)
    //MatrixStack lastViewMatrixAbsolute;

    //// mLastLevelProjMatrix | Offset: 0x148 (328)
    //MatrixStack lastProjectionMatrix;

    //// mLastLevelWorldMatrix | Offset: 0x188 (392)
    //MatrixStack lastWorldMatrix;

    //// mHUDMatrixPatch | Offset: 0x1C8 (456)
    //MatrixStack hudMatrixPatch;

    //// mVRTransitionMatrixPatch | Offset: 0x208 (520)
    //MatrixStack vrTransitionMatrixPatch;

};