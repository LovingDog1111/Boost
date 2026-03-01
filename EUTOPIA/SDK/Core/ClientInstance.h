#pragma once

#include "../../Utils/MemoryUtil.h"
#include "GuiData.h"
#include "MinecraftGame.h"
#include "Minecraft.h"
#include "../World/Level/BlockSource.h"
#include "../World/Actor/LocalPlayer.h"
#include "../Render/LevelRender.h"
#include "GLMatrix.h"
#include "MinecraftSim.h"
#include "../NetWork/PacketSender.h"
#include "../World/Level/Level.h"
#include "../../Utils/Maths.h"
#include "ClientHMDState.h"
#include "Options.h"
#include "../World/Level/BlockTessellator.h"
#include "../World/Level/BlockEntityRenderDispatcher.h"
#include "..\Utils\Minecraft\Utils.h"
#include "GLMatrix.h"
class ClientInstance {
   public:
    CLASS_MEMBER(MinecraftGame*, minecraftGame, 0xD0);
    CLASS_MEMBER(Minecraft*, minecraft, 0xD0);
    CLASS_MEMBER(MinecraftSim*, minecraftSim, 0xD8);
    CLASS_MEMBER(GuiData*, guiData, 0x5B8);
    CLASS_MEMBER(PacketSender*, packetSender, 0xF8);
    CLASS_MEMBER(GLMatrix, glMatrix, 0x388);
    CLASS_MEMBER(ClientHMDState*, clientHMDState, 0x5A8);
    CLASS_MEMBER(mce::Camera*, camera, 0x2C8)
    CLASS_MEMBER(BlockTessellator*, blockTessellator, 0x578)
    CLASS_MEMBER(BlockEntityRenderDispatcher*, blockEntityDispatcher, 0x580)

    ClientHMDState* getClientHMDState() {
        return MemoryUtil::callVirtualFunc<ClientHMDState*>(407,this);
    }
    BlockSource* getRegion() {
        return MemoryUtil::callVirtualFunc<BlockSource*>(30, this);
    }

    
    MinecraftSim* getMinecraftSim() {
        return *reinterpret_cast<MinecraftSim**>((uintptr_t)this + 0xD8);
    }


    LocalPlayer* getLocalPlayer() {
        return MemoryUtil::callVirtualFunc<LocalPlayer*>(31, this);
    }

    Level* getLevel() {
        return MemoryUtil::callVirtualFunc<Level*>(191, this);
    }

    void grabVMouse() {
        return MemoryUtil::callVirtualFunc<void>(347, this);
    }

    void releaseVMouse() {
        return MemoryUtil::callVirtualFunc<void>(348, this);
    }

    void playUi(const std::string& soundName, float volume,
                                float pitch) {  
        minecraftGame->playUI(soundName, volume, pitch);
    }

    LevelRenderer* getLevelRenderer() {
        return MemoryUtil::callVirtualFunc<LevelRenderer*>(216, this);
    }
    Options*getOptions() {
        return MemoryUtil::callVirtualFunc<Options*>(203, this);
    }

    bool WorldToScreen(Vec3<float> pos, Vec2<float>& screen, const Vec3<float>& origin, 
                       const Vec2<float>& fov) { // bro i dont fucking know 
        pos.x -= origin.x;
        pos.y -= origin.y;
        pos.z -= origin.z;


        if(pos.z >= 0)  
            return false;

        float displayX = 1920;  
        float displayY = 1080;

        float mX = displayX / 2.0f;
        float mY = displayY / 2.0f;

        screen.x = mX + (mX * pos.x / -pos.z * fov.x);
        screen.y = mY - (mY * pos.y / -pos.z * fov.y);

        return true;
    }



    std::string getScreenName() {
        std::string screen = "no_screen";
        // sig is better
        static auto sig = MemoryUtil::findSignature(
            "48 89 ? ? ? 48 89 ? ? ? 48 89 ? ? ? 57 48 83 EC ? 48 8B ? 48 8B ? 48 "
            "8D ? ? ? 48 8B ? ? ? ? ? FF 15 ? ? ? ? 48 8B ? 48 8B ? 48 85 ? 0F 84 "
            "? ? ? ? 80 38 ? 0F 84 ? ? ? ? 48 8B ? ? 48 85 ? 74 ? F0 FF ? ? 48 8B "
            "? 48 89 ? ? ? 48 8B ? ? 48 89 ? ? ? 48 8B ? ? 48 89 ? ? ? 48 8B ? ? "
            "48 39 ? ? 74 ? 48 8B ? ? 48 85 ? 74 ? 48 8B ? 48 8B ? 48 8B ? ? ? ? ? "
            "FF 15 ? ? ? ? EB ? 48 8D ? ? ? ? ? 48 8B ? E8 ? ? ? ? 90 BF ? ? ? ? "
            "48 85 ? 74 ? 8B C7 F0 0F ? ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? "
            "FF 15 ? ? ? ? 8B C7 F0 0F ? ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? "
            "? FF 15 ? ? ? ? 90 48 C7 44 24 28 ? ? ? ? 48 8B ? ? ? 33 DB 48 89 ? ? "
            "? 48 85 ? 74 ? 8B C7 F0 0F ? ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? "
            "FF 15 ? ? ? ? 8B C7 F0 0F ? ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? "
            "? FF 15 ? ? ? ? 48 8B ? ? ? 48 C7 44 24 38 ? ? ? ? 48 85 ? 74 ? 8B C7 "
            "F0 0F ? ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? FF 15 ? ? ? ? F0 0F "
            "? ? ? 83 FF ? 75 ? 48 8B ? 48 8B ? 48 8B ? ? FF 15 ? ? ? ? 48 8B ? 48 "
            "8B ? ? ? 48 8B ? ? ? 48 8B ? ? ? ? ? ? 48 83 C4 ? 5F C3 E8 ? ? ? ? 90 "
            "CC 48 89");
        auto fn = reinterpret_cast<std::string&(__thiscall*)(ClientInstance*, std::string&)>(sig);
        screen = fn(this, screen);
        return screen;
    }

    //inline bool WorldToScreen(Vec3<float> pos, Vec2<float>& screen) {
    //    // Quick validation of required components
    //    if(!guiData|| !getLevelRenderer()) {
    //        return false;
    //    }

    //    // Get necessary data
    //    Vec2<float> displaySize = guiData->windowSizeReal;
    //    Vec3<float> origin = getLevelRenderer()->getrenderplayer()->getorigin();
    //    Vec2<float> fov = getFov();

    //    // Get and correct the view matrix
    //    auto viewMatrix = getMatrixView();
    //    if(!viewMatrix) {
    //        return false;
    //    }

    //    std::shared_ptr<GLMatrix> correctedMatrix =
    //        std::shared_ptr<GLMatrix>(viewMatrix.correct());
    //    if(!correctedMatrix) {
    //        return false;
    //    }

    //    // Directly use GLMatrix's optimized method for conversion
    //    return correctedMatrix->OWorldToScreen(origin, pos, screen, fov, displaySize);
    //}


    // Get field of view
    Vec2<float> getFov() {
        float x = getLevelRenderer()->getrenderplayer()->getFovX();
        float y = getLevelRenderer()->getrenderplayer()->getFovY();
        return Vec2<float>(x, y);
    }
};