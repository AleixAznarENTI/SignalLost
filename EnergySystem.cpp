#include "EnergySystem.h"
#include <algorithm>

EnergySystem::EnergySystem(float maxEnergy, float drainRate)
	: m_energy(maxEnergy)
	, m_maxEnergy(maxEnergy)
	, m_drainRate(drainRate)
{}

void EnergySystem::update(float dt) {
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