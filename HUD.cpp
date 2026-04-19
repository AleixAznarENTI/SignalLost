#include "HUD.h"
#include <cstdint>
#include <iostream>

HUD::HUD(sf::RenderWindow& window, const sf::Font& font)
	: m_window(window)
	, m_font(font)
	, m_typewriter(font, 18, 0.035f)
{}

void HUD::onStateChanged(GameState newState) {
	switch (newState) {
	case GameState::Intro:
		m_typewriter.start(
			"SIGNAL LOST\n\n"
			"The abandoned station\n"
			"keeps an SOS signal.\n\n"
			"Find the signar before\n"
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
			"your ubication.\n\n"
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
		break;
	case GameState::Victory:
	case GameState::GameOver:
		drawEnergyBar(energyPercent);
		drawEndScreen(state);
		break;
	}
	
	m_window.setView(prev);
}