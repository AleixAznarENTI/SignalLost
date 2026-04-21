#include "HazardZoneSystem.h"
#include <cstdlib>

void HazardZoneSystem::generate(const Map& map, int zoneCount) {
    m_zones.clear();

    // Tipos disponibles en rotación aleatoria
    HazardType types[] = {
        HazardType::Radiation,
        HazardType::Cold,
        HazardType::Electric
    };

    int attempts = 0;

    while (static_cast<int>(m_zones.size()) < zoneCount && attempts < 1000) {
        ++attempts;

        // Posición aleatoria en el mapa
        int tx = 1 + rand() % (map.getWidth() - 2);
        int ty = 1 + rand() % (map.getHeight() - 2);

        // Solo colocamos zonas sobre tiles de suelo
        if (map.getTile(tx, ty) != TileType::Floor) continue;

        // No colocamos zona encima del spawn ni de la señal
        sf::Vector2i start = map.getStartPosition();
        sf::Vector2i signal = map.getSignalPosition();

        int dxS = tx - start.x, dyS = ty - start.y;
        int dxG = tx - signal.x, dyG = ty - signal.y;

        if (dxS * dxS + dyS * dyS < 100) continue; // muy cerca del spawn
        if (dxG * dxG + dyG * dyG < 64)  continue; // muy cerca de la señal

        HazardZone zone;
        zone.origin = { tx, ty };
        zone.radius = 3 + rand() % 3; // radio entre 3 y 5 tiles
        zone.type = types[rand() % 3];

        m_zones.push_back(zone);
    }
}

HazardType HazardZoneSystem::getHazardAt(int tx, int ty) const {
    for (const auto& zone : m_zones)
        if (zone.contains(tx, ty))
            return zone.type;
    return HazardType::None;
}