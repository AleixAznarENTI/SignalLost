#include "EnergySystem.h"
#include <algorithm>

EnergySystem::EnergySystem(float maxEnergy, float drainRate)
	: m_energy(maxEnergy)
	, m_maxEnergy(maxEnergy)
	, m_drainRate(drainRate)
{}

void EnergySystem::update(float dt, bool inSafeRoom) {
	if (inSafeRoom) {
		m_energy += 3.f * dt;
		m_energy = std::clamp(m_energy, 0.f, m_maxEnergy);
		return;
	}
	m_energy -= m_drainRate * dt;
	m_energy = std::clamp(m_energy, 0.f,m_maxEnergy);
}

void EnergySystem::applyPenalty(float amount) {
	m_energy -= amount;
	m_energy = std::clamp(m_energy, 0.f, m_maxEnergy);
}

void EnergySystem::restore(float amount) {
	m_energy += amount;
	m_energy = std::clamp(m_energy, 0.f, m_maxEnergy);
}