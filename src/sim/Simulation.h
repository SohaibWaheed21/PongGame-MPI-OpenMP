#pragma once
#include "BallState.h"
#include "PaddleState.h"
#include "Events.h"
#include "..\common\Constants.h"
#include <vector>

class Simulation {
public:
    // Simulate one ball for one tick
    // Returns events generated (hit, score)
    static std::vector<GameEvent> simulateBallTick(
        BallState& ball,
        const PaddleState& paddle1,
        const PaddleState& paddle2,
        float gameWidth,
        float gameHeight,
        float dt
    );
    
    // Initialize ball state from ball ID and rarity type
    static void initializeBallState(BallState& ball, int ballId, int rarityType, float startX, float startY, float radius);
    
    // Respawn ball (reset position/velocity, keep rarity)
    static void respawnBall(BallState& ball, float startX, float startY);
};
