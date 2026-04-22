#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Map.h"
#include "Camera.h"
#include "Battery.h"
#include "Enemy.h"
#include "PowerUp.h"

class Renderer
{
public:
	Renderer(sf::RenderWindow& window, float tileSize, sf::Font font);

	void bakeMap(const Map& map);

	void drawMap();

	void drawPlayer(sf::Vector2f position);
	void drawSignal(sf::Vector2f position);
	void drawBatteries(const std::vector<Battery>& batteries);
	void drawEnemies(const std::vector<Enemy>& enemies);
	void drawPowerUps(const std::vector<PowerUp>& powerUps);

private:
	sf::RenderWindow& m_window;
	float			  m_tileSize;
	sf::RectangleShape m_tileShape;
	sf::Clock		  m_clock;
	sf::Font		  m_font;

	sf::RenderTexture m_mapTexture;
	sf::Sprite m_mapSprite;

	sf::CircleShape m_playerShape;
	sf::CircleShape m_signalShape;
	sf::RectangleShape m_batteryShape;

	sf::Vector2f toPixels(int x, int y) const;
};

