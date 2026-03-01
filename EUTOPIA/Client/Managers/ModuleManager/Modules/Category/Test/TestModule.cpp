#include "TestModule.h"
#include "../../../../../../SDK/GlobalInstance.h"
#include <Minecraft/InvUtil.h>
#include <DrawUtil.h>
#include <Minecraft/WorldUtil.h>
#include <Minecraft/PlayerUtil.h>
#include <Minecraft/PacketUtil.h>
#include <AnimationUtil.h>
TestModule::TestModule() : Module("Test", "A Test Module", Category::CLIENT) {
	Enum1 = (EnumSetting*)registerSetting(new EnumSetting("Bool", "NULL", { "1", "2", "3" }, &EnumMode1, 0));
	Enum2 = (EnumSetting*)registerSetting(new EnumSetting("Bool2", "NULL", { "A", "B", "C" }, &EnumMode2, 0));
	Enum3 = (EnumSetting*)registerSetting(new EnumSetting("Bool3", "NULL", { "X", "Y", "Z" }, &EnumMode3, 0));
	registerSetting(new SliderSetting<int>("MyInt1", "NULL", &MyInt1, 1, 1, 100));
    registerSetting(new SliderSetting<int>("MyInt2", "NULL", &MyInt2, 1, 1, 30));
    registerSetting(new SliderSetting<int>("MyInt3", "NULL", &MyInt3, 1, 1, 30));
    registerSetting(new BoolSetting("myBool1", "NULL", &myBool1, false));
    registerSetting(new BoolSetting("myBool2", "NULL", &myBool2, false));
    registerSetting(new BoolSetting("myBool3", "NULL", &myBool3, false));
    registerSetting(new BoolSetting("myBool4", "NULL", &myBool4, false));
    registerSetting(new SliderSetting<float>("myFloat1", "NULL", &myFloat1, 0.5f, 0.0f, 2.0f));
    registerSetting(new SliderSetting<float>("myFloat2", "NULL", &myFloat2, 1.0f, 0.0f, 2.0f));
    registerSetting(new SliderSetting<float>("myFloat3", "NULL", &myFloat3, 1.5f, 0.0f, 2.0f));
    registerSetting(new ColorSetting("myColor1", "NULL", &myColor1, UIColor(255, 0, 0, 255), false));
    registerSetting(new ColorSetting("myColor2", "NULL", &myColor2, UIColor(0, 255, 0, 255), false));
    registerSetting(new ColorSetting("myColor3", "NULL", &myColor3, UIColor(0, 0, 255, 255), false));
    registerSetting(new SliderSetting<int>("TPS", "ticks per second", &timerValue, 40, 0, 200));


}
int getItemID(ItemStack* item) {
    if (item == nullptr || item->getItem() == nullptr || item == ItemStack::getEmptyItem()) {
        return -1;
    }
    return item->getItem()->mItemId;
}

int getItemInHandID() {
    auto lp = GI::getLocalPlayer();
    int slot = lp->supplies->getmSelectedSlot();
    ItemStack* item = lp->supplies->container->getItem(slot);
    if (item == nullptr || item->getItem() == nullptr || item == ItemStack::getEmptyItem()) {
        return -1;
    }
    return item->getItem()->mItemId;
}


bool isVaildItem(ItemStack* item) {// i will make it in a util later
    if (item == nullptr || item->getItem() == nullptr || item == ItemStack::getEmptyItem()) {
        return false;
    }
    return true;
}

TestModule::~TestModule()
{
}
bool TestModule::isVisible() {
    return true;
}

void TestModule::onKeyUpdate(int key, bool isDown)
{
}

void TestModule::onClientTick() {

}

int startingEmptySlot2 = -1;

int getFirstEmptySlot2() {
    auto player = GI::getLocalPlayer();
    for (int i = 0; i < 36; i++) {
        if (startingEmptySlot2 != -1 && i < startingEmptySlot2)
            continue;

        ItemStack* stack = player->supplies->container->getItem(i);
        if (!InvUtil::isVaildItem(stack)) {
            startingEmptySlot2 = i + 1;
            return i;
        }
    }

    return -1;
}

void TestModule::onNormalTick(LocalPlayer* localPlayer)
{

    if (!localPlayer)
        return;
    Vec3<float> pos = localPlayer->getPos();
    Vec3<float> feetPos = pos.add(Vec3<float>(1, -2, 1));

    if(GI::isKeyDown('C'))
Game.getLocalPlayer()->SetPosition(Game.getLocalPlayer()->getStateVectorComponent(), 0.f);
    
    int id = getItemInHandID();
    if (GI::isKeyDown('I'))
        GI::DisplayClientMessage("Item id : %d", id);

    if (GI::isKeyDown('J')) {
        int hurttime = localPlayer->getMobHurtTimeComponent()->mHurtTime;
        GI::DisplayClientMessage("hurttime : %d", hurttime);
    }

    if (GI::isKeyDown('P')) {
        int sb = InvUtil::getSelectedSlot();
        localPlayer->supplies->swapSlots(sb, 8);
    }

    if (GI::isKeyDown('V')) {
        int originalSlot = InvUtil::getSelectedSlot();
        int bestslot = -1;
        for (int i = 0; i < 9; i++) {
            ItemStack* item = InvUtil::getItem(i);
            if(InvUtil::isVaildItem(item)) {
                if(item->isBlock()) {
                    bestslot = i;
                    break;
                }
            }
        }
 /*       PacketUtil::spoofSlot(bestslot, true);*/
        InvUtil::switchSlot(bestslot);
        WorldUtil::placeBlock(feetPos.toInt(), -1);
        InvUtil::switchSlot(originalSlot);
        GI::DisplayClientMessage("Placed block at %f %f %f", feetPos.x, feetPos.y, feetPos.z);
        InvUtil::sendMobEquipment(originalSlot);

    }

    if (GI::isKeyDown('N')) {
        if (id != 1) {
            GI::DisplayClientMessage("Placing block at %f %f %f", feetPos.x, feetPos.y, feetPos.z);
            BlockPos blockPos = feetPos.toInt();
            GI::DisplayClientMessage("BlockPos: %d %d %d", blockPos.x, blockPos.y, blockPos.z);
            bool sb = WorldUtil::getBlock(feetPos.toInt())->isInteractiveBlock();
            if(!sb) {
                if(WorldUtil::isValidPlacePos(feetPos.toInt()))
                WorldUtil::placeBlock(feetPos.toInt(), -1);
                // localPlayer->gamemode->buildBlock(feetPos.toInt(), 0, true);
            }
        }
    }


    if (GI::isKeyDown('U')) {
        localPlayer->setGameType(1);
    }
    if (GI::isKeyDown('Y')) {
		localPlayer->setGameType(0);
    }

    bool IsinWaterOrRain = localPlayer->IsinWaterOrRain();
    if(GI::isKeyDown('K')) {
        if(IsinWaterOrRain) {
            GI::DisplayClientMessage("Is in water or rain");
        } else {
            GI::DisplayClientMessage("Is not in water or rain");
        }
    }
}


void TestModule::onLevelTick(Level* level) {
    if(!level)
        return;
    if(GI::isKeyDown('M')) {
        GI::getClientInstance()->minecraftSim->setSimTimer(50.f);
    } else {
        GI::getClientInstance()->minecraftSim->setSimTimer(20.f);
    }

     BlockSource* region =GI::getRegion();
     if (region == nullptr)
    	return;


     HitResult* hitResult = level->getHitResult();
     if (hitResult == nullptr)
    	return;
     if(GI::isKeyDown('J')) {
         if(hitResult->type == HitResultType::BLOCK) {
             BlockPos blockLookingAt = hitResult->blockPos;
             Block* block = region->getBlock(blockLookingAt);
             int id = block->getblockLegcy()->blockid;
             std::string blockName = block->blockLegcy->mNameSpace;
             std::string blok = block->blockLegcy->mtranslateName;
             GI::DisplayClientMessage("Block ID: %d, Name: %s, Translated Name: %s", id,
                                      blockName.c_str(), blok.c_str());
         }
     }

}
void TestModule::onUpdateRotation(LocalPlayer* localPlayer)
{
    /*if(!localPlayer)
        return;
    ActorRotationComponent* rotation = localPlayer->getActorRotationComponent();
    ActorHeadRotationComponent* headRot = localPlayer->getActorHeadRotationComponent();
    MobBodyRotationComponent* bodyRot = localPlayer->getMobBodyRotationComponent();
    if(GI::isKeyDown('Y')) {
        rotation->mYaw = 90;
        rotation->mPitch = 90;
        headRot->mHeadRot = 90;
        bodyRot->yBodyRot = 90;
    }*/
}
void TestModule::onSendPacket(Packet* packet) {

}



	std::string getEffectTimeLeftStr(MobEffectInstance* mEffectInstance) {
    uint32_t timeLeft = mEffectInstance->mDuration;
    uint32_t timeReal = (uint32_t)(timeLeft / 20);
    std::string m = std::to_string(timeReal / 60);
    std::string s;
    if(timeReal % 60 < 10)
        s += "0";
    s += std::to_string(timeReal % 60);
    return m + ":" + s;
}

void TestModule::onD2DRender() {
    auto localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;
    Vec4<float> rect1(200,200, 300, 400);
    RenderUtil::drawAdvancedGlowEffect(Vec2<float>(800, 600), 10.0f, UIColor(0, 0, 0, 120),
                                       UIColor(255, 255, 255, 120), 120.f, 1.f, false
                                      );
    RenderUtil::drawRectGlowEffect(rect1, UIColor(0, 0, 0, 80), 20.f, 1.f);
        // Ê¾Àý1£ºÊ¹ÓÃ×Ô¶¨ÒåÃû³Æ»º´æÍ¼Æ¬
 /*   Vec4<float> rect1(50, 50, 200, 200);
    RenderUtil::drawImageFromUrl(rect1, "https://img.remit.ee/api/file/BQACAgUAAyEGAASHRsPbAAJLfGiHA_EHnbxGAAG9bvplfLVOsgpjjQAC6RoAAkCiOVQHNWwc05FsYjYE.jpg", "my_avatar",1.f);
    */
    }

void TestModule::onLevelRender() {
        auto lp = GI::getLocalPlayer();

        if(!lp) {
            return;
        }

        if(GI::isKeyDown('G')) {


                Vec3<float> playerPos = lp->getPos();
            float yaw = lp->getActorRotationComponent()->mYaw;
            float time = GetTickCount64() / 1000.0f;

            // äÖÈ¾³á°ò
            UIColor wingColor = UIColor(255, 255, 255, 200);     // °×É«³á°ò
            UIColor outlineColor = UIColor(200, 200, 200, 255);  // »ÒÉ«ÂÖÀª

            DrawUtil::drawWings(playerPos.add(Vec3<float>(0, -0.5, 0)), yaw, 2.0f, time, wingColor,
                                outlineColor);
        }
    }
    

void TestModule::onMCRender(MinecraftUIRenderContext* mcr) {
    if(!mcr)
        return;
    auto lp = GI::getLocalPlayer();
    if(!lp) {
        return;
    }

}

void TestModule::onDisable()
{}

void TestModule::onReceivePacket(Packet* packet, bool* cancel) {}

void TestModule::onRenderActorBefore(Actor* entity, Vec3<float>* cameraTargetPos,
                                     Vec3<float>* pos) {
    auto lp = GI::getLocalPlayer();  ;
    if(!lp || !entity)
        return;

    Vec3<float> sb(0, 0, 0);
    if (entity->getActorTypeComponent()->id == ActorType::Player) {
        cameraTargetPos = &sb;
        pos = &sb;
    }
    
    
}

void TestModule::onRenderActorAfter(Actor* entity) {}

void TestModule::onEnable() {
}