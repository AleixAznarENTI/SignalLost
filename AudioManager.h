#pragma once

#include <SFML/Audio.hpp>
#include "HazardZone.h"
#include <optional>
#include <string>

class AudioManager
{
public:
	AudioManager();

	bool loadAll(const std::string& assetsPath);

	void playAmbient();

	void update(float energyPercent);
	void updateStalkerMusic(float proximityAlpha);
	void updateZoneAudio(HazardType currentHazard);

	void playSignalFound();
	void playGameOver();
	void playBatteryPickup();
	void playEnemyAlert();
	void playTypewriterClick();
	void stopAll();
private:
	sf::Music m_ambient;
	sf::Music m_heartbeat;
	sf::Music m_stalkerMusic;

	// --- Zone Streams ---
	sf::Music m_zoneRadiation;
	sf::Music m_zoneCold;
	sf::Music m_zoneElectric;

	sf::SoundBuffer m_signalBuffer;
	sf::SoundBuffer m_gameoverBuffer;
	sf::SoundBuffer m_batteryBuffer;
	sf::SoundBuffer m_typewriterBuffer;
	sf::SoundBuffer m_alertBuffer;
	std::optional<sf::Sound> m_alertSound;
	std::optional<sf::Sound> m_typewriterSound;
	std::optional<sf::Sound> m_signalSound;
	std::optional<sf::Sound> m_gameOver;
	std::optional<sf::Sound> m_batterySound;

	HazardType m_activeZoneAudio = HazardType::None;
	bool m_alertPlayerd = false;
	bool m_heartbeatPlaying = false;

	void updateHeartbeat(float energyPercent);

};

