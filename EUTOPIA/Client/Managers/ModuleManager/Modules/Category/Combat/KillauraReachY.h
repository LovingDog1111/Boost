#pragma once
#include "../../ModuleBase/Module.h"

class KillauraReachY : public Module {
   private:
    int mode = 0;
    int modeNH = 0;
    int modeNZ = 0;
    int lastMoveTime = 30;
    int reverseTrick = 0;
    int canMove = 0;
    int timedd = 1700;
    int sleeped = 20;
    int mode2 = 1;
    float speed = 3.1f;
    float rangeLERP = 1.0f;
    float HorizontalSpeed = 1.f;
    float valuePaipy = 1.f;
    float maxTPDistance = 1.0f;
    int tpDelay = 1;
    int speedX = 14.0f;        // X axis speed
    int slowSpeedX = 14.0f;    // Slow speed for left movement
    int maxMoveSpeed = 14.0f;  // Max movement speed
    int attackdelay = 600.0f;  // Delay for attack
    int moveDistance = 14.0f;  // Movement distance
    int delayVariable = 0;     // Return delay
    int Mode = 0;              // Mode setting
    int tickCount;

    float startY = 0.f;       // Y do player ao ativar
    float stepY = 1.7f;       // incremento vertical
    bool reachedTop = false;  // chegou em 313

    float moveDirection = 1.0f;  // 1.0f for forward, -1.0f for backward
    // Variáveis novas para o controle de tempo de teleporte
    std::chrono::steady_clock::time_point lastTeleportTime;  // Armazena o tempo do último teleporte
    const std::chrono::milliseconds minDelay = std::chrono::milliseconds(510);  // 500ms de delay

    // New variables for delay limiter
    std::chrono::steady_clock::time_point lastFKeyPressTime;
    bool canTeleport = true;
    int lastFPressTime = 0;
    bool drawBox = false;
    // Helper function to check the delay limit
    bool isTeleportAllowed();

    // New variables for cooldown and press duration control
    std::chrono::steady_clock::time_point fCooldownStart;
    std::chrono::steady_clock::time_point fPressStart;
    int fCooldown = 500;       // Cooldown in milliseconds
    int fPressDuration = 200;  // Press duration in milliseconds

    // Function to track F key cooldown
    void handleFKeyCooldown();

   public:
    KillauraReachY();
    std::string getModeText() override;
    void onEnable();

    float baseY = 0.f;  // Y fixo (313)
    void onDisable();
    void onLevelRender();
};