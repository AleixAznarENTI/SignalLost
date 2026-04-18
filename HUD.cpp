#include "HUD.h"

HUD::HUD(sf::RenderWindow& window, const sf::Font& font)
	: m_window(window)
	, m_font(font)
{}

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
	sf::Text label(m_font, "Energy", 11);
	label.setFillColor(sf::Color(180, 180, 180));
	label.setPosition({ margin, screenH - margin - barHeight - 16.f });
	m_window.draw(label);
}

void HUD::drawCenteredText(const std::string& text, sf::Color color) {
	sf::Text msg(m_font, text, 36);
	msg.setFillColor(color);
	msg.setStyle(sf::Text::Bold);

	sf::FloatRect bounds = msg.getLocalBounds();
	msg.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
	msg.setPosition(sf::Vector2f(m_window.getSize()) / 2.f);
	m_window.draw(msg);
}

void HUD::draw(float energyPercent, GameState state) {
	sf::View prev = m_window.getView();
	m_window.setView(m_window.getDefaultView());

	drawEnergyBar(energyPercent);

	if (state == GameState::Victory)
		drawCenteredText("SIGNAL FOUND", sf::Color(80, 255, 120));
	else if (state == GameState::GameOver)
		drawCenteredText("ENERGY DEPLETED", sf::Color(220, 60, 60));

	m_window.setView(prev);
}