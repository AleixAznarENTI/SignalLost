#include "AudioManager.h"
#include <iostream>

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

	m_signalSound.emplace(m_signalBuffer);
	m_gameOver.emplace(m_gameoverBuffer);
	m_batterySound.emplace(m_batteryBuffer);
	m_typewriterSound.emplace(m_typewriterBuffer);
	m_alertSound.emplace(m_alertBuffer);

	m_typewriterSound.value().setVolume(60.f);
	m_alertSound.value().setVolume(70.f);

	m_ambient.setLooping(true);
	m_ambient.setVolume(40.f);

	m_heartbeat.setLooping(true);
	m_heartbeat.setVolume(0.f);

	m_stalkerMusic.setLooping(true);
	m_stalkerMusic.setVolume(0.f);
	m_stalkerMusic.play();

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

void AudioManager::update(float energyPercent) {
	updateHeartbeat(energyPercent);
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
	m_signalSound.value().stop();
	m_gameOver.value().stop();
	m_batterySound.value().stop();
	m_typewriterSound.value().stop();
	m_heartbeatPlaying = false;
	m_alertPlayerd = false;
}