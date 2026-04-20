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
	void setSignalInfo(sf::Vector2f playerPos,
					   sf::Vector2f signalPos,
					   bool			inControlRoom);

private:
	sf::RenderWindow& m_window;
	const sf::Font& m_font;
	AudioManager& m_audio;
	TypewriterText m_typewriter;

	float m_signalAngle = 0.f;
	float m_signalDistance = 0.f;
	bool m_showIndicator = false;

	void drawEnergyBar(float energyPercentage);
	void drawIntro();
	void drawEndScreen(GameState state);
	void drawSignalIndicator();
};