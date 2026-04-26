#include "Enemy.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#define M_PI  3.14159265358979323846

Enemy::Enemy(sf::Vector2f position, float tileSize, EnemyType type)
    : m_position(position)
    , m_target(position)
    , m_lastSeenPos(position)
    , m_tileSize(tileSize)
    , m_type(type)
    , m_speed(baseSpeed())
    , m_visible(type != EnemyType::Phantom)
{}

// --- Rangos por tipo ---

float Enemy::detectionRange() const {
    switch (m_type) {
    case EnemyType::Lurker:  return 100.f; // poco rango
    case EnemyType::Drainer: return 0.f;   // no detecta
    case EnemyType::Phantom: return 150.f;
    default:                 return 200.f; // Stalker
    }
}

float Enemy::chaseRange() const {
    switch (m_type) {
    case EnemyType::Lurker:  return 140.f;
    case EnemyType::Drainer: return 0.f;
    case EnemyType::Phantom: return 180.f;
    default:                 return 280.f;
    }
}

float Enemy::baseSpeed() const {
    switch (m_type) {
    case EnemyType::Lurker:  return 140.f; // muy rápido
    case EnemyType::Drainer: return 20.f;  // casi quieto
    case EnemyType::Phantom: return 50.f;  // lento y silencioso
    default:                 return 60.f;  // Stalker
    }
}

float Enemy::dangerSpeed() const {
    switch (m_type) {
    case EnemyType::Lurker:  return 200.f;
    case EnemyType::Drainer: return 30.f;
    case EnemyType::Phantom: return 70.f;
    default:                 return 110.f;
    }
}

// --- Helpers ---

bool Enemy::catchesPlayer(sf::Vector2f playerPos, float tileSize) const {
    if (m_type == EnemyType::Drainer) return false; // no mata
    sf::Vector2f diff = m_position - playerPos;
    float distSq = diff.x * diff.x + diff.y * diff.y;
    float threshold = tileSize * 0.8f;
    return distSq < threshold * threshold;
}

float Enemy::getDrainAmount(sf::Vector2f playerPos, float dt) const {
    if (m_type != EnemyType::Drainer) return 0.f;

    sf::Vector2f diff = m_position - playerPos;
    float        distSq = diff.x * diff.x + diff.y * diff.y;
    float        range = m_tileSize * 4.f;

    if (distSq > range * range) return 0.f;

    // Más cerca = más drain
    float dist = std::sqrt(distSq);
    float intensity = 1.f - (dist / range);
    return intensity * 12.f * dt; // hasta 12 energía/segundo
}

bool Enemy::hasLOS(sf::Vector2f playerPos, const Map& map) const {
    sf::Vector2f dir = playerPos - m_position;
    float        len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0.f) return true;

    sf::Vector2f step = dir / len * (m_tileSize * 0.4f);
    int          steps = static_cast<int>(len / (m_tileSize * 0.4f));

    for (int i = 1; i < steps; ++i) {
        sf::Vector2f p = m_position + step * static_cast<float>(i);
        int          tx = static_cast<int>(p.x / m_tileSize);
        int          ty = static_cast<int>(p.y / m_tileSize);
        if (map.getTile(tx, ty) == TileType::Wall)
            return false;
    }
    return true;
}

bool Enemy::isLitByFlashlight(float flashAngle,
    float halfAperture) const
{
    // El Phantom se revela si está dentro del cono de la linterna
    // Lo comprobamos desde la posición del jugador — se pasa en update
    // Aquí solo calculamos el ángulo relativo
    return true; // se calcula en updatePhantom con la posición del jugador
}

sf::Vector2f Enemy::randomNearbyFloor(const Map& map) const {
    for (int attempt = 0; attempt < 30; ++attempt) {
        int dx = (rand() % 17) - 8;
        int dy = (rand() % 17) - 8;
        int tx = static_cast<int>(m_position.x / m_tileSize) + dx;
        int ty = static_cast<int>(m_position.y / m_tileSize) + dy;
        if (map.getTile(tx, ty) == TileType::Floor)
            return sf::Vector2f((tx + 0.5f) * m_tileSize,
                (ty + 0.5f) * m_tileSize);
    }
    return m_position;
}

void Enemy::moveTowards(sf::Vector2f target, float dt, const Map& map) {
    sf::Vector2f dir = target - m_position;
    float        dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (dist < 2.f) return;

    sf::Vector2f vel = dir / dist;

    sf::Vector2f newPosX = m_position + sf::Vector2f(vel.x * m_speed * dt, 0.f);
    int txX = static_cast<int>(newPosX.x / m_tileSize);
    int tyX = static_cast<int>(newPosX.y / m_tileSize);
    if (map.getTile(txX, tyX) != TileType::Wall)
        m_position.x = newPosX.x;

    sf::Vector2f newPosY = m_position + sf::Vector2f(0.f, vel.y * m_speed * dt);
    int txY = static_cast<int>(newPosY.x / m_tileSize);
    int tyY = static_cast<int>(newPosY.y / m_tileSize);
    if (map.getTile(txY, tyY) != TileType::Wall)
        m_position.y = newPosY.y;
}

void Enemy::updatePatrol(float dt, const Map& map) {
    m_patrolTimer -= dt;
    if (m_patrolTimer <= 0.f) {
        m_target = randomNearbyFloor(map);
        m_patrolTimer = 2.f + (rand() % 20) * 0.1f;
    }
    moveTowards(m_target, dt, map);
}

void Enemy::updateChase(float dt, sf::Vector2f playerPos,
    const Map& map) {
    m_target = playerPos;
    m_lastSeenPos = playerPos;
    moveTowards(m_target, dt, map);
}

void Enemy::updateSearch(float dt, const Map& map) {
    m_searchTimer -= dt;
    moveTowards(m_lastSeenPos, dt, map);

    sf::Vector2f diff = m_position - m_lastSeenPos;
    float distSq = diff.x * diff.x + diff.y * diff.y;

    if (distSq < 100.f || m_searchTimer <= 0.f) {
        m_state = EnemyState::Patrol;
        m_patrolTimer = 0.f;
    }
}

// --- Comportamientos por tipo ---

void Enemy::updateStalker(float dt, sf::Vector2f playerPos,
    const Map& map, bool inDanger) {
    m_speed = inDanger ? dangerSpeed() : baseSpeed();

    sf::Vector2f diff = playerPos - m_position;
    float        distSq = diff.x * diff.x + diff.y * diff.y;
    bool canSee = distSq < detectionRange() * detectionRange()
        && hasLOS(playerPos, map);
    bool inChaseRange = distSq < chaseRange() * chaseRange();

    switch (m_state) {
    case EnemyState::Patrol:
        if (canSee) { m_state = EnemyState::Chase; m_lastSeenPos = playerPos; }
        break;
    case EnemyState::Chase:
        if (!inChaseRange || !hasLOS(playerPos, map)) {
            m_state = EnemyState::Search; m_searchTimer = 3.f;
        }
        break;
    case EnemyState::Search:
        if (canSee) m_state = EnemyState::Chase;
        break;
    default: break;
    }

    switch (m_state) {
    case EnemyState::Patrol: updatePatrol(dt, map);           break;
    case EnemyState::Chase:  updateChase(dt, playerPos, map); break;
    case EnemyState::Search: updateSearch(dt, map);           break;
    default: break;
    }
}

void Enemy::updateLurker(float dt, sf::Vector2f playerPos,
    const Map& map, bool inDanger) {
    // Igual que Stalker pero con sprint — speed es muy alta
    // y pierde el chase muy rápido si el jugador escapa
    m_speed = inDanger ? dangerSpeed() : baseSpeed();

    sf::Vector2f diff = playerPos - m_position;
    float        distSq = diff.x * diff.x + diff.y * diff.y;
    bool canSee = distSq < detectionRange() * detectionRange()
        && hasLOS(playerPos, map);

    // Lurker pierde el rastro más rápido que el Stalker
    bool inChaseRange = distSq < chaseRange() * chaseRange();

    switch (m_state) {
    case EnemyState::Patrol:
        if (canSee) { m_state = EnemyState::Chase; }
        break;
    case EnemyState::Chase:
        if (!inChaseRange || !hasLOS(playerPos, map)) {
            // Se confunde rápido — solo busca 1 segundo
            m_state = EnemyState::Search; m_searchTimer = 1.f;
        }
        break;
    case EnemyState::Search:
        if (canSee) m_state = EnemyState::Chase;
        break;
    default: break;
    }

    switch (m_state) {
    case EnemyState::Patrol: updatePatrol(dt, map);           break;
    case EnemyState::Chase:  updateChase(dt, playerPos, map); break;
    case EnemyState::Search: updateSearch(dt, map);           break;
    default: break;
    }
}

void Enemy::updateDrainer(float dt, const Map& map) {
    // El Drainer patrulla lentamente — su arma es el aura, no el contacto
    m_speed = baseSpeed();
    updatePatrol(dt, map);
}

void Enemy::updatePhantom(float dt, sf::Vector2f playerPos,
    const Map& map,
    float flashAngle, float halfAperture)
{
    m_speed = baseSpeed();

    // Comprobar si la linterna lo ilumina directamente
    sf::Vector2f diff = m_position - playerPos;
    float        dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    float        angle = std::atan2(diff.y, diff.x);
    float        angleDiff = angle - flashAngle;

    while (angleDiff > M_PI) angleDiff -= 2.f * M_PI;
    while (angleDiff < -M_PI) angleDiff += 2.f * M_PI;

    bool litByFlash = std::abs(angleDiff) < halfAperture
        && dist < 200.f
        && hasLOS(playerPos, map);

    m_visible = litByFlash;

    if (litByFlash) {
        // Iluminado → huye del jugador
        m_state = EnemyState::Flee;
        m_fleeTimer = 2.f;
    }

    if (m_state == EnemyState::Flee) {
        m_fleeTimer -= dt;

        // Huye en dirección opuesta al jugador
        sf::Vector2f awayDir = m_position - playerPos;
        float        len = std::sqrt(awayDir.x * awayDir.x +
            awayDir.y * awayDir.y);
        if (len > 0.f) {
            sf::Vector2f fleeTarget = m_position +
                (awayDir / len) * 100.f;
            moveTowards(fleeTarget, dt, map);
        }

        if (m_fleeTimer <= 0.f) {
            m_state = EnemyState::Patrol;
            m_visible = false;
        }
        return;
    }

    // Invisible → patrulla silencioso, pero mata al contacto
    updatePatrol(dt, map);
}

// --- Update principal ---

void Enemy::update(float        dt,
    sf::Vector2f playerPos,
    const Map& map,
    bool         inDangerZone,
    float        flashAngle,
    float        halfAperture)
{
    switch (m_type) {
    case EnemyType::Stalker:
        updateStalker(dt, playerPos, map, inDangerZone);
        break;
    case EnemyType::Lurker:
        updateLurker(dt, playerPos, map, inDangerZone);
        break;
    case EnemyType::Drainer:
        updateDrainer(dt, map);
        break;
    case EnemyType::Phantom:
        updatePhantom(dt, playerPos, map, flashAngle, halfAperture);
        break;
    }
}