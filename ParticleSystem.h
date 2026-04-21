#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "HazardZone.h"

struct Particle {
	sf::Vector2f position;
	sf::Vector2f velocity;
	float lifetime;
	float maxLife;
	sf::Color color = sf::Color(200, 210, 255);
};

class ParticleSystem
{
public:
	ParticleSystem(int maxParticles = 200);

	void emit(sf::Vector2f origin, float dt);

	void update(float dt);

	void draw(sf::RenderWindow& window);

	void emitHazard(sf::Vector2f origin, HazardType type, float dt);

private:
	std::vector<Particle> m_particles;
	sf::VertexArray m_vertices;
	int m_maxParticles;

	static float randRange(float min, float max);
};

