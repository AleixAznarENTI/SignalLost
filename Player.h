#pragma once

#include <SFML/System/Vector2.hpp>
#include "Map.h"

class Player
{
public:
	Player(sf::Vector2f startPosition, float tileSize);

	void move(sf::Vector2f velocity, float dt, const Map& map);

	sf::Vector2f getPosition() const { return m_position; }
	float		 getRadius() const { return m_radius; }
	void setSpeedMultiplier(float multiplier) { m_speedMultiplier = multiplier; }
	float getSpeedMultiplier() const { return m_speedMultiplier; }
private:
	sf::Vector2f m_position;
	float 		 m_radius;
	float		 m_tileSize;
	float 		 m_speed;
	float		 m_speedMultiplier = 1.f;

	bool collidesWithWall(sf::Vector2f point, const Map& map) const;
};

