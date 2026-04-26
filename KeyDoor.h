#pragma once

#include <SFML/System/Vector2.hpp>

struct Key {
    sf::Vector2f position;
    int          doorId = 0;
    bool         collected = false;
    void collect() { collected = true; }
};

struct Door {
    sf::Vector2i tilePos;  // posición en tiles
    int          keyId = 0;
    bool         open = false;
    void unlock() { open = true; }
};