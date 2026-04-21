#pragma once

#include <SFML/Audio.hpp>
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

	bool m_alertPlayerd = false;
	bool m_heartbeatPlaying = false;

	void updateHeartbeat(float energyPercent);

};

