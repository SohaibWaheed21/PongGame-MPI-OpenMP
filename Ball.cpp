#include "Ball.h"
#include <algorithm>

Ball::Ball(float x, float y, float diameter, int ballId, Rarity rarity)
    : m_ballId(ballId), m_rarity(rarity) {
    m_shape.setRadius(diameter / 2.f);

    setupRarityAttributes();

    m_shape.setFillColor(m_color);
    m_shape.setPosition(x, y);

    // Apply speed multiplier to base velocities
    m_xVelocity = m_baseXVelocity * m_speedMultiplier;
    m_yVelocity = m_baseYVelocity * m_speedMultiplier;
}

void Ball::setupRarityAttributes() {
    switch (m_rarity) {
    case Rarity::Common:
        m_color = sf::Color::Yellow;
        m_trailColor = sf::Color::Yellow;
        m_pointsValue = 1;
        m_speedMultiplier = 1.0f;
        break;
    case Rarity::Rare:
        m_color = sf::Color::Red;
        m_trailColor = sf::Color::Red;
        m_pointsValue = 3;
        m_speedMultiplier = 1.0f;
        break;
    case Rarity::Epic:
        m_color = sf::Color(128, 0, 128); // Purple
        m_trailColor = sf::Color(128, 0, 128);
        m_pointsValue = 5;
        m_speedMultiplier = 1.0f;
        break;
    case Rarity::Legendary:
        m_color = sf::Color::Cyan;
        m_trailColor = sf::Color::Cyan;
        m_pointsValue = 10;
        m_speedMultiplier = 1.5f;
        break;
    }
}

void Ball::setPosition(float x, float y) {
    m_shape.setPosition(x, y);

    auto b = bounds();
    sf::Vector2f center(b.left + b.width / 2.f, b.top + b.height / 2.f);
    m_trail.push_front(center);
    if (m_trail.size() > TRAIL_LENGTH) {
        m_trail.pop_back();
    }
}

sf::Vector2f Ball::position() const {
    return m_shape.getPosition();
}

sf::FloatRect Ball::bounds() const {
    return m_shape.getGlobalBounds();
}

void Ball::move() {
    m_shape.move(m_xVelocity, m_yVelocity);

    // Add center to trail
    auto b = bounds();
    sf::Vector2f center(b.left + b.width / 2.f, b.top + b.height / 2.f);

    m_trail.push_front(center);
    if (m_trail.size() > TRAIL_LENGTH) {
        m_trail.pop_back();
    }
}

void Ball::setXDirection(float xDir) {
    m_xVelocity = xDir * m_speedMultiplier;
}

void Ball::setYDirection(float yDir) {
    m_yVelocity = yDir * m_speedMultiplier;
}

void Ball::draw(sf::RenderWindow& window) const {
    // Draw trail (using ball's rarity color, fading)
    float diameter = m_shape.getRadius() * 2.f;
    for (std::size_t i = 0; i < m_trail.size(); ++i) {
        int alpha = 255 - static_cast<int>(i * (255.f / static_cast<float>(TRAIL_LENGTH)));
        alpha = std::max(alpha, 0);

        sf::CircleShape t(diameter / 2.f);
        sf::Color trailColorWithAlpha = m_trailColor;
        trailColorWithAlpha.a = static_cast<sf::Uint8>(alpha);
        t.setFillColor(trailColorWithAlpha);
        t.setPosition(m_trail[i].x - diameter / 2.f, m_trail[i].y - diameter / 2.f);
        window.draw(t);
    }

    window.draw(m_shape);
}