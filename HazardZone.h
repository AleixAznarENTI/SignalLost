#pragma once

#include <SFML/System/Vector2.hpp>
#include <string>
#include <SFML/Graphics/Color.hpp>

enum class HazardType {
    None,
    Radiation,  // drena energía extra
    Cold,       // ralentiza al jugador
    Electric    // parpadeo agresivo de linterna
};

struct HazardZone {
    sf::Vector2i origin;    // tile superior izquierdo
    int          radius;    // radio en tiles
    HazardType   type;

    // Comprueba si un tile está dentro de esta zona
    bool contains(int tx, int ty) const {
        int dx = tx - origin.x;
        int dy = ty - origin.y;
        return dx * dx + dy * dy <= radius * radius;
    }

    // Nombre para mostrar en pantalla
    std::string getName() const {
        switch (type) {
        case HazardType::Radiation: return "RADIOACTIVE ZONE";
        case HazardType::Cold:      return "SUBZERO ZONE";
        case HazardType::Electric:  return "HIGH VOLTAGE ZONE";
        default:                    return "";
        }
    }

    // Color asociado
    sf::Color getColor() const {
        switch (type) {
        case HazardType::Radiation: return sf::Color(80, 200, 80);
        case HazardType::Cold:      return sf::Color(100, 180, 255);
        case HazardType::Electric:  return sf::Color(255, 220, 50);
        default:                    return sf::Color::White;
        }
    }
};