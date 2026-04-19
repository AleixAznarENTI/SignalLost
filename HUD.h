#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"
#include "TypewriterText.h"
#include "AudioManager.h"

class HUD
{
public:
	HUD(sf::RenderWindow& window, const sf::Font& font, AudioManager& audio);

	void update(float dt, GameState gameState);
	void draw(float energyPercentage, GameState gameState);

	void onStateChanged(GameState newState);

private:
	sf::RenderWindow& m_window;
	const sf::Font& m_font;
	AudioManager& m_audio;
	TypewriterText m_typewriter;

	void drawEnergyBar(float energyPercentage);
	void drawIntro();
	void drawEndScreen(GameState state);
	void drawHint(const std::string& text);
};