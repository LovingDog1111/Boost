#pragma once
#include "..\Utils\Minecraft\Utils.h"
#include "Modules/Category/Movement/AutoMove.h"
#include "Modules/Category/World/Baritone.h"
#include "Modules/Category/World/Crasher.h"
#include "Modules/Category/World/Dih.h"
#include "Modules/Category/World/PortalFinder.h"
#include "Modules/Category/World/StashFinder.h"
#include "Modules/Category/Client/ClickGUI.h"
#include "Modules/Category/Misc/KillCounter.h"
#include "Modules/Category/Client/ColorsMod.h"
#include "Modules/Category/Combat/AntiHit.h"
#include "Modules/Category/Client/CustomFont.h"
#include "Modules/Category/Client/ToggleSounds.h"
#include "Modules/Category/Combat/AutoKill.h"
#include "Modules/Category/Combat/KillAuraH.h"
#include "Modules/Category/Misc/FriendJoin.h"
#include "Modules/Category/Misc/PacketCounter.h"
#include "Modules/Category/Misc/PlayerCounter.h"
#include "Modules/Category/Player/AutoArmor.h"
#include "Modules/Category/Player/TargetInfo.h"
#include "Modules/Category/Render/BlockTracer.h"
#include "Modules/Category/Render/Watermark.h"
#include "Modules/Category/Render/Hud.h"
#include "Modules/Category/Render/Arraylist.h"
#include "Modules/Category/Render/TargetHUD.h"
#include "Modules/Category/World/AntiVoid.h"
#include "Modules/Category/World/BaseBot.h"
#include "Modules/Category/World/SMTransfer.h"
#include "Modules/Category/World/WartFinder.h"
#include "Modules/ModuleBase/Module.h"
#include "Modules/Category/Movement/FastStop.h"
#include "Modules/Category/Player/AntiCrystal.h"
#include "Modules/Category/Player/AutoEat.h"
#include "Modules/Category/Player/AutoPickaxe.h"
#include "Modules/Category/Player/BlockReach.h"
#include "Modules/Category/Player/Clip.h"
#include "Modules/Category/Player/EnemyTP.h"
#include "Modules/Category/Player/MidClick.h"
#include "Modules/Category/Player/Offhand.h"
#include "Modules/Category/Player/PacketMine.h"
#include "Modules/Category/Player/PlayerInfo.h"
#include "Modules/Category/Player/PopCounter.h"
#include "Modules/Category/Player/SurfaceTP.h"
#include "Modules/Category/World/WorldScanner.h"
#include "Modules/Category/Test/TestModule.h"
#include "Modules/Category/Combat/AboveAura.h"
#include "Modules/Category/Combat/AutoClicker.h"
#include "Modules/Category/Combat/BowSpam.h"
#include "Modules/Category/Combat/Criticals.h"
#include "Modules/Category/Combat/CriticalsLB.h"
#include "Modules/Category/Combat/DamageText.h"
#include "Modules/Category/Combat/Hitbox.h"
#include "Modules/Category/Combat/InfinityAura.h"
#include "Modules/Category/Combat/Reach.h"
#include "Modules/Category/Combat/Surround.h"
#include "Modules/Category/Combat/Switcher.h"
#include "Modules/Category/Combat/TPAura.h"
#include "Modules/Category/Combat/TriggerBot.h"
#include "Modules/Category/Movement/MotionFly.h"
#include "Modules/Category/Render/HitVisuals.h"
#include "Modules/Category/Movement/AutoJump.h"
#include "Modules/Category/Movement/AutoSneak.h"
#include "Modules/Category/Movement/AutoSprint.h"
#include "Modules/Category/Movement/ElytraFly.h"
#include "Modules/Category/Movement/Fly.h"
#include "Modules/Category/Movement/FlyLB.h"
#include "Modules/Category/Movement/Jesus.h"
#include "Modules/Category/Movement/Jetpack.h"
#include "Modules/Category/Movement/NoClip.h"
#include "Modules/Category/Movement/NoSlow.h"
#include "Modules/Category/Movement/Phase.h"
#include "Modules/Category/Movement/Speed.h"
#include "Modules/Category/Movement/Spider.h"
#include "Modules/Category/Movement/Step.h"
#include "Modules/Category/Movement/Velocity.h"
#include "Modules/Category/Render/BlockHighLight.h"
#include "Modules/Category/Render/CameraTweaks.h"
#include "Modules/Category/Render/ChunkBorders.h"
#include "Modules/Category/Render/CrystalChams.h"
#include "Modules/Category/Render/CustomFov.h"
#include "Modules/Category/Render/CustomSky.h"
#include "Modules/Category/Render/DeathPos.h"
#include "Modules/Category/Render/ESP.h"
#include "Modules/Category/Render/FullBright.h"
#include "Modules/Category/Render/GlintColor.h"
#include "Modules/Category/Render/Keystrokes.h"
#include "Modules/Category/Render/NameTag.h"
#include "Modules/Category/Render/NewChunks.h"
#include "Modules/Category/Render/NoHurtCam.h"
#include "Modules/Category/Render/NoRender.h"
#include "Modules/Category/Render/NoSwing.h"
#include "Modules/Category/Render/OreESP.h"
#include "Modules/Category/Render/PortalESP.h"
#include "Modules/Category/Render/SetColor.h"
#include "Modules/Category/Render/StorageESP.h"
#include "Modules/Category/Render/Swing.h"
#include "Modules/Category/Render/Tracer.h"
#include "Modules/Category/Render/ViewModel.h"
#include "Modules/Category/World/MobTP.h"
#include "Modules/Category/World/Spawn.h"
#include "Modules/Category/World/Nuker.h"
#include "Modules/Category/World/Top.h"
#include "Modules/Category/World/TpMine.h"
#include "Modules/Category/Combat/AutoSwitch.h"
#include "Modules/Category/World/AirPlace.h"
#include "Modules/Category/World/AutoEZ.h"
#include "Modules/Category/World/AutoHub.h"
#include "Modules/Category/World/ChestTP.h"
#include "Modules/Category/World/Hub.h"
#include "Modules/Category/World/ItemTeleport.h"
#include "Modules/Category/World/NoFall.h"
#include "Modules/Category/World/OreMiner.h"
#include "Modules/Category/World/PlayerTP.h"
#include "Modules/Category/World/SMTeleport.h"
#include "Modules/Category/World/Scaffold.h"
#include "Modules/Category/World/StashTP.h"
#include "Modules/Category/World/WaterTP.h"
#include "Modules/Category/Client/notifs.h"
#include "Modules/Category/Misc/AntiBot.h"
#include "Modules/Category/Misc/Chat.h"
#include "Modules/Category/Misc/DeviceSpoofer.h"
#include "Modules/Category/Misc/Disabler.h"
#include "Modules/Category/Misc/DisablerCC.h"
#include "Modules/Category/Misc/DisablerNew.h"
#include "Modules/Category/Misc/Loader.h"
#include "Modules/Category/Misc/ModWarning.h"
#include "Modules/Category/Misc/NameProtect.h"
#include "Modules/Category/Misc/NoPacket.h"
#include "Modules/Category/Misc/PacketLogger.h"
#include "Modules/Category/Misc/PlayerJoin.h"
#include "Modules/Category/Misc/Spammer.h"
#include "Modules/Category/Misc/Timer.h"
#include "Modules/Category/Misc/AntiLagBack.h"
#include "Modules/Category/Misc/Timer2.h"
#include "Modules/Category/Misc/Twerk.h"
#include "Modules/Category/Render/Breakindicators.h"
#include "Modules/Category/Render/Freecam.h"
#include "Modules/Category/Render/MobTags.h"
#include "Modules/Category/Render/NoCameraClip.h"
#include "Modules/Category/Player/PlayerScanner.h"

class ModuleManager {
   public:
    static inline std::vector<Module*> moduleList;

    static void init();
    static void minit();
    static void shutdown();

    template <typename TRet>
    static TRet* getModule() {
        for(Module* mod : moduleList) {
            TRet* result = dynamic_cast<TRet*>(mod);
            if(result == nullptr)
                continue;
            return result;
        }
        return nullptr;
    }

    static Module* getModuleByName(const std::string& name) {
        for(Module* mod : moduleList) {
            if(mod && mod->getModuleName() == name)
                return mod;
        }
        return nullptr;
    }

    static void onKeyUpdate(int key, bool isDown);
    static void onClientTick();
    static void onNormalTick(LocalPlayer* localPlayer);
    static void onLevelTick(Level* level);
    static void onUpdateRotation(LocalPlayer* localPlayer);
    static void onSendPacket(Packet* packet);
    static void onRecievePacket(Packet* packet, bool* cancel);
    static void onD2DRender();
    static void onMCRender(MinecraftUIRenderContext* renderCtx);
    static void onLevelRender();
    static void onRenderActorBefore(Actor* actor, Vec3<float>* camera, Vec3<float>* pos);
    static void onRenderActorAfter(Actor* actor);
    static void onLoadConfig(void* conf);
    static void onSaveConfig(void* conf);
    static void onRenderNameTag(Actor* actor, Vec3<float>* pos, bool unknownFlag, float delta,
                                mce::Color* color);
    static inline bool hasInitialized = false;
};