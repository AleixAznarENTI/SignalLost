#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Map.h"
#include "Camera.h"
#include "Battery.h"
#include "Enemy.h"
#include "PowerUp.h"
#include "DataLogSystem.h"
#include "TerminalSystem.h"
#include "KeyDoorSystem.h"

class Renderer
{
public:
	Renderer(sf::RenderWindow& window, float tileSize, sf::Font font);

	void bakeMap(const Map& map);

	void drawMap();

	void drawPlayer(sf::Vector2f position);
	void drawSignal(sf::Vector2f position);
	void drawBatteries(const std::vector<Battery>& batteries,
						sf::Vector2f playerPos);
	void drawEnemies(const std::vector<Enemy>& enemies);
	void drawPowerUps(const std::vector<PowerUp>& powerUps);
	void drawDataLogs(const std::vector<DataLog>& logs);
	void drawTerminals(const std::vector<Terminal>& terminals);
	void drawKeys(const std::vector<Key>& keys);
	void drawDoors(const std::vector<Door>& doors);

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
	void drawProp(const Map& map, int x, int y,
		sf::Color fill, sf::Color outline,
		float sizeRatio);
	void drawPropLight(int x, int y, sf::Color color);
	void drawDebris(int x, int y);
};

