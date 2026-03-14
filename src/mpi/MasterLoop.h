#pragma once
#include <SFML/Graphics.hpp>
#include "../sim/BallState.h"
#include "../sim/PaddleState.h"
#include "../common/Constants.h"
#include "../../Paddle.h"
#include "../../Ball.h"
#include "../../Score.h"
#include <array>
#include <cstdint>

class MasterLoop {
public:
    MasterLoop();
    void run();

private:
    void initBalls();
    void respawnBall(int ballId);
    void newPaddles();
    void resetMatch();
    void togglePause();
    
    void sendTickInput();
    void receiveTickOutput();
    void processEvents();
    
    void handleInput();
    void updatePaddles();
    void draw();
    
    void endGame();
    double calculateWinRate(int playerScore, int playerHits) const;
    void showLeaderboard();
    
    // Convert between SFML and simulation state
    void syncBallStateToSFML(Ball& ball, const BallState& state);

private:
    sf::RenderWindow m_window;
    sf::Clock m_secondClock;
    
    int m_timeLeft = Constants::GAME_DURATION_SECONDS;
    bool m_running = true;
    bool m_paused = false;
    bool m_leaderboardShown = false;
    
    // SFML rendering objects
    Paddle m_p1;
    Paddle m_p2;
    std::array<Ball, Constants::NUM_BALLS> m_balls;
    Score m_score;
    
    // Simulation state
    std::array<BallState, Constants::NUM_BALLS> m_ballStates;
    PaddleState m_p1State;
    PaddleState m_p2State;
    
    // Respawn tracking
    bool m_respawnFlags[Constants::NUM_BALLS];
    float m_respawnX[Constants::NUM_BALLS];
    float m_respawnY[Constants::NUM_BALLS];
    
    int m_player1Hits = 0;
    int m_player2Hits = 0;
    std::int64_t m_tickNumber = 0;
    
    sf::Font m_font;
};
