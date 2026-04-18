#include "TypewriterText.h"
TypewriterText::TypewriterText(const sf::Font& font,
							   unsigned int	   charSize,
							   float		   charDelay)
	: m_font(font)
	, m_charSize(charSize)
	, m_charDelay(charDelay)
	, m_timer(0.f)
	, m_finished(false)
	, m_sfText(font, "", charSize)
{}

void TypewriterText::start(const std::string& fullText) {
	m_full = fullText;
	m_visible.clear();
	m_timer = 0.f;
	m_finished = false;
}

bool TypewriterText::update(float dt) {
	if (m_finished) return true;

	m_timer += dt;

	size_t targetVisible = static_cast<size_t>(m_timer / m_charDelay);
	targetVisible = std::min(targetVisible, m_full.size());

	m_visible = m_full.substr(0, targetVisible);
	m_finished = (targetVisible >= m_full.size());

	return m_finished;
}

void TypewriterText::draw(sf::RenderWindow& window,
						  sf::Vector2f		position,
						  sf::Color			color) 
{
	m_sfText.setString(m_visible);
	m_sfText.setFillColor(color);

	sf::FloatRect bounds = m_sfText.getLocalBounds();
	m_sfText.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
	m_sfText.setPosition(position);

	window.draw(m_sfText);
}