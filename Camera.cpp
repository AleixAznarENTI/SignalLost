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

void Camera::triggerShake(float duration, float intensity) {
	m_shakeTimer = duration;
	m_shakeDuration = duration;
	m_shakeIntensity = intensity;
}

void Camera::updateShake(float dt) {
	if (m_shakeTimer <= 0.f) return;

	m_shakeTimer -= dt;

	float progress = m_shakeTimer / m_shakeDuration;
	float currentI = m_shakeIntensity * progress;

	float offsetX = ((rand() % 200) - 100) / 100.f * currentI;
	float offsetY = ((rand() % 200) - 100) / 100.f * currentI;

	m_view.move({ offsetX, offsetY });
}