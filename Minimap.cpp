#include "Minimap.h"
#include <cmath>

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

void Minimap::revealAt(sf::Vector2i playerTile, const Map& map) {
    const int REVEAL_RADIUS = 6;

    bool anyNew = false;

    for (int dy = -REVEAL_RADIUS; dy <= REVEAL_RADIUS; ++dy) {
        for (int dx = -REVEAL_RADIUS; dx <= REVEAL_RADIUS; ++dx) {
            int tx = playerTile.x + dx;
            int ty = playerTile.y + dy;

            if (tx < 0 || tx >= m_mapW || ty < 0 || ty >= m_mapH) continue;
            if (m_revealed[ty][tx]) continue;

            float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
            if (dist > REVEAL_RADIUS) continue;

            m_revealed[ty][tx] = true;
            anyNew = true;

            TileType     tile = map.getTile(tx, ty);
            RoomType     rtype = map.getRoomTypeAt(tx, ty);

            if (tile == TileType::Wall) {
                m_tileShape.setFillColor(sf::Color(40, 40, 60));
            }
            else {
                switch (rtype) {
                case RoomType::Storage:
                    m_tileShape.setFillColor(sf::Color(50, 100, 60));
                    break;
                case RoomType::Danger:
                    m_tileShape.setFillColor(sf::Color(100, 40, 40));
                    break;
                case RoomType::Control:
                    m_tileShape.setFillColor(sf::Color(40, 60, 110));
                    break;
                default:
                    m_tileShape.setFillColor(sf::Color(90, 90, 120));
                    break;
                }
            }

            m_tileShape.setPosition(toMiniPos(tx, ty));
            m_texture.draw(m_tileShape);
        }
    }

    if (anyNew)
        m_texture.display();
}

void Minimap::draw(sf::Vector2f playerWorldPos, float worldTileSize) {
    sf::View prev = m_window.getView();
    m_window.setView(m_window.getDefaultView());

    float screenW = static_cast<float>(m_window.getSize().x);
    float screenH = static_cast<float>(m_window.getSize().y);

    float miniW = m_mapW * m_tileSize;
    float miniH = m_mapH * m_tileSize;

    sf::Vector2f miniOrigin(
        screenW - miniW - m_margin,
        screenH - miniH - m_margin
    );

    sf::RectangleShape border({ miniW, miniH });
    border.setPosition(miniOrigin);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(80, 80, 120, 180));
    border.setOutlineThickness(1.f);

    sf::RectangleShape bg({ miniW, miniH });
    bg.setPosition(miniOrigin);
    bg.setFillColor(sf::Color(0, 0, 0, 160));
    m_window.draw(bg);

    sf::Sprite sprite(m_texture.getTexture());
    sprite.setPosition(miniOrigin);
    m_window.draw(sprite);

    m_window.draw(border);

    sf::Vector2i playerTile(
        static_cast<int>(playerWorldPos.x / worldTileSize),
        static_cast<int>(playerWorldPos.y / worldTileSize)
    );

    sf::CircleShape playerDot(2.5f);
    playerDot.setFillColor(sf::Color(200, 230, 255));
    playerDot.setOrigin({ 2.5f, 2.5f });
    playerDot.setPosition({
        miniOrigin.x + playerTile.x * m_tileSize,
        miniOrigin.y + playerTile.y * m_tileSize
        });
    m_window.draw(playerDot);

    m_window.setView(prev);
}