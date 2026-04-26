#pragma once

#include <SFML/Graphics.hpp>
#include "Map.h"
#include "Enemy.h"

class Minimap {
public:
    Minimap(sf::RenderWindow& window, float tileSize = 3.f, float margin = 16.f);

    void bake(const Map& map);

    void revealAt(sf::Vector2i playerTile, 
                  float        flashAngle,
                  float        halfAperture,
                  float        flashRadius,
                  const Map&   map,
                  float        worldTileSize);
	void revealSignal(sf::Vector2f signalWorldPos, float worldTileSize);

    void draw(sf::Vector2f playerWorldPos, float worldTileSize);
    void drawEnemyDots(const std::vector<Enemy>& enemies,
        sf::Vector2f playerWorldPos,
        float worldTileSize,
        bool radarActive);
    void setAlpha(uint8_t alpha) { m_alpha = alpha; }

private:
    sf::Clock m_clock;
    sf::RenderWindow& m_window;
    float             m_tileSize;
    float             m_margin;
    uint8_t m_alpha = 255;

    sf::RenderTexture m_texture;
    sf::RectangleShape m_tileShape;

    std::vector<std::vector<bool>> m_revealed;

    int m_mapW = 0;
    int m_mapH = 0;

    bool isInCone(int dx, int dy,
                  float flashAngle,
                  float halfAperture) const;

    sf::Color tileColor(TileType tile, RoomType rtype) const;
    sf::Vector2f toMiniPos(int tx, int ty) const;
    bool hasLineOfSight(sf::Vector2i from,
                        sf::Vector2i to,
                        const Map&   map) const;

};