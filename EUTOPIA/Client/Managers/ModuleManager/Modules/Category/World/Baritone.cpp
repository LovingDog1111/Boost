#define NOMINMAX
#include "Baritone.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <vector>

#include "../../../ModuleManager.h"
#include "../../SDK/GlobalInstance.h"
#include "../../SDK/NetWork/Packets/CommandRequestPacket.h"
#include "../Client/Client.h"
#include "../Player/PacketMine.h"
#include "../SDK/Render/MeshHelpers.h"
#include "..\Client\Managers\ModuleManager\Modules\Category\World\OreMiner.h"

    // stuff for faster compile
    BlockPos targetBlockPos;
bool isPausedAtTarget = false;
float tpDelay = 3.f;
float tpTimer = 0.f;
static bool stone = false;
std::string currentActionMessage = "";
bool chests = false;
int minCluster = 1;
std::vector<BlockPos> currentVein;
float normaltimer = 20.f;
float spedup = 100.f;
bool dbgmsgs = false;
bool timersettingz;
int minChestCluster = 1;
bool classicMode = false;
bool StashBlocks;
bool canAutoBreak;
bool antilava;
bool AutoLog2;
int healthThreshold = 20;
bool antiBedrock;
bool smborderanti;
bool eatauto;

Baritone::Baritone() : Module("AutoMine", "Auto tp to blocks", Category::WORLD) {
    registerSetting(new BoolSetting("AutoBreak", "Auto break (ores only)", &canAutoBreak, true));
    registerSetting(
        new SliderSetting<float>("MineSpeed", "PacketMine speed", &pckminespeed, 1.f, 0.1f, 2.f));
    registerSetting(new BoolSetting("AutoLog", "log if health low/ danger", &AutoLog2, true));
    registerSetting(new SliderSetting<int>("Health", "Minimum health to trigger autolog",
                                           &healthThreshold, 1, 1, 20));
    registerSetting(new BoolSetting("SMBorder", "Avoid the SM border", &smborderanti, true));
    registerSetting(new BoolSetting("AntiLava", "Avoid lava", &antilava, true));
    registerSetting(new BoolSetting("AntiBedrock", "Limit y to avoid bedrock", &antiBedrock, true));
    registerSetting(new SliderSetting<int>("Range", "Scan range", &range, 80, 10, 150));
    registerSetting(new BoolSetting("Timer", "boost the speed", &timersettingz, true));
    registerSetting(new BoolSetting("Classic", "Teleport once then disable", &classicMode, false));
    registerSetting(new BoolSetting("diamond", "Mine Diamond", &mineDiamond, true));
    registerSetting(new BoolSetting("iron", "Mine Iron", &mineIron, false));
    registerSetting(new BoolSetting("gold", "Mine Gold", &mineGold, false));
    registerSetting(new BoolSetting("coal", "Mine Coal", &mineCoal, false));
    registerSetting(new BoolSetting("lapis", "Mine Lapis", &mineLapis, false));
    registerSetting(new BoolSetting("quartz", "Mine Quartz", &mineQuartz, false));
    registerSetting(new BoolSetting("redstone", "Mine Redstone", &mineRedstone, false));
    registerSetting(new BoolSetting("emerald", "Mine Emerald", &mineEmerald, false));
    registerSetting(new BoolSetting("ancientDebris", "Mine Ancient Debris", &mineAncient, false));
    // registerSetting(new BoolSetting("AutoSprint", "Automatically sprint", &autoSprint, true));
    registerSetting(
        new SliderSetting<float>("Mine Range", "Mining range", &bmineRange, 6.f, 1.f, 12.f));
    registerSetting(new SliderSetting<float>(
        "Continue Reset", "The break progress at the 2nd and more mine if using continue mode",
        &bcontinueReset, 0.f, 0.f, 1.f));
    registerSetting(new EnumSetting("Mine Type",
                                    "Mining Type\nNormal: Mines normally\nContinue: Mines the same "
                                    "block over and over again\nInstant: Instamines all blocks",
                                    {"Normal", "Continue", "Instant"}, &bmineType, 0));
    registerSetting(new EnumSetting(
        "MultiTask",
        "Multitasking mode\nNormal: When eating the client will continue mining but will only "
        "break when the player is no longer eating/doing something\nStrict: Breaking will reset "
        "when eating/bowing/using an item\nStrict2: Eating will reset when placing, breaking, "
        "attacking, using an item\nNone: Mines even if doing something",
        {"Normal", "Strict", "Strict2", "None"}, &bmultiTask, 0));
    registerSetting(new BoolSetting(
        "Switch Back",
        "Switches back to old slot after mining, this may nullify the mine on some servers",
        &bswitchBack, false));

    registerSetting(new BoolSetting("DebugMsgs", "Show debug messages", &dbgmsgs, false));
    registerSetting(new BoolSetting("ChestsLB", "Teleport to chests", &chests, false));
    // registerSetting(new BoolSetting("StashBlocks", "Teleport to stash blocks", &StashBlocks,
    // true));
    registerSetting(new SliderSetting<float>("TP Delay", "Delay before teleporting after mining",
                                             &tpDelay, 3.0f, 1.0f, 10.0f));
    registerSetting(new SliderSetting<int>("Chest size", "Minimum chest cluster size",
                                           &minChestCluster, 1, 1, 2));

    registerSetting(
        new SliderSetting<int>("Vein size", "Minimum vein size", &minCluster, 1, 1, 12));
}

static void SendAutoMineMessage(const std::string& text) {
    std::string msg = std::string(MCTF::GRAY) + "[" + MCTF::BLUE + "AutoMine" + MCTF::GRAY + "] " +
                      MCTF::WHITE + text;
    GI::DisplayClientMessage(msg.c_str());
}

static void enableAutoEat() {
    AutoEat* autoEat = ModuleManager::getModule<AutoEat>();
    if(autoEat && !autoEat->isEnabled()) {
        autoEat->setEnabled(true);
    }
}

static void disableAutoEat() {
    AutoEat* autoEat = ModuleManager::getModule<AutoEat>();
    if(autoEat && autoEat->isEnabled()) {
        autoEat->setEnabled(false);
    }
}

bool isBedrock(BlockSource* region, BlockPos bp) {
    Block* b = region->getBlock(bp);
    if(!b || !b->blockLegacy)
        return false;
    int id = b->blockLegacy->blockId;
    return id == 7 || id == 166;
}

bool isBedrockAround(BlockSource* region, BlockPos bp) {
    static std::vector<Vec3<int>> d = {{1, 0, 0},  {-1, 0, 0}, {0, 1, 0},
                                       {0, -1, 0}, {0, 0, 1},  {0, 0, -1}};
    for(auto& o : d) {
        if(isBedrock(region, bp.add(o)))
            return true;
    }
    return false;
}

// FUCK LAVA

bool isOreSafe(BlockSource* region, BlockPos bp) {
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            for(int z = -1; z <= 1; z++) {
                BlockPos check = bp.add(Vec3<int>(x, y, z));
                Block* blk = region->getBlock(check);

                if(blk && blk->blockLegacy) {
                    int id = blk->blockLegacy->blockId;

                    if(id == 10 || id == 11)
                        return false;
                }
            }
        }
    }
    return true;
}

bool isLava(Block* blk) {
    if(!blk || !blk->blockLegacy)
        return false;

    int id = blk->blockLegacy->blockId;
    return (id == 10 || id == 11);
}
void resettimertodefault() {
    if(dbgmsgs) {
        GI::DisplayClientMessage("[TPBot] [DEBUG] Setting timer to 20 ticks..");
    }
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(normaltimer);
            mc->setSimSpeed(normaltimer / 20.f);
        }
    }
}

static bool shouldWaitForAutoEat() {
    if(!eatauto)
        return false;

    AutoEat* autoEat = ModuleManager::getModule<AutoEat>();
    if(!autoEat || !autoEat->isEnabled())
        return false;

    return autoEat->isEating();
}

void autolog() {
    resettimertodefault();
    GI::DisplayClientMessage("%s[AutoMine] Health low or danger, sending to hub..", MCTF::RED);
    std::shared_ptr<Packet> packet = MinecraftPacket::createPacket(PacketID::CommandRequest);
    auto* pkt = reinterpret_cast<CommandRequestPacket*>(packet.get());

    pkt->mCommand = "/hub";
    pkt->mInternalSource = false;
    pkt->mOrigin.mType = CommandOriginType::Player;
    pkt->mOrigin.mPlayerId = 0;
    pkt->mOrigin.mRequestId = "0";
    pkt->mOrigin.mUuid = mce::UUID();

    auto client = GI::getClientInstance();
    if(!client || !client->packetSender)
        return;

    client->packetSender->sendToServer(pkt);
    GI::DisplayClientMessage("%s[AutoMine] Teleported to hub", MCTF::RED);
}

void settimertofast() {
    if(dbgmsgs) {
        GI::DisplayClientMessage("[TPBot] [DEBUG] Setting timer to 100 ticks..");
    }
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(spedup);
            mc->setSimSpeed(spedup / 20.f);
        }
    }
}

// oreBlockPos.x + 0.5f, oreBlockPos.y + 1.0f, oreBlockPos.z + 0.5f

void Baritone::onEnable() {
    GI::DisplayClientMessage("%s[%sAutoMine%s] %sEnabled", MCTF::GRAY, MCTF::BLUE, MCTF::GRAY,
                             MCTF::WHITE);

    if(!chests) {
        GI::DisplayClientMessage(
            "%s[%sAutoMine%s] %sOnly use packetmine settings if you know what you're doing!",
            MCTF::GRAY, MCTF::BLUE, MCTF::GRAY, MCTF::WHITE);
    }

    if(chests) {
        GI::DisplayClientMessage("%s[%sAutoMine%s] %sSearching for chests..", MCTF::GRAY,
                                 MCTF::BLUE, MCTF::GRAY, MCTF::WHITE);
    }

    if(eatauto) {
        GI::DisplayClientMessage("%s[%sAutoMine%s] %sAutoEat enabled, hold food!", MCTF::GRAY,
                                 MCTF::BLUE, MCTF::GRAY, MCTF::WHITE);
    }

    hasTarget = false;
    targetPos = Vec3<float>(0, 0, 0);
    currentActionMessage = "";
}

void floodFillVein(BlockSource* region, BlockPos start, int id, std::vector<BlockPos>& out) {
    std::vector<BlockPos> q = {start};
    std::unordered_set<long long> vis;
    auto key = [&](BlockPos p) {
        return (long long)p.x << 40 | (long long)p.y << 20 | (long long)p.z;
    };
    while(!q.empty()) {
        BlockPos p = q.back();
        q.pop_back();
        long long k = key(p);
        if(vis.count(k))
            continue;
        vis.insert(k);
        Block* b = region->getBlock(p);
        if(!b || b->blockLegacy->blockId != id)
            continue;
        out.push_back(p);
        q.push_back(BlockPos(p.x + 1, p.y, p.z));
        q.push_back(BlockPos(p.x - 1, p.y, p.z));
        q.push_back(BlockPos(p.x, p.y + 1, p.z));
        q.push_back(BlockPos(p.x, p.y - 1, p.z));
        q.push_back(BlockPos(p.x, p.y, p.z + 1));
        q.push_back(BlockPos(p.x, p.y, p.z - 1));
    }
}

void Baritone::drawTracerToBlock(const Vec3<float>& blockPos) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;
    Tessellator* tessellator = DrawUtil::tessellator;
    if(!tessellator)
        return;
    Vec3<float> origin = Game.getClientInstance()->getLevelRenderer()->renderplayer->origin;
    Vec3<float> start = localPlayer->getEyePos().sub(origin);
    Vec3<float> end = blockPos.sub(origin);
    DrawUtil::setColor(UIColor(255, 0, 0, 200));
    tessellator->begin(VertextFormat::LINE_LIST, 2);
    tessellator->vertex(start.x, start.y, start.z);
    tessellator->vertex(end.x, end.y, end.z);
    MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, tessellator, DrawUtil::blendMaterial);
}

void Baritone::onLevelRender() {
    if(hasTarget)
        drawTracerToBlock(targetPos);
}

void Baritone::onDisable() {
    resettimertodefault();
    if(timersettingz) {
        resettimertodefault();
    }
    OreMiner* oreMiner = ModuleManager::getModule<OreMiner>();
    if(oreMiner && oreMiner->isEnabled()) {
        oreMiner->setEnabled(false);
    }
    /*
    AutoEat* autoEat = ModuleManager::getModule<AutoEat>();
    if(autoEat && autoEat->isEnabled()) {
        autoEat->setEnabled(false);
    }*/

    GI::DisplayClientMessage("%s[%sAutoMine%s] %sDisabled", MCTF::GRAY, MCTF::BLUE, MCTF::GRAY,
                             MCTF::WHITE);
    LocalPlayer* p = GI::getLocalPlayer();
    if(p) {
        auto* move = p->getMoveInputComponent();
        if(move) {
            move->mForward = false;
            move->mBackward = false;
            move->mLeft = false;
            move->mRight = false;
            move->mMoveVector = {0.f, 0.f};
        }
        p->setSprinting(false);
    }
    hasTarget = false;
    isPausedAtTarget = false;
    tpTimer = 0.f;
    targetBlockPos = BlockPos(0, 0, 0);
    targetPos = Vec3<float>(0, 0, 0);
    currentVein.clear();
    currentActionMessage = "";
}

bool isValidOre(Block* blk, const std::vector<int>& ores) {
    if(!blk || !blk->blockLegacy)
        return false;
    int id = blk->blockLegacy->blockId;
    return std::find(ores.begin(), ores.end(), id) != ores.end();
}

void Baritone::onNormalTick(LocalPlayer* player) {
    if(!player || !player->isAlive())
        return;

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region)
        return;

    if(!this->isEnabled())
        return;
    if(AutoLog2) {
        static bool alreadyTeleported = false;

        if(!player || player->getHealth() <= 0)
            return;

        float currentHealth = player->getHealth();

        if(currentHealth <= healthThreshold) {
            if(!alreadyTeleported) {
                isPausedAtTarget = true;
                resettimertodefault();
                OreMiner* oreMiner = ModuleManager::getModule<OreMiner>();
                if(oreMiner && oreMiner->isEnabled()) {
                    oreMiner->setEnabled(false);
                }
                autolog();
                this->setEnabled(false);
                alreadyTeleported = true;  // uh xd
            }
        } else {
            alreadyTeleported = false;
        }
    }

    // detect if the player is stuck
    Vec3<float> pos = player->getPos();

    static Vec3<float> lastPos(0, 0, 0);
    static float stuckTimer = 0.f;

    float movedDist = std::sqrt((pos.x - lastPos.x) * (pos.x - lastPos.x) +
                                (pos.y - lastPos.y) * (pos.y - lastPos.y) +
                                (pos.z - lastPos.z) * (pos.z - lastPos.z));

    if(movedDist < 0.1f) {
        stuckTimer += 1.f / 20.f;
    } else {
        stuckTimer = 0.f;
    }
    lastPos = pos;

    if(chests) {
        mineDiamond = false;
        mineEmerald = false;
        mineCoal = false;
        mineGold = false;
        mineAncient = false;
        mineIron = false;
        mineLapis = false;
        minChestCluster = 1;
        minCluster = 1;
        antilava = false;
        canAutoBreak = false;
        mineRedstone = false;
        mineQuartz = false;
    }

    if(canAutoBreak) {
        OreMiner* oreMiner = ModuleManager::getModule<OreMiner>();
        if(!oreMiner || !oreMiner->isEnabled())
            oreMiner->setEnabled(true);
        oreMiner->orerange = 6;
    }

    BlockPos base((int)pos.x, (int)pos.y, (int)pos.z);

    if(antilava) {
        BlockPos playerPos((int)player->getPos().x, (int)player->getPos().y,
                           (int)player->getPos().z);

        for(int x = -1; x <= 1; x++) {
            for(int y = -1; y <= 1; y++) {
                for(int z = -1; z <= 1; z++) {
                    BlockPos adjacentPos = playerPos.add(Vec3<int>(x, y, z));
                    Block* blk = region->getBlock(adjacentPos);

                    if(isLava(blk)) {
                        GI::DisplayClientMessage("%s[%sAutoMine%s] %sDetected lava!", MCTF::GRAY,
                                                 MCTF::BLUE, MCTF::GRAY, MCTF::WHITE);
                        resettimertodefault();
                        OreMiner* oreMiner = ModuleManager::getModule<OreMiner>();
                        if(oreMiner && oreMiner->isEnabled()) {
                            oreMiner->setEnabled(false);
                        }
                        /*
                        AutoEat* autoEat = ModuleManager::getModule<AutoEat>();
                        if(autoEat && autoEat->isEnabled()) {
                            autoEat->setEnabled(false);
                        }*/

                        if(AutoLog2) {
                            Hub* hub = ModuleManager::getModule<Hub>();
                            if(hub && !hub->isEnabled()) {
                                hub->setEnabled(true);
                            }
                        }

                        this->setEnabled(false);

                        return;
                    }
                }
            }
        }
    }

    std::vector<int> ores;
    if(mineDiamond)
        ores.push_back(56);
    if(mineIron)
        ores.push_back(15);
    if(mineGold)
        ores.push_back(14);
    if(mineCoal)
        ores.push_back(16);
    if(mineQuartz)
        ores.push_back(153);
    if(mineLapis)
        ores.push_back(21);
    if(mineRedstone)
        ores.push_back(74);
    if(mineEmerald)
        ores.push_back(129);
    if(mineAncient)
        ores.push_back(526);
    if(chests)
        ores.push_back(54);
    if(stone)
        ores.insert(ores.end(), {1,  2,   3,   4,   12,  13,  18,  45,  48,  87,  88, 89,
                                 98, 103, 110, 112, 113, 114, 155, 243, 486, 487, 491});

    if(StashBlocks)
        ores.insert(ores.end(), {5, 116, 154, 145});

    if(isPausedAtTarget) {
        if(timersettingz) {
            resettimertodefault();
        }
        Block* targetBlock = region->getBlock(targetBlockPos);
        int id = targetBlock->blockLegacy->blockId;

        OreMiner* oreMiner = ModuleManager::getModule<OreMiner>();
        AutoEat* autoEat = ModuleManager::getModule<AutoEat>();

        if(oreMiner) {
            if(!chests && !oreMiner->itemCollected) {
                if(currentActionMessage != "Mining") {
                    SendAutoMineMessage("Mining..");
                    currentActionMessage = "Mining";
                }
                return;
            }
        }

        /*
        if(autoEat) {
            if(autoEat->eating) {
                if(currentActionMessage != "Eating") {
                    GI::DisplayClientMessage("[AutoMine] AutoEat is eating, waiting");
                    currentActionMessage = "Eating";
                }
                return;
            }
        }
        */

        if(stuckTimer >= 12.f) {
            SendAutoMineMessage("Stuck for too long, teleporting to next target");
            isPausedAtTarget = false;
            hasTarget = false;
            tpTimer = 0.f;
            currentActionMessage = "";
            stuckTimer = 0.f;
            return;
        }

        /*
       if(eatauto) {
            AutoEat* autoEat = ModuleManager::getModule<AutoEat>();
            if(!autoEat)
                return;


            if(!autoEat->isEnabled())
                autoEat->setEnabled(true);


            if(autoEat->isEating()) {
                if(currentActionMessage != "Eating before teleport") {
                    GI::DisplayClientMessage("%s[%sAutoMine%s] %sAttempting to eat...", MCTF::GRAY,
                                             MCTF::BLUE, MCTF::GRAY, MCTF::WHITE);
                    currentActionMessage = "Eating before teleport";
                }
                return;
            }
        }*/

        if(!(isValidOre(targetBlock, ores) || (chests && id == 54))) {
            if(tpTimer == 0.f) {
                tpTimer = tpDelay;
                if(currentActionMessage != "Teleporting to next target") {
                    std::string msg = MCTF::GRAY + std::string("[") + MCTF::BLUE + "AutoMine" +
                                      MCTF::GRAY + "] " + MCTF::WHITE +
                                      "Teleporting to next target in " + std::to_string(tpDelay) +
                                      "s...";
                    GI::DisplayClientMessage(msg.c_str());
                    currentActionMessage = "Teleporting to next target";
                }

            } else {
                tpTimer -= 1.f / 20.f;
            }
            if(tpTimer <= 0.f) {
                isPausedAtTarget = false;
                pausedBeforeTeleport = false;
                tpTimer = 0.f;
                hasTarget = false;
                currentActionMessage = "";
            }
            return;
        } else {
            tpTimer = 0.f;
            return;
        }
    }

    float closestDist = FLT_MAX;
    BlockPos closestOre(0, 0, 0);
    for(int x = -range; x <= range; x++)
        for(int y = -3; y <= 3; y++)
            for(int z = -range; z <= range; z++) {
                BlockPos bp = base.add2(x, y, z);
                if(bp.y <= 3)
                    continue;
                if(smborderanti) {
                    if(bp.x < 41 || bp.x > 1109 || bp.y > 161 || bp.z < 41 || bp.z > 1109) {
                        if(currentActionMessage != "Skipping block outside SM border") {
                            std::string msg = std::string(MCTF::GRAY) + "[" + MCTF::BLUE +
                                              "AutoMine" + MCTF::GRAY + "] " + MCTF::WHITE +
                                              "Skipping block outside SM border";
                            GI::DisplayClientMessage(msg.c_str());
                            currentActionMessage = "Skipping block outside SM border";
                        }
                        continue;
                    }
                }

                Block* blk = region->getBlock(bp);
                if(!isValidOre(blk, ores))
                    continue;

                if(antilava && !isOreSafe(region, bp)) {
                    if(dbgmsgs && currentActionMessage != "Skipping dangerous ore") {
                        SendAutoMineMessage("Ignoring ore: Lava detected nearby.");
                        currentActionMessage = "Skipping dangerous ore";
                    }
                    continue;
                }

                if(antiBedrock && isBedrockAround(region, bp)) {
                    if(currentActionMessage != "Skipping ore near bedrock") {
                        GI::DisplayClientMessage((std::string(MCTF::GRAY) + "[" + MCTF::BLUE +
                                                  "AutoMine" + MCTF::GRAY + "] " + MCTF::WHITE +
                                                  "Skipping ore near bedrock")
                                                     .c_str());
                        currentActionMessage = "Skipping ore near bedrock";
                    }
                    continue;
                }
                /*
                if(antilava && !isOreSafe(region, bp)) {
                    if(currentActionMessage != "Skipping ore near lava") {
                        GI::DisplayClientMessage((std::string(MCTF::GRAY) + "[" + MCTF::BLUE +
                                                  "AutoMine" + MCTF::GRAY + "] " + MCTF::WHITE +
                                                  "Skipping ore near lava")
                                                     .c_str());
                        currentActionMessage = "Skipping ore near lava";
                    }
                    continue;
                }*/

                if(chests && blk->blockLegacy->blockId == 54) {
                    if(bp.y < 60 || bp.y > 85)
                        continue;

                    Block* above = region->getBlock(BlockPos(bp.x, bp.y + 1, bp.z));
                    if(!above || above->blockLegacy->blockId != 0)
                        continue;
                }

                float dist =
                    std::sqrt((pos.x - bp.x) * (pos.x - bp.x) + (pos.y - bp.y) * (pos.y - bp.y) +
                              (pos.z - bp.z) * (pos.z - bp.z));
                std::vector<BlockPos> vein;
                if(chests && blk->blockLegacy->blockId == 54) {
                    floodFillVein(region, bp, 54, vein);
                    if(vein.size() < minChestCluster)
                        continue;
                }

                int id3 = blk->blockLegacy->blockId;
                if(StashBlocks && (id3 == 5 || id3 == 116 || id3 == 154 || id3 == 145)) {
                    vein.clear();
                    vein.push_back(bp);
                }

                else {
                    floodFillVein(region, bp, blk->blockLegacy->blockId, vein);
                    if(vein.size() < minCluster)
                        continue;
                }
                if(dist < closestDist) {
                    closestDist = dist;
                    closestOre = bp;
                    currentVein = vein;
                }
            }

    if(closestDist == FLT_MAX) {
        hasTarget = false;
        isPausedAtTarget = false;
        GI::DisplayClientMessage((std::string(MCTF::GRAY) + "[" + MCTF::BLUE + "AutoMine" +
                                  MCTF::GRAY + "] " + MCTF::RED + "No blocks found")
                                     .c_str());
        OreMiner* oreMiner = ModuleManager::getModule<OreMiner>();
        if(oreMiner && oreMiner->isEnabled()) {
            oreMiner->setEnabled(false);
        }
        this->setEnabled(false);
        currentActionMessage = "";
        return;
    }

    targetBlockPos = closestOre;
    if(chests && region->getBlock(closestOre)->blockLegacy->blockId == 54) {
        targetPos = closestOre.toFloat().add2(0.5f, 1.5f, 0.5f);
    } else if(StashBlocks && (region->getBlock(closestOre)->blockLegacy->blockId == 5 ||
                              region->getBlock(closestOre)->blockLegacy->blockId == 116 ||
                              region->getBlock(closestOre)->blockLegacy->blockId == 154 ||
                              region->getBlock(closestOre)->blockLegacy->blockId == 145)) {
        targetPos = closestOre.toFloat().add2(0.5f, 1.5f, 0.5f);

    } else {
        targetPos = closestOre.toFloat().add2(0.5f, 0.5f, 0.5f);
    }
    hasTarget = true;

    float pauseDistance =
        ((chests && region->getBlock(closestOre)->blockLegacy->blockId == 54) ||
         (StashBlocks && (region->getBlock(closestOre)->blockLegacy->blockId == 5 ||
                          region->getBlock(closestOre)->blockLegacy->blockId == 116 ||
                          region->getBlock(closestOre)->blockLegacy->blockId == 154 ||
                          region->getBlock(closestOre)->blockLegacy->blockId == 145)))
            ? 2.2f
            : 1.0f;

    if(closestDist < pauseDistance) {
        isPausedAtTarget = true;

        if(classicMode) {
            SendAutoMineMessage("Reached target, disabling AutoMine (Classic Mode)");
            resettimertodefault();
            OreMiner* oreMiner = ModuleManager::getModule<OreMiner>();
            if(oreMiner && oreMiner->isEnabled()) {
                oreMiner->setEnabled(false);
            }
            this->setEnabled(false);
        }

        return;
    } else {
        if(currentActionMessage != "Moving to target") {
            std::string msg =
                std::string(MCTF::GRAY) + "[" + MCTF::BLUE + "AutoMine" + MCTF::GRAY + "] " +
                MCTF::WHITE + "Moving to target at " + std::to_string(targetBlockPos.x) + "," +
                std::to_string(targetBlockPos.y) + "," + std::to_string(targetBlockPos.z);

            GI::DisplayClientMessage(msg.c_str());
            currentActionMessage = "Moving to target";
        }
    }

    Vec2<float> rot = pos.CalcAngle(targetPos);
    if(auto* rotComp = player->getActorRotationComponent()) {
        rotComp->mYaw = rot.y;
        rotComp->mPitch = rot.x;
        rotComp->mOldYaw = rot.y;
        rotComp->mOldPitch = rot.x;
    }
    if(auto* headComp = player->getActorHeadRotationComponent())
        headComp->mHeadRot = rot.y;
    if(auto* bodyComp = player->getMobBodyRotationComponent())
        bodyComp->yBodyRot = rot.y;

    Vec3<float> delta = targetPos.sub(pos);
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    float step = std::min(1.5f, dist);
    Vec3<float> movement = delta * (step / dist);
    if(timersettingz) {
        settimertofast();
    }
    player->lerpMotion(movement);
    AABB a = player->getAABB(true);
    a.lower = a.lower.add(movement);
    a.upper = a.upper.add(movement);
    player->setAABB(a);
}