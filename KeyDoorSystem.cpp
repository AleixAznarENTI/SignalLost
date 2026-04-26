#include "KeyDoorSystem.h"
#include <cstdlib>

void KeyDoorSystem::generate(const Map& map, float tileSize) {
    m_keys.clear();
    m_doors.clear();
    m_nextId = 0;

    const auto& rooms = map.getRooms();
    if (rooms.size() < 4) return;

    // Máximo 2 pares llave-puerta por mapa
    int pairs = 1 + rand() % 2;
    int attempts = 0;

    while (static_cast<int>(m_keys.size()) < pairs && attempts < 100) {
        ++attempts;

        // Sala aleatoria para la llave (no spawn, no señal)
        int idxKey = rand() % rooms.size();
        if (idxKey == 0) continue;

        const Room& keyRoom = rooms[idxKey];
        if (keyRoom.type == RoomType::Safe) continue;

        // Sala diferente para la puerta
        int idxDoor = rand() % rooms.size();
        if (idxDoor == idxKey) continue;

        const Room& doorRoom = rooms[idxDoor];

        // Posición de la llave
        int kx = keyRoom.x + 1 + rand() % (keyRoom.w - 2);
        int ky = keyRoom.y + 1 + rand() % (keyRoom.h - 2);
        if (map.getTile(kx, ky) != TileType::Floor) continue;

        // Posición de la puerta — en el borde de la sala
        int dx = doorRoom.x + rand() % doorRoom.w;
        int dy = doorRoom.y;

        Key key;
        key.position = sf::Vector2f(
            (kx + 0.5f) * tileSize,
            (ky + 0.5f) * tileSize
        );
        key.doorId = m_nextId;

        Door door;
        door.tilePos = { dx, dy };
        door.keyId = m_nextId;

        m_keys.push_back(key);
        m_doors.push_back(door);
        ++m_nextId;
    }
}

void KeyDoorSystem::onKeyCollected(int doorId, Map& map) {
    for (auto& door : m_doors) {
        if (door.keyId != doorId || door.open) continue;
        door.open = true;
        // Abrimos el tile de la puerta en el mapa
        map.setTile(door.tilePos.x, door.tilePos.y, TileType::Floor);
    }
}