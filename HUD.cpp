#include "HUD.h"
#include <cstdint>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

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
	case GameState::Intro: {
		// Pick a random intro message each run
		const std::string intros[] = {
			"SIGNAL LOST\n\n"
			"The station has been silent\n"
			"for 72 hours.\n\n"
			"Find the rescue signal.\n"
			"Don't run out of power.\n\n"
			"WASD to move\n"
			"Mouse to aim flashlight\n\n"
			"[ PRESS SPACE TO BEGIN ]",

			"SIGNAL LOST\n\n"
			"You are the last crew member.\n"
			"Something is still moving\n"
			"in the dark.\n\n"
			"Find the signal. Get out.\n\n"
			"WASD to move\n"
			"Mouse to aim flashlight\n\n"
			"[ PRESS SPACE TO BEGIN ]",

			"SIGNAL LOST\n\n"
			"Power reserves critical.\n"
			"The station reconfigures\n"
			"itself each time.\n"
			"You won't remember the way back.\n\n"
			"WASD to move\n"
			"Mouse to aim flashlight\n\n"
			"[ PRESS SPACE TO BEGIN ]",

			"SIGNAL LOST\n\n"
			"Three expeditions.\n"
			"None returned.\n"
			"You volunteered anyway.\n\n"
			"WASD to move\n"
			"Mouse to aim flashlight\n\n"
			"[ PRESS SPACE TO BEGIN ]",

			"SIGNAL LOST\n\n"
			"The rescue team is waiting.\n"
			"They won't wait forever.\n\n"
			"Find the signal before\n"
			"your battery dies.\n\n"
			"WASD to move\n"
			"Mouse to aim flashlight\n\n"
			"[ PRESS SPACE TO BEGIN ]"
		};

		int index = rand() % 5;
		m_typewriter.start(intros[index]);
		break;
	}
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
	tick(m_zoneNotifTimer);
	tick(m_deathFlashTimer);

	if (state == GameState::Playing && m_uiFadeTimer < UI_FADE_DURATION)
		m_uiFadeTimer += dt;
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


void HUD::drawScoreScreen(GameState state) {
	float w = static_cast<float>(m_window.getSize().x);
	float h = static_cast<float>(m_window.getSize().y);

	// Título según estado
	std::string title = (state == GameState::Victory)
		? "MISION COMPLETED" : "MISION FAILED";
	sf::Color titleColor = (state == GameState::Victory)
		? sf::Color(80, 255, 120) : sf::Color(220, 60, 60);

	// --- Título ---
	sf::Text titleText(m_font, title, 32);
	titleText.setFillColor(titleColor);
	titleText.setStyle(sf::Text::Bold);
	sf::FloatRect tb = titleText.getLocalBounds();
	titleText.setOrigin({
		tb.position.x + tb.size.x / 2.f,
		tb.position.y + tb.size.y / 2.f
		});
	titleText.setPosition({ w / 2.f, h / 2.f - 120.f });
	m_window.draw(titleText);

	// Línea separadora
	sf::RectangleShape line({ 300.f, 1.f });
	line.setOrigin({ 150.f, 0.f });
	line.setPosition({ w / 2.f, h / 2.f - 90.f });
	line.setFillColor(sf::Color(titleColor.r, titleColor.g,
		titleColor.b, 120));
	m_window.draw(line);

	// --- Métricas ---
	// Formato: tiempo, baterías, distancia, salas
	int    minutes = static_cast<int>(m_statTime) / 60;
	int    seconds = static_cast<int>(m_statTime) % 60;
	std::ostringstream timeStr;
	timeStr << minutes << ":" << std::setw(2)
		<< std::setfill('0') << seconds;

	struct Metric {
		std::string label;
		std::string value;
	};

	std::vector<Metric> metrics = {
		{ "TIME",            timeStr.str()                              },
		{ "BATTERIES",          std::to_string(m_statBatteries)            },
		{ "DISTANCE",         std::to_string(static_cast<int>(
								   m_statDistance)) + " steps"            },
		{ "VISITED ROOMS",   std::to_string(m_statRooms)                },
	};

	float startY = h / 2.f - 60.f;
	float stepY = 40.f;

	for (size_t i = 0; i < metrics.size(); ++i) {
		float y = startY + i * stepY;

		// Label (izquierda)
		sf::Text label(m_font, metrics[i].label, 14);
		label.setFillColor(sf::Color(150, 150, 170));
		sf::FloatRect lb = label.getLocalBounds();
		label.setOrigin({ lb.position.x + lb.size.x, lb.position.y });
		label.setPosition({ w / 2.f - 20.f, y });
		m_window.draw(label);

		// Valor (derecha)
		sf::Text value(m_font, metrics[i].value, 14);
		value.setFillColor(sf::Color(220, 220, 240));
		value.setStyle(sf::Text::Bold);
		sf::FloatRect vb = value.getLocalBounds();
		value.setOrigin({ vb.position.x, vb.position.y });
		value.setPosition({ w / 2.f + 20.f, y });
		m_window.draw(value);
	}

	// Hint de reinicio
	sf::Text hint(m_font, "[ R para reiniciar  |  Escape para salir ]", 13);
	hint.setFillColor(sf::Color(120, 120, 140));
	sf::FloatRect hb = hint.getLocalBounds();
	hint.setOrigin({
		hb.position.x + hb.size.x / 2.f,
		hb.position.y + hb.size.y / 2.f
		});
	hint.setPosition({ w / 2.f, h / 2.f + 110.f });
	m_window.draw(hint);
}

void HUD::drawEnergyBar(float energyPercentage) {
	uint8_t a = getUIAlpha();
	const float barWidth  = 200.f;
	const float barHeight = 14.f;
	const float margin    = 16.f;
	float screenH		  = static_cast<float>(m_window.getSize().y);

	// Background
	sf::RectangleShape bg({ barWidth, barHeight });
	bg.setFillColor(sf::Color(40, 40, 40, a));
	bg.setPosition({ margin, screenH - margin - barHeight });
	m_window.draw(bg);

	// Filler
	sf::Color fillColor;
	if (energyPercentage > .5f) fillColor = sf::Color(80, 220, 120, a);
	else if (energyPercentage > .25f) fillColor = sf::Color(240, 200, 50, a);
	else fillColor = sf::Color(220, 60, 60, a);

	sf::RectangleShape fill({ barWidth * energyPercentage, barHeight });
	fill.setFillColor(fillColor);
	fill.setPosition(bg.getPosition());
	m_window.draw(fill);

	// Border
	sf::RectangleShape border({ barWidth, barHeight });
	border.setFillColor(sf::Color::Transparent);
	border.setOutlineColor(sf::Color(150, 150, 150, a));
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
	label.setFillColor(sf::Color(180, 180, 180, a));
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
	overlay.setFillColor(sf::Color(0, 0, 0, 180));
	m_window.draw(overlay);

	drawScoreScreen(state); 
}

void HUD::draw(float energyPercent, GameState state) {

	sf::View prev = m_window.getView();
	m_window.setView(m_window.getDefaultView());

	switch (state) {
	case GameState::Intro:
		drawIntro();
		break;
	case GameState::Waking:
		if (m_wakingTimer < 0.8f) {
			sf::Text bootText(m_font, "...starting systems", 14);
			bootText.setFillColor(sf::Color(80, 120, 80, 180));
			sf::FloatRect b = bootText.getLocalBounds();
			bootText.setOrigin({
				b.position.x + b.size.x / 2.f,
				b.position.y + b.size.y / 2.f
				});
			bootText.setPosition({
				m_window.getSize().x / 2.f,
				m_window.getSize().y / 2.f + 60.f
				});
			m_window.draw(bootText);
		}
		break;
	case GameState::Playing:
	case GameState::Dying:
		drawEnergyBar(energyPercent);
		drawSignalIndicator();
		drawActivePowerUps();
		drawFeedback();
		drawZoneNotification();
		break;
	case GameState::Winning:
		// No UI while winning sequence
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
void HUD::setEnemyProximity(float alpha) {
	m_enemyProximityAlpha = alpha;
}
void HUD::drawSignalIndicator() {
	if (!m_showIndicator) return;
	uint8_t a = getUIAlpha();

	// Indicator position: top right corner
	float screenW = static_cast<float>(m_window.getSize().x);
	float margin = 20.f;
	sf::Vector2f center(screenW - 60.f, 60.f);

	// --- Background circle ---
	sf::CircleShape bg(40.f);
	bg.setOrigin({ 40.f, 40.f });
	bg.setPosition(center);
	bg.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(160 * a / 255.f)));
	bg.setOutlineColor(sf::Color(40, 60, 90, static_cast<uint8_t>(200 * a / 255.f)));
	bg.setOutlineThickness(2.f);
	m_window.draw(bg);

	// --- Directional Arrow ---
	float arrowLen = 28.f;
	sf::Vector2f tip(
		center.x + arrowLen * std::cos(m_signalAngle),
		center.y + arrowLen * std::sin(m_signalAngle)
	);

	sf::Vertex line[] = {
		sf::Vertex(center, sf::Color(80, 255, 120, a)),
		sf::Vertex(tip,	   sf::Color(80, 255, 120, a))
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
		sf::Vertex(tip,	  sf::Color(80, 255, 120, a)),
		sf::Vertex(head1, sf::Color(80, 255, 120, a))
	};
	sf::Vertex arrow2[] = {
	sf::Vertex(tip,	  sf::Color(80, 255, 120, a)),
	sf::Vertex(head2, sf::Color(80, 255, 120, a))
	};
	m_window.draw(arrow1, 2, sf::PrimitiveType::Lines);
	m_window.draw(arrow2, 2, sf::PrimitiveType::Lines);
	
	// --- Distance in tiles ---
	float distInTiles = m_signalDistance / 24.f; //TILE_SIZE = 24
	std::string distText = std::to_string(static_cast<int>(distInTiles));

	sf::Text dist(m_font, distText, 11);
	dist.setFillColor(sf::Color(80, 255, 120, static_cast<uint8_t>(200 * a / 255.f)));

	sf::FloatRect b = dist.getLocalBounds();
	dist.setOrigin({ b.position.x + b.size.x / 2.f,
					 b.position.y + b.size.y / 2.f });
	dist.setPosition({ center.x, center.y + 52.f });
	m_window.draw(dist);

	// --- "CONTROL" label ---
	sf::Text label(m_font, "CONTROL", 9);
	label.setFillColor(sf::Color(40, 60, 90, static_cast<uint8_t>(220 * a / 255.f)));

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

	// 6. Viñeta de proximidad al enemigo - rojo pulsante
	if (m_enemyProximityAlpha > 0.f) {
		float pulse = std::abs(std::sin(
			m_clock.getElapsedTime().asSeconds() * 4.f
		));
		float finalAlpha = m_enemyProximityAlpha * (0.4f + 0.3f * pulse);
		drawVignette(sf::Color(200, 20, 20), finalAlpha);
	}

	// 7. Death flash — instantáneo y muy brillante
	if (m_deathFlashTimer > 0.f) {
		float alpha = (m_deathFlashTimer / 0.3f);
		sf::RectangleShape flash(sf::Vector2f(m_window.getSize()));
		flash.setFillColor(sf::Color(255, 30, 30,
			static_cast<uint8_t>(alpha * 230.f)));
		m_window.draw(flash);
	}
}

void HUD::drawActivePowerUps() {
	if (!m_powerUps) return;

	const auto& actives = m_powerUps->getActives();
	if (actives.empty()) return;

	float screenW = static_cast<float>(m_window.getSize().x);
	float startX = screenW - 60.f;
	float startY = 130.f;  // debajo de la brújula
	float iconSize = 32.f;
	float gap = 8.f;

	for (size_t i = 0; i < actives.size(); ++i) {
		const auto& active = actives[i];
		PowerUpDef  def = getPowerUpDef(active.type);
		float       y = startY + i * (iconSize + gap);

		// Fondo del icono
		sf::RectangleShape bg({ iconSize, iconSize });
		bg.setOrigin({ iconSize / 2.f, 0.f });
		bg.setPosition({ startX, y });
		bg.setFillColor(sf::Color(0, 0, 0, 160));
		bg.setOutlineColor(sf::Color(
			def.color.r, def.color.g, def.color.b, 180));
		bg.setOutlineThickness(1.5f);
		m_window.draw(bg);

		// Icono letra
		sf::Text icon(m_font, def.icon, 18);
		icon.setFillColor(def.color);
		sf::FloatRect ib = icon.getLocalBounds();
		icon.setOrigin({
			ib.position.x + ib.size.x / 2.f,
			ib.position.y + ib.size.y / 2.f
			});
		icon.setPosition({ startX, y + iconSize / 2.f });
		m_window.draw(icon);

		// Barra de tiempo restante
		if (active.timeRemaining > 0.f) {
			float maxDur = getPowerUpDef(active.type).duration;
			float percent = active.timeRemaining / maxDur;

			sf::RectangleShape timerBg({ iconSize, 4.f });
			timerBg.setOrigin({ iconSize / 2.f, 0.f });
			timerBg.setPosition({ startX, y + iconSize + 2.f });
			timerBg.setFillColor(sf::Color(40, 40, 40));
			m_window.draw(timerBg);

			sf::RectangleShape timerFill({ iconSize * percent, 4.f });
			timerFill.setOrigin({ iconSize / 2.f, 0.f });
			timerFill.setPosition({ startX, y + iconSize + 2.f });
			timerFill.setFillColor(def.color);
			m_window.draw(timerFill);
		}
		else if (active.type == PowerUpType::Shield) {
			// Escudo: símbolo estático en vez de barra
			sf::Text shield(m_font, "■", 8);
			shield.setFillColor(sf::Color(
				def.color.r, def.color.g, def.color.b, 180));
			shield.setPosition({ startX - 6.f, y + iconSize + 2.f });
			m_window.draw(shield);
		}
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

void HUD::triggerDeathFlash() {
	m_deathFlashTimer = 0.3f;
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

void HUD::setStats(float time, int batteries,
	float distance, int rooms)
{
	m_statTime = time;
	m_statBatteries = batteries;
	m_statDistance = distance;
	m_statRooms = rooms;
}

uint8_t HUD::getUIAlpha() const {
	float progress = std::min(m_uiFadeTimer / UI_FADE_DURATION, 1.f);
	progress = progress * progress;
	return static_cast<uint8_t>(progress * 255.f);
}
