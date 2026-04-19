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

	void playSignalFound();
	void playGameOver();
	void playBatteryPickup();
	void playTypewriterClick();
	void stopAll();
private:
	sf::Music m_ambient;
	sf::Music m_heartbeat;

	sf::SoundBuffer m_signalBuffer;
	sf::SoundBuffer m_gameoverBuffer;
	sf::SoundBuffer m_batteryBuffer;
	sf::SoundBuffer m_typewriterBuffer;
	std::optional<sf::Sound> m_typewriterSound;
	std::optional<sf::Sound> m_signalSound;
	std::optional<sf::Sound> m_gameOver;
	std::optional<sf::Sound> m_batterySound;

	bool m_heartbeatPlaying = false;

	void updateHeartbeat(float energyPercent);
};

