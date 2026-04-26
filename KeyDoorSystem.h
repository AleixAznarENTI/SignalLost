#pragma once
#include <vector>
#include "KeyDoor.h"
#include "Map.h"

class KeyDoorSystem {
public:
    void generate(const Map& map, float tileSize);
    void reset() { m_keys.clear(); m_doors.clear(); }

    const std::vector<Key>& getKeys()  const { return m_keys; }
    const std::vector<Door>& getDoors() const { return m_doors; }
    std::vector<Key>& getKeys() { return m_keys; }
    std::vector<Door>& getDoors() { return m_doors; }

    // Abre las puertas asociadas a una llave recogida
    void onKeyCollected(int doorId, Map& map);

private:
    std::vector<Key>  m_keys;
    std::vector<Door> m_doors;
    int               m_nextId = 0;
};