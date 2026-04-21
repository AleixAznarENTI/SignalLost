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
			TileType tile = map.getTile(x, y);

			if (tile == TileType::Wall) {
				m_tileShape.setFillColor(sf::Color(30, 30, 50));
			}
			else {
				RoomType rtype = map.getRoomTypeAt(x, y);
				switch (rtype) {
				case RoomType::Storage:
					m_tileShape.setFillColor(sf::Color(50, 80, 60));
					break;
				case RoomType::Danger:
					m_tileShape.setFillColor(sf::Color(80, 40, 40));
					break;
				case RoomType::Control:
					m_tileShape.setFillColor(sf::Color(40, 60, 90));
					break;
				default:
					m_tileShape.setFillColor(sf::Color(70,70,100));
					break;
				}
			}

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
	float time = m_clock.getElapsedTime().asSeconds();
	float pulse = std::abs(std::sin(time * 2.f));
	float fastP = std::abs(std::sin(time * 6.f));

	for (const auto& battery : batteries) {
		if (battery.isCollected()) continue;

		sf::Vector2f pos = battery.getPosition();

		// --- Exterior Halo ---
		float haloRadius = m_tileSize * (1.8f + 0.4f * pulse);
		sf::CircleShape halo(haloRadius);
		halo.setOrigin({ haloRadius, haloRadius });
		halo.setPosition(pos);
		halo.setFillColor(sf::Color(
			180, 220, 50,
			static_cast<uint8_t>(30 + 25 * pulse)
		));
		m_window.draw(halo);

		// --- Middle Halo ---
		float midRadius = m_tileSize * (0.9f + 0.2f * pulse);
		sf::CircleShape mid(midRadius);
		mid.setOrigin({ midRadius, midRadius });
		mid.setPosition(pos);
		mid.setFillColor(sf::Color(
			200, 240, 80,
			static_cast<uint8_t>(60 + 40 * pulse)
		));
		m_window.draw(mid);

		// --- Core ---
		float bodyW = m_tileSize * 0.4f;
		float bodyH = m_tileSize * 0.55f;

		m_batteryShape.setSize({ bodyW, bodyH });
		m_batteryShape.setOrigin({ bodyW / 2.f, bodyH / 2.f });

		uint8_t r = static_cast<uint8_t>(200 + 55 * pulse);
		uint8_t g = static_cast<uint8_t>(180 + 40 * pulse);
		m_batteryShape.setFillColor(sf::Color(r, g, 30));
		m_batteryShape.setOutlineColor(sf::Color(255, 255, 120, 200));
		m_batteryShape.setOutlineThickness(1.5f);
		m_batteryShape.setPosition(pos);
		m_window.draw(m_batteryShape);

		// --- Positive pole ---
		float poleW = bodyW * 0.35f;
		float poleH = m_tileSize * 0.08f;
		sf::RectangleShape pole({ poleW, poleH });
		pole.setOrigin({ poleW / 2.f, poleH });
		pole.setFillColor(sf::Color(255, 255, 150, 200));
		pole.setPosition({ pos.x, pos.y - bodyH / 2.f });
		m_window.draw(pole);
	}
}