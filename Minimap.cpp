#include "Minimap.h"
#include <cmath>
#define M_PI  3.14159265358979323846

Minimap::Minimap(sf::RenderWindow& window, float tileSize, float margin)
    : m_window(window)
    , m_tileSize(tileSize)
    , m_margin(margin)
{
    m_tileShape.setSize(sf::Vector2f(tileSize, tileSize));
}

sf::Vector2f Minimap::toMiniPos(int tx, int ty) const {
    return sf::Vector2f(tx * m_tileSize, ty * m_tileSize);
}

void Minimap::bake(const Map& map) {
    m_mapW = map.getWidth();
    m_mapH = map.getHeight();

    m_revealed.assign(m_mapH, std::vector<bool>(m_mapW, false));

    sf::Vector2u texSize(
        static_cast<unsigned>(m_mapW * m_tileSize),
        static_cast<unsigned>(m_mapH * m_tileSize)
    );

    if (!m_texture.resize(texSize))
        throw std::runtime_error("No se pudo crear la RenderTexture del minimapa");

    m_texture.clear(sf::Color::Transparent);

    for (int y = 0; y < m_mapH; ++y) {
        for (int x = 0; x < m_mapW; ++x) {
            TileType tile = map.getTile(x, y);

            if (tile == TileType::Wall)
                m_tileShape.setFillColor(sf::Color(20, 20, 35));
            else
                m_tileShape.setFillColor(sf::Color(15, 15, 25)); // suelo no revelado

            m_tileShape.setPosition(toMiniPos(x, y));
            m_texture.draw(m_tileShape);
        }
    }

    m_texture.display();
}
bool Minimap::hasLineOfSight(sf::Vector2i from,
                             sf::Vector2i to,
                             const Map& map) const
{
    float dx = static_cast<float>(to.x - from.x);
    float dy = static_cast<float>(to.y - from.y);
    float steps = std::max(std::abs(dx), std::abs(dy)) * 2.f;

    if (steps == 0.f) return true;

    for (float i = 1.f; i < steps; ++i) {
        float t = i / steps;
        int tx = static_cast<int>(from.x + dx * t + 0.5f);
        int ty = static_cast<int>(from.y + dy * t + 0.5f);

        if (map.getTile(tx, ty) == TileType::Wall) {
            return false;
        }
    }

    return true;
}
sf::Color Minimap::tileColor(TileType tile, RoomType rtype) const {
    if (tile == TileType::Wall)
        return sf::Color(40, 40, 60);

    switch (rtype) {
    case RoomType::Storage: return sf::Color(50, 100, 60);
    case RoomType::Danger:  return sf::Color(100, 40, 40);
    case RoomType::Control: return sf::Color(40, 60, 110);
    default:                return sf::Color(90, 90, 120);
    }
}

void Minimap::revealAt(sf::Vector2i playerTile,
    float        flashAngle,
    float        halfAperture,
    float        flashRadius,
    const Map& map,
    float        worldTileSize)
{
    int  radiusTiles = static_cast<int>(flashRadius / worldTileSize) + 1;
    bool anyNew = false;

    for (int dy = -radiusTiles; dy <= radiusTiles; ++dy) {
        for (int dx = -radiusTiles; dx <= radiusTiles; ++dx) {
            int tx = playerTile.x + dx;
            int ty = playerTile.y + dy;

            if (tx < 0 || tx >= m_mapW || ty < 0 || ty >= m_mapH) continue;
            if (m_revealed[ty][tx]) continue;

            float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
            if (dist > radiusTiles) continue;

            // Halo ambiental: siempre revelamos tiles muy cercanos
            if (dist > 2.f && !isInCone(dx, dy, flashAngle, halfAperture))
                continue;

            if (!hasLineOfSight(playerTile, { tx, ty }, map)) continue;

            m_revealed[ty][tx] = true;
            anyNew = true;

            m_tileShape.setFillColor(
                tileColor(map.getTile(tx, ty),
                    map.getRoomTypeAt(tx, ty))
            );
            m_tileShape.setPosition(toMiniPos(tx, ty));
            m_texture.draw(m_tileShape);
        }
    }

    if (anyNew) m_texture.display();
}

void Minimap::drawEnemyDots(const std::vector<Enemy>& enemies,
    sf::Vector2f              playerPos,
    float                     worldTileSize,
    bool                      radarActive)
{
    if (!radarActive) return;

    sf::View prev = m_window.getView();
    m_window.setView(m_window.getDefaultView());

    float screenW = static_cast<float>(m_window.getSize().x);
    float screenH = static_cast<float>(m_window.getSize().y);
    float miniW = m_mapW * m_tileSize;
    float miniH = m_mapH * m_tileSize;
    sf::Vector2f origin(m_margin, m_margin);

    for (const auto& enemy : enemies) {
        sf::Vector2i tile(
            static_cast<int>(enemy.getPosition().x / worldTileSize),
            static_cast<int>(enemy.getPosition().y / worldTileSize)
        );

        sf::CircleShape dot(3.f);
        dot.setFillColor(sf::Color(255, 60, 60, m_alpha));
        dot.setOrigin({ 3.f, 3.f });
        dot.setPosition({
            origin.x + tile.x * m_tileSize,
            origin.y + tile.y * m_tileSize
            });
        m_window.draw(dot);
    }

    m_window.setView(prev);
}

bool Minimap::isInCone(int   dx, int   dy,
    float flashAngle,
    float halfAperture) const
{
    float tileAngle = std::atan2(
        static_cast<float>(dy),
        static_cast<float>(dx)
    );
    
    float diff = tileAngle - flashAngle;
    while (diff > M_PI) diff -= 2.f * M_PI;
    while (diff < -M_PI) diff += 2.f * M_PI;

    return std::abs(diff) < halfAperture;
}

void Minimap::draw(sf::Vector2f playerWorldPos, float worldTileSize) {
    if (m_alpha == 0) return;

    sf::View prev = m_window.getView();
    m_window.setView(m_window.getDefaultView());

    float screenW = static_cast<float>(m_window.getSize().x);
    float screenH = static_cast<float>(m_window.getSize().y);
    float miniW = m_mapW * m_tileSize;
    float miniH = m_mapH * m_tileSize;

    sf::Vector2f miniOrigin(
        m_margin,
        m_margin
    );

    // Background
    sf::RectangleShape bg({ miniW, miniH });
    bg.setPosition(miniOrigin);
    bg.setFillColor(sf::Color(0, 0, 0, 160));
    m_window.draw(bg);

    // Map texture
    sf::Sprite sprite(m_texture.getTexture());
    sprite.setPosition(miniOrigin);
    sprite.setColor(sf::Color(255, 255, 255, m_alpha));
    m_window.draw(sprite);

    // Border
    sf::RectangleShape border({ miniW, miniH });
    border.setPosition(miniOrigin);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(80, 80, 120, 180));
    border.setOutlineThickness(1.f);
    m_window.draw(border);

    sf::Vector2i playerTile(
        static_cast<int>(playerWorldPos.x / worldTileSize),
        static_cast<int>(playerWorldPos.y / worldTileSize)
    );

    sf::CircleShape playerDot(2.5f);
    playerDot.setFillColor(sf::Color(200, 230, 255, m_alpha));
    playerDot.setOrigin({ 2.5f, 2.5f });
    playerDot.setPosition({
        miniOrigin.x + playerTile.x * m_tileSize,
        miniOrigin.y + playerTile.y * m_tileSize
        });
    m_window.draw(playerDot);

    m_window.setView(prev);
}