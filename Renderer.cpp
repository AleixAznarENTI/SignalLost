#include "Renderer.h"
#include <cstdint>

Renderer::Renderer(sf::RenderWindow& window, float tileSize)
	: m_window(window)
	, m_tileSize(tileSize)
	, m_mapSprite(m_mapTexture.getTexture())
	, m_playerShape(tileSize * .35f)
	, m_signalShape(tileSize * .3f)
	, m_batteryShape({ tileSize * 0.4f, tileSize * 0.55f })
{
	m_tileShape.setSize(sf::Vector2f(m_tileSize - 1.f, m_tileSize - 1.f));

	m_playerShape.setOrigin({ m_playerShape.getRadius(), m_playerShape.getRadius() });
	m_signalShape.setOrigin({ m_signalShape.getRadius(), m_signalShape.getRadius() });
	m_batteryShape.setOrigin({ m_batteryShape.getSize().x / 2.f,
							   m_batteryShape.getSize().y / 2.f });

	m_batteryShape.setOutlineColor(sf::Color(255, 255, 100, 180));
	m_batteryShape.setOutlineThickness(1.f);
}

sf::Vector2f Renderer::toPixels(int x, int y) const {
	return { x * m_tileSize, y * m_tileSize };
}

void Renderer::bakeMap(const Map& map) {
	sf::Vector2u textureSize(
		static_cast<unsigned>(map.getWidth() * m_tileSize),
		static_cast<unsigned>(map.getHeight() * m_tileSize)
	);

	if (!m_mapTexture.resize(textureSize))
		throw std::runtime_error("Failed to create map texture");

	m_mapTexture.clear(sf::Color(10, 10, 20));

	for (int y = 0; y < map.getHeight(); ++y) {
		for (int x = 0; x < map.getWidth(); ++x) {
			if (map.getTile(x, y) == TileType::Wall)
				m_tileShape.setFillColor(sf::Color(30, 30, 50));
			else
				m_tileShape.setFillColor(sf::Color(70, 70, 100));

			m_tileShape.setPosition(toPixels(x, y));
			m_mapTexture.draw(m_tileShape);
		}
	}

	m_mapTexture.display();

	m_mapSprite = sf::Sprite(m_mapTexture.getTexture());

}

void Renderer::drawMap() {
	m_window.draw(m_mapSprite);
}

void Renderer::drawPlayer(sf::Vector2f position) {
	m_playerShape.setFillColor(sf::Color(200, 230, 255));
	m_playerShape.setPosition(position);
	m_window.draw(m_playerShape);
}

void Renderer::drawSignal(sf::Vector2f position) {
	float pulse = std::abs(std::sin(m_clock.getElapsedTime().asSeconds() * 3.f));
	uint8_t bright = static_cast<uint8_t>(150 + 105 * pulse);
	m_signalShape.setFillColor(sf::Color(0, bright, 0));
	m_signalShape.setPosition(position);
	m_window.draw(m_signalShape);
}

void Renderer::drawBatteries(const std::vector<Battery>& batteries) {
	float pulse = std::abs(std::sin(m_clock.getElapsedTime().asSeconds() * 2.f));
	
	for (const auto& battery : batteries) {
		if (battery.isCollected()) continue;

		uint8_t r = static_cast<uint8_t>(200 + 55 * pulse);
		uint8_t g = static_cast<uint8_t>(180 + 40 * pulse);
		m_batteryShape.setFillColor(sf::Color(r, g, 30));
		m_batteryShape.setPosition(battery.getPosition());
		m_window.draw(m_batteryShape);
	}
}