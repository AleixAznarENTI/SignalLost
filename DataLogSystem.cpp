#include "DataLogSystem.h"
#include <cstdlib>

void DataLogSystem::generate(const Map& map, float tileSize) {
    m_logs.clear();

    // Mensajes de historia — se eligen aleatoriamente
    const std::string messages[] = {
        "Day 47. The station went dark\n"
        "three hours ago. No response\n"
        "from command.",

        "Reactor output nominal.\n"
        "But something triggered\n"
        "the containment protocol.",

        "They said it was just\n"
        "a routine inspection.\n"
        "Nobody came back.",

        "Emergency beacon activated.\n"
        "Battery reserves critical.\n"
        "If anyone finds this...",

        "Sector 7 is compromised.\n"
        "Do not enter under\n"
        "any circumstances.",

        "I can hear it moving\n"
        "in the vents. It knows\n"
        "I am here.",

        "Station log 2847:\n"
        "All crew accounted for.\n"
        "Anomaly still unresolved.",

        "The signal is real.\n"
        "Follow it. Get out.\n"
        "Don't look back."
    };

    const int MSG_COUNT = 8;

    for (const auto& room : map.getRooms()) {
        // 25% de probabilidad por sala, máximo 1 log por sala
        if (rand() % 4 != 0) continue;

        for (int attempt = 0; attempt < 20; ++attempt) {
            int tx = room.x + 1 + rand() % (room.w - 2);
            int ty = room.y + 1 + rand() % (room.h - 2);

            if (map.getTile(tx, ty) != TileType::Floor) continue;

            DataLog log;
            log.position = sf::Vector2f(
                (tx + 0.5f) * tileSize,
                (ty + 0.5f) * tileSize
            );
            log.message = messages[rand() % MSG_COUNT];
            log.energyBonus = 15.f;

            m_logs.push_back(log);
            break;
        }
    }
}