#pragma once
#include <SFML/System//Vector2.hpp>
#include "Map.h"

enum class EnemyState {
	Patrol,
	Chase,
	Search
};

class Enemy
{
public:
	Enemy(sf::Vector2f position, float tileSize);

	void update(float		 dt,
				sf::Vector2f playerPos,
				const Map&   map,
				bool		 inDangerZone);
	sf::Vector2f getPosition() const { return m_position; }
	EnemyState getState() const { return m_state; }
	bool isDead() const { return m_dead; }

	bool catchesPlayer(sf::Vector2f playerPos, float tileSize) const;

private:
	sf::Vector2f m_position;
	sf::Vector2f m_target;
	sf::Vector2f m_lastSeenPos;
	float m_tileSize;
	float m_speed;
	EnemyState m_state = EnemyState::Patrol;
	bool m_dead = false;
	float m_patrolTimer = 0.f;
	float m_searchTimer = 0.f;

	static constexpr float DETECTION_RANGE = 200.f;
	static constexpr float CHASE_RANGE = 280.f;
	static constexpr float BASE_SPEED = 60.f;
	static constexpr float DANGER_SPEED = 110.f;

	void updatePatrol(float dt, const Map& map);
	void updateChase(float dt, sf::Vector2f playerPos, const Map& map);
	void updateSearch(float dt, const Map& map);

	void moveTowards(sf::Vector2f target, float dt, const Map& map);

	sf::Vector2f randomNearbyFloor(const Map& map) const;

	bool hasLOS(sf::Vector2f playerPos, const Map& map) const;
};

