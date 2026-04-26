#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>

struct DataLog {
    sf::Vector2f position;
    std::string  message;    // fragmento de historia
    float        energyBonus = 15.f;
    bool         collected = false;

    void collect() { collected = true; }
};