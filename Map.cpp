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
	m_rooms.clear();
	m_batteries.clear();
}

TileType Map::getTile(int x, int y) const {
	if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		return TileType::Wall; // Treat out-of-bounds as walls
	return m_grid[y][x];
}

RoomType Map::getRoomTypeAt(int tx, int ty) const {
	for (const auto& room : m_rooms) {
		if (room.contains(tx, ty))
			return room.type;
	}
	return RoomType::Normal; // Default type for non-room tiles
}

void Map::carveRoom(const Room& room) {
	for (int y = room.y; y < room.y + room.h; ++y) {
		for (int x = room.x; x < room.x + room.w; ++x) {
			m_grid[y][x] = TileType::Floor;
		}
	}
}

void Map::carveCorridor(sf::Vector2i a, sf::Vector2i b) {
	int x = a.x;
	int y = a.y;

	while (x != b.x) {
		m_grid[y][x] = TileType::Floor;
		x += (b.x > x) ? 1 : -1;
	}

	while (y != b.y) {
		m_grid[y][x] = TileType::Floor;
		y += (b.y > y) ? 1 : -1;
	}

	m_grid[y][x] = TileType::Floor;
}

void Map::generate(int roomAttempts) {
	fillWithWalls();

	auto randInt = [](int min, int max) {
		return min + rand() % (max - min + 1);
	};

	for (int i = 0; i < roomAttempts; ++i) {
		Room candidate;
		candidate.w = randInt(8, 16);
		candidate.h = randInt(4, 9);
		candidate.x = randInt(1, m_width - candidate.w - 1);
		candidate.y = randInt(1, m_height - candidate.h - 1);

		bool valid = true;
		for (const auto& existing : m_rooms) {
			if (candidate.overlaps(existing)) {
				valid = false;
				break;
			}
		}

		if (valid) {
			carveRoom(candidate);
			m_rooms.push_back(candidate);
		}
	}

	for (size_t i = 1; i < m_rooms.size(); ++i) {
		carveCorridor(m_rooms[i - 1].center(), m_rooms[i].center());
	}

	assignRoomTypes();

	m_startPosition = m_rooms.empty()
		? sf::Vector2i(m_width / 2, m_height / 2)
		: m_rooms.front().center();
	
	placeSignal();
	placeBatteries(24.f);
}

void Map::assignRoomTypes() {
	for (size_t i = 1; i + 1 < m_rooms.size(); ++i) {
		int roll = rand() % 10;
		if		(roll < 3) m_rooms[i].type = RoomType::Storage;
		else if (roll < 5) m_rooms[i].type = RoomType::Danger;
		else if (roll < 6) m_rooms[i].type = RoomType::Control;
		else			   m_rooms[i].type = RoomType::Normal;
	}
}

void Map::placeSignal() {
	if (m_rooms.empty()) {
		m_signalPosition = { m_width / 2, m_height / 2 };
		return;
	}

	float maxDist = -1.f;
	m_signalPosition = m_rooms.back().center();

	for (const auto& room : m_rooms) {
		sf::Vector2i c = room.center();
		float dist = std::sqrt(
			std::pow(c.x - m_startPosition.x, 2.f) +
			std::pow(c.y - m_startPosition.y, 2.f)
		);
		if (dist > maxDist) {
			maxDist = dist;
			m_signalPosition = c;
		}
	}
}

void Map::placeBatteries(float tileSize) {
	m_batteries.clear();

	for (const auto& room : m_rooms) {
		int count = (room.type == RoomType::Storage)
			? 2 + rand() % 2
			: rand() % 2;
		
		for (int i = 0; i < count; ++i) {
			int tx = room.x + 1 + rand() % (room.w - 2);
			int ty = room.y + 1 + rand() % (room.h - 2);

			sf::Vector2f worldPos(
				(tx + 0.5f) * tileSize,
				(ty + 0.5f) * tileSize
			);
			m_batteries.emplace_back(worldPos, 25.f);
		}
	}

}


