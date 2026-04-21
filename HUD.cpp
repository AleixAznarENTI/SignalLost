#include "HUD.h"
#include <cstdint>
#include <iostream>

HUD::HUD(sf::RenderWindow& window, const sf::Font& font, AudioManager& audio)
	: m_window(window)
	, m_font(font)
	, m_audio(audio)
	, m_typewriter(font, 18, 0.035f)
{}

void HUD::onStateChanged(GameState newState) {

	m_typewriter.sestOnCharCallback([this]() {
		m_audio.playTypewriterClick();
		});

	switch (newState) {
	case GameState::Intro:
		m_typewriter.start(
			"SIGNAL LOST\n\n"
			"The abandoned station\n"
			"keeps an SOS signal.\n\n"
			"Find the signal before\n"
			"you run out of energy.\n\n"
			"WASD to move\n"
			"Use MOUSE to control your flashlight\n\n"
			"[ PRESS SPACE TO START ]"
		);
		break;
	case GameState::Victory:
		m_typewriter.start(
			"SIGNAL FOUND\n\n"
			"The rescuers have recieved\n"
			"your location.\n\n"
			"[ PRESS R TO RESTART ]"
		);
		break;
	case GameState::GameOver:
		m_typewriter.start(
			"RUN OUT OF ENERGY\n\n"
			"The shadows have consumed you.\n"
			"No one won't ever know you were here.\n\n"
			"[ PRESS R TO RESTART ]"
		);
		break;
	default:
		m_typewriter.reset();
		break;
	}
}

void HUD::update(float dt, GameState state) {
	if (state != GameState::Playing)
		m_typewriter.update(dt);

	auto tick = [&](float& timer) {
		if (timer > 0.f) timer = std::max(0.f, timer - dt);
	};

	tick(m_batteryFlashTimer);
	tick(m_dangerVignetteTimer);
	tick(m_controlVignetteTimer);
	tick(m_signalFlashTimer);
	tick(m_gameOverFadeTimer);
	tick(m_zoneNotifTimer);
}

void HUD::setSignalInfo(sf::Vector2f playerPos,
	sf::Vector2f signalPos,
	bool		 inControlRoom)
{
	m_showIndicator = inControlRoom;
	if (!inControlRoom) return;

	sf::Vector2f diff = signalPos - playerPos;
	m_signalAngle = std::atan2(diff.y, diff.x);
	m_signalDistance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
}
#pragma region DRAW



void HUD::drawEnergyBar(float energyPercentage) {
	const float barWidth  = 200.f;
	const float barHeight = 14.f;
	const float margin    = 16.f;
	float screenH		  = static_cast<float>(m_window.getSize().y);

	sf::RectangleShape bg({ barWidth, barHeight });
	bg.setFillColor(sf::Color(40, 40, 40));
	bg.setPosition({ margin, screenH - margin - barHeight });
	m_window.draw(bg);

	sf::Color fillColor;
	if (energyPercentage > .5f) fillColor = sf::Color(80, 220, 120);
	else if (energyPercentage > .25f) fillColor = sf::Color(240, 200, 50);
	else fillColor = sf::Color(220, 60, 60);

	sf::RectangleShape fill({ barWidth * energyPercentage, barHeight });
	fill.setFillColor(fillColor);
	fill.setPosition(bg.getPosition());
	m_window.draw(fill);

	// Border
	sf::RectangleShape border({ barWidth, barHeight });
	border.setFillColor(sf::Color::Transparent);
	border.setOutlineColor(sf::Color(150, 150, 150));
	border.setOutlineThickness(1.f);
	border.setPosition(bg.getPosition());
	m_window.draw(border);

	// Flash verde encima de la barra al recoger batería
	if (m_batteryFlashTimer > 0.f) {
		float alpha = m_batteryFlashTimer / 0.4f;  // 1→0
		sf::RectangleShape flash({ barWidth, barHeight });
		flash.setPosition(bg.getPosition());
		flash.setFillColor(sf::Color(
			100, 255, 100,
			static_cast<uint8_t>(alpha * 200.f)
		));
		m_window.draw(flash);
	}

	// Etiqueta
	sf::Text label(m_font, "ENERGY", 11);
	label.setFillColor(sf::Color(180, 180, 180));
	label.setPosition({ margin, screenH - margin - barHeight - 16.f });
	m_window.draw(label);
}

void HUD::drawIntro() {
	// Overlay oscuro
	sf::RectangleShape overlay(sf::Vector2f(m_window.getSize()));
	overlay.setFillColor(sf::Color(0, 0, 0, 210));
	m_window.draw(overlay);

	// Posición central de la ventana
	sf::Vector2f center(
		m_window.getSize().x / 2.f,
		m_window.getSize().y / 2.f
	);

	m_typewriter.draw(m_window, center, sf::Color(180, 220, 255));
}

void HUD::drawEndScreen(GameState state) {
	sf::RectangleShape overlay(sf::Vector2f(m_window.getSize()));
	overlay.setFillColor(sf::Color(0, 0, 0, 170));
	m_window.draw(overlay);

	sf::Vector2f center(
		m_window.getSize().x / 2.f,
		m_window.getSize().y /2.f
	);

	sf::Color textColor = (state == GameState::Victory)
		? sf::Color(80, 255, 120) // Green
		: sf::Color(220, 60, 60); // Red

	m_typewriter.draw(m_window, center, textColor);
}

void HUD::draw(float energyPercent, GameState state) {

	sf::View prev = m_window.getView();
	m_window.setView(m_window.getDefaultView());

	switch (state) {
	case GameState::Intro:
		drawIntro();
		break;
	case GameState::Playing:
		drawEnergyBar(energyPercent);
		drawSignalIndicator();
		drawFeedback();
		drawZoneNotification();
		break;
	case GameState::Victory:
	case GameState::GameOver:
		drawEnergyBar(energyPercent);
		drawFeedback();
		drawEndScreen(state);
		break;
	}
	
	m_window.setView(prev);
}

void HUD::drawSignalIndicator() {
	if (!m_showIndicator) return;

	// Indicator position: top right corner
	float screenW = static_cast<float>(m_window.getSize().x);
	float margin = 20.f;
	sf::Vector2f center(screenW - 60.f, 60.f);

	// --- Background circle ---
	sf::CircleShape bg(40.f);
	bg.setOrigin({ 40.f, 40.f });
	bg.setPosition(center);
	bg.setFillColor(sf::Color(0, 0, 0, 160));
	bg.setOutlineColor(sf::Color(40, 60, 90, 200));
	bg.setOutlineThickness(2.f);
	m_window.draw(bg);

	// --- Directional Arrow ---
	float arrowLen = 28.f;
	sf::Vector2f tip(
		center.x + arrowLen * std::cos(m_signalAngle),
		center.y + arrowLen * std::sin(m_signalAngle)
	);

	sf::Vertex line[] = {
		sf::Vertex(center, sf::Color(80, 255, 120)),
		sf::Vertex(tip,	   sf::Color(80,255,120))
	};
	m_window.draw(line, 2, sf::PrimitiveType::Lines);

	float headLen = 8.f;
	float headAngle = 0.5f; // radians (28 degrees aprox)

	sf::Vector2f head1(
		tip.x - headLen * std::cos(m_signalAngle - headAngle),
		tip.y - headLen * std::sin(m_signalAngle - headAngle)
	);

	sf::Vector2f head2(
		tip.x - headLen * std::cos(m_signalAngle + headAngle),
		tip.y - headLen * std::sin(m_signalAngle + headAngle)
	);

	sf::Vertex arrow1[] = {
		sf::Vertex(tip,	  sf::Color(80,255,120)),
		sf::Vertex(head1, sf::Color(80,255,120))
	};
	sf::Vertex arrow2[] = {
	sf::Vertex(tip,	  sf::Color(80,255,120)),
	sf::Vertex(head1, sf::Color(80,255,120))
	};
	m_window.draw(arrow1, 2, sf::PrimitiveType::Lines);
	m_window.draw(arrow2, 2, sf::PrimitiveType::Lines);
	
	// --- Distance in tiles ---
	float distInTiles = m_signalDistance / 24.f; //TILE_SIZE = 24
	std::string distText = std::to_string(static_cast<int>(distInTiles));

	sf::Text dist(m_font, distText, 11);
	dist.setFillColor(sf::Color(80, 255, 120, 200));

	sf::FloatRect b = dist.getLocalBounds();
	dist.setOrigin({ b.position.x + b.size.x / 2.f,
					 b.position.y + b.size.y / 2.f });
	dist.setPosition({ center.x, center.y + 52.f });
	m_window.draw(dist);

	// --- "CONTROL" label ---
	sf::Text label(m_font, "CONTROL", 9);
	label.setFillColor(sf::Color(40, 60, 90, 220));

	sf::FloatRect lb = label.getLocalBounds();
	label.setOrigin({ lb.position.x + lb.size.x / 2.f,
					  lb.position.y + lb.size.y / 2.f });
	label.setPosition({ center.x, center.y - 52.f });
	m_window.draw(label);

}

// --- Viñeta (overlay en los bordes de pantalla) ---

void HUD::drawVignette(sf::Color color, float alpha) {
	if (alpha <= 0.f) return;

	float w = static_cast<float>(m_window.getSize().x);
	float h = static_cast<float>(m_window.getSize().y);
	float thickness = 120.f;

	auto a0 = static_cast<uint8_t>(alpha * 255.f); // opaco en el borde
	sf::Color solid(color.r, color.g, color.b, a0);
	sf::Color clear(color.r, color.g, color.b, 0);  // transparente al centro

	// Cada borde es un quad con gradiente: opaco afuera, transparente adentro
	// Top
	sf::VertexArray top(sf::PrimitiveType::TriangleStrip, 4);
	top[0] = { { 0.f, 0.f           }, solid };
	top[1] = { { w,   0.f           }, solid };
	top[2] = { { 0.f, thickness     }, clear };
	top[3] = { { w,   thickness     }, clear };
	m_window.draw(top);

	// Bottom
	sf::VertexArray bot(sf::PrimitiveType::TriangleStrip, 4);
	bot[0] = { { 0.f, h             }, solid };
	bot[1] = { { w,   h             }, solid };
	bot[2] = { { 0.f, h - thickness }, clear };
	bot[3] = { { w,   h - thickness }, clear };
	m_window.draw(bot);

	// Left
	sf::VertexArray left(sf::PrimitiveType::TriangleStrip, 4);
	left[0] = { { 0.f,       0.f }, solid };
	left[1] = { { thickness, 0.f }, clear };
	left[2] = { { 0.f,       h   }, solid };
	left[3] = { { thickness, h   }, clear };
	m_window.draw(left);

	// Right
	sf::VertexArray right(sf::PrimitiveType::TriangleStrip, 4);
	right[0] = { { w,             0.f }, solid };
	right[1] = { { w - thickness, 0.f }, clear };
	right[2] = { { w,             h   }, solid };
	right[3] = { { w - thickness, h   }, clear };
	m_window.draw(right);
}

// --- Feedback general ---

void HUD::drawFeedback() {
	float w = static_cast<float>(m_window.getSize().x);
	float h = static_cast<float>(m_window.getSize().y);

	// 1. Flash verde al recoger batería — pulso en la barra
	//    (se gestiona en drawEnergyBar con m_batteryFlashTimer)

	// 2. Viñeta roja sala Danger
	if (m_dangerVignetteTimer > 0.f) {
		float alpha = (m_dangerVignetteTimer / 1.2f) * 0.6f;
		drawVignette(sf::Color(180, 20, 20), alpha);
	}

	// 3. Viñeta azul sala Control
	if (m_controlVignetteTimer > 0.f) {
		float alpha = (m_controlVignetteTimer / 1.2f) * 0.5f;
		drawVignette(sf::Color(20, 60, 180), alpha);
	}

	// 4. Flash blanco al encontrar la señal
	if (m_signalFlashTimer > 0.f) {
		float progress = 1.f - (m_signalFlashTimer / 1.5f); // 0→1
		float alpha;
		if (progress < 0.2f)
			alpha = progress / 0.2f;         // fade in rápido
		else
			alpha = 1.f - (progress - 0.2f) / 0.8f; // fade out lento

		sf::RectangleShape flash(sf::Vector2f(w, h));
		flash.setFillColor(sf::Color(
			255, 255, 255,
			static_cast<uint8_t>(alpha * 200.f)
		));
		m_window.draw(flash);
	}

	// 5. Fade a negro al quedarse sin energía
	if (m_gameOverFadeTimer > 0.f) {
		float progress = 1.f - (m_gameOverFadeTimer / 3.f); // 0→1
		float alpha = progress * progress; // cuadrático = más dramático

		sf::RectangleShape fade(sf::Vector2f(w, h));
		fade.setFillColor(sf::Color(0, 0, 0,
			static_cast<uint8_t>(alpha * 255.f)));
		m_window.draw(fade);
	}
}
void HUD::drawZoneNotification() {
	if (m_zoneNotifTimer <= 0.f) return;

	// Fade in rápido (0.3s) → visible → fade out lento
	float progress = 1.f - (m_zoneNotifTimer / 2.5f);
	float alpha;
	if (progress < 0.12f)
		alpha = progress / 0.12f;
	else if (progress < 0.7f)
		alpha = 1.f;
	else
		alpha = 1.f - (progress - 0.7f) / 0.3f;

	alpha = std::max(0.f, std::min(1.f, alpha));

	sf::Text text(m_font, m_zoneNotifText, 22);
	text.setStyle(sf::Text::Bold);
	text.setFillColor(sf::Color(
		m_zoneNotifColor.r,
		m_zoneNotifColor.g,
		m_zoneNotifColor.b,
		static_cast<uint8_t>(alpha * 255.f)
	));

	// Centrado en la parte superior
	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin({
		bounds.position.x + bounds.size.x / 2.f,
		bounds.position.y + bounds.size.y / 2.f
		});
	text.setPosition({
		m_window.getSize().x / 2.f,
		40.f
		});

	// Línea decorativa debajo del texto
	float lineW = bounds.size.x + 40.f;
	sf::RectangleShape line({ lineW, 1.f });
	line.setOrigin({ lineW / 2.f, 0.f });
	line.setPosition({ m_window.getSize().x / 2.f, 56.f });
	line.setFillColor(sf::Color(
		m_zoneNotifColor.r,
		m_zoneNotifColor.g,
		m_zoneNotifColor.b,
		static_cast<uint8_t>(alpha * 180.f)
	));

	m_window.draw(text);
	m_window.draw(line);
}

#pragma endregion
#pragma region TRIGGERS

void HUD::triggerZoneNotification(const std::string& name, sf::Color color) {
	m_zoneNotifText = name;
	m_zoneNotifColor = color;
	m_zoneNotifTimer = 2.5f;
}

void HUD::triggerBatteryPickup() {
	m_batteryFlashTimer = 0.4f;  // dura 0.4 segundos
}

void HUD::triggerDangerEnter() {
	m_dangerVignetteTimer = 1.2f;
}

void HUD::triggerControlEnter() {
	m_controlVignetteTimer = 1.2f;
}

void HUD::triggerSignalFound() {
	m_signalFlashTimer = 1.5f;
}

void HUD::triggerGameOver() {
	m_gameOverFadeTimer = 3.f;  // fade a negro en 3 segundos
}

void HUD::setCurrentRoom(RoomType room) {
	// Detectamos transición de sala
	if (room != m_prevRoom) {
		if (room == RoomType::Danger)  triggerDangerEnter();
		if (room == RoomType::Control) triggerControlEnter();
		m_prevRoom = room;
	}
	m_currentRoom = room;
}


#pragma endregion