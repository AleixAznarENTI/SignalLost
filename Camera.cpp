#include "Camera.h"
Camera::Camera(sf::RenderWindow& window, float tileSize)
	: m_tileSize(tileSize)
{
	m_view.setSize(sf::Vector2f(window.getSize()));
	m_view.setCenter(sf::Vector2f(window.getSize()) / 2.f);
}

void Camera:: follow(sf::Vector2f target) {

	//TODO : clamp target to map bounds
	//TODO : add some easing
	m_view.setCenter(target);
}