#include "Player.h"
#include <cmath>

Player::Player(sf::Vector2f startPosition, float tileSize)
	: m_position(startPosition)
	, m_tileSize(tileSize)
	, m_radius(tileSize * .35f)
	, m_speed(120.f)
{}

bool Player::collidesWithWall(sf::Vector2f point, const Map& map) const {
	int tileX = static_cast<int>(point.x / m_tileSize);
	int tileY = static_cast<int>(point.y / m_tileSize);
	return map.isSolid(tileX, tileY);
}

void Player::move(sf::Vector2f velocity, float dt, const Map& map) {
	m_lastVelocity = velocity * m_speed * m_speedMultiplier;
	if (velocity.x != 0.f && velocity.y != 0.f) {
		velocity.x *= 1.f / std::sqrt(2.f);
		velocity.y *= 1.f / std::sqrt(2.f);
	}

	float moveX = velocity.x * m_speed * m_speedMultiplier * dt;
	float moveY = velocity.y * m_speed * m_speedMultiplier * dt;
	float r		= m_radius - 1.f;

	sf::Vector2f newPosX = m_position + sf::Vector2f(moveX, 0.f);

	bool blockedX = false;
	if (moveX > 0.f) {
		blockedX = collidesWithWall({ newPosX.x + r, newPosX.y - r }, map)
				|| collidesWithWall({ newPosX.x + r, newPosX.y + r }, map);
	}
	else if (moveX < 0.f) {
		blockedX = collidesWithWall({ newPosX.x - r, newPosX.y - r }, map)
				|| collidesWithWall({ newPosX.x - r, newPosX.y + r }, map);
	}

	if (!blockedX)
		m_position.x = newPosX.x;

	sf::Vector2f newPosY = m_position + sf::Vector2f(0.f, moveY);

	bool blockedY = false;

	if (moveY > 0.f) {
		blockedY = collidesWithWall({ newPosY.x - r, newPosY.y + r }, map)
				|| collidesWithWall({ newPosY.x + r, newPosY.y + r }, map);
	}
	else if (moveY < 0.f) {
		blockedY = collidesWithWall({ newPosY.x - r, newPosY.y - r }, map)
				|| collidesWithWall({ newPosY.x + r, newPosY.y - r }, map);
	}

	if (!blockedY)
		m_position.y = newPosY.y;
}