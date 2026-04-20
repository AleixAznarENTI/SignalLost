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
}

void HUD::drawEnergyBar(float energyPercentage) {
	const float barwidth  = 200.f;
	const float barHeight = 14.f;
	const float margin    = 16.f;
	float screenH		  = static_cast<float>(m_window.getSize().y);

	sf::RectangleShape bg({ barwidth, barHeight });
	bg.setFillColor(sf::Color(40, 40, 40));
	bg.setPosition({ margin, screenH - margin - barHeight });
	m_window.draw(bg);

	sf::Color fillColor;
	if (energyPercentage > .5f) fillColor = sf::Color(80, 220, 120);
	else if (energyPercentage > .25f) fillColor = sf::Color(240, 200, 50);
	else fillColor = sf::Color(220, 60, 60);

	sf::RectangleShape fill({ barwidth * energyPercentage, barHeight });
	fill.setFillColor(fillColor);
	fill.setPosition(bg.getPosition());
	m_window.draw(fill);

	// Border
	sf::RectangleShape border({ barwidth, barHeight });
	border.setFillColor(sf::Color::Transparent);
	border.setOutlineColor(sf::Color(150, 150, 150));
	border.setOutlineThickness(1.f);
	border.setPosition(bg.getPosition());
	m_window.draw(border);

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
		break;
	case GameState::Victory:
	case GameState::GameOver:
		drawEnergyBar(energyPercent);
		drawEndScreen(state);
		break;
	}
	
	m_window.setView(prev);
}

void HUD::setSignalInfo(sf::Vector2f playerPos,
						sf::Vector2f signalPos,
						bool		 inControlRoom) 
{
	m_showIndicator = inControlRoom;
	if (!inControlRoom) return;

	sf::Vector2f diff = signalPos - playerPos;
	m_signalAngle	  = std::atan2(diff.y, diff.x);
	m_signalDistance  = std::sqrt(diff.x * diff.x + diff.y * diff.y);
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