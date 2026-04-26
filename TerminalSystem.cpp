#include "TerminalSystem.h"

void TerminalSystem::generate(const Map& map, float tileSize) {
    m_terminals.clear();

    // Una terminal por sala Control
    for (const auto& room : map.getRooms()) {
        if (room.type != RoomType::Control) continue;

        sf::Vector2i center = room.center();
        Terminal t;
        t.position = sf::Vector2f(
            (center.x + 0.5f) * tileSize,
            (center.y + 0.5f) * tileSize
        );
        m_terminals.push_back(t);
    }
}

void TerminalSystem::update(float dt) {
    for (auto& t : m_terminals)
        t.update(dt);
}

bool TerminalSystem::isSignalRevealed() const {
    for (const auto& t : m_terminals)
        if (t.isRevealing()) return true;
    return false;
}