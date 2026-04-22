#pragma once

#include <vector>
#include <queue>
#include <map>
#include "PowerUp.h"
#include "Map.h"
#include <optional>

// Estado de un power-up activo
struct ActivePowerUp {
    PowerUpType type;
    float       timeRemaining; // -1 = permanente hasta evento
};

class PowerUpSystem {
public:
    // Genera power-ups en salas Storage
    void generate(const Map& map, float tileSize);

    // Actualiza timers de activos
    void update(float dt);

    // Intenta recoger power-ups cercanos al jugador
    // Devuelve el tipo recogido o nullopt
    std::optional<PowerUpType> tryCollect(sf::Vector2f playerPos,
        float        tileSize);

    // Activa un power-up — si es del mismo tipo, se encola
    void activate(PowerUpType type);

    // Consultas de estado
    bool  isActive(PowerUpType type)    const;
    float getTimeRemaining(PowerUpType) const;
    bool  hasShield()                   const;
    void  consumeShield();

    const std::vector<PowerUp>& getPickups()  const { return m_pickups; }
    const std::vector<ActivePowerUp>& getActives() const { return m_actives; }

    void reset();

private:
    std::vector<PowerUp>       m_pickups;  // power-ups en el mundo
    std::vector<ActivePowerUp> m_actives;  // activos simultáneos

    // Cola por tipo para acumulación
    std::map<PowerUpType, std::queue<float>> m_queues;

    // Activa el siguiente de la cola si hay
    void activateNext(PowerUpType type);
};