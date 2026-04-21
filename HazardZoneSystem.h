#pragma once

#include <vector>
#include "HazardZone.h"
#include "Map.h"

class HazardZoneSystem {
public:
    // Genera zonas aleatorias sobre el mapa ya generado
    void generate(const Map& map, int zoneCount = 4);

    // Devuelve el tipo de hazard en un tile (None si no hay)
    HazardType getHazardAt(int tx, int ty) const;

    const std::vector<HazardZone>& getZones() const { return m_zones; }

private:
    std::vector<HazardZone> m_zones;
};