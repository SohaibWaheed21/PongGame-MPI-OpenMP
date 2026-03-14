#pragma once
#include <SFML/Graphics.hpp>

class Paddle {
public:
    Paddle(float x, float y, float w, float h, int id);

    void handleInput(); // reads keyboard state
    void move(float gameHeight);
    void draw(sf::RenderWindow& window) const;

    sf::FloatRect bounds() const;

private:
    sf::RectangleShape m_shape;
    int m_id = 1;

    float m_yVelocity = 0.f;
    float m_speed = 10.f;
};
