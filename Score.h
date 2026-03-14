#pragma once
#include <SFML/Graphics.hpp>

class Score {
public:
    Score(float gameWidth, float gameHeight);

    void draw(sf::RenderWindow& window) const;

    int player1 = 0;
    int player2 = 0;

private:
    float m_gameWidth = 0.f;
    float m_gameHeight = 0.f;

    sf::Font m_font;
};
