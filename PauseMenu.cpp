#include "PauseMenu.h"
#include <algorithm>

PauseMenu::PauseMenu(sf::RenderWindow& window, const sf::Font& font)
    : m_window(window), m_font(font)
{
}

void PauseMenu::addItem(const std::string& label,
    std::function<void()> action)
{
    m_items.push_back({ label, action, false, nullptr });
}

void PauseMenu::addSlider(const std::string& label,
    float* value, float min, float max)
{
    m_items.push_back({ label, nullptr, true, value, min, max });
}

void PauseMenu::handleInput(const sf::Event& event) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key) return;

    int count = static_cast<int>(m_items.size());

    if (key->code == sf::Keyboard::Key::W ||
        key->code == sf::Keyboard::Key::Up) {
        m_selected = (m_selected - 1 + count) % count;
    }

    if (key->code == sf::Keyboard::Key::S ||
        key->code == sf::Keyboard::Key::Down) {
        m_selected = (m_selected + 1) % count;
    }

    // Acción o slider
    const auto& item = m_items[m_selected];

    if (key->code == sf::Keyboard::Key::Enter ||
        key->code == sf::Keyboard::Key::Space) {
        if (item.action) item.action();
    }

    if (item.isSlider && item.sliderValue) {
        float step = (item.sliderMax - item.sliderMin) * 0.05f;
        if (key->code == sf::Keyboard::Key::A ||
            key->code == sf::Keyboard::Key::Left) {
            *item.sliderValue = std::max(
                item.sliderMin, *item.sliderValue - step);
        }
        if (key->code == sf::Keyboard::Key::D ||
            key->code == sf::Keyboard::Key::Right) {
            *item.sliderValue = std::min(
                item.sliderMax, *item.sliderValue + step);
        }
    }
}

void PauseMenu::drawItem(const MenuItem& item,
    float y, bool selected, size_t index)
{
    float w = static_cast<float>(m_window.getSize().x);

    if (item.isSlider) {
        // Label
        sf::Text label(m_font, item.label, 16);
        label.setFillColor(selected
            ? sf::Color(220, 220, 255)
            : sf::Color(140, 140, 160));
        sf::FloatRect lb = label.getLocalBounds();
        label.setOrigin({
            lb.position.x + lb.size.x / 2.f,
            lb.position.y + lb.size.y / 2.f
            });
        label.setPosition({ w / 2.f, y - 12.f });
        m_window.draw(label);

        // Slider bar
        float barW = 200.f;
        float barH = 6.f;
        float percent = (*item.sliderValue - item.sliderMin) /
            (item.sliderMax - item.sliderMin);

        sf::RectangleShape barBg({ barW, barH });
        barBg.setOrigin({ barW / 2.f, barH / 2.f });
        barBg.setPosition({ w / 2.f, y + 8.f });
        barBg.setFillColor(sf::Color(50, 50, 70));
        m_window.draw(barBg);

        sf::RectangleShape barFill({ barW * percent, barH });
        barFill.setOrigin({ barW / 2.f, barH / 2.f });
        barFill.setPosition({ w / 2.f, y + 8.f });
        barFill.setFillColor(selected
            ? sf::Color(150, 180, 255)
            : sf::Color(80, 100, 160));
        m_window.draw(barFill);

        // Knob
        sf::CircleShape knob(5.f);
        knob.setOrigin({ 5.f, 5.f });
        knob.setPosition({
            w / 2.f - barW / 2.f + barW * percent,
            y + 8.f
            });
        knob.setFillColor(sf::Color(200, 210, 255));
        m_window.draw(knob);

    }
    else {
        // Regular item
        sf::Text text(m_font, item.label, 20);
        text.setFillColor(selected
            ? sf::Color(255, 255, 255)
            : sf::Color(120, 120, 150));

        if (selected) text.setStyle(sf::Text::Bold);

        // Arrow indicator
        if (selected) {
            sf::Text arrow(m_font, ">", 20);
            arrow.setFillColor(sf::Color(150, 180, 255));
            sf::FloatRect ab = arrow.getLocalBounds();
            arrow.setOrigin({
                ab.position.x + ab.size.x / 2.f,
                ab.position.y + ab.size.y / 2.f
                });
            arrow.setPosition({ w / 2.f - 120.f, y });
            m_window.draw(arrow);
        }

        sf::FloatRect tb = text.getLocalBounds();
        text.setOrigin({
            tb.position.x + tb.size.x / 2.f,
            tb.position.y + tb.size.y / 2.f
            });
        text.setPosition({ w / 2.f, y });
        m_window.draw(text);
    }
}

void PauseMenu::draw() {
    float w = static_cast<float>(m_window.getSize().x);
    float h = static_cast<float>(m_window.getSize().y);

    // Overlay semitransparente
    sf::RectangleShape overlay(sf::Vector2f(m_window.getSize()));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    m_window.draw(overlay);

    // Título
    sf::Text title(m_font, "PAUSED", 36);
    title.setFillColor(sf::Color(180, 200, 255));
    title.setStyle(sf::Text::Bold);
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin({
        tb.position.x + tb.size.x / 2.f,
        tb.position.y + tb.size.y / 2.f
        });
    title.setPosition({ w / 2.f, h / 2.f - 160.f });
    m_window.draw(title);

    // Línea separadora
    sf::RectangleShape line({ 250.f, 1.f });
    line.setOrigin({ 125.f, 0.f });
    line.setPosition({ w / 2.f, h / 2.f - 130.f });
    line.setFillColor(sf::Color(80, 100, 160, 150));
    m_window.draw(line);

    // Items — centrados verticalmente
    float startY = h / 2.f - 80.f;
    float stepY = 55.f;

    for (size_t i = 0; i < m_items.size(); ++i) {
        drawItem(m_items[i], startY + i * stepY,
            static_cast<int>(i) == m_selected, i);
    }

    // Hint de navegación
    sf::Text hint(m_font, "W/S to navigate   A/D to adjust   Enter to confirm", 12);
    hint.setFillColor(sf::Color(80, 80, 100));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin({
        hb.position.x + hb.size.x / 2.f,
        hb.position.y + hb.size.y / 2.f
        });
    hint.setPosition({ w / 2.f, h / 2.f + 200.f });
    m_window.draw(hint);
}