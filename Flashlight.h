#pragma once

#include <SFML/Graphics.hpp>
#include "Map.h"

class Flashlight {
public:
    Flashlight(sf::Vector2u windowSize, float radius = 180.f);

    void update(sf::Vector2f      playerPos,
        const sf::View& cameraView,
        sf::RenderWindow& window,
        const Map& map,
        float             tileSize);

    void draw(sf::RenderWindow& window);
    void setRadius(float radius) { m_radius = radius; }

    float getAngle()        const { return m_angle; }
    float getHalfAperture() const { return m_halfAperture; }
    float getRadius()       const { return m_radius; }

private:
    sf::RenderTexture m_maskTexture;
    float             m_radius;
    float             m_angle = 0.f;
    float             m_halfAperture = 0.f;
    sf::Clock         m_clock;

    sf::Vector2f castRay(sf::Vector2f origin,
        float        angle,
        float        maxDist) const;

    void buildLightCone(sf::Vector2f origin,
        float        dirAngle,
        float        halfAperture);

    void buildAmbientGlow(sf::Vector2f origin);
};