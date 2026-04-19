#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"
#include "TypewriterText.h"

class HUD
{
public:
	HUD(sf::RenderWindow& window, const sf::Font& font);

	void update(float dt, GameState gameState);
	void draw(float energyPercentage, GameState gameState);

	void onStateChanged(GameState newState);

private:
	sf::RenderWindow& m_window;
	const sf::Font& m_font;

	TypewriterText m_typewriter;

	void drawEnergyBar(float energyPercentage);
	void drawIntro();
	void drawEndScreen(GameState state);
	void drawHint(const std::string& text);
};