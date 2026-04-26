#pragma once

#include <SFML/System/Vector2.hpp>

struct Terminal {
    sf::Vector2f position;
    bool         activated = false;
    float        revealTimer = 0.f; // cuánto tiempo queda de revelado

    void activate() { activated = true; revealTimer = 10.f; }
    bool isRevealing()  const { return revealTimer > 0.f; }
    void update(float dt) {
        if (revealTimer > 0.f) revealTimer -= dt;
    }
};