#pragma once
#include "../sim/BallState.h"
#include "../sim/PaddleState.h"
#include "../common/Constants.h"
#include <array>
#include <cstdint>

class WorkerLoop {
public:
    WorkerLoop();
    void run();

private:
    struct PackedEvent {
        int type = 0;
        int data1 = 0;
        int data2 = 0;
        int data3 = 0;
    };

    void initBalls();
    void receiveTickInput();
    void sendTickOutput();
    void simulateTick();
    
private:
    std::array<BallState, Constants::NUM_BALLS> m_ballStates;
    PaddleState m_paddle1;
    PaddleState m_paddle2;
    float m_gameWidth = static_cast<float>(Constants::GAME_WIDTH);
    float m_gameHeight = static_cast<float>(Constants::GAME_HEIGHT);
    float m_dt = Constants::FIXED_DT;
    std::int64_t m_tickNumber = 0;
    
    bool m_running = true;
};
