#pragma once
class EnergySystem
{
public:
	EnergySystem(float maxEnegery = 100.f, float drainRate = 4.f);

	void update(float dt);

	void applyPenalty(float amount);
	void restore(float amount);
	float getEnergy() const { return m_energy; }
	float getMaxEnergy() const { return m_maxEnergy; }
	bool isDepleted() const { return m_energy <= 0.f; }

	float getPercent() const { return m_energy / m_maxEnergy; }

private:
	float m_energy;
	float m_maxEnergy;
	float m_drainRate;
};

