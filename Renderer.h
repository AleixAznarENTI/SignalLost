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

	void bakeMap(const Map& map);

	void drawMap();

	void drawPlayer(sf::Vector2f position);
	void drawSignal(sf::Vector2f position);
	void drawBatteries(const std::vector<Battery>& batteries);

private:
	sf::RenderWindow& m_window;
	float			  m_tileSize;
	sf::RectangleShape m_tileShape;
	sf::Clock		  m_clock;

	sf::RenderTexture m_mapTexture;
	sf::Sprite m_mapSprite;

	sf::CircleShape m_playerShape;
	sf::CircleShape m_signalShape;
	sf::RectangleShape m_batteryShape;

	sf::Vector2f toPixels(int x, int y) const;
};

