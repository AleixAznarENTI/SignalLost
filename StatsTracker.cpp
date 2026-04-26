#include "StatsTracker.h"
#include <cmath>
#include <sstream>
#include <iomanip>

void StatsTracker::reset() {
    m_time = 0.f;
    m_batteries = 0;
    m_distance = 0.f;
    m_roomsVisited = 0;
    m_logsFound = 0;
    m_firstUpdate = true;
    m_visitedRooms.clear();
}

void StatsTracker::update(float dt, sf::Vector2f playerPos, float tileSize) {
    m_time += dt;

    // Distancia recorrida en tiles
    if (!m_firstUpdate) {
        sf::Vector2f diff = playerPos - m_lastPos;
        float        dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        m_distance += dist / tileSize;
    }

    m_lastPos = playerPos;
    m_firstUpdate = false;
}

void StatsTracker::registerBatteryPickup() {
    ++m_batteries;
}

void StatsTracker::registerRoomVisit(int roomIndex) {
    if (m_visitedRooms.insert(roomIndex).second)
        ++m_roomsVisited; // solo cuenta si es nueva
}

std::string StatsTracker::getFormattedTime() const {
    int minutes = static_cast<int>(m_time) / 60;
    int seconds = static_cast<int>(m_time) % 60;

    std::ostringstream oss;
    oss << minutes << ":" << std::setw(2) << std::setfill('0') << seconds;
    return oss.str();
}

void StatsTracker::registerLogFound() {
	++m_logsFound;
}