#include "ParticleSystem.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>
#define M_PI 3.14159265358979323846

////////////////////////////////////////////////
///	Faster and short particles -> Wind Sensation
///	float speed = randRange(20.f, 50.f);
/// p.lifetime = randRange(0.5f, 1.5f);
/// 
/// 
/// Slow and long particles -> Dust Sensation
/// float speed = randRange(1.f, 5.f);
/// p.lifetime = randRange(4.f, 8.f);
/// 
/// 
/// More particles -> More intense sensation
/// ParticleSystem particles(500);
/// 
/// 
/// Warmer colors -> Oxided metal sensation
/// vertices[i].color = sf::Color(255, 200, 150, alpha);
////////////////////////////////////////////////



ParticleSystem::ParticleSystem(int maxParticles) 
	: m_maxParticles(maxParticles)
	, m_vertices(sf::PrimitiveType::Points)
{
	m_particles.reserve(maxParticles);
}

float ParticleSystem::randRange(float min, float max) {
	return min + (max - min) * (rand() / static_cast<float>(RAND_MAX));
}

void ParticleSystem::emit(sf::Vector2f origin, float dt) {
	if (static_cast<int>(m_particles.size()) >= m_maxParticles)
		return;

	int toEmit = static_cast<int>(30.f * dt) + (randRange(0.f, 1.f) < 30.f * dt ? 1 : 0);

	for (int i = 0; i < toEmit; ++i) {
		if (static_cast<int>(m_particles.size()) >= m_maxParticles)
			break;

		Particle p;

		float angle = randRange(0.f, 2.f * M_PI);
		float radius = randRange(10.f, 100.f);
		p.position = {
			origin.x + radius * std::cos(angle),
			origin.y + radius * std::sin(angle)
		};

		float speed = randRange(2.f, 12.f);
		float velAngle = randRange(0.f, 2.f * M_PI);
		p.velocity = {
			speed * std::cos(velAngle),
			speed * std::sin(velAngle)
		};

		p.lifetime = randRange(1.5f, 3.5f);
		p.maxLife = p.lifetime;

		m_particles.push_back(p);
	}
}

void ParticleSystem::update(float dt) {
	for (auto& p : m_particles) {
		p.position += p.velocity * dt;
		p.lifetime -= dt;

		p.velocity *= 0.98f; // Damping
	}

	m_particles.erase(
		std::remove_if(m_particles.begin(), m_particles.end(),
			[](const Particle& p) {return p.lifetime <= 0.f; }),
		m_particles.end()
	);
}

void ParticleSystem::draw(sf::RenderWindow& window) {
	m_vertices.resize(m_particles.size());

	for (size_t i = 0; i < m_particles.size(); ++i) {
		const Particle& p = m_particles[i];
		float lifeFraction = p.lifetime / p.maxLife;
		uint8_t alpha = static_cast<uint8_t>(lifeFraction * 180.f);

		m_vertices[i].position = p.position;
		m_vertices[i].color = sf::Color(p.color.r, p.color.g, p.color.b, alpha);
	}

	window.draw(m_vertices);
}

void ParticleSystem::emitHazard(sf::Vector2f origin,
	HazardType   type,
	float        dt)
{
	if (type == HazardType::None) return;
	if (static_cast<int>(m_particles.size()) >= m_maxParticles) return;

	// Cada zona emite más partículas que el polvo normal
	int toEmit = static_cast<int>(60.f * dt) +
		(randRange(0.f, 1.f) < 60.f * dt ? 1 : 0);

	for (int i = 0; i < toEmit; ++i) {
		if (static_cast<int>(m_particles.size()) >= m_maxParticles) break;

		Particle p;

		float angle = randRange(0.f, 2.f * 3.14159265f);
		float radius = randRange(5.f, 60.f);
		p.position = {
			origin.x + radius * std::cos(angle),
			origin.y + radius * std::sin(angle)
		};

		switch (type) {
		case HazardType::Radiation:
			// Partículas que suben lentamente — humo radiactivo
			p.velocity = { randRange(-8.f, 8.f), randRange(-20.f, -5.f) };
			p.lifetime = randRange(1.f, 2.5f);
			p.color = sf::Color(80, 220, 80);   // verde
			break;

		case HazardType::Cold:
			// Copos que caen y derivan — nieve/escarcha
			p.velocity = { randRange(-5.f, 5.f), randRange(5.f, 15.f) };
			p.lifetime = randRange(1.5f, 3.f);
			p.color = sf::Color(100, 180, 255); // azul claro
			break;

		case HazardType::Electric:
			// Chispas rápidas en todas direcciones
			p.velocity = {
				randRange(-40.f, 40.f),
				randRange(-40.f, 40.f)
			};
			p.lifetime = randRange(0.1f, 0.4f);    // muy cortas
			p.color = sf::Color(255, 240, 80);  // amarillo
			break;

		default: break;
		}

		p.maxLife = p.lifetime;
		m_particles.push_back(p);
	}
}

void ParticleSystem::emitBattery(sf::Vector2f origin, float dt) {
	if (static_cast<int>(m_particles.size()) >= m_maxParticles) return;

	int toEmit = static_cast<int>(15.f * dt) +
		(randRange(0.f, 1.f) < 15.f * dt ? 1 : 0);

	for (int i = 0; i < toEmit; ++i) {
		if (static_cast<int>(m_particles.size()) >= m_maxParticles) break;

		Particle p;
		float angle = randRange(0.f, 2.f * M_PI);
		float radius = randRange(2.f, 12.f);
		p.position = {
			origin.x + radius * std::cos(angle),
			origin.y + radius * std::sin(angle)
		};
		p.velocity = { randRange(-6.f, 6.f), randRange(-15.f, -3.f) };
		p.lifetime = randRange(0.3f, 0.8f);
		p.maxLife = p.lifetime;
		p.color = sf::Color(255, 220, 50); // golden
		m_particles.push_back(p);
	}
}

void ParticleSystem::emitEnemy(sf::Vector2f origin,
							   bool isChasing,
							   float dt) 
{
	if (static_cast<int>(m_particles.size()) >= m_maxParticles) return;

	float rate = isChasing ? 80.f : 25.f;
	int toEmit = static_cast<int>(rate * dt) +
		(randRange(0.f, 1.f) < rate * dt ? 1 : 0);

	for (int i = 0; i < toEmit; ++i) {
		if (static_cast<int>(m_particles.size()) >= m_maxParticles) break;

		Particle p;
		float angle = randRange(0.f, 2.f * M_PI);
		float radius = randRange(2.f, isChasing ? 20.f : 10.f);

		p.position = {
			origin.x + radius * std::cos(angle),
			origin.y + radius * std::sin(angle)
		};

		if (isChasing) {
			p.velocity = {
				std::cos(angle) * randRange(15.f, 40.f),
				std::sin(angle) * randRange(15.f, 40.f)
			};
			p.lifetime = randRange(0.15f, 0.4f);
			p.color = sf::Color(255, 60, 60);
		}
		else {
			p.velocity = {
				randRange(-5.f, 5.f),
				randRange(-15.f, -4.f)
			};
			p.lifetime = randRange(0.5f, 1.2f);
			p.color = sf::Color(140, 30, 30);
		}

		p.maxLife = p.lifetime;
		m_particles.push_back(p);
	}
}

void ParticleSystem::emitBurst(sf::Vector2f origin,
	sf::Color    color,
	int          count)
{
	for (int i = 0; i < count; ++i) {
		if (static_cast<int>(m_particles.size()) >= m_maxParticles) break;

		Particle p;
		float angle = randRange(0.f, 2.f * 3.14159265f);
		float speed = randRange(30.f, 150.f);

		p.position = origin;
		p.velocity = {
			std::cos(angle) * speed,
			std::sin(angle) * speed
		};
		p.lifetime = randRange(0.8f, 2.f);
		p.maxLife = p.lifetime;
		p.color = color;
		m_particles.push_back(p);
	}
}