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

	size_t prevSize = m_visible.size();

	m_timer += dt;
	size_t targetVisible = static_cast<size_t>(m_timer / m_charDelay);
	targetVisible = std::min(targetVisible, m_full.size());
	m_visible = m_full.substr(0, targetVisible);
	m_finished = (targetVisible >= m_full.size());

	if (m_visible.size() > prevSize && m_onChar) {
		char newChar = m_full[m_visible.size() - 1];
		if (newChar != '\n') {
			m_onChar();
		}
	}

	return m_finished;
}

void TypewriterText::draw(sf::RenderWindow& window,
	sf::Vector2f      position,
	sf::Color         color)
{
	if (m_visible.empty()) return;

	m_sfText.setString(m_full);
	m_sfText.setOrigin({ 0.f, 0.f });
	m_sfText.setPosition({ 0.f, 0.f });

	sf::FloatRect bounds = m_sfText.getLocalBounds();

	m_sfText.setOrigin({
		bounds.position.x + bounds.size.x / 2.f,
		bounds.position.y + bounds.size.y / 2.f
		});

	// Ahora sí ponemos el texto visible
	m_sfText.setString(m_visible);
	m_sfText.setFillColor(color);
	m_sfText.setPosition(position);

	window.draw(m_sfText);
}