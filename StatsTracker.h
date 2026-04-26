#pragma once

#include <SFML/System/Vector2.hpp>
#include <set>
#include <string>

class StatsTracker {
public:
    void reset();
    void update(float dt, sf::Vector2f playerPos, float tileSize);

    void registerBatteryPickup();
    void registerRoomVisit(int roomIndex);
    void registerLogFound();

    float        getTime()              const { return m_time; }
    int          getBatteriesCollected()const { return m_batteries; }
    float        getDistanceTraveled()  const { return m_distance; }
    int          getRoomsVisited()      const { return m_roomsVisited; }
	int          getLogsFound()         const { return m_logsFound; }

    // Formatea el tiempo como "1:23"
    std::string getFormattedTime() const;

private:
    float        m_time = 0.f;
    int          m_batteries = 0;
    float        m_distance = 0.f;
    int          m_roomsVisited = 0;
	int          m_logsFound = 0;

    sf::Vector2f m_lastPos;
    bool         m_firstUpdate = true;

    std::set<int> m_visitedRooms; // índices de salas visitadas
};