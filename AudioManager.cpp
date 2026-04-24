#include "AudioManager.h"
#include <iostream>
#include "HazardZone.h"

AudioManager::AudioManager() 
	: m_signalSound(m_signalBuffer)
	, m_gameOver(m_gameoverBuffer)
{}

bool AudioManager::loadAll(const std::string& assetsPath) {
	bool ok = true;

	if (!m_ambient.openFromFile(assetsPath + "ambient.ogg")) {
		std::cerr << "Failed to load ambient.ogg\n";
		ok = false;
	}
	if (!m_heartbeat.openFromFile(assetsPath + "heartbeat.ogg")) {
		std::cerr << "Failed to load heartbeat.ogg\n";
		ok = false;
	}
	if (!m_signalBuffer.loadFromFile(assetsPath + "signal.ogg")) {
		std::cerr << "Failed to load signal.ogg\n";
		ok = false;
	}
	if (!m_gameoverBuffer.loadFromFile(assetsPath + "gameover.ogg")) {
		std::cerr << "Failed to load gameover.ogg\n";
		ok = false;
	}
	if (!m_batteryBuffer.loadFromFile(assetsPath + "battery.ogg")) {
		std::cerr << "Failed to load battery.ogg\n";
		ok = false;
	}
	if (!m_typewriterBuffer.loadFromFile(assetsPath + "typewriter.ogg")) {
		std::cerr << "Failed to load typewriter.ogg\n";
		ok = false;
	}
	if (!m_alertBuffer.loadFromFile(assetsPath + "alert.ogg")) {
		std::cerr << "Error: alert.ogg\n";
		ok = false;
	}
	if (!m_stalkerMusic.openFromFile(assetsPath + "stalker.ogg")) {
		std::cerr << "Error: stalker.ogg\n";
		ok = false;
	}
	if (!m_zoneRadiation.openFromFile(assetsPath + "zone_radiation.ogg")) {
		std::cerr << "Error: zone_radiation.ogg\n";
	}
	if (!m_zoneCold.openFromFile(assetsPath + "zone_cold.ogg")) {
		std::cerr << "Error: zone_cold.ogg\n";
	}
	if (!m_zoneElectric.openFromFile(assetsPath + "zone_electric.ogg")) {
		std::cerr << "Error: zone_electric.ogg\n";
	}

	m_signalSound.emplace(m_signalBuffer);
	m_gameOver.emplace(m_gameoverBuffer);
	m_batterySound.emplace(m_batteryBuffer);
	m_typewriterSound.emplace(m_typewriterBuffer);
	m_alertSound.emplace(m_alertBuffer);

	m_typewriterSound.value().setVolume(60.f);
	m_alertSound.value().setVolume(70.f);

	m_ambient.setLooping(true);
	m_heartbeat.setLooping(true);
	m_zoneRadiation.setLooping(true);
	m_zoneCold.setLooping(true);
	m_zoneElectric.setLooping(true);
	m_stalkerMusic.setLooping(true);

	m_ambient.setVolume(40.f);
	m_heartbeat.setVolume(0.f);
	m_zoneRadiation.setVolume(0.f);
	m_zoneCold.setVolume(0.f);
	m_zoneElectric.setVolume(0.f);
	m_stalkerMusic.setVolume(0.f);


	m_stalkerMusic.play();
	m_zoneRadiation.play();
	m_zoneCold.play();
	m_zoneElectric.play();

	return ok;
}

void AudioManager::playAmbient() {
	m_ambient.play();
	m_heartbeat.play();
	m_heartbeatPlaying = true;
}

void AudioManager::playEnemyAlert() {
	m_alertSound.value().stop();
	m_alertSound.value().play();
}

void AudioManager::updateStalkerMusic(float proximityAlpha) {
	m_stalkerMusic.setVolume(proximityAlpha * 50.f);
}

void AudioManager::update(float energyPercent) {
	updateHeartbeat(energyPercent);
}

void AudioManager::updateHeartbeat(float energyPercent) {
	if(!m_heartbeatPlaying) return;

	if (energyPercent > .4f) {
		m_heartbeat.setVolume(0.f);
		return;
	}

	float t = 1.f - energyPercent / .4f;
	float volume = 70.f * t;
	m_heartbeat.setVolume(volume);

	float pitch = 1.f + 0.4f * t;
	m_heartbeat.setPitch(pitch);
}

void AudioManager::updateZoneAudio(HazardType currentHazard) {
	float targetRad = (currentHazard == HazardType::Radiation) ? 45.f : 0.f;
	float targetCold = (currentHazard == HazardType::Cold) ? 45.f : 0.f;
	float targetElec = (currentHazard == HazardType::Electric) ? 45.f : 0.f;

	auto fade = [](sf::Music& music, float target) {
		float current = music.getVolume();
		float next = current + (target - current) * 0.05f;
		music.setVolume(next);
		};

	fade(m_zoneRadiation, targetRad);
	fade(m_zoneCold, targetCold);
	fade(m_zoneElectric, targetElec);
}


void AudioManager::playSignalFound() {
	m_ambient.stop();
	m_heartbeat.stop();
	m_signalSound.value().play();
}

void AudioManager::playGameOver() {
	m_ambient.stop();
	m_heartbeat.stop();
	m_gameOver.value().play();
}

void AudioManager::playBatteryPickup() {
	m_batterySound.value().play();
}

void AudioManager::playTypewriterClick() {
	m_typewriterSound.value().stop();
	m_typewriterSound.value().play();
}

void AudioManager::stopAll() {
	m_ambient.stop();
	m_heartbeat.stop();
	m_stalkerMusic.stop();
	m_zoneRadiation.stop();
	m_zoneCold.stop();
	m_zoneElectric.stop();
	m_signalSound.value().stop();
	m_gameOver.value().stop();
	m_batterySound.value().stop();
	m_typewriterSound.value().stop();
	m_heartbeatPlaying = false;
	m_alertPlayerd = false;
}

void AudioManager::setMasterVolume(float volume) {
	m_signalSound.value().setVolume(volume);
	m_gameOver.value().setVolume(volume);
	m_batterySound.value().setVolume(volume);
	m_typewriterSound.value().setVolume(volume * 0.6f);
}

void AudioManager::setMusicVolume(float volume) {
	m_ambient.setVolume(volume * 0.4f);
	m_heartbeat.setVolume(
		m_heartbeat.getVolume() > 0.f ? volume * 0.7f : 0.f);
	m_stalkerMusic.setVolume(
		m_stalkerMusic.getVolume() > 0.f ? volume * 0.5f : 0.f);
}