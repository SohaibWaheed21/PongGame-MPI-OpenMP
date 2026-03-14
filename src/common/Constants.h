#pragma once

namespace Constants {
    // Modes
    constexpr bool BENCHMARK_MODE = false;
    constexpr int BENCHMARK_BALL_COUNT = 200;

    // Game dimensions
    constexpr int GAME_WIDTH = 1000;
    constexpr int GAME_HEIGHT = static_cast<int>(GAME_WIDTH * 0.555);

    // Ball settings
    constexpr int NUM_BALLS = BENCHMARK_MODE ? BENCHMARK_BALL_COUNT : 4;
    constexpr float BALL_DIAMETER = 20.f;
    constexpr float BALL_RADIUS = BALL_DIAMETER / 2.f;

    // Paddle settings
    constexpr float PADDLE_WIDTH = 25.f;
    constexpr float PADDLE_HEIGHT = 100.f;

    // Physics settings
    constexpr float BASE_BALL_SPEED = 45.f;
    constexpr int COLLISION_COOLDOWN = 10; // frames

    // Game timer
    constexpr int GAME_DURATION_SECONDS = 60;

    // Frame rate
    constexpr int TARGET_FPS = 60;
    constexpr float FIXED_DT = 1.f / TARGET_FPS;

    // Debug logging
    constexpr bool DEBUG_LOGGING = false;
    constexpr int DEBUG_LOG_EVERY_N_TICKS = 60;
}
