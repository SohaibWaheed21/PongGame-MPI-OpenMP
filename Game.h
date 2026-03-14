#pragma once
#include <SFML/Graphics.hpp>
#include "Paddle.h"
#include "Ball.h"
#include "Score.h"
#include "src/sim/BallState.h"
#include "src/sim/PaddleState.h"
#include "src/common/Constants.h"
#include <array>

class Game {
public:
    Game();
    void run();

private:
    void newPaddles();
    void initBalls();
    void respawnBall(int ballId);
    void resetMatch();
    void togglePause();

    void update();
    void updateSimulation();
    void draw();

    void endGame();

    double calculateWinRate(int playerScore, int playerHits) const;

    // Convert between SFML and simulation state
    void syncPaddleStateFromSFML(PaddleState& state, const Paddle& paddle);
    void syncBallStateToSFML(Ball& ball, const BallState& state);

private:
    sf::RenderWindow m_window;
    sf::Clock m_frameClock;

    // game timer (60s)
    int m_timeLeft = Constants::GAME_DURATION_SECONDS;
    sf::Clock m_secondClock;

    bool m_running = true;
    bool m_paused = false;
    bool m_leaderboardShown = false;

    // SFML rendering objects
    Paddle m_p1;
    Paddle m_p2;
    std::array<Ball, Constants::NUM_BALLS> m_balls;
    Score m_score;

    // Simulation state (plain data)
    std::array<BallState, Constants::NUM_BALLS> m_ballStates;
    PaddleState m_p1State;
    PaddleState m_p2State;

    int m_player1Hits = 0;
    int m_player2Hits = 0;

    sf::Font m_font; // for timer + leaderboard text
};
