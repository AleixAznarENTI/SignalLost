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
	int width = 1 + rand() % 2;
	bool horizontalFirst = (rand() % 2 == 0);

	int x = a.x;
	int y = a.y;

	// Punto de giro con desviación aleatoria
	// En vez de girar exactamente en b.x o b.y, giramos un poco antes o después
	int jitterX = (rand() % 5) - 2; // -2 a +2 tiles de desviación
	int jitterY = (rand() % 5) - 2;

	int pivotX = b.x + jitterX;
	int pivotY = b.y + jitterY;

	// Clampeamos para no salir del mapa
	pivotX = std::max(1, std::min(m_width - 2, pivotX));
	pivotY = std::max(1, std::min(m_height - 2, pivotY));

	auto carveH = [&](int fromX, int toX, int atY) {
		int step = (toX > fromX) ? 1 : -1;
		for (int cx = fromX; cx != toX; cx += step)
			for (int w = 0; w < width; ++w)
				if (atY + w < m_height - 1)
					m_grid[atY + w][cx] = TileType::Floor;
		};

	auto carveV = [&](int fromY, int toY, int atX) {
		int step = (toY > fromY) ? 1 : -1;
		for (int cy = fromY; cy != toY; cy += step)
			for (int w = 0; w < width; ++w)
				if (atX + w < m_width - 1)
					m_grid[cy][atX + w] = TileType::Floor;
		};

	if (horizontalFirst) {
		carveH(x, pivotX, y);
		// Ensanchamiento en el giro
		for (int w = -1; w <= width; ++w)
			if (y + w > 0 && y + w < m_height - 1)
				m_grid[y + w][pivotX] = TileType::Floor;
		carveV(y, pivotY, pivotX);
		// Segundo tramo hasta b
		carveH(pivotX, b.x, pivotY);
		carveV(pivotY, b.y, b.x);
	}
	else {
		carveV(y, pivotY, x);
		for (int w = -1; w <= width; ++w)
			if (x + w > 0 && x + w < m_width - 1)
				m_grid[pivotY][x + w] = TileType::Floor;
		carveH(x, pivotX, pivotY);
		carveV(pivotY, b.y, pivotX);
		carveH(pivotX, b.x, b.y);
	}

	// Celda final
	for (int w = 0; w < width; ++w)
		for (int h = 0; h < width; ++h)
			if (b.x + w < m_width - 1 && b.y + h < m_height - 1)
				m_grid[b.y + h][b.x + w] = TileType::Floor;
}

void Map::generate(int roomAttempts) {
	fillWithWalls();

	auto randInt = [](int min, int max) {
		return min + rand() % (max - min + 1);
		};

	for (int i = 0; i < roomAttempts; ++i) {
		Room candidate;
		// Salas más grandes — sensación de módulos espaciales
		candidate.w = randInt(8, 16);
		candidate.h = randInt(6, 12);
		candidate.x = randInt(2, m_width - candidate.w - 2);
		candidate.y = randInt(2, m_height - candidate.h - 2);

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

	for (size_t i = 1; i < m_rooms.size(); ++i)
		carveCorridor(m_rooms[i - 1].center(), m_rooms[i].center());

	placeSecondaryRooms();
	connectSecondaryRooms();
	carveExtraCorridors();
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

float Map::roomDistance(const Room& a, const Room& b) const {
	sf::Vector2i ca = a.center();
	sf::Vector2i cb = b.center();
	float dx = static_cast<float>(ca.x - cb.x);
	float dy = static_cast<float>(ca.y - cb.y);
	return std::sqrt(dx * dx + dy * dy);
}

void Map::carveExtraCorridors() {
	if (m_rooms.size() < 3) return;

	// Número de conexiones extra: 20-30% del número de salas
	int extras = std::max(1, static_cast<int>(m_rooms.size() * 0.25f));

	// Distancia máxima para considerar una conexión extra
	// (no queremos conectar salas demasiado lejanas — el corredor sería
	//  muy largo y atravesaría otras salas)
	const float MAX_DIST = 20.f;

	int attempts = 0;
	int placed = 0;

	while (placed < extras && attempts < 200) {
		++attempts;

		// Dos salas aleatorias no adyacentes en la cadena
		int idxA = rand() % m_rooms.size();
		int idxB = rand() % m_rooms.size();

		if (idxA == idxB) continue;
		if (std::abs(idxA - idxB) == 1) continue; // ya están conectadas

		const Room& a = m_rooms[idxA];
		const Room& b = m_rooms[idxB];

		// Solo conectamos si están suficientemente cerca
		if (roomDistance(a, b) > MAX_DIST) continue;

		carveCorridor(a.center(), b.center());
		++placed;
	}
}

void Map::placeSecondaryRooms() {
	if (m_rooms.empty()) return;
	size_t primaryCount = m_rooms.size();

	auto randInt = [](int min, int max) {
		return min + rand() % (max - min + 1);
		};

	// Intentamos colocar una sala secundaria en cada corredor
	// entre salas principales adyacentes
	int secondaryCount = 0;
	const int MAX_SECONDARY = static_cast<int>(m_rooms.size() * 0.6f);

	for (size_t i = 0; i + 1 < m_rooms.size() &&
		secondaryCount < MAX_SECONDARY; ++i) {

		sf::Vector2i a = m_rooms[i].center();
		sf::Vector2i b = m_rooms[i + 1].center();

		// Punto medio entre las dos salas
		int midX = (a.x + b.x) / 2;
		int midY = (a.y + b.y) / 2;

		// Sala secundaria pequeña centrada en el punto medio
		Room secondary;
		secondary.w = randInt(3, 6);
		secondary.h = randInt(3, 5);
		secondary.x = midX - secondary.w / 2;
		secondary.y = midY - secondary.h / 2;

		// Verificar límites
		if (secondary.x < 1 || secondary.y < 1 ||
			secondary.x + secondary.w >= m_width - 1 ||
			secondary.y + secondary.h >= m_height - 1)
			continue;

		// Verificar que no solapa con salas existentes
		bool valid = true;
		for (const auto& existing : m_rooms) {
			if (secondary.overlaps(existing)) {
				valid = false;
				break;
			}
		}

		if (!valid) continue;

		// Asignar tipo — las secundarias son mayormente normales
		// pero ocasionalmente Storage o Danger
		int roll = rand() % 10;
		if (roll < 2) secondary.type = RoomType::Storage;
		else if (roll < 4) secondary.type = RoomType::Danger;
		else               secondary.type = RoomType::Normal;

		carveRoom(secondary);
		m_rooms.push_back(secondary);
		++secondaryCount;
	}
}

void Map::connectSecondaryRooms() {
	// Las salas secundarias (las últimas añadidas) se conectan
	// a la sala principal más cercana
	size_t primaryCount = m_rooms.size();

	for (size_t i = primaryCount; i < m_rooms.size(); ++i) {
		const Room& secondary = m_rooms[i];
		float       minDist = std::numeric_limits<float>::max();
		size_t      closest = 0;

		// Buscar la sala principal más cercana
		for (size_t j = 0; j < primaryCount; ++j) {
			float dist = roomDistance(secondary, m_rooms[j]);
			if (dist < minDist) {
				minDist = dist;
				closest = j;
			}
		}

		carveCorridor(secondary.center(), m_rooms[closest].center());
	}
}