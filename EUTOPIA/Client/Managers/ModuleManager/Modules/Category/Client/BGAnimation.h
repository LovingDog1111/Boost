#pragma once
#include "../../ModuleBase/Module.h"
#include <vector>
#include <cstdlib>
#include <ctime>

class BGAnimation : public Module {
private:
    struct Raindrop {
        Vec2<float> pos;
        float length;
        float speed;
        float alpha;
    };
    struct Halo {
        Vec2<float> pos;
        float radius;
        float alpha;
        float speed;
        float life;   // 0~1
    };

    struct Star {
        Vec2<float> pos;       // position on screen
        float radius;          // star size (pixels)
        float twinkleSpeed;    // speed of alpha oscillation
        float phase;           // phase offset for twinkle animation
        float speed;           // upward movement speed (pixels/sec)
        UIColor color;         // star color
        int type;              // 0=small, 1=medium, 2=large/bright
        float glowRadius;      // radius of glow effect
        float tailLength;      // length of the trailing effect
    };

    // Node used for spider-web style background effect
    struct WebNode {
        Vec2<float> pos; // position on screen
        Vec2<float> vel; // velocity vector
    };


    std::vector<Raindrop> raindrops;
    std::vector<Halo> halos;
    std::vector<Star> stars;
    std::vector<WebNode> webNodes; // nodes for spider-web network


    int raindropCount = 120;
    int haloCount = 15;
    int starCount = 250;
    int nebulaPatchesCount = 30;
    int nebulaLayerCount = 3;

    int maxRaindrops = 120;
    int maxHalos = 15;
    int maxStars = 250;

    // Spider-web effect parameters
    bool enableSpiderWeb = true;
    int  webNodeCount = 70;          // number of nodes
    float webNodeSpeedMul = 1.0f;    // node movement speed multiplier
    float webConnectionDist = 130.f; // max distance at which nodes connect
    float webLineWidth = 1.0f;       // width of connection lines
    float mouseRepelRadius = 120.f;  // radius within which mouse repels nodes
    float mouseRepelStrength = 300.f; // strength of repulsion

    bool enableRaindrops = true;
    bool enableHalos = true;
    bool enableStars = true;


    float raindropSpeedMul = 1.5f;
    float raindropSizeMul = 1.0f;
    float starTwinkleSpeedMul = 1.0f;
    float starSizeMul = 1.0f;
    float starSpeedMul = 1.0f;
    float haloSpeedMul = 1.0f;
    float haloSizeMul = 1.0f;

    bool enableStarTrails = true;
    float starTrailLength = 1.0f;
    bool varySizes = true;
    bool varyColors = true;


    UIColor backgroundColor = UIColor(10, 15, 40);
    bool enableBackgroundColor = true;
    int backgroundAlpha = 30;


    void ensureCounts();
    void updateEffects(float dt);
    void resetRaindrop(Raindrop& drop);
    void resetStar(Star& star);

    // Spider-web helpers
    void resetWebNode(WebNode& node);
    void updateSpiderWeb(float dt);
    void renderSpiderWeb();

    void renderStar(const Star& star, float dt);
    UIColor getStarBaseColor(int type);

    void renderBackground();

public:
    BGAnimation();

    void onEnable() override;
    void onDisable() override;
    void onClientTick() override;

    void renderOverGUI();

    // Hide from arraylist
    bool isVisible() override { return false; }
};