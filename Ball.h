#pragma once
#include <SFML/Graphics.hpp>
#include <deque>
#include <string>

class Ball {
public:
    enum class Rarity {
        Common,
        Rare,
        Epic,
        Legendary
    };

    Ball(float x, float y, float diameter, int ballId, Rarity rarity);
    Ball() = default; // Default constructor for array initialization

    void move();
    void setXDirection(float xDir);
    void setYDirection(float yDir);

    void draw(sf::RenderWindow& window) const;

    sf::FloatRect bounds() const;
    sf::Vector2f position() const;

    float xVelocity() const { return m_xVelocity; }
    float yVelocity() const { return m_yVelocity; }

    void setPosition(float x, float y);

    int ballId() const { return m_ballId; }
    int pointsValue() const { return m_pointsValue; }
    Rarity rarity() const { return m_rarity; }

    void resetCooldown() { m_cooldownFrames = COLLISION_COOLDOWN; }
    bool isInCooldown() const { return m_cooldownFrames > 0; }
    void decrementCooldown() { if (m_cooldownFrames > 0) m_cooldownFrames--; }

private:
    static constexpr std::size_t TRAIL_LENGTH = 50;
    static constexpr int COLLISION_COOLDOWN = 10; // frames

    void setupRarityAttributes();

    sf::CircleShape m_shape;         // current ball
    std::deque<sf::Vector2f> m_trail; // trail points (center positions)

    int m_ballId = 0;
    Rarity m_rarity = Rarity::Common;
    int m_pointsValue = 1;
    float m_speedMultiplier = 1.0f;
    sf::Color m_color = sf::Color::Yellow;
    sf::Color m_trailColor = sf::Color::Yellow;

    float m_baseXVelocity = 45.f;
    float m_baseYVelocity = 45.f;
    float m_xVelocity = 45.f;
    float m_yVelocity = 45.f;

    int m_cooldownFrames = 0;
};
