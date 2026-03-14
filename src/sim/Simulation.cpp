#include "Simulation.h"
#include <cmath>
#include <algorithm>
#include <random>

namespace {
    constexpr float PADDLE_BOUNCE_ANGLE_FACTOR = 0.75f;
    constexpr float HIT_SPEED_RAMP = 1.05f;
    constexpr float MAX_SPEED_MULTIPLIER = 2.0f;

    float clampf(float v, float lo, float hi) {
        return std::max(lo, std::min(hi, v));
    }

    float randomDir() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> d(0, 1);
        return d(gen) == 0 ? -1.f : 1.f;
    }

    void applyPaddleBounce(BallState& ball, const PaddleState& paddle, bool isLeftPaddle) {
        const float paddleCenter = paddle.y + (paddle.h * 0.5f);
        const float offset = clampf((ball.y - paddleCenter) / (paddle.h * 0.5f), -1.f, 1.f);

        float speed = std::sqrt(ball.vx * ball.vx + ball.vy * ball.vy);
        const float maxSpeed = Constants::BASE_BALL_SPEED * ball.speedMultiplier * MAX_SPEED_MULTIPLIER;
        speed = std::min(speed * HIT_SPEED_RAMP, maxSpeed);

        const float newVy = offset * speed * PADDLE_BOUNCE_ANGLE_FACTOR;
        const float remaining = std::max(speed * speed - newVy * newVy, 1.f);
        const float newVxMag = std::sqrt(remaining);

        ball.vy = newVy;
        ball.vx = isLeftPaddle ? newVxMag : -newVxMag;
    }
}

void Simulation::initializeBallState(BallState& ball, int ballId, int rarityType, float startX, float startY, float radius) {
    ball.id = ballId;
    ball.type = rarityType;
    ball.x = startX;
    ball.y = startY;
    ball.radius = radius;
    ball.cooldownFrames = 0;
    
    // Set attributes based on rarity type
    switch (rarityType) {
    case 0: // Common
        ball.colorR = 255; ball.colorG = 255; ball.colorB = 0; // Yellow
        ball.pointsValue = 1;
        ball.speedMultiplier = 1.0f;
        break;
    case 1: // Rare
        ball.colorR = 255; ball.colorG = 0; ball.colorB = 0; // Red
        ball.pointsValue = 3;
        ball.speedMultiplier = 1.0f;
        break;
    case 2: // Epic
        ball.colorR = 128; ball.colorG = 0; ball.colorB = 128; // Purple
        ball.pointsValue = 5;
        ball.speedMultiplier = 1.0f;
        break;
    case 3: // Legendary
        ball.colorR = 0; ball.colorG = 255; ball.colorB = 255; // Cyan
        ball.pointsValue = 10;
        ball.speedMultiplier = 1.5f;
        break;
    default:
        ball.colorR = 255; ball.colorG = 255; ball.colorB = 255; // White fallback
        ball.pointsValue = 1;
        ball.speedMultiplier = 1.0f;
        break;
    }
    
    // Set initial velocities with speed multiplier
    ball.vx = Constants::BASE_BALL_SPEED * ball.speedMultiplier * randomDir();
    ball.vy = Constants::BASE_BALL_SPEED * ball.speedMultiplier * randomDir();
}

void Simulation::respawnBall(BallState& ball, float startX, float startY) {
    ball.x = startX;
    ball.y = startY;
    ball.vx = Constants::BASE_BALL_SPEED * ball.speedMultiplier * randomDir();
    ball.vy = Constants::BASE_BALL_SPEED * ball.speedMultiplier * randomDir();
    ball.cooldownFrames = 0;
}

std::vector<GameEvent> Simulation::simulateBallTick(
    BallState& ball,
    const PaddleState& paddle1,
    const PaddleState& paddle2,
    float gameWidth,
    float gameHeight,
    float dt
) {
    std::vector<GameEvent> events;
    
    // Decrement cooldown
    ball.decrementCooldown();
    
    // Move ball
    ball.x += ball.vx * dt;
    ball.y += ball.vy * dt;
    
    // Top/bottom wall collision
    if (ball.y - ball.radius <= 0.f) {
        ball.y = ball.radius;
        ball.vy = std::abs(ball.vy);
    }
    if (ball.y + ball.radius >= gameHeight) {
        ball.y = gameHeight - ball.radius;
        ball.vy = -std::abs(ball.vy);
    }
    
    // Paddle collision (with cooldown)
    if (!ball.isInCooldown()) {
        // Check paddle 1 (left)
        if (paddle1.intersects(ball.x, ball.y, ball.radius)) {
            applyPaddleBounce(ball, paddle1, true);
            ball.resetCooldown(Constants::COLLISION_COOLDOWN);
            events.push_back(GameEvent::createHitEvent(1, ball.id));
        }
        
        // Check paddle 2 (right)
        if (paddle2.intersects(ball.x, ball.y, ball.radius)) {
            applyPaddleBounce(ball, paddle2, false);
            ball.resetCooldown(Constants::COLLISION_COOLDOWN);
            events.push_back(GameEvent::createHitEvent(2, ball.id));
        }
    }
    
    // Score detection (ball goes off left/right edge)
    if (ball.x - ball.radius <= 0.f) {
        // Player 2 scores
        events.push_back(GameEvent::createScoreEvent(2, ball.pointsValue, ball.id));
    }
    if (ball.x + ball.radius >= gameWidth) {
        // Player 1 scores
        events.push_back(GameEvent::createScoreEvent(1, ball.pointsValue, ball.id));
    }
    
    return events;
}
