#pragma once

#include <SFML/Graphics.hpp>
#include "Map.h"

class Minimap {
public:
    Minimap(sf::RenderWindow& window, float tileSize = 3.f, float margin = 16.f);

    void bake(const Map& map);

    void revealAt(sf::Vector2i playerTile, const Map& map);

    void draw(sf::Vector2f playerWorldPos, float worldTileSize);

private:
    sf::RenderWindow& m_window;
    float             m_tileSize;
    float             m_margin;

    sf::RenderTexture m_texture;
    sf::RectangleShape m_tileShape;

    std::vector<std::vector<bool>> m_revealed;

    int m_mapW = 0;
    int m_mapH = 0;

    sf::Vector2f toMiniPos(int tx, int ty) const;
};