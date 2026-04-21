#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
class Camera
{
public:
	Camera(sf::RenderWindow& window, float tileSize);

	void follow(sf::Vector2f target);
	void triggerShake(float duration = 0.4f, float intensity = 8.f);
	void updateShake(float dt);
	bool isShaking() const { return m_shakeTimer > 0.f; }

	const sf::View& getView() const { return m_view; }

private:
	sf::View m_view;
	float	 m_tileSize;
	float m_shakeTimer = 0.f;
	float m_shakeDuration = 0.f;
	float m_shakeIntensity = 0.f;
};

