#pragma once

#include <SFML/Graphics.hpp>

class PostProcess {
public:
    PostProcess(sf::RenderWindow& window);

    // Dibuja grain y scanlines encima de todo
    // intensity: 0.0 - 1.0
    void draw(float intensity = 0.4f);

private:
    sf::RenderWindow& m_window;
    sf::Clock         m_clock;

    void drawGrain(float intensity);
    void drawScanlines();
};