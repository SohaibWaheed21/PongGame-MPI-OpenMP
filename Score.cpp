#include "Score.h"
#include <stdexcept>

Score::Score(float gameWidth, float gameHeight)
    : m_gameWidth(gameWidth), m_gameHeight(gameHeight) {

    // Put a font file next to your .exe, e.g. arial.ttf
    if (!m_font.loadFromFile("arial.ttf")) {
        throw std::runtime_error("Failed to load font: arial.ttf (place it next to the .exe)");
    }
}

void Score::draw(sf::RenderWindow& window) const {
    // Midline
    sf::Vertex line[] = {
        sf::Vertex(sf::Vector2f(m_gameWidth / 2.f, 0.f), sf::Color::White),
        sf::Vertex(sf::Vector2f(m_gameWidth / 2.f, m_gameHeight), sf::Color::White)
    };
    window.draw(line, 2, sf::Lines);

    sf::Text t1;
    t1.setFont(m_font);
    t1.setCharacterSize(60);
    t1.setFillColor(sf::Color::White);
    t1.setString(std::to_string(player1 / 10) + std::to_string(player1 % 10));
    t1.setPosition(m_gameWidth / 2.f - 85.f, 10.f);

    sf::Text t2 = t1;
    t2.setString(std::to_string(player2 / 10) + std::to_string(player2 % 10));
    t2.setPosition(m_gameWidth / 2.f + 20.f, 10.f);

    window.draw(t1);
    window.draw(t2);
}