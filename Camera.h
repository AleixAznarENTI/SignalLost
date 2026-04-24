#pragma once

#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera(sf::RenderWindow& window, float tileSize);

    // Llama cada frame con la posición objetivo y velocidad actual
    void update(sf::Vector2f targetPos,
        sf::Vector2f playerVelocity,
        float        dt);

    void triggerShake(float duration = 0.4f, float intensity = 8.f);

    const sf::View& getView() const { return m_view; }
    bool            isShaking() const { return m_shakeTimer > 0.f; }

    // Compatibilidad con código existente
    void follow(sf::Vector2f target) { m_targetPos = target; }
    void updateShake(float dt);
    void setPauseZoom(bool paused);

private:
    sf::RenderWindow& m_window;
    sf::View          m_view;
    float             m_tileSize;

    sf::Vector2f m_currentPos;   // posición actual de la cámara (lerped)
    sf::Vector2f m_targetPos;    // posición objetivo (jugador)
    sf::Vector2f m_baseCenter;   // centro sin shake

    float        m_currentZoom = 1.f;  // zoom actual (lerped)
    float        m_targetZoom = 1.f;  // zoom objetivo

    // Shake
    float        m_shakeTimer = 0.f;
    float        m_shakeDuration = 0.f;
    float        m_shakeIntensity = 0.f;

    // Pause
    float m_pauseZoom = 1.f;
    float m_pauseTarget = 1.f;

    // Configuración
    static constexpr float LERP_SPEED = 6.f;   // suavidad del seguimiento
    static constexpr float ZOOM_LERP = 3.f;   // suavidad del zoom
    static constexpr float ZOOM_MIN = 0.85f; // alejado (corriendo)
    static constexpr float ZOOM_MAX = 1.10f; // cercano (quieto)
    static constexpr float SPEED_THRESHOLD = 20.f; // velocidad mínima para zoom out
    static constexpr float PAUSE_ZOOM_OUT = 1.4f;
};