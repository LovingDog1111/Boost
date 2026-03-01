#include "ModWarning.h"
#include "../../SDK/NetWork/Packets/CommandRequestPacket.h"

bool ModWarning::kickSettingEnabled = true;
std::unordered_set<std::string> ModWarning::warnedMods;
bool ModWarning::autoKickEnabled = true;

ModWarning::ModWarning()
    : Module("ModWarning", "Warns you if a mod joins (LB)", Category::MISC) {
    registerSetting(
        new BoolSetting("AutoHub", "Teleports to hub when a mod joins.", &autoKickEnabled, true));
}

void ModWarning::onNormalTick(LocalPlayer* localPlayer) {
    if(localPlayer == nullptr)
        return;

    std::vector<std::string> currentPlayerList = getCurrentPlayerList();
    static const std::unordered_set<std::string> specialMods = {"ItzRubyRose2",
                                                                "xyunami",
                                                                "ItzNotRust1",
                                                                "Modulo 7",
                                                                "iSSMYT",
                                                                "MobFox",
                                                                "snow flow7010",
                                                                "My1Legocraft",
                                                                "xPandaLadyx",
                                                                "HolyDeesus",
                                                                "itsjustclar",
                                                                "Mootburrito9652",
                                                                "slilvy",
                                                                "LiquidRhyx",
                                                                "Oreoaxolotl1",
                                                                "SloopyKatie",
                                                                "DuckyMuffins",
                                                                "GeeWhiizz",
                                                                "Eeviumm",
                                                                "Dozaiiv",
                                                                "Mylo20032868",
                                                                "ClubAmericas",
                                                                "Lazinesses",
                                                                "zBlqde1654",
                                                                "KingK6316",
                                                                "RedHeadGamer314",
                                                                "MightyPieAJR",
                                                                "Shgru",
                                                                "ineptsky3255696",
                                                                "mapajama",
                                                                "itzOreoX",
                                                                "unova1111",
                                                                "LCdestroysU",
                                                                "a1frostbite0100",
                                                                "puggy231",
                                                                "itzmay3173",
                                                                "Critly6084",
                                                                "NotRanean",
                                                                "Watermelon5418",
                                                                "Tanmanstyle",
                                                                "P0qsicle",
                                                                "BassBump8",
                                                                "allied forc3",
                                                                "SilverStream688",
                                                                "Ashara FoS",
                                                                "IzukuDekuMHA",
                                                                "XxDRXTEXxX",
                                                                "CookieChaos323",
                                                                "Wizard101",
                                                                "Jolie Morenita",
                                                                "AL4LA",
                                                                "Mr Blue II",
                                                                "HHeavennLeaa",
                                                                "Misqkii",
                                                                "x0Cyn",
                                                                "GPS82017",
                                                                "Sasuqkeet",
                                                                "QqLucaspQ",
                                                                "kittykristyy",
                                                                "simouxgboss",
                                                                "delusionaltoed",
                                                                "MyDadCraft",
                                                                "MrDestrot",
                                                                "RapidHitz",
                                                                "iKzSlayerxPvP",
                                                                "Shiny Skelly",
                                                                "HydreonGamer",
                                                                "LumiThePotat",
                                                                "kuba7747",
                                                                "xBon 27",
                                                                "PandemiA UwU",
                                                                "Necryii",
                                                                "Hydreon",
                                                                "CaseCrown",
                                                                "RabidFly",
                                                                "Soturi/Crusad3r",
                                                                "W1ngHer0",
                                                                "x0Cyn",

                                                                "LunnaRosse",
                                                                "Angqls x",
                                                                "Necrii",
                                                                "EmpNeon",
                                                                "xoAeriee",
                                                                "Ohheycreeper",
                                                                "KarmalV6",
                                                                "Nachoesx",
                                                                "Asdon505",
                                                                "xYunami",
                                                                "ItzOreo",
                                                                "IkzSlayerPvp",
                                                                "XKqizn",
                                                                "Rabbidfly",
                                                                "x0cyn",
                                                                "Aggrio",
                                                                "Sabrixna9",
                                                                "Pvkcc",
                                                                "GeeWhiizz",
                                                                "Qlucas XA",
                                                                "karmaiv",
                                                                "Echoshardss",
                                                                "VenuEquine",
                                                                "Twilightzqne",
                                                                "Mootburito9652",
                                                                "BossCailou",
                                                                "NotKeru",
                                                                "Slilval",
                                                                "Chrissel2208",
                                                                "Riiiinak",
                                                                "GSP THEPERUV",
                                                                "Modulo 7",
                                                                "Ineptsky",
                                                                "SilverStream",
                                                                "Ashara Fos",
                                                                "IzukuDekuMHA",
                                                                "Mr Blue II",
                                                                "CaseCrown",
                                                                "Crusad3r",
                                                                "W1ngHer0",
                                                                "Oman Blue",
                                                                "Vavze",
                                                                "Talib",
                                                                "Shy",
                                                                "Timy",
                                                                "GET OWNED5091",
                                                                "Fly4what",
                                                                "EstBoy",
                                                                "India sosweet",
                                                                "HydreonGamer",
                                                                "PandemiA UwU",
                                                                "xBon 27",
                                                                "kuba7747",
                                                                "LumiThePotat",
                                                                "Shiny Skelly",
                                                                "RapidHitz",
                                                                "MrDestrot",
                                                                "MyDadCraft",
                                                                "Delusionaltoad",
                                                                "Simouxgboss",
                                                                "Saduqkeet",
                                                                "QqLucaspQ",
                                                                "unova1111",
                                                                "zBlade1654",
                                                                "Oreoaxolotl1",
                                                                "LiquidRhyz",
                                                                "ItsjustClar",
                                                                "ItzRubyRose2",
                                                                "MobFox",
                                                                "puggy231",
                                                                "mapajama",
                                                                "p `0pqsicle",
                                                                "Tanmanstyle",
                                                                "BassBump8",
                                                                "allied forc3",
                                                                "Wizard101",
                                                                "Jolie Morentita",
                                                                "AL4LA",
                                                                "HHeavennLeaa",
                                                                "Mylo20032868",
                                                                "ClubAmericas",
                                                                "Lazinesses",
                                                                "KingK6316",
                                                                "RedHeadGamer314",
                                                                "MightyPieAJR",
                                                                "Dozaivv",
                                                                "Eeviumm",
                                                                "DuckyMuffins",
                                                                "HolyDeesus",
                                                                "xPandaLadyx",
                                                                "My1Legocraft",
                                                                "Snow Flow7010",
                                                                "SaltOfficialTTV",
                                                                "IccedArticus",
                                                                "Riddledlace4000"};


    static std::unordered_set<std::string> seenMods;

    for(const auto& player : currentPlayerList) {
        if(specialMods.find(player) != specialMods.end() &&
           seenMods.find(player) == seenMods.end()) {
            GI::DisplayClientMessage("%s[+]%s %s %sMod Joined!", MCTF::GREEN, MCTF::WHITE,
                                      player.c_str(), MCTF::GREEN);

            if(autoKickEnabled) {
                GI::DisplayClientMessage("%s%s%s %s%sWARNING%s: %sMod %s has joined. Kicking...",
                                          MCTF::GREEN, MCTF::BOLD, MCTF::GOLD, MCTF::RED,
                                          MCTF::BOLD, MCTF::GREEN, MCTF::GOLD, player.c_str());
                kickLocalPlayer();
                return;
            }

            warnedMods.insert(player);
            seenMods.insert(player);
        }
    }

    for(auto it = seenMods.begin(); it != seenMods.end();) {
        if(std::find(currentPlayerList.begin(), currentPlayerList.end(), *it) ==
           currentPlayerList.end()) {
            GI::DisplayClientMessage("%s[-]%s %s %sMod Left", MCTF::RED,
                                                         MCTF::WHITE, it->c_str(),
                                      MCTF::RED);
            warnedMods.erase(*it);
            it = seenMods.erase(it);
        } else {
            ++it;
        }
    }

    for(const auto& player : seenMods) {
        if(warnedMods.find(player) != warnedMods.end()) {
            GI::DisplayClientMessage("%s%s%s %s%sWARNING%s: %sMod %s still online!", MCTF::GREEN,
                                      MCTF::BOLD, MCTF::GOLD, MCTF::RED, MCTF::BOLD, MCTF::GREEN,
                                      MCTF::GOLD, player.c_str());
        }
    }
}

void ModWarning::kickLocalPlayer() {
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
}
std::vector<std::string> ModWarning::getCurrentPlayerList() {
    std::vector<std::string> playerList;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    Level* level = localPlayer ? localPlayer->level : nullptr;
    if(level) {
        auto* playerMap = level->getPlayerList();
        if(playerMap) {
            for(const auto& pair : *playerMap) {
                playerList.push_back(pair.second.name);
            }
        }
    }
    return playerList;
}

void ModWarning::toggleKickSetting() {
    autoKickEnabled = !autoKickEnabled;
    onEnable();
}
