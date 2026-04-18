#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct Particle {
	sf::Vector2f position;
	sf::Vector2f velocity;
	float lifetime;
	float maxLife;
};

class ParticleSystem
{
public:
	ParticleSystem(int maxParticles = 200);

	void emit(sf::Vector2f origin, float dt);

	void update(float dt);

	void draw(sf::RenderWindow& window);

private:
	std::vector<Particle> m_particles;
	int m_maxParticles;

	static float randRange(float min, float max);
};

