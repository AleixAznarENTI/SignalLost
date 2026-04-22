#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <string>

enum class PowerUpType {
    Flashlight,   // 🔦 linterna ampliada
    Speed,        // ⚡ velocidad
    Shield,       // 🛡️ escudo
    Radar,        // 📡 radar de enemigos
    Battery       // 🔋 batería instantánea
};

struct PowerUpDef {
    float       duration;   // 0 = instantáneo
    std::string name;
    std::string icon;
    sf::Color   color;
};

// Definiciones globales de cada tipo
inline PowerUpDef getPowerUpDef(PowerUpType type) {
    switch (type) {
    case PowerUpType::Flashlight:
        return { 15.f, "FLASHLIGHT",  "F", sf::Color(255, 240, 100) };
    case PowerUpType::Speed:
        return { 10.f, "VELOCITY", "V", sf::Color(100, 220, 255) };
    case PowerUpType::Shield:
        return { 0.f, "SHIELD",    "S", sf::Color(100, 180, 255) };
    case PowerUpType::Radar:
        return { 20.f, "RADAR",     "R", sf::Color(80, 255, 160) };
    case PowerUpType::Battery:
        return { 0.f, "BATTERY",   "B", sf::Color(255, 220,  50) };
    default:
        return { 0.f, "",          "",  sf::Color::White };
    }
}

class PowerUp {
public:
    PowerUp(sf::Vector2f position, PowerUpType type);

    sf::Vector2f getPosition()  const { return m_position; }
    PowerUpType  getType()      const { return m_type; }
    bool         isCollected()  const { return m_collected; }
    PowerUpDef   getDef()       const { return getPowerUpDef(m_type); }

    void collect() { m_collected = true; }

private:
    sf::Vector2f m_position;
    PowerUpType  m_type;
    bool         m_collected = false;
};