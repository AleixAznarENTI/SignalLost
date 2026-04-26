#pragma once
#include <SFML/System//Vector2.hpp>
#include "Map.h"
#include "EnemyType.h"

enum class EnemyState {
	Patrol,
	Chase,
	Search,
	Flee
};

class Enemy
{
public:
	Enemy(sf::Vector2f position, float tileSize,
		  EnemyType type = EnemyType::Stalker);

	void update(float		 dt,
				sf::Vector2f playerPos,
				const Map&   map,
				bool		 inDangerZone,
				float		 flashAngle,
				float		 halfAperture);
	sf::Vector2f getPosition() const { return m_position; }
	EnemyState getState() const { return m_state; }
	EnemyType getType() const { return m_type; }
	bool isVisible() const { return m_visible; }

	bool catchesPlayer(sf::Vector2f playerPos, float tileSize) const;
	float getDrainAmount(sf::Vector2f playerPos, float dt) const;

private:
	sf::Vector2f m_position;
	sf::Vector2f m_target;
	sf::Vector2f m_lastSeenPos;
	float m_tileSize;
	float m_speed;
	EnemyType m_type;
	EnemyState m_state = EnemyState::Patrol;
	bool m_visible = true;


	float m_patrolTimer = 0.f;
	float m_searchTimer = 0.f;
	float m_fleeTimer = 0.f;

	float detectionRange() const;
	float chaseRange()     const;
	float baseSpeed()      const;
	float dangerSpeed()    const;

	void updateStalker(float dt, sf::Vector2f playerPos,
		const Map& map, bool inDanger);
	void updateLurker(float dt, sf::Vector2f playerPos,
		const Map& map, bool inDanger);
	void updateDrainer(float dt, const Map& map);
	void updatePhantom(float dt, sf::Vector2f playerPos,
		const Map& map,
		float flashAngle, float halfAperture);
	void updatePatrol(float dt, const Map& map);
	void updateChase(float dt, sf::Vector2f playerPos, const Map& map);
	void updateSearch(float dt, const Map& map);
	
	void moveTowards(sf::Vector2f target, float dt, const Map& map);
	sf::Vector2f randomNearbyFloor(const Map& map) const;
	bool hasLOS(sf::Vector2f playerPos, const Map& map) const;
	bool isLitByFlashlight(float flashAngle, float halfAperture) const;

	static constexpr float DETECTION_RANGE = 200.f;
	static constexpr float CHASE_RANGE = 280.f;
	static constexpr float BASE_SPEED = 60.f;
	static constexpr float DANGER_SPEED = 110.f;
};

