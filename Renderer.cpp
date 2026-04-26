#include "Renderer.h"
#include <cstdint>

Renderer::Renderer(sf::RenderWindow& window, float tileSize, sf::Font font)
	: m_window(window)
	, m_tileSize(tileSize)
	, m_mapSprite(m_mapTexture.getTexture())
	, m_playerShape(tileSize * .35f)
	, m_signalShape(tileSize * .3f)
	, m_batteryShape({ tileSize * 0.4f, tileSize * 0.55f })
	, m_font(font)
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
			RoomType rtype = map.getRoomTypeAt(x, y);

			switch (tile) {
			case TileType::Wall:
				m_tileShape.setSize(sf::Vector2f(m_tileSize - 1.f,
					m_tileSize - 1.f));
				m_tileShape.setFillColor(sf::Color(30, 30, 50));
				m_tileShape.setOutlineThickness(0.f);
				m_tileShape.setPosition(toPixels(x, y));
				m_mapTexture.draw(m_tileShape);
				break;

			case TileType::Floor:
				m_tileShape.setSize(sf::Vector2f(m_tileSize - 1.f,
					m_tileSize - 1.f));
				m_tileShape.setOutlineThickness(0.f);
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
					m_tileShape.setFillColor(sf::Color(70, 70, 100));
					break;
				}
				m_tileShape.setPosition(toPixels(x, y));
				m_mapTexture.draw(m_tileShape);
				break;

			case TileType::PropCrate:
				drawProp(map, x, y,
					sf::Color(100, 80, 50),    // marrón metálico
					sf::Color(140, 110, 70),   // borde más claro
					0.75f);                    // tamaño relativo
				break;

			case TileType::PropConsole:
				drawProp(map, x, y,
					sf::Color(30, 60, 90),     // azul oscuro
					sf::Color(50, 150, 200),   // borde cyan
					0.8f);
				// Punto de luz de la consola
				drawPropLight(x, y, sf::Color(0, 200, 255, 180));
				break;

			case TileType::PropColumn:
				drawProp(map, x, y,
					sf::Color(50, 50, 70),     // gris azulado
					sf::Color(80, 80, 110),    // borde más claro
					0.7f);
				break;

			case TileType::PropBarrel:
				drawProp(map, x, y,
					sf::Color(80, 50, 30),     // marrón oxidado
					sf::Color(180, 80, 20),    // borde naranja
					0.65f);
				// Advertencia en barriles de peligro
				drawPropLight(x, y, sf::Color(220, 80, 0, 150));
				break;

			case TileType::PropDebris:
				drawDebris(x, y);
				break;
			}
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

void Renderer::drawBatteries(const std::vector<Battery>& batteries,
	sf::Vector2f                playerPos)
{
	float time = m_clock.getElapsedTime().asSeconds();
	float pulse = std::abs(std::sin(time * 2.f));
	float fastP = std::abs(std::sin(time * 6.f));

	for (const auto& battery : batteries) {
		if (battery.isCollected()) continue;

		sf::Vector2f pos = battery.getPosition();

		// Atracción hacia el jugador si está cerca
		sf::Vector2f diff = playerPos - pos;
		float        distSq = diff.x * diff.x + diff.y * diff.y;
		float        attractRange = m_tileSize * 3.f;

		if (distSq < attractRange * attractRange && distSq > 0.f) {
			float dist = std::sqrt(distSq);
			float strength = 1.f - (dist / attractRange); // 0→1
			float bounce = std::sin(time * 10.f) * 3.f * strength;
			// Desplazamos la posición de dibujo levemente hacia el jugador
			sf::Vector2f dir = diff / dist;
			pos += dir * bounce;
		}

		// --- Halos y cuerpo igual que antes ---
		float haloRadius = m_tileSize * (1.8f + 0.4f * pulse);
		sf::CircleShape halo(haloRadius);
		halo.setOrigin({ haloRadius, haloRadius });
		halo.setPosition(pos);
		halo.setFillColor(sf::Color(180, 220, 50,
			static_cast<uint8_t>(30 + 25 * pulse)));
		m_window.draw(halo);

		float midRadius = m_tileSize * (0.9f + 0.2f * pulse);
		sf::CircleShape mid(midRadius);
		mid.setOrigin({ midRadius, midRadius });
		mid.setPosition(pos);
		mid.setFillColor(sf::Color(200, 240, 80,
			static_cast<uint8_t>(60 + 40 * pulse)));
		m_window.draw(mid);

		float bodyW = m_tileSize * 0.4f;
		float bodyH = m_tileSize * 0.55f;
		m_batteryShape.setSize({ bodyW, bodyH });
		m_batteryShape.setOrigin({ bodyW / 2.f, bodyH / 2.f });

		uint8_t r = static_cast<uint8_t>(210 + 45 * fastP);
		uint8_t g = static_cast<uint8_t>(230 + 25 * fastP);
		m_batteryShape.setFillColor(sf::Color(r, g, 30));
		m_batteryShape.setOutlineColor(sf::Color(255, 255, 120, 200));
		m_batteryShape.setOutlineThickness(1.5f);
		m_batteryShape.setPosition(pos);
		m_window.draw(m_batteryShape);

		float poleW = bodyW * 0.35f;
		float poleH = m_tileSize * 0.08f;
		sf::RectangleShape pole({ poleW, poleH });
		pole.setOrigin({ poleW / 2.f, poleH });
		pole.setFillColor(sf::Color(255, 255, 150, 200));
		pole.setPosition({ pos.x, pos.y - bodyH / 2.f });
		m_window.draw(pole);
	}
}

void Renderer::drawPowerUps(const std::vector<PowerUp>& powerUps) {
	float time = m_clock.getElapsedTime().asSeconds();
	float pulse = std::abs(std::sin(time * 2.5f));

	for (const auto& p : powerUps) {
		if (p.isCollected()) continue;

		PowerUpDef   def = p.getDef();
		sf::Vector2f pos = p.getPosition();

		// Halo exterior pulsante
		float haloR = m_tileSize * (1.4f + 0.3f * pulse);
		sf::CircleShape halo(haloR);
		halo.setOrigin({ haloR, haloR });
		halo.setPosition(pos);
		halo.setFillColor(sf::Color(
			def.color.r, def.color.g, def.color.b,
			static_cast<uint8_t>(25 + 20 * pulse)
		));
		m_window.draw(halo);

		// Cuerpo — rombo (cuadrado rotado 45°)
		float bodyR = m_tileSize * 0.35f;
		sf::RectangleShape body({ bodyR * 1.4f, bodyR * 1.4f });
		body.setOrigin({ bodyR * 0.7f, bodyR * 0.7f });
		body.setRotation(sf::degrees(45.f));
		body.setPosition(pos);
		body.setFillColor(sf::Color(
			def.color.r / 2,
			def.color.g / 2,
			def.color.b / 2
		));
		body.setOutlineColor(def.color);
		body.setOutlineThickness(1.5f);
		m_window.draw(body);

		// Letra identificativa centrada
		sf::Text icon(m_font, def.icon, 11);
		icon.setFillColor(def.color);
		sf::FloatRect ib = icon.getLocalBounds();
		icon.setOrigin({
			ib.position.x + ib.size.x / 2.f,
			ib.position.y + ib.size.y / 2.f
			});
		icon.setPosition(pos);
		m_window.draw(icon);
	}
}

void Renderer::drawEnemies(const std::vector<Enemy>& enemies) {
	float time = m_clock.getElapsedTime().asSeconds();

	for (const auto& enemy : enemies) {
		// Phantom invisible no se dibuja
		if (!enemy.isVisible() &&
			enemy.getType() == EnemyType::Phantom) continue;

		sf::Vector2f pos = enemy.getPosition();
		float        pulse = std::abs(std::sin(
			time * (enemy.getState() == EnemyState::Chase ? 8.f : 2.f)));

		switch (enemy.getType()) {

		case EnemyType::Stalker: {
			// Triángulo rojo — igual que antes
			sf::Color bodyColor;
			switch (enemy.getState()) {
			case EnemyState::Chase:  bodyColor = sf::Color(255, 30, 30);  break;
			case EnemyState::Search: bodyColor = sf::Color(180, 80, 80);  break;
			default:                 bodyColor = sf::Color(120, 40, 40);  break;
			}
			float haloR = m_tileSize * (1.2f + 0.3f * pulse);
			sf::CircleShape halo(haloR);
			halo.setOrigin({ haloR, haloR });
			halo.setPosition(pos);
			halo.setFillColor(sf::Color(bodyColor.r, bodyColor.g,
				bodyColor.b, static_cast<uint8_t>(20 + 20 * pulse)));
			m_window.draw(halo);

			sf::CircleShape body(m_tileSize * 0.38f, 3);
			body.setOrigin({ m_tileSize * 0.38f, m_tileSize * 0.38f });
			body.setPosition(pos);
			body.setFillColor(bodyColor);
			body.setOutlineColor(sf::Color(255, 80, 80, 180));
			body.setOutlineThickness(1.5f);
			m_window.draw(body);
			break;
		}

		case EnemyType::Lurker: {
			// Rombo púrpura pequeño y rápido
			sf::Color color = (enemy.getState() == EnemyState::Chase)
				? sf::Color(220, 80, 255)
				: sf::Color(140, 40, 180);

			sf::RectangleShape body({
				m_tileSize * 0.45f,
				m_tileSize * 0.45f
				});
			body.setOrigin({ m_tileSize * 0.225f, m_tileSize * 0.225f });
			body.setRotation(sf::degrees(45.f));
			body.setPosition(pos);
			body.setFillColor(color);
			body.setOutlineColor(sf::Color(200, 100, 255, 200));
			body.setOutlineThickness(1.5f);
			m_window.draw(body);
			break;
		}

		case EnemyType::Drainer: {
			// Círculo naranja con anillo pulsante
			float innerR = m_tileSize * 0.3f;
			float outerR = m_tileSize * (1.5f + 0.5f * pulse);

			// Aura de drenaje
			sf::CircleShape aura(outerR);
			aura.setOrigin({ outerR, outerR });
			aura.setPosition(pos);
			aura.setFillColor(sf::Color(0, 0, 0, 0));
			aura.setOutlineColor(sf::Color(255, 120, 0,
				static_cast<uint8_t>(40 + 40 * pulse)));
			aura.setOutlineThickness(2.f);
			m_window.draw(aura);

			// Núcleo
			sf::CircleShape body(innerR);
			body.setOrigin({ innerR, innerR });
			body.setPosition(pos);
			body.setFillColor(sf::Color(200, 80, 0));
			body.setOutlineColor(sf::Color(255, 160, 0, 200));
			body.setOutlineThickness(1.5f);
			m_window.draw(body);
			break;
		}

		case EnemyType::Phantom: {
			// Triángulo blanco fantasmal — semitransparente
			uint8_t alpha = static_cast<uint8_t>(
				enemy.isVisible() ? 180 + 40 * pulse : 0);

			sf::CircleShape body(m_tileSize * 0.35f, 3);
			body.setOrigin({ m_tileSize * 0.35f, m_tileSize * 0.35f });
			body.setPosition(pos);
			body.setFillColor(sf::Color(220, 230, 255, alpha));
			body.setOutlineColor(sf::Color(180, 200, 255,
				static_cast<uint8_t>(alpha * 0.7f)));
			body.setOutlineThickness(1.f);
			m_window.draw(body);
			break;
		}
		}
	}
}

void Renderer::drawProp(const Map& map, int x, int y,
	sf::Color fill, sf::Color outline,
	float sizeRatio)
{
	// Primero dibujamos el suelo debajo del prop
	RoomType rtype = map.getRoomTypeAt(x, y);
	m_tileShape.setSize(sf::Vector2f(m_tileSize - 1.f, m_tileSize - 1.f));
	m_tileShape.setOutlineThickness(0.f);
	switch (rtype) {
	case RoomType::Storage: m_tileShape.setFillColor(sf::Color(50, 80, 60));  break;
	case RoomType::Danger:  m_tileShape.setFillColor(sf::Color(80, 40, 40));  break;
	case RoomType::Control: m_tileShape.setFillColor(sf::Color(40, 60, 90));  break;
	default:                m_tileShape.setFillColor(sf::Color(70, 70, 100)); break;
	}
	m_tileShape.setPosition(toPixels(x, y));
	m_mapTexture.draw(m_tileShape);

	// Luego el prop encima
	float size = m_tileSize * sizeRatio;
	float offset = (m_tileSize - size) / 2.f;

	sf::RectangleShape prop({ size, size });
	prop.setFillColor(fill);
	prop.setOutlineColor(outline);
	prop.setOutlineThickness(1.5f);
	prop.setPosition(toPixels(x, y) + sf::Vector2f(offset, offset));
	m_mapTexture.draw(prop);
}

void Renderer::drawPropLight(int x, int y, sf::Color color) {
	// Pequeño punto de luz en la esquina del prop
	float dotSize = m_tileSize * 0.12f;
	sf::CircleShape dot(dotSize);
	dot.setFillColor(color);
	dot.setPosition(toPixels(x, y) +
		sf::Vector2f(m_tileSize * 0.75f, m_tileSize * 0.15f));
	m_mapTexture.draw(dot);
}

void Renderer::drawDebris(int x, int y) {
	// Escombros: 3-4 rectángulos pequeños en posiciones aleatorias
	// Usamos x,y como semilla para que sean consistentes
	srand(x * 1000 + y); // semilla determinista

	for (int i = 0; i < 3; ++i) {
		float w = m_tileSize * (0.15f + (rand() % 20) * 0.01f);
		float h = m_tileSize * (0.1f + (rand() % 15) * 0.01f);
		float px = (rand() % static_cast<int>(m_tileSize * 0.7f));
		float py = (rand() % static_cast<int>(m_tileSize * 0.7f));

		sf::RectangleShape piece({ w, h });
		piece.setFillColor(sf::Color(
			60 + rand() % 30,
			55 + rand() % 25,
			70 + rand() % 20
		));
		piece.setRotation(sf::degrees(static_cast<float>(rand() % 360)));
		piece.setPosition(toPixels(x, y) + sf::Vector2f(px, py));
		m_mapTexture.draw(piece);
	}

	// Restauramos la semilla aleatoria del juego
	srand(static_cast<unsigned>(time(nullptr)));
}