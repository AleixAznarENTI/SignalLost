#include "Renderer.h"
#include <cstdint>

Renderer::Renderer(sf::RenderWindow& window, float tileSize)
	: m_window(window), m_tileSize(tileSize)
{
	m_tileShape.setSize(sf::Vector2f(m_tileSize - 1.f, m_tileSize - 1.f));
}

sf::Vector2f Renderer::toPixels(int x, int y) const {
	return { x * m_tileSize, y * m_tileSize };
}

void Renderer::drawMap(const Map& map) {
	for (int row = 0; row < map.getHeight(); ++row) {
		for (int col = 0; col < map.getWidth(); ++col) {

			TileType tile = map.getTile(col, row);

			if (tile == TileType::Wall) {
				m_tileShape.setFillColor(sf::Color(30, 30, 50));
			}
			else {
				m_tileShape.setFillColor(sf::Color(70, 70, 100));
			}

			m_tileShape.setPosition(toPixels(col, row));
			m_window.draw(m_tileShape);

		}
	}
}

void Renderer::drawPlayer(sf::Vector2f position) {
	sf::CircleShape player(m_tileSize * .35f);
	player.setFillColor(sf::Color(200, 230, 255));
	player.setOrigin(sf::Vector2f(player.getRadius(), player.getRadius()));
	player.setPosition(position);
	m_window.draw(player);
}

void Renderer::drawSignal(sf::Vector2f position) {
	float pulse = std::abs(std::sin(m_clock.getElapsedTime().asSeconds() * 3.f));
	std::uint8_t brightness = static_cast<uint8_t>(150 + 105 * pulse);
	
	sf::CircleShape signal(m_tileSize * .3f);
	signal.setFillColor(sf::Color(0, brightness, 0));
	signal.setOrigin({ signal.getRadius(), signal.getRadius() });
	signal.setPosition(position);
	m_window.draw(signal);
}

void Renderer::drawBatteries(const std::vector<Battery>& batteries) {
	float pulse = std::abs(std::sin(m_clock.getElapsedTime().asSeconds() * 2.f));
	for (const auto& battery : batteries) {
		if (battery.isCollected()) continue;

		sf::RectangleShape body({ m_tileSize * 0.4f, m_tileSize * 0.55f });
		body.setOrigin({ body.getSize().x / 2.f, body.getSize().y / 2.f });
		body.setPosition(battery.getPosition());

		uint8_t r = static_cast<uint8_t>(200 + 55 * pulse);
		uint8_t g = static_cast<uint8_t>(180 + 40 * pulse);
		body.setFillColor(sf::Color(r, g, 30));
		body.setOutlineColor(sf::Color(255, 255, 100, 180));
		body.setOutlineThickness(1.f);
		
		m_window.draw(body);
	}
}