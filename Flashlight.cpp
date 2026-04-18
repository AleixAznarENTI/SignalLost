#include "Flashlight.h"
#include <cmath>
#include <cstdint>
#include <stdexcept>
#define M_PI 3.14159265358979323846

////////////////////////////////////////////////
/// Cone with a narrower aperture → more tension, less visibility
/// float halfAperture = 35.f * (3.14159265f / 180.f);
/// 
/// 
/// Cone with a wider aperture → more comfortable, less atmospheric
/// float halfAperture = 70.f * (3.14159265f / 180.f);
/// 
/// 
/// Cone with an aperture that narrows as energy depletes → more tension, encourages energy conservation
/// float halfAperture = (30.f + 20.f * energyPercent) * (3.14159265f / 180.f);
////////////////////////////////////////////////

Flashlight::Flashlight(sf::Vector2u windowSize, float radius)
	: m_radius(radius)
{
	if(!m_maskTexture.resize(windowSize))
		throw std::runtime_error("Failed to create flashlight mask texture");
}

void Flashlight::buildAmbientGlow(sf::Vector2f origin) {
	const int SEGMENTS = 32;
	const float ambientR = m_radius * 0.25f;

	sf::VertexArray glow(sf::PrimitiveType::Triangles, SEGMENTS * 3);

	for (int i = 0; i < SEGMENTS; ++i) {
		float a0 = i * (2.f * M_PI / SEGMENTS);
		float a1 = (i + 1) * (2.f * M_PI / SEGMENTS);

		glow[i * 3 + 0].position = origin;
		glow[i * 3 + 0].color = sf::Color(80, 80, 120);

		glow[i * 3 + 1].position = { origin.x + ambientR * std::cos(a0),
									 origin.y + ambientR * std::sin(a0) };
		glow[i * 3 + 1].color = sf::Color::Black;

		glow[i * 3 + 2].position = { origin.x + ambientR * std::cos(a1),
									 origin.y + ambientR * std::sin(a1) };
		glow[i * 3 + 2].color = sf::Color::Black;
	}
	m_maskTexture.draw(glow);
}

void Flashlight::buildLightCone(sf::Vector2f origin,
								float dirAngle,
								float halfAperture) 
{
	float flicker = 1.f + .03f * std::sin(m_clock.getElapsedTime().asSeconds() * 7.f);
	float r		  = m_radius * flicker;

	const int SEGMENTS = 64;
	sf::VertexArray cone(sf::PrimitiveType::Triangles, SEGMENTS * 3);

	for (int i = 0; i < SEGMENTS; ++i) {
		float t0 = static_cast<float>(i) / SEGMENTS;
		float t1 = static_cast<float>(i + 1) / SEGMENTS;

		float angle0 = dirAngle - halfAperture + t0 * 2.f * halfAperture;
		float angle1 = dirAngle - halfAperture + t1 * 2.f * halfAperture;

		cone[i*3 + 0].position = origin;
		cone[i*3 + 0].color = sf::Color(255,240,200);
		cone[i*3 + 1].position = {
			origin.x + r * std::cos(angle0),
			origin.y + r * std::sin(angle0)
		};
		cone[i*3 + 1].color = sf::Color::Black;

		cone[i*3 + 2].position = {
			origin.x + r * std::cos(angle1),
			origin.y + r * std::sin(angle1)
		};
		cone[i*3 + 2].color = sf::Color::Black;
	}

	m_maskTexture.draw(cone);
}

void Flashlight::update(sf::Vector2f playerWorldPosition, 
						const sf::View& cameraView,
						sf::RenderWindow& window) 
{
	sf::Vector2i mousePixels = sf::Mouse::getPosition(window);
	sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixels, cameraView);

	sf::Vector2f dir = mouseWorld - playerWorldPosition;
	float angle = std::atan2(dir.y, dir.x);

	float halfAperture = 50.f * M_PI / 180.f;

	m_maskTexture.setView(cameraView);
	m_maskTexture.clear(sf::Color::Black);

	buildAmbientGlow(playerWorldPosition);
	buildLightCone(playerWorldPosition, angle, halfAperture);

	m_maskTexture.display();
}

void Flashlight::draw(sf::RenderWindow& window) {
	sf::View previousView = window.getView();
	window.setView(window.getDefaultView());

	sf::Sprite maskSprite(m_maskTexture.getTexture());
	window.draw(maskSprite, sf::BlendMultiply);

	window.setView(previousView);
}