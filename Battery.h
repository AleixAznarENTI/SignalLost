#pragma once

#include <SFML/System/Vector2.hpp>

class Battery
{
public:
	Battery(sf::Vector2f position, float restoreAmount = 25.f);

	sf::Vector2f getPosition() const { return m_position; }
	float getRestoreAmount() const { return m_restoreAmount; }
	bool isCollected() const { return m_collected; }

	void collect() { m_collected = true; }

private:
	sf::Vector2f m_position;
	float m_restoreAmount;
	bool m_collected = false;
};

