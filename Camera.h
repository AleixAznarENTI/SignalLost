#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
class Camera
{
public:
	Camera(sf::RenderWindow& window, float tileSize);

	void follow(sf::Vector2f target);

	const sf::View& getView() const { return m_view; }

private:
	sf::View m_view;
	float	 m_tileSize;
};

