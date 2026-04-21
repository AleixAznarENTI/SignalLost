#include "Flashlight.h"
#include <cmath>
#include <stdexcept>

Flashlight::Flashlight(sf::Vector2u windowSize, float radius)
    : m_radius(radius)
{
    if (!m_maskTexture.resize(windowSize))
        throw std::runtime_error("No se pudo crear la RenderTexture de la linterna");
}

void Flashlight::buildAmbientGlow(sf::Vector2f origin) {
    const int   SEGMENTS = 32;
    const float ambientR = m_radius * 0.25f;

    sf::VertexArray glow(sf::PrimitiveType::Triangles, SEGMENTS * 3);

    for (int i = 0; i < SEGMENTS; ++i) {
        float a0 = i * (2.f * 3.14159265f / SEGMENTS);
        float a1 = (i + 1) * (2.f * 3.14159265f / SEGMENTS);

        glow[i * 3 + 0].position = origin;
        glow[i * 3 + 0].color = sf::Color(80, 80, 120);

        glow[i * 3 + 1].position = { origin.x + ambientR * std::cos(a0),
                                  origin.y + ambientR * std::sin(a0) };
        glow[i * 3 + 1].color = sf::Color::Black;

        glow[i * 3 + 2].position = { origin.x + ambientR * std::cos(a1),
                                  origin.y + ambientR * std::sin(a1) };
        glow[i * 3 + 2].color = sf::Color::Black;
    }

    m_maskTexture.draw(glow);
}

sf::Vector2f Flashlight::castRay(sf::Vector2f origin,
    float        angle,
    float        maxDist) const
{
    float dx = std::cos(angle);
    float dy = std::sin(angle);
    return origin + sf::Vector2f(dx, dy) * maxDist;
}

void Flashlight::buildLightCone(sf::Vector2f origin,
    float        dirAngle,
    float        halfAperture)
{
    float flicker = 1.f + 0.03f * std::sin(m_clock.getElapsedTime().asSeconds() * 7.f);
    float r = m_radius * flicker;

    const int RAY_COUNT = 64;
    sf::VertexArray cone(sf::PrimitiveType::Triangles, RAY_COUNT * 3);

    for (int i = 0; i < RAY_COUNT; ++i) {
        float t0 = static_cast<float>(i) / RAY_COUNT;
        float t1 = static_cast<float>(i + 1) / RAY_COUNT;

        float a0 = dirAngle - halfAperture + t0 * 2.f * halfAperture;
        float a1 = dirAngle - halfAperture + t1 * 2.f * halfAperture;

        cone[i * 3 + 0] = { origin,                 sf::Color(255, 240, 200) };
        cone[i * 3 + 1] = { castRay(origin, a0, r), sf::Color::Black };
        cone[i * 3 + 2] = { castRay(origin, a1, r), sf::Color::Black };
    }

    m_maskTexture.draw(cone);
}

void Flashlight::update(sf::Vector2f      playerPos,
    const sf::View& cameraView,
    sf::RenderWindow& window,
    const Map& map,
    float             tileSize)
{
    // map y tileSize se mantienen en la firma para no cambiar Game.cpp
    // pero no se usan en esta versión sin raycasting
    (void)map;
    (void)tileSize;

    sf::Vector2f mouseWorld = window.mapPixelToCoords(
        sf::Mouse::getPosition(window), cameraView);

    sf::Vector2f diff = mouseWorld - playerPos;
    m_angle = std::atan2(diff.y, diff.x);
    m_halfAperture = 50.f * (3.14159265f / 180.f);

    m_maskTexture.setView(cameraView);
    m_maskTexture.clear(sf::Color::Black);

    buildAmbientGlow(playerPos);
    buildLightCone(playerPos, m_angle, m_halfAperture);

    m_maskTexture.display();
}

void Flashlight::draw(sf::RenderWindow& window) {
    sf::View prev = window.getView();
    window.setView(window.getDefaultView());

    sf::Sprite maskSprite(m_maskTexture.getTexture());
    window.draw(maskSprite, sf::BlendMultiply);

    window.setView(prev);
}