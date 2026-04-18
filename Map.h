#pragma once
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "Battery.h"

enum class TileType {
	Wall,
	Floor
};

class Map
{

public:
	Map(int width, int height);
	void generate(int steps = 2000);

	TileType getTile(int x, int y) const;
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	sf::Vector2i getStartPosition() const { return m_startPosition; }
	sf::Vector2i getSignalPosition() const { return m_signalPosition; }
	const std::vector<Battery>& getBatteries() const { return m_batteries; }

private:
	int m_width;
	int m_height;
	std::vector<std::vector<TileType>> m_grid;
	sf::Vector2i m_startPosition;
	sf::Vector2i m_signalPosition;
	std::vector<Battery> m_batteries;
	void fillWithWalls();
	void placeSignal();
	sf::Vector2i findOpenSpot() const;
	void placeBatteries(int count, float tileSize);
};