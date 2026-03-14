#pragma once
#include "../sim/BallState.h"
#include "../sim/PaddleState.h"
#include "../common/Constants.h"
#include <cstdint>

struct PaddleStateMsg {
    float x = 0.f;
    float y = 0.f;
    float w = 0.f;
    float h = 0.f;
};

struct BallStateMsg {
    std::int32_t id = 0;
    std::int32_t type = 0;
    std::int32_t pointsValue = 0;

    std::uint8_t colorR = 0;
    std::uint8_t colorG = 0;
    std::uint8_t colorB = 0;
    std::uint8_t colorA = 0;

    float x = 0.f;
    float y = 0.f;
    float vx = 0.f;
    float vy = 0.f;
    float radius = 0.f;
    float speedMultiplier = 0.f;

    std::int32_t cooldownFrames = 0;
};

inline PaddleStateMsg toMessage(const PaddleState& s) {
    PaddleStateMsg m;
    m.x = s.x;
    m.y = s.y;
    m.w = s.w;
    m.h = s.h;
    return m;
}

inline PaddleState fromMessage(const PaddleStateMsg& m) {
    PaddleState s;
    s.x = m.x;
    s.y = m.y;
    s.w = m.w;
    s.h = m.h;
    return s;
}

inline BallStateMsg toMessage(const BallState& s) {
    BallStateMsg m;
    m.id = s.id;
    m.type = s.type;
    m.pointsValue = s.pointsValue;
    m.colorR = s.colorR;
    m.colorG = s.colorG;
    m.colorB = s.colorB;
    m.colorA = s.colorA;
    m.x = s.x;
    m.y = s.y;
    m.vx = s.vx;
    m.vy = s.vy;
    m.radius = s.radius;
    m.speedMultiplier = s.speedMultiplier;
    m.cooldownFrames = s.cooldownFrames;
    return m;
}

inline BallState fromMessage(const BallStateMsg& m) {
    BallState s;
    s.id = m.id;
    s.type = m.type;
    s.pointsValue = m.pointsValue;
    s.colorR = m.colorR;
    s.colorG = m.colorG;
    s.colorB = m.colorB;
    s.colorA = m.colorA;
    s.x = m.x;
    s.y = m.y;
    s.vx = m.vx;
    s.vy = m.vy;
    s.radius = m.radius;
    s.speedMultiplier = m.speedMultiplier;
    s.cooldownFrames = m.cooldownFrames;
    return s;
}

// Message from Rank 0 to Rank 1 (input for physics tick)
struct TickInput {
    PaddleStateMsg paddle1;
    PaddleStateMsg paddle2;
    float gameWidth = Constants::GAME_WIDTH;
    float gameHeight = Constants::GAME_HEIGHT;
    float dt = Constants::FIXED_DT;
    std::uint8_t running = 1;
    
    // Optional: respawn flags for balls
    std::uint8_t respawnFlags[Constants::NUM_BALLS] = {0};
    float respawnX[Constants::NUM_BALLS] = {0.f};
    float respawnY[Constants::NUM_BALLS] = {0.f};
};

// Message from Rank 1 to Rank 0 (output from physics tick)
struct TickOutput {
    BallStateMsg balls[Constants::NUM_BALLS];
    
    // Allow headroom for benchmark mode and bursty collisions
    static constexpr int MAX_EVENTS = Constants::NUM_BALLS * 4;
    std::int32_t eventTypes[MAX_EVENTS]; // 0=None, 1=Hit, 2=Score
    std::int32_t eventData1[MAX_EVENTS]; // paddleId/playerId
    std::int32_t eventData2[MAX_EVENTS]; // ballId
    std::int32_t eventData3[MAX_EVENTS]; // points (for score events)
    std::int32_t eventCount = 0;
};
