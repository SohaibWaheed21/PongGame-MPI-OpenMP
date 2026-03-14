#include "Game.h"
#include "src/sim/Simulation.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <vector>

static float randomBallY() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(Constants::BALL_RADIUS, (Constants::GAME_HEIGHT) - Constants::BALL_RADIUS);
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

Game::Game()
    : m_window(sf::VideoMode(Constants::GAME_WIDTH, Constants::GAME_HEIGHT), "PONG GAME")
    , m_p1(0.f, (Constants::GAME_HEIGHT / 2.f) - (Constants::PADDLE_HEIGHT / 2.f), Constants::PADDLE_WIDTH, Constants::PADDLE_HEIGHT, 1)
    , m_p2(Constants::GAME_WIDTH - Constants::PADDLE_WIDTH, (Constants::GAME_HEIGHT / 2.f) - (Constants::PADDLE_HEIGHT / 2.f), Constants::PADDLE_WIDTH, Constants::PADDLE_HEIGHT, 2)
    , m_score(static_cast<float>(Constants::GAME_WIDTH), static_cast<float>(Constants::GAME_HEIGHT)) {

    m_window.setFramerateLimit(Constants::TARGET_FPS);

    if (!m_font.loadFromFile("arial.ttf")) {
        throw std::runtime_error("Failed to load font: arial.ttf (place it next to the .exe)");
    }

    initBalls();
}

void Game::newPaddles() {
    m_p1 = Paddle(0.f, (Constants::GAME_HEIGHT / 2.f) - (Constants::PADDLE_HEIGHT / 2.f), Constants::PADDLE_WIDTH, Constants::PADDLE_HEIGHT, 1);
    m_p2 = Paddle(Constants::GAME_WIDTH - Constants::PADDLE_WIDTH, (Constants::GAME_HEIGHT / 2.f) - (Constants::PADDLE_HEIGHT / 2.f), Constants::PADDLE_WIDTH, Constants::PADDLE_HEIGHT, 2);
}

void Game::initBalls() {
    float startX = (Constants::GAME_WIDTH / 2.f);
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
    }
}

void Game::respawnBall(int ballId) {
    if (ballId < 0 || ballId >= Constants::NUM_BALLS) return;

    float startX = (Constants::GAME_WIDTH / 2.f);
    std::vector<float> occupiedYs;
    occupiedYs.reserve(Constants::NUM_BALLS - 1);
    const float minSpawnDistance = Constants::BALL_DIAMETER * 2.5f;
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        if (i == ballId) continue;
        occupiedYs.push_back(m_ballStates[i].y);
    }
    float startY = randomSpawnYWithSpacing(occupiedYs, minSpawnDistance);

    // Respawn simulation state
    Simulation::respawnBall(m_ballStates[ballId], startX, startY);

    // Respawn SFML ball
    Ball::Rarity rarity = m_balls[ballId].rarity();
    m_balls[ballId] = Ball(startX - Constants::BALL_RADIUS, startY - Constants::BALL_RADIUS, Constants::BALL_DIAMETER, ballId, rarity);
}

void Game::resetMatch() {
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

void Game::togglePause() {
    if (!m_running) {
        return;
    }
    m_paused = !m_paused;
}

double Game::calculateWinRate(int playerScore, int playerHits) const {
    int totalScore = m_score.player1 + m_score.player2;
    int totalHits = m_player1Hits + m_player2Hits;

    double scoreContribution = (totalScore > 0) ? (double)playerScore / (double)totalScore : 0.0;
    double hitsContribution = (totalHits > 0) ? (double)playerHits / (double)totalHits : 0.0;

    return (scoreContribution * 0.7) + (hitsContribution * 0.3) * 100.0;
}

void Game::endGame() {
    m_running = false;
}

void Game::syncPaddleStateFromSFML(PaddleState& state, const Paddle& paddle) {
    auto bounds = paddle.bounds();
    state.x = bounds.left;
    state.y = bounds.top;
    state.w = bounds.width;
    state.h = bounds.height;
}

void Game::syncBallStateToSFML(Ball& ball, const BallState& state) {
    // Update SFML ball position to match simulation state
    ball.setPosition(state.x - state.radius, state.y - state.radius);
    ball.setXDirection(state.vx);
    ball.setYDirection(state.vy);
}

void Game::updateSimulation() {
    // Sync paddle states from SFML
    syncPaddleStateFromSFML(m_p1State, m_p1);
    syncPaddleStateFromSFML(m_p2State, m_p2);

    // Simulate each ball
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        auto events = Simulation::simulateBallTick(
            m_ballStates[i],
            m_p1State,
            m_p2State,
            static_cast<float>(Constants::GAME_WIDTH),
            static_cast<float>(Constants::GAME_HEIGHT),
            Constants::FIXED_DT
        );

        // Process events
        for (const auto& event : events) {
            if (event.type == EventType::Hit) {
                if (event.hit.paddleId == 1) {
                    m_player1Hits++;
                } else if (event.hit.paddleId == 2) {
                    m_player2Hits++;
                }
            }
            else if (event.type == EventType::Score) {
                // Add points
                if (event.score.playerId == 1) {
                    m_score.player1 += event.score.points;
                } else if (event.score.playerId == 2) {
                    m_score.player2 += event.score.points;
                }

                // Respawn the ball and reset paddles
                newPaddles();
                respawnBall(event.score.ballId);
            }
        }

        // Sync SFML ball from simulation state
        syncBallStateToSFML(m_balls[i], m_ballStates[i]);
    }
}

void Game::update() {
    // 60s countdown
    if (m_secondClock.getElapsedTime().asSeconds() >= 1.f) {
        m_secondClock.restart();
        if (m_timeLeft > 0) m_timeLeft--;
        if (m_timeLeft <= 0) endGame();
    }

    // input
    m_p1.handleInput();
    m_p2.handleInput();

    // movement
    m_p1.move(static_cast<float>(Constants::GAME_HEIGHT));
    m_p2.move(static_cast<float>(Constants::GAME_HEIGHT));

    // Run simulation
    updateSimulation();
}

void Game::draw() {
    m_window.clear(sf::Color::Black);

    m_p1.draw(m_window);
    m_p2.draw(m_window);

    // Draw all balls
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        m_balls[i].draw(m_window);
    }

    m_score.draw(m_window);

    // timer text (top-left)
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

    // Legend (top-right)
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

void Game::run() {
    while (m_window.isOpen()) {
        sf::Event event{};
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                m_window.close();
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
                update();
            }
            draw();
        }
        else {
            // Show leaderboard once, then just keep window open until close
            if (!m_leaderboardShown) {
                m_leaderboardShown = true;

                // Build leaderboard data (2 players)
                struct Row { int rank; std::string name; int score; int hits; double winRate; };
                std::array<Row, 2> rows = {
                    Row{1, "Player 1", m_score.player1, m_player1Hits, calculateWinRate(m_score.player1, m_player1Hits)},
                    Row{2, "Player 2", m_score.player2, m_player2Hits, calculateWinRate(m_score.player2, m_player2Hits)}
                };

                std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
                    return a.winRate > b.winRate;
                    });

                // Simple "leaderboard screen" in same window (instead of new JFrame)
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

                    sf::Text header("Rank        Player        Score        Hits        Win Rate",
                        m_font, 24);
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
            else {
                // If somehow we get here, just draw last frame
                draw();
            }
        }
    }
}