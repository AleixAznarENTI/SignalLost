#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

struct MenuItem {
    std::string             label;
    std::function<void()>   action;
    bool                    isSlider = false;   // para volumen
    float* sliderValue = nullptr;
    float                   sliderMin = 0.f;
    float                   sliderMax = 1.f;
};

class PauseMenu {
public:
    PauseMenu(sf::RenderWindow& window, const sf::Font& font);

    void addItem(const std::string& label,
        std::function<void()> action);

    void addSlider(const std::string& label,
        float* value,
        float  min,
        float  max);

    // Navegación con teclado
    void handleInput(const sf::Event& event);

    void draw();

    // Reset de selección al abrir
    void open() { m_selected = 0; }

private:
    sf::RenderWindow& m_window;
    const sf::Font& m_font;
    std::vector<MenuItem> m_items;
    int                  m_selected = 0;

    void drawItem(const MenuItem& item, float y,
        bool selected, size_t index);
};