#include "Paddle.h"

Paddle::Paddle(float x, float y, float w, float h, int id) : m_id(id) {
    m_shape.setSize({ w, h });
    m_shape.setPosition(x, y);

    if (m_id == 1) m_shape.setFillColor(sf::Color::Blue);
    else m_shape.setFillColor(sf::Color::Red);
}

sf::FloatRect Paddle::bounds() const {
    return m_shape.getGlobalBounds();
}

void Paddle::handleInput() {
    m_yVelocity = 0.f;

    if (m_id == 1) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) m_yVelocity = -m_speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) m_yVelocity = m_speed;
    }
    else {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))   m_yVelocity = -m_speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) m_yVelocity = m_speed;
    }
}

void Paddle::move(float gameHeight) {
    auto pos = m_shape.getPosition();
    float newY = pos.y + m_yVelocity;

    if (newY < 0.f) newY = 0.f;

    float h = m_shape.getSize().y;
    if (newY > gameHeight - h) newY = gameHeight - h;

    m_shape.setPosition(pos.x, newY);
}

void Paddle::draw(sf::RenderWindow& window) const {
    window.draw(m_shape);
}