#pragma once

#include <SFML/Graphics.hpp>
#include "Map.h"
#include "Camera.h"
#include "Battery.h"
#include <vector>

class Renderer
{
public:
	Renderer(sf::RenderWindow& window, float tileSize);

	void drawMap(const Map& map);
	void drawPlayer(sf::Vector2f position);
	void drawSignal(sf::Vector2f position);
	void drawBatteries(const std::vector<Battery>& batteries);

private:
	sf::RenderWindow& m_window;
	float			  m_tileSize;
	sf::Clock		  m_clock;

	sf::RectangleShape m_tileShape;

	sf::Vector2f toPixels(int x, int y) const;
};

