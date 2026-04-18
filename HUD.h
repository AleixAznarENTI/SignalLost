#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"
class HUD
{
public:
	HUD(sf::RenderWindow& window, const sf::Font& font);

	void draw(float energyPercentage, GameState gameState);

private:
	sf::RenderWindow& m_window;
	const sf::Font& m_font;

	void drawEnergyBar(float energyPercentage);
	void drawCenteredText(const std::string& text, sf::Color color);
};

