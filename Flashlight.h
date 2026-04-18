#pragma once

#include <SFML/Graphics.hpp>

class Flashlight
{
public:
	Flashlight(sf::Vector2u windowSize, float radius = 180.f);

	void update(sf::Vector2f playerWorldPos, const sf::View& cameraView);

	void draw(sf::RenderWindow& window);

	void setRadius(float radius) { m_radius = radius; }

private:
	sf::RenderTexture m_maskTexture;
	float			  m_radius;
	sf::Clock		  m_clock;

	void buildLightCone(sf::Vector2f center);
};

