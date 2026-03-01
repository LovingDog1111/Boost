#include "ModuleManager.h"

#include "../../Client.h"
#include "Modules/Category/Movement/Airjump.h"
#include "Modules/Category/World/TestTpmine.h"

void ModuleManager::init() {
    if(hasInitialized)
        return;

    hasInitialized = true;

    moduleList.push_back(new ClickGUI());
    moduleList.push_back(new Colors());
    moduleList.push_back(new CustomFont());
    moduleList.push_back(new AirJump());
    moduleList.push_back(new TestTpmine());
    moduleList.push_back(new ToggleSounds());
    moduleList.push_back(new notifs());
    moduleList.push_back(new AntiHit());
    moduleList.push_back(new AutoKill());
    moduleList.push_back(new AboveAura());
    moduleList.push_back(new KillAura());
    moduleList.push_back(new AutoClicker());
    moduleList.push_back(new BowSpam());
    moduleList.push_back(new Watermark());
    moduleList.push_back(new Hud());
    moduleList.push_back(new ArrayList());
    moduleList.push_back(new Criticals());
    moduleList.push_back(new CriticalsLB());
    moduleList.push_back(new DamageText());
    moduleList.push_back(new Hitbox());
    moduleList.push_back(new InfinityAura());
    moduleList.push_back(new Reach());
    moduleList.push_back(new Surround());
    moduleList.push_back(new Switcher());
    moduleList.push_back(new TPAura());
    moduleList.push_back(new TriggerBot());
    moduleList.push_back(new AutoSwitch());
    moduleList.push_back(new AutoMove());
    moduleList.push_back(new FastStop());
    moduleList.push_back(new MotionFly());
    moduleList.push_back(new AutoJump());
    moduleList.push_back(new AutoSneak());
    moduleList.push_back(new AutoSprint());
    moduleList.push_back(new ElytraFly());
    moduleList.push_back(new Fly());
    moduleList.push_back(new FlyLB());
    moduleList.push_back(new Jesus());
    moduleList.push_back(new Jetpack());
    moduleList.push_back(new NoClip());
    moduleList.push_back(new NoSlow());
    moduleList.push_back(new Phase());
    moduleList.push_back(new Speed());
    moduleList.push_back(new Spider());
    moduleList.push_back(new Step());
    moduleList.push_back(new Velocity());
    moduleList.push_back(new AutoArmor());
    moduleList.push_back(new TargetInfo());
    moduleList.push_back(new AntiCrystal());
    moduleList.push_back(new AutoEat());
    moduleList.push_back(new AutoPickaxe());
    moduleList.push_back(new BlockReach());
    moduleList.push_back(new Clip());
    moduleList.push_back(new EnemyTP());
    moduleList.push_back(new MidClick());
    moduleList.push_back(new Offhand());
    moduleList.push_back(new PacketMine());
    moduleList.push_back(new PlayerInfo());
    moduleList.push_back(new PopCounter());
    moduleList.push_back(new SurfaceTP());
    moduleList.push_back(new PlayerScanner());
    moduleList.push_back(new BlockTracer());
    moduleList.push_back(new TargetHUD());
    moduleList.push_back(new HitVisuals());
    moduleList.push_back(new BlockHighlight());
    moduleList.push_back(new CameraTweaks());
    moduleList.push_back(new ChunkBorders());
    moduleList.push_back(new CrystalChams());
    moduleList.push_back(new CustomFov());
    moduleList.push_back(new CustomSky());
    moduleList.push_back(new DeathPos());
    moduleList.push_back(new ESP());
    moduleList.push_back(new Fullbright());
    moduleList.push_back(new GlintColor());
    moduleList.push_back(new Keystrokes());
    moduleList.push_back(new NameTags());
    moduleList.push_back(new NewChunks());
    moduleList.push_back(new NoHurtCam());
    moduleList.push_back(new NoRender());
    moduleList.push_back(new NoSwing());
    moduleList.push_back(new OreESP());
    moduleList.push_back(new PortalESP());
    moduleList.push_back(new SetColor());
    moduleList.push_back(new StorageESP());
    moduleList.push_back(new Swing());
    moduleList.push_back(new Tracer());
    moduleList.push_back(new ViewModel());
    moduleList.push_back(new BreakIndicators());
    moduleList.push_back(new Freecam());
    moduleList.push_back(new MobTags());
    moduleList.push_back(new NoCameraClip());
    moduleList.push_back(new Baritone());
    moduleList.push_back(new Crasher());
    moduleList.push_back(new Dih());
    moduleList.push_back(new PortalFinder());
    moduleList.push_back(new StashFinder());
    moduleList.push_back(new AntiVoid());
    moduleList.push_back(new BaseBot());
    moduleList.push_back(new SMTransfer());
    moduleList.push_back(new WartFinder());
    moduleList.push_back(new WorldScanner());
    moduleList.push_back(new MobTP());
    moduleList.push_back(new Spawn());
    moduleList.push_back(new Nuker());
    moduleList.push_back(new Top());
    moduleList.push_back(new TpMine());
    moduleList.push_back(new AirPlace());
    moduleList.push_back(new AutoEZ());
    moduleList.push_back(new AutoHub());
    moduleList.push_back(new ChestTP());
    moduleList.push_back(new Hub());
    moduleList.push_back(new ItemTeleport());
    moduleList.push_back(new NoFall());
    moduleList.push_back(new OreMiner());
    moduleList.push_back(new PlayerTP());
    moduleList.push_back(new SMTeleport());
    moduleList.push_back(new Scaffold());
    moduleList.push_back(new StashTP());
    moduleList.push_back(new WaterTP());
    moduleList.push_back(new FriendJoin());
    moduleList.push_back(new PacketCounter());
    moduleList.push_back(new PlayerCounter());
    moduleList.push_back(new AntiBot());
    moduleList.push_back(new Chat());
    moduleList.push_back(new DeviceSpoofer());
    moduleList.push_back(new Disabler());
    moduleList.push_back(new DisablerCC());
    moduleList.push_back(new DisablerNew());
    moduleList.push_back(new Loader());
    moduleList.push_back(new ModWarning());
    moduleList.push_back(new NameProtect());
    moduleList.push_back(new NoPacket());
    moduleList.push_back(new PacketLogger());
    moduleList.push_back(new PlayerJoin());
    moduleList.push_back(new Spammer());
    moduleList.push_back(new Timer());
    moduleList.push_back(new Timer2());
    moduleList.push_back(new AntiLagBack());
    moduleList.push_back(new Twerk());
    moduleList.push_back(new TestModule());

    std::sort(moduleList.begin(), moduleList.end(),
              [](Module* lhs, Module* rhs) { return lhs->getModuleName() < rhs->getModuleName(); });

    getModule<ClickGUI>()->InitClickGUI();
}

void ModuleManager::shutdown() {
    for(auto& mod : moduleList) {
        mod->setEnabled(false);
    }
    for(auto& mod : moduleList) {
        delete mod;
        mod = nullptr;
    }
    moduleList.clear();
}

void ModuleManager::onKeyUpdate(int key, bool isDown) {
    if(!Client::isInitialized())
        return;

    for(auto& mod : moduleList) {
        mod->onKeyUpdate(key, isDown);
    }
}

void ModuleManager::onClientTick() {
    if(!Client::isInitialized())
        return;

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
            mod->onClientTick();
        }
    }
}

void ModuleManager::onNormalTick(LocalPlayer* localPlayer) {
    if(!GI::getLocalPlayer()) {
        return;
    }
    if(!Client::isInitialized() || !localPlayer)
        return;

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
            mod->onNormalTick(localPlayer);
        }
    }
}
void ModuleManager::onLevelTick(Level* level) {
    if(!Client::isInitialized())
        return;

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
            mod->onLevelTick(level);
        }
    }
}

void ModuleManager::onUpdateRotation(LocalPlayer* localPlayer) {
    if(!Client::isInitialized())
        return;

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
            mod->onUpdateRotation(localPlayer);
        }
    }
}

void ModuleManager::onSendPacket(Packet* packet) {
    if(!Client::isInitialized())
        return;
    if(!GI::getLocalPlayer())
        return;
    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
            bool ok = false;
            mod->onSendPacket(packet);
        }
    }
}

void ModuleManager::onRecievePacket(Packet* packet, bool* cancel) {
    if(!Client::isInitialized())
        return;
    if(!GI::getLocalPlayer())
        return;
    if(!packet)
        return;

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
            mod->onRecievePacket(packet, cancel);
        }
    }
}

void ModuleManager::onRenderNameTag(Actor* actor, Vec3<float>* pos, bool unknownFlag, float delta,
                                    mce::Color* color) {
    if(!Client::isInitialized() || !actor)
        return;

    NameTags* nameTags = getModule<NameTags>();
    if(nameTags && nameTags->isEnabled()) {
        return;
    }

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
        }
    }
}

void ModuleManager::onRenderActorBefore(Actor* actor, Vec3<float>* camera, Vec3<float>* pos) {
    if(!Client::isInitialized())
        return;

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
        }
    }
}
void ModuleManager::onRenderActorAfter(Actor* actor) {
    if(!Client::isInitialized())
        return;

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
        }
    }
}

void ModuleManager::onD2DRender() {
    if(!Client::isInitialized())
        return;

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
            if(mod->getModuleName() != "IRC Chat")
                mod->onD2DRender();
        }
    }
}

void ModuleManager::onMCRender(MinecraftUIRenderContext* renderCtx) {
    if(!Client::isInitialized())
        return;

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
            mod->onMCRender(renderCtx);
        }
    }
}

void ModuleManager::onLevelRender() {
    if(!Client::isInitialized())
        return;

    for(auto& mod : moduleList) {
        if(mod->isEnabled() || mod->runOnBackground()) {
            mod->onLevelRender();
        }
    }
}

void ModuleManager::onLoadConfig(void* conf) {
    if(!Client::isInitialized())
        return;

    if(!ModuleManager::hasInitialized) {
        //Client::DisplayClientMessage("Insert your key first! .key [key]", MCTF::RED);
        return;
    }

    for(auto& mod : moduleList) {
        mod->onLoadConfig(conf);
    }
}


void ModuleManager::onSaveConfig(void* conf) {
    if(!Client::isInitialized())
        return;

    if(!ModuleManager::hasInitialized) {
        //Client::DisplayClientMessage("Insert your key first! .key [key]", MCTF::RED);
        return;
    }

    for(auto& mod : moduleList) {
        mod->onSaveConfig(conf);
    }
}