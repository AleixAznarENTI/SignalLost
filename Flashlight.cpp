#include "Flashlight.h"
#include <cmath>
#include <cstdint>
#include <stdexcept>
#define M_PI 3.14159265358979323846

Flashlight::Flashlight(sf::Vector2u windowSize, float radius)
	: m_radius(radius)
{
	if(!m_maskTexture.resize(windowSize))
		throw std::runtime_error("Failed to create flashlight mask texture");
}

void Flashlight::buildLightCone(sf::Vector2f center) {
	float flicker = 1.f + .03f * std::sin(m_clock.getElapsedTime().asSeconds() * 7.f);
	float r		  = m_radius * flicker;

	const int SEGMENTS = 64;
	sf::VertexArray cone(sf::PrimitiveType::Triangles, SEGMENTS * 3);

	cone[0].position = center;
	cone[0].color = sf::Color(255,240,200);

	for (int i = 0; i < SEGMENTS; ++i) {
		float angle0 = i * 2.f * M_PI / SEGMENTS;
		float angle1 = (i + 1) * 2.f * M_PI / SEGMENTS;

		cone[i*3 + 0].position = center;
		cone[i*3 + 0].color = sf::Color(255,240,200);
		cone[i*3 + 1].position = {
			center.x + r * std::cos(angle0),
			center.y + r * std::sin(angle0)
		};
		cone[i*3 + 1].color = sf::Color::Black;
		cone[i*3 + 2].position = {
			center.x + r * std::cos(angle1),
			center.y + r * std::sin(angle1)
		};
		cone[i*3 + 2].color = sf::Color::Black;
	}

	m_maskTexture.draw(cone);
}

void Flashlight::update(sf::Vector2f playerWorldPosition, const sf::View& cameraView) {
	m_maskTexture.setView(cameraView);
	m_maskTexture.clear(sf::Color::Black);
	buildLightCone(playerWorldPosition);
	m_maskTexture.display();
}

void Flashlight::draw(sf::RenderWindow& window) {
	sf::View previousView = window.getView();
	window.setView(window.getDefaultView());

	sf::Sprite maskSprite(m_maskTexture.getTexture());
	window.draw(maskSprite, sf::BlendMultiply);

	window.setView(previousView);
}