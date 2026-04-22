#include "PowerUpSystem.h"
#include <cstdlib>
#include <algorithm>

void PowerUpSystem::reset() {
    m_pickups.clear();
    m_actives.clear();
    m_queues.clear();
}

void PowerUpSystem::generate(const Map& map, float tileSize) {
    reset();

    PowerUpType allTypes[] = {
        PowerUpType::Flashlight,
        PowerUpType::Speed,
        PowerUpType::Shield,
        PowerUpType::Radar,
        PowerUpType::Battery
    };

    for (const auto& room : map.getRooms()) {
        if (room.type != RoomType::Storage) continue;

        // 1-2 power-ups por sala Storage
        int count = 1 + rand() % 2;

        for (int i = 0; i < count; ++i) {
            int tx = room.x + 1 + rand() % (room.w - 2);
            int ty = room.y + 1 + rand() % (room.h - 2);

            PowerUpType type = allTypes[rand() % 5];

            m_pickups.emplace_back(
                sf::Vector2f((tx + 0.5f) * tileSize,
                    (ty + 0.5f) * tileSize),
                type
            );
        }
    }
}

void PowerUpSystem::update(float dt) {
    for (auto& active : m_actives) {
        if (active.timeRemaining < 0.f) continue; // shield/permanente
        active.timeRemaining -= dt;
    }

    // Eliminamos los expirados y activamos el siguiente de la cola
    for (auto it = m_actives.begin(); it != m_actives.end(); ) {
        if (it->timeRemaining >= 0.f && it->timeRemaining <= 0.f) {
            PowerUpType type = it->type;
            it = m_actives.erase(it);
            activateNext(type); // ← siguiente en cola
        }
        else {
            ++it;
        }
    }
}

std::optional<PowerUpType> PowerUpSystem::tryCollect(
    sf::Vector2f playerPos, float tileSize)
{
    for (auto& pickup : m_pickups) {
        if (pickup.isCollected()) continue;

        sf::Vector2f diff = playerPos - pickup.getPosition();
        float distSq = diff.x * diff.x + diff.y * diff.y;
        float threshold = tileSize * 0.8f * tileSize * 0.8f;

        if (distSq < threshold) {
            pickup.collect();
            return pickup.getType();
        }
    }
    return std::nullopt;
}

void PowerUpSystem::activate(PowerUpType type) {
    PowerUpDef def = getPowerUpDef(type);

    // Instantáneos (Battery) no se encolan — efecto inmediato en Game
    if (def.duration == 0.f && type != PowerUpType::Shield) return;

    // Shield: solo uno activo — si ya hay, se encola
    if (type == PowerUpType::Shield) {
        if (!hasShield()) {
            m_actives.push_back({ type, -1.f }); // -1 = hasta consumir
        }
        else {
            m_queues[type].push(-1.f);
        }
        return;
    }

    // Durables: si ya hay uno activo del mismo tipo, encolar
    bool alreadyActive = isActive(type);
    if (alreadyActive) {
        m_queues[type].push(def.duration);
    }
    else {
        m_actives.push_back({ type, def.duration });
    }
}

void PowerUpSystem::activateNext(PowerUpType type) {
    auto it = m_queues.find(type);
    if (it == m_queues.end() || it->second.empty()) return;

    float duration = it->second.front();
    it->second.pop();
    m_actives.push_back({ type, duration });
}

bool PowerUpSystem::isActive(PowerUpType type) const {
    for (const auto& a : m_actives)
        if (a.type == type) return true;
    return false;
}

float PowerUpSystem::getTimeRemaining(PowerUpType type) const {
    for (const auto& a : m_actives)
        if (a.type == type) return a.timeRemaining;
    return 0.f;
}

bool PowerUpSystem::hasShield() const {
    return isActive(PowerUpType::Shield);
}

void PowerUpSystem::consumeShield() {
    auto it = std::find_if(m_actives.begin(), m_actives.end(),
        [](const ActivePowerUp& a) {
            return a.type == PowerUpType::Shield;
        });

    if (it != m_actives.end()) {
        m_actives.erase(it);
        activateNext(PowerUpType::Shield);
    }
}