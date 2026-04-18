#include "Map.h"
#include <cstdlib>
#include <cmath>
#include <stdexcept>

Map::Map(int width, int height)
	: m_width(width), m_height(height)
{
	m_grid.assign(height, std::vector<TileType>(width, TileType::Wall));
}

void Map::fillWithWalls() {
	for (auto& row : m_grid)
		std::fill(row.begin(), row.end(), TileType::Wall);
}

void Map::generate(int steps) {
	fillWithWalls();

	int x = m_width / 2;
	int y = m_height / 2;

	const int dx[] = { 0, 0, -1, 1 };
	const int dy[] = { -1, 1, 0, 0 };

	for (int i = 0; i < steps; ++i) {
		m_grid[y][x] = TileType::Floor;

		int dir = rand() % 4;
		int nx = x + dx[dir];
		int ny = y + dy[dir];

		if (nx > 0 && nx < m_width - 1 && ny > 0 && ny < m_height - 1) {
			x = nx;
			y = ny;
		}
	}

	m_startPosition = findOpenSpot();
	
	placeSignal();

	placeBatteries(6, 24.f);
}

void Map::placeBatteries(int count, float tileSize) {
	m_batteries.clear();

	const float minDistFromStart = 5.f;

	int placed = 0;
	int attempts = 0;

	while (placed < count && attempts < 10000) {
		++attempts;

		int x = rand() % (m_width - 2) + 1;
		int y = rand() % (m_height - 2) + 1;

		if (m_grid[y][x] != TileType::Floor) continue;

		float dist = std::sqrt(
			std::pow(x - m_startPosition.x, 2.f) +
			std::pow(y - m_startPosition.y, 2.f)
		);

		if (dist < minDistFromStart) continue;

		if (sf::Vector2i(x, y) == m_signalPosition) continue;

		sf::Vector2f worldPos(
			(x + 0.5f) * tileSize,
			(y + 0.5f) * tileSize
		);
		
		m_batteries.emplace_back(worldPos, 25.f);
		++placed;
	}

}

void Map::placeSignal() {
	float minDist = 15.f;
	float maxDist = -1.f;
	m_signalPosition = m_startPosition;

	for (int row = 0; row < m_height; ++row) {
		for (int col = 0; col < m_width; ++col) {
			if (m_grid[row][col] != TileType::Floor) continue;

			float dist = std::sqrt(
				std::pow(col - m_startPosition.x, 2.f) +
				std::pow(row - m_startPosition.y, 2.f)
			);

			if (dist > minDist && dist > maxDist) {
				maxDist = dist;
				m_signalPosition = { col, row };
			}
		}
	}
}

TileType Map::getTile(int x, int y) const {
	if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		return TileType::Wall; // Treat out-of-bounds as walls
	return m_grid[y][x];
}

sf::Vector2i Map::findOpenSpot() const {
	for (int y = 1; y < m_height - 1; ++y) {
		for (int x = 1; x < m_width - 1; ++x) {
			if (m_grid[y][x] != TileType::Floor) continue;

			bool open = getTile(x, y - 1) == TileType::Floor
				&& getTile(x, y + 1) == TileType::Floor
				&& getTile(x - 1, y) == TileType::Floor
				&& getTile(x + 1, y) == TileType::Floor;

			if (open) { return { x, y }; }
		}
	}
	return { m_width/2, m_height/2 }; // Return the center position if no open spot is found
}