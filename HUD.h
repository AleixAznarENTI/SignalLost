#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"
#include "TypewriterText.h"
#include "AudioManager.h"
#include "Map.h"

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

	// Feedback triggers
	void triggerBatteryPickup();
	void triggerDangerEnter();
	void triggerControlEnter();
	void triggerSignalFound();
	void triggerGameOver();
	void triggerZoneNotification(const std::string& name, sf::Color color);

	void setCurrentRoom(RoomType room);

private:
	sf::RenderWindow& m_window;
	const sf::Font& m_font;
	AudioManager& m_audio;
	TypewriterText m_typewriter;

	// Signal state
	float m_signalAngle = 0.f;
	float m_signalDistance = 0.f;
	bool m_showIndicator = false;

	// --- Feedback timers ---
	float m_batteryFlashTimer = 0.f;
	float m_dangerVignetteTimer = 0.f;
	float m_controlVignetteTimer = 0.f;
	float m_signalFlashTimer = 0.f;
	float m_gameOverFadeTimer = 0.f;

	RoomType m_currentRoom = RoomType::Normal;
	RoomType m_prevRoom = RoomType::Normal;

	// --- ZoneNotificators ---
	float m_zoneNotifTimer = 0.f;
	std::string m_zoneNotifText;
	sf::Color m_zoneNotifColor;

	// --- Draw Helpers ---
	void drawEnergyBar(float energyPercentage);
	void drawIntro();
	void drawEndScreen(GameState state);
	void drawSignalIndicator();
	void drawVignette(sf::Color color, float alpha);
	void drawFeedback();
	void drawZoneNotification();
};