#include "Enemy.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(sf::Vector2f position, float tileSize)
	: m_position(position)
	, m_target(position)
	, m_lastSeenPos(position)
	, m_tileSize(tileSize)
	, m_speed(BASE_SPEED)
{}

bool Enemy::catchesPlayer(sf::Vector2f playerPos, float tileSize) const {
	sf::Vector2f diff = m_position - playerPos;
	float distSq = diff.x * diff.x + diff.y * diff.y;
	float threshold = tileSize * 0.8f;
	return distSq < threshold * threshold;
}

bool Enemy::hasLOS(sf::Vector2f playerPos, const Map& map) const {
	sf::Vector2f dir = playerPos - m_position;
	float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (len == 0.f) return true;

	sf::Vector2f step = dir / len * (m_tileSize * 0.4f);
	int steps = static_cast<int>(len / (m_tileSize * 0.4f));

	for (int i = 1; i < steps; ++i) {
		sf::Vector2f p = m_position + step * static_cast<float>(i);
		int tx = static_cast<int>(p.x / m_tileSize);
		int ty = static_cast<int>(p.y / m_tileSize);
		if (map.getTile(tx, ty) == TileType::Wall)
			return false;
	}
	return true;
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

	sf::Vector2f vel = (dir / dist); // normalizado

	// Movimiento eje X
	sf::Vector2f newPosX = m_position + sf::Vector2f(vel.x * m_speed * dt, 0.f);
	int txX = static_cast<int>(newPosX.x / m_tileSize);
	int tyX = static_cast<int>(newPosX.y / m_tileSize);
	if (map.getTile(txX, tyX) != TileType::Wall)
		m_position.x = newPosX.x;

	// Movimiento eje Y
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

void Enemy::updateChase(float dt, sf::Vector2f playerPos, const Map& map) {
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

void Enemy::update(float dt,
	sf::Vector2f playerPos,
	const Map& map,
	bool inDangerZone)
{
	m_speed = inDangerZone ? DANGER_SPEED : BASE_SPEED;

	sf::Vector2f diff = playerPos - m_position;
	float distSq = diff.x * diff.x + diff.y * diff.y;

	bool playerInRange = distSq < DETECTION_RANGE * DETECTION_RANGE;
	bool playerInChase = distSq < CHASE_RANGE * CHASE_RANGE;
	bool canSeePlayer = playerInRange && hasLOS(playerPos, map);

	switch (m_state) 
	{
	case EnemyState::Patrol:
		if (canSeePlayer) {
			m_state = EnemyState::Chase;
			m_lastSeenPos = playerPos;
		}
		break;
	case EnemyState::Chase:
		if (!playerInChase || !hasLOS(playerPos, map)) {
			m_state = EnemyState::Search;
			m_searchTimer = 3.f;
		}
		break;
	case EnemyState::Search:
		if (canSeePlayer)
			m_state = EnemyState::Chase;
		break;
	}

	switch (m_state) {
	case EnemyState::Patrol: updatePatrol(dt, map);           break;
	case EnemyState::Chase:  updateChase(dt, playerPos, map); break;
	case EnemyState::Search: updateSearch(dt, map);           break;
	}
}