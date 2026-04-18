#pragma once
#include <SFML/Graphics.hpp>
#include <string>
class TypewriterText
{
public:
	TypewriterText(const sf::Font& font,
		unsigned int charSize = 20,
		float charDelay = 0.04f);

	void start(const std::string& fullText);

	bool update(float dt);

	void draw(sf::RenderWindow& window, sf::Vector2f position, sf::Color color);

	bool isFinished() const { return m_finished; }
	void reset()			{ m_visible.clear(); m_timer = 0.f; m_finished = false; }

private:
	const sf::Font& m_font;
	unsigned int	m_charSize;
	float			m_charDelay;

	std::string		m_full;
	std::string		m_visible;
	float			m_timer;
	bool			m_finished;

	sf::Text		m_sfText

};

