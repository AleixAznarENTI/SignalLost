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
	// Talla el rectángulo principal
	for (int y = room.y; y < room.y + room.h; ++y)
		for (int x = room.x; x < room.x + room.w; ++x)
			m_grid[y][x] = TileType::Floor;

	// Talla la extensión si es L-shape
	if (room.shape == RoomShape::LShape && room.extW > 0) {
		for (int y = room.extY; y < room.extY + room.extH; ++y)
			for (int x = room.extX; x < room.extX + room.extW; ++x)
				m_grid[y][x] = TileType::Floor;
	}
}

void Map::carveCorridor(sf::Vector2i a, sf::Vector2i b) {
	// Anchura aleatoria: 1, 2 o 3 tiles
	int width = 1 + rand() % 2; // 1 o 2 tiles de ancho

	int x = a.x;
	int y = a.y;

	// Horizontal
	while (x != b.x) {
		for (int w = 0; w < width; ++w) {
			if (y + w < m_height - 1)
				m_grid[y + w][x] = TileType::Floor;
		}
		x += (b.x > x) ? 1 : -1;
	}

	// Vertical
	while (y != b.y) {
		for (int w = 0; w < width; ++w) {
			if (x + w < m_width - 1)
				m_grid[y][x + w] = TileType::Floor;
		}
		y += (b.y > y) ? 1 : -1;
	}

	// Celda final
	for (int w = 0; w < width; ++w)
		for (int h = 0; h < width; ++h)
			if (x + w < m_width - 1 && y + h < m_height - 1)
				m_grid[y + h][x + w] = TileType::Floor;
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

	applyRoomShapes();

	for (size_t i = 1; i < m_rooms.size(); ++i) {
		carveCorridor(m_rooms[i - 1].center(), m_rooms[i].center());
	}

	assignRoomTypes();
	m_startPosition = m_rooms.empty()
		? sf::Vector2i(m_width / 2, m_height / 2)
		: m_rooms.front().center();
	placeSignal();
	placeProps(24.f);
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
			for (int attempt = 0; attempt < 30; ++attempt) {
				int tx = room.x + 1 + rand() % (room.w - 2);
				int ty = room.y + 1 + rand() % (room.h - 2);

				// ← solo Floor puro, no props ni paredes
				if (m_grid[ty][tx] != TileType::Floor) continue;

				// No encima de la señal
				if (sf::Vector2i(tx, ty) == m_signalPosition) continue;

				sf::Vector2f worldPos(
					(tx + 0.5f) * tileSize,
					(ty + 0.5f) * tileSize
				);
				m_batteries.emplace_back(worldPos, 25.f);
				break;
			}
		}
	}
}

bool Map::isSolid(int x, int y) const {
	TileType t = getTile(x, y);
	return t == TileType::Wall ||
		t == TileType::PropCrate ||
		t == TileType::PropConsole ||
		t == TileType::PropColumn ||
		t == TileType::PropBarrel ||
		t == TileType::PropDebris;
}

void Map::placeProps(float tileSize) {
	for (const auto& room : m_rooms) {
		// Número de props según tipo de sala
		int propCount = 0;
		switch (room.type) {
		case RoomType::Normal:  propCount = 2 + rand() % 3; break;
		case RoomType::Storage: propCount = 3 + rand() % 4; break;
		case RoomType::Control: propCount = 2 + rand() % 3; break;
		case RoomType::Danger:  propCount = 3 + rand() % 4; break;
		}

		for (int i = 0; i < propCount; ++i) {
			// Intentamos colocar el prop en un tile libre de la sala
			for (int attempt = 0; attempt < 20; ++attempt) {
				int tx = room.x + 1 + rand() % (room.w - 2);
				int ty = room.y + 1 + rand() % (room.h - 2);

				// No colocamos encima de otro prop ni cerca del centro
				sf::Vector2i center = room.center();
				int dx = tx - center.x;
				int dy = ty - center.y;
				if (dx * dx + dy * dy < 4) continue; // muy cerca del centro
				if (isSolid(tx, ty)) continue;

				// Elegir tipo según sala
				TileType prop;
				switch (room.type) {
				case RoomType::Storage:
					prop = (rand() % 2) ? TileType::PropCrate
						: TileType::PropDebris;
					break;
				case RoomType::Control:
					prop = (rand() % 2) ? TileType::PropConsole
						: TileType::PropColumn;
					break;
				case RoomType::Danger:
					prop = (rand() % 2) ? TileType::PropBarrel
						: TileType::PropDebris;
					break;
				default:
					prop = (rand() % 3 == 0) ? TileType::PropColumn
						: TileType::PropCrate;
					break;
				}

				m_grid[ty][tx] = prop;
				break;
			}
		}

		// Columnas en esquinas de salas grandes
		if (room.w >= 8 && room.h >= 6) {
			const int corners[4][2] = {
				{ room.x + 1,       room.y + 1       },
				{ room.x + room.w - 2, room.y + 1       },
				{ room.x + 1,       room.y + room.h - 2 },
				{ room.x + room.w - 2, room.y + room.h - 2 }
			};
			for (auto& c : corners) {
				if (!isSolid(c[0], c[1]))
					m_grid[c[1]][c[0]] = TileType::PropColumn;
			}
		}
	}
}

void Map::applyRoomShapes() {
	for (auto& room : m_rooms) {
		// 30% L-shape, 30% pilares, 40% rectangular normal
		int roll = rand() % 10;

		if (roll < 3 && room.w >= 6 && room.h >= 6) {
			// --- L-Shape ---
			room.shape = RoomShape::LShape;

			// La extensión sale de una esquina aleatoria
			int corner = rand() % 4;
			int extW = room.w / 2 + rand() % 2;
			int extH = room.h / 2 + rand() % 2;

			switch (corner) {
			case 0: // esquina inferior derecha
				room.extX = room.x + room.w;
				room.extY = room.y + room.h - extH;
				break;
			case 1: // esquina inferior izquierda
				room.extX = room.x - extW;
				room.extY = room.y + room.h - extH;
				break;
			case 2: // esquina superior derecha
				room.extX = room.x + room.w;
				room.extY = room.y;
				break;
			case 3: // esquina superior izquierda
				room.extX = room.x - extW;
				room.extY = room.y;
				break;
			}
			room.extW = extW;
			room.extH = extH;

			// Verificamos que la extensión está dentro del mapa
			if (room.extX < 1 || room.extY < 1 ||
				room.extX + room.extW >= m_width - 1 ||
				room.extY + room.extH >= m_height - 1) {
				// Fuera de límites → dejamos rectangular
				room.shape = RoomShape::Rectangle;
				room.extW = 0;
			}
			else {
				carveRoom(room); // tallamos la extensión
			}

		}
		else if (roll < 6 && room.w >= 8 && room.h >= 6) {
			// --- Pilares ---
			room.shape = RoomShape::Pillars;

			// Pilares en cuadrantes interiores
			// Dividimos la sala en cuadrantes y ponemos un pilar en cada uno
			int qw = room.w / 3;
			int qh = room.h / 3;

			// Cuatro pilares simétricamente distribuidos
			int pillarPositions[4][2] = {
				{ room.x + qw,           room.y + qh           },
				{ room.x + 2 * qw,         room.y + qh           },
				{ room.x + qw,           room.y + 2 * qh         },
				{ room.x + 2 * qw,         room.y + 2 * qh         }
			};

			for (auto& p : pillarPositions) {
				// Solo ponemos si es suelo (no queremos tapar el centro)
				if (m_grid[p[1]][p[0]] == TileType::Floor)
					m_grid[p[1]][p[0]] = TileType::PropColumn;
			}
		}
	}
}