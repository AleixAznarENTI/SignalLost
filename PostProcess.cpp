#include "PostProcess.h"
#include <cstdlib>
#include <cmath>

PostProcess::PostProcess(sf::RenderWindow& window)
    : m_window(window)
{
}

void PostProcess::drawGrain(float intensity) {
    float w = static_cast<float>(m_window.getSize().x);
    float h = static_cast<float>(m_window.getSize().y);

    // Generamos puntos de ruido aleatorios cada frame
    // Solo pintamos una fracción de los píxeles — suficiente para el efecto
    const int GRAIN_COUNT = static_cast<int>(w * h * 0.008f * intensity);

    sf::VertexArray grain(sf::PrimitiveType::Points, GRAIN_COUNT);

    for (int i = 0; i < GRAIN_COUNT; ++i) {
        float px = static_cast<float>(rand() % static_cast<int>(w));
        float py = static_cast<float>(rand() % static_cast<int>(h));

        uint8_t bright = static_cast<uint8_t>(rand() % 180);
        uint8_t alpha = static_cast<uint8_t>(40 + rand() % 60);

        grain[i].position = { px, py };
        grain[i].color = sf::Color(bright, bright, bright, alpha);
    }

    m_window.draw(grain);
}

void PostProcess::drawScanlines() {
    float w = static_cast<float>(m_window.getSize().x);
    float h = static_cast<float>(m_window.getSize().y);

    // Una línea oscura cada 3 píxeles — efecto CRT
    const int STEP = 3;
    sf::VertexArray lines(sf::PrimitiveType::Lines);

    for (int y = 0; y < static_cast<int>(h); y += STEP) {
        lines.append({ { 0.f, static_cast<float>(y) },
                        sf::Color(0, 0, 0, 35) });
        lines.append({ { w,   static_cast<float>(y) },
                        sf::Color(0, 0, 0, 35) });
    }

    m_window.draw(lines);
}

void PostProcess::draw(float intensity) {
    sf::View prev = m_window.getView();
    m_window.setView(m_window.getDefaultView());

    drawScanlines();
    drawGrain(intensity);

    m_window.setView(prev);
}