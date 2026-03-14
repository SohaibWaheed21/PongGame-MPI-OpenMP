#include "MasterLoop.h"
#include "Messages.h"
#include "../sim/Simulation.h"
#include "MpiCompat.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <vector>

static float randomBallY() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(Constants::BALL_RADIUS, Constants::GAME_HEIGHT - Constants::BALL_RADIUS);
    return dist(gen);
}

static float randomSpawnYWithSpacing(const std::vector<float>& existingYs, float minDistance) {
    for (int attempt = 0; attempt < 64; ++attempt) {
        const float candidate = randomBallY();
        bool valid = true;
        for (float y : existingYs) {
            if (std::abs(y - candidate) < minDistance) {
                valid = false;
                break;
            }
        }
        if (valid) {
            return candidate;
        }
    }
    return randomBallY();
}

MasterLoop::MasterLoop()
    : m_window(sf::VideoMode(Constants::GAME_WIDTH, Constants::GAME_HEIGHT), "PONG GAME - MPI Master")
    , m_p1(0.f, (Constants::GAME_HEIGHT / 2.f) - (Constants::PADDLE_HEIGHT / 2.f), Constants::PADDLE_WIDTH, Constants::PADDLE_HEIGHT, 1)
    , m_p2(Constants::GAME_WIDTH - Constants::PADDLE_WIDTH, (Constants::GAME_HEIGHT / 2.f) - (Constants::PADDLE_HEIGHT / 2.f), Constants::PADDLE_WIDTH, Constants::PADDLE_HEIGHT, 2)
    , m_score(static_cast<float>(Constants::GAME_WIDTH), static_cast<float>(Constants::GAME_HEIGHT)) {
    
    m_window.setFramerateLimit(Constants::TARGET_FPS);
    
    if (!m_font.loadFromFile("arial.ttf")) {
        throw std::runtime_error("Failed to load font: arial.ttf");
    }
    
    // Initialize respawn flags
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        m_respawnFlags[i] = false;
        m_respawnX[i] = 0.f;
        m_respawnY[i] = 0.f;
    }
    
    initBalls();
}

void MasterLoop::initBalls() {
    float startX = Constants::GAME_WIDTH / 2.f;
    std::vector<float> occupiedYs;
    occupiedYs.reserve(Constants::NUM_BALLS);
    const float minSpawnDistance = Constants::BALL_DIAMETER * 2.5f;
    
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        float startY = randomSpawnYWithSpacing(occupiedYs, minSpawnDistance);
        int rarityType = i % 4;
        occupiedYs.push_back(startY);
        
        // Initialize simulation state
        Simulation::initializeBallState(m_ballStates[i], i, rarityType, startX, startY, Constants::BALL_RADIUS);
        
        // Create Ball rarity enum from type
        Ball::Rarity rarity;
        switch (rarityType) {
            case 0: rarity = Ball::Rarity::Common; break;
            case 1: rarity = Ball::Rarity::Rare; break;
            case 2: rarity = Ball::Rarity::Epic; break;
            case 3: rarity = Ball::Rarity::Legendary; break;
            default: rarity = Ball::Rarity::Common; break;
        }
        
        // Initialize SFML ball for rendering
        m_balls[i] = Ball(startX - Constants::BALL_RADIUS, startY - Constants::BALL_RADIUS, Constants::BALL_DIAMETER, i, rarity);

        // Sync worker start positions on first tick
        m_respawnFlags[i] = true;
        m_respawnX[i] = startX;
        m_respawnY[i] = startY;
    }
}

void MasterLoop::respawnBall(int ballId) {
    if (ballId < 0 || ballId >= Constants::NUM_BALLS) return;
    
    float startX = Constants::GAME_WIDTH / 2.f;
    std::vector<float> occupiedYs;
    occupiedYs.reserve(Constants::NUM_BALLS - 1);
    const float minSpawnDistance = Constants::BALL_DIAMETER * 2.5f;
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        if (i == ballId) continue;
        occupiedYs.push_back(m_ballStates[i].y);
    }
    float startY = randomSpawnYWithSpacing(occupiedYs, minSpawnDistance);
    
    // Mark for respawn (will be sent to worker)
    m_respawnFlags[ballId] = true;
    m_respawnX[ballId] = startX;
    m_respawnY[ballId] = startY;
    
    // Update local state
    Simulation::respawnBall(m_ballStates[ballId], startX, startY);
    
    // Respawn SFML ball
    Ball::Rarity rarity = m_balls[ballId].rarity();
    m_balls[ballId] = Ball(startX - Constants::BALL_RADIUS, startY - Constants::BALL_RADIUS, Constants::BALL_DIAMETER, ballId, rarity);
}

void MasterLoop::resetMatch() {
    m_score.player1 = 0;
    m_score.player2 = 0;
    m_player1Hits = 0;
    m_player2Hits = 0;
    m_timeLeft = Constants::GAME_DURATION_SECONDS;
    m_secondClock.restart();
    m_running = true;
    m_paused = false;
    m_leaderboardShown = false;
    newPaddles();
    initBalls();
}

void MasterLoop::togglePause() {
    if (!m_running) {
        return;
    }
    m_paused = !m_paused;
}

void MasterLoop::newPaddles() {
    m_p1 = Paddle(0.f, (Constants::GAME_HEIGHT / 2.f) - (Constants::PADDLE_HEIGHT / 2.f), Constants::PADDLE_WIDTH, Constants::PADDLE_HEIGHT, 1);
    m_p2 = Paddle(Constants::GAME_WIDTH - Constants::PADDLE_WIDTH, (Constants::GAME_HEIGHT / 2.f) - (Constants::PADDLE_HEIGHT / 2.f), Constants::PADDLE_WIDTH, Constants::PADDLE_HEIGHT, 2);
}

void MasterLoop::sendTickInput() {
    TickInput input;
    input.gameWidth = static_cast<float>(Constants::GAME_WIDTH);
    input.gameHeight = static_cast<float>(Constants::GAME_HEIGHT);
    input.dt = Constants::FIXED_DT;
    
    // Sync paddle states
    auto b1 = m_p1.bounds();
    PaddleState p1;
    p1.x = b1.left;
    p1.y = b1.top;
    p1.w = b1.width;
    p1.h = b1.height;
    input.paddle1 = toMessage(p1);
    
    auto b2 = m_p2.bounds();
    PaddleState p2;
    p2.x = b2.left;
    p2.y = b2.top;
    p2.w = b2.width;
    p2.h = b2.height;
    input.paddle2 = toMessage(p2);
    
    input.running = m_running ? 1 : 0;
    
    // Send respawn info
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        input.respawnFlags[i] = m_respawnFlags[i] ? 1 : 0;
        input.respawnX[i] = m_respawnX[i];
        input.respawnY[i] = m_respawnY[i];
        m_respawnFlags[i] = false; // Clear after sending
    }
    
    MPI_Send(&input, sizeof(TickInput), MPI_BYTE, 1, 0, MPI_COMM_WORLD);
}

void MasterLoop::receiveTickOutput() {
    TickOutput output;
    MPI_Recv(&output, sizeof(TickOutput), MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    ++m_tickNumber;
    
    // Update ball states
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        m_ballStates[i] = fromMessage(output.balls[i]);
        syncBallStateToSFML(m_balls[i], m_ballStates[i]);
    }
    
    // Process events
    for (int i = 0; i < output.eventCount; ++i) {
        int type = output.eventTypes[i];
        int data1 = output.eventData1[i];
        int data2 = output.eventData2[i];
        int data3 = output.eventData3[i];
        
        if (type == 1) { // Hit event
            int paddleId = data1;
            if (paddleId == 1) {
                m_player1Hits++;
            } else if (paddleId == 2) {
                m_player2Hits++;
            }
        }
        else if (type == 2) { // Score event
            int playerId = data1;
            int ballId = data2;
            int points = data3;
            
            if (playerId == 1) {
                m_score.player1 += points;
            } else if (playerId == 2) {
                m_score.player2 += points;
            }
            
            // Respawn ball and reset paddles
            newPaddles();
            respawnBall(ballId);
        }
    }

    if (Constants::DEBUG_LOGGING && (m_tickNumber % Constants::DEBUG_LOG_EVERY_N_TICKS == 0)) {
        std::cout << "[rank 0] tick=" << m_tickNumber << " events=" << output.eventCount << "\n";
    }
}

void MasterLoop::syncBallStateToSFML(Ball& ball, const BallState& state) {
    ball.setPosition(state.x - state.radius, state.y - state.radius);
    ball.setXDirection(state.vx);
    ball.setYDirection(state.vy);
}

void MasterLoop::handleInput() {
    m_p1.handleInput();
    m_p2.handleInput();
}

void MasterLoop::updatePaddles() {
    m_p1.move(static_cast<float>(Constants::GAME_HEIGHT));
    m_p2.move(static_cast<float>(Constants::GAME_HEIGHT));
}

void MasterLoop::draw() {
    m_window.clear(sf::Color::Black);
    
    m_p1.draw(m_window);
    m_p2.draw(m_window);
    
    // Draw all balls
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        m_balls[i].draw(m_window);
    }
    
    m_score.draw(m_window);
    
    // Timer text
    sf::Text timerText;
    timerText.setFont(m_font);
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::White);
    timerText.setString("Time: " + std::to_string(m_timeLeft));
    timerText.setPosition(10.f, 10.f);
    m_window.draw(timerText);

    sf::Text hitsText;
    hitsText.setFont(m_font);
    hitsText.setCharacterSize(20);
    hitsText.setFillColor(sf::Color::White);
    hitsText.setString("Hits P1: " + std::to_string(m_player1Hits) +
                       "  P2: " + std::to_string(m_player2Hits) +
                       "  Total: " + std::to_string(m_player1Hits + m_player2Hits));
    hitsText.setPosition(10.f, 40.f);
    m_window.draw(hitsText);

    sf::Text controlsText;
    controlsText.setFont(m_font);
    controlsText.setCharacterSize(18);
    controlsText.setFillColor(sf::Color(220, 220, 220));
    controlsText.setString("P: Pause/Resume   R: Restart Match");
    controlsText.setPosition(10.f, 68.f);
    m_window.draw(controlsText);
    
    // Legend
    sf::Text legendText;
    legendText.setFont(m_font);
    legendText.setCharacterSize(18);
    legendText.setFillColor(sf::Color::White);
    legendText.setString("Balls: Yellow=1 | Red=3 | Purple=5 | Cyan=10 (Fast)");
    legendText.setPosition(Constants::GAME_WIDTH - 520.f, 10.f);
    m_window.draw(legendText);

    if (m_paused && m_running) {
        sf::Text pausedText;
        pausedText.setFont(m_font);
        pausedText.setCharacterSize(42);
        pausedText.setFillColor(sf::Color::White);
        pausedText.setString("PAUSED");
        pausedText.setPosition((Constants::GAME_WIDTH / 2.f) - 90.f, (Constants::GAME_HEIGHT / 2.f) - 20.f);
        m_window.draw(pausedText);
    }
    
    m_window.display();
}

void MasterLoop::endGame() {
    m_running = false;
}

double MasterLoop::calculateWinRate(int playerScore, int playerHits) const {
    int totalScore = m_score.player1 + m_score.player2;
    int totalHits = m_player1Hits + m_player2Hits;
    
    double scoreContribution = (totalScore > 0) ? (double)playerScore / (double)totalScore : 0.0;
    double hitsContribution = (totalHits > 0) ? (double)playerHits / (double)totalHits : 0.0;
    
    return (scoreContribution * 0.7) + (hitsContribution * 0.3) * 100.0;
}

void MasterLoop::showLeaderboard() {
    struct Row { int rank; std::string name; int score; int hits; double winRate; };
    std::array<Row, 2> rows = {
        Row{1, "Player 1", m_score.player1, m_player1Hits, calculateWinRate(m_score.player1, m_player1Hits)},
        Row{2, "Player 2", m_score.player2, m_player2Hits, calculateWinRate(m_score.player2, m_player2Hits)}
    };
    
    std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
        return a.winRate > b.winRate;
    });
    
    while (m_window.isOpen()) {
        sf::Event e{};
        while (m_window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) m_window.close();
        }
        
        m_window.clear(sf::Color::Black);
        
        sf::Text title("LEADERBOARD", m_font, 48);
        title.setFillColor(sf::Color::White);
        title.setPosition(20.f, 20.f);
        m_window.draw(title);
        
        sf::Text header("Rank        Player        Score        Hits        Win Rate", m_font, 24);
        header.setFillColor(sf::Color::White);
        header.setPosition(20.f, 100.f);
        m_window.draw(header);
        
        float y = 150.f;
        for (std::size_t i = 0; i < rows.size(); ++i) {
            std::ostringstream oss;
            oss.setf(std::ios::fixed);
            oss.precision(2);
            
            oss << (int)(i + 1) << "           "
                << rows[i].name << "        "
                << rows[i].score << "            "
                << rows[i].hits << "          "
                << rows[i].winRate;
            
            sf::Text rowText(oss.str(), m_font, 24);
            rowText.setFillColor(sf::Color::White);
            rowText.setPosition(20.f, y);
            y += 40.f;
            
            m_window.draw(rowText);
        }
        
        sf::Text hint("Close the window to exit.", m_font, 20);
        hint.setFillColor(sf::Color(200, 200, 200));
        hint.setPosition(20.f, static_cast<float>(Constants::GAME_HEIGHT) - 40.f);
        m_window.draw(hint);
        
        m_window.display();
    }
}

void MasterLoop::run() {
    while (m_window.isOpen()) {
        sf::Event event{};
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                m_window.close();
                endGame();
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::P) {
                    togglePause();
                }
                else if (event.key.code == sf::Keyboard::R) {
                    resetMatch();
                }
            }
        }
        
        if (m_running) {
            if (!m_paused) {
                // Timer countdown
                if (m_secondClock.getElapsedTime().asSeconds() >= 1.f) {
                    m_secondClock.restart();
                    if (m_timeLeft > 0) m_timeLeft--;
                    if (m_timeLeft <= 0) endGame();
                }

                // Handle input and update paddles
                handleInput();
                updatePaddles();

                // Send paddle state to worker
                sendTickInput();

                // Receive ball state and events from worker
                receiveTickOutput();
            }

            // Render
            draw();
        }
        else {
            // Show leaderboard once
            if (!m_leaderboardShown) {
                m_leaderboardShown = true;
                // Send final "not running" message to worker
                sendTickInput();
                showLeaderboard();
            }
        }
    }

    // Ensure worker is released from blocking receive on shutdown.
    m_running = false;
    sendTickInput();
}
