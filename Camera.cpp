#include "Camera.h"
#include <cmath>
#include <cstdlib>

Camera::Camera(sf::RenderWindow& window, float tileSize)
    : m_window(window)
    , m_tileSize(tileSize)
{
    m_view.setSize(sf::Vector2f(window.getSize()));
    m_view.setCenter(sf::Vector2f(window.getSize()) / 2.f);
    m_currentPos = m_view.getCenter();
    m_targetPos = m_currentPos;
}

void Camera::update(sf::Vector2f targetPos,
    sf::Vector2f playerVelocity,
    float        dt)
{
    m_targetPos = targetPos;

    // --- Lerp de posición ---
    // Nos acercamos LERP_SPEED * dt % hacia el objetivo cada frame
    float t = 1.f - std::exp(-LERP_SPEED * dt); // framerate-independiente
    m_currentPos = m_currentPos + (m_targetPos - m_currentPos) * t;
    m_baseCenter = m_currentPos;
    m_view.setCenter(m_currentPos);

    // --- Zoom dinámico ---
    float speed = std::sqrt(
        playerVelocity.x * playerVelocity.x +
        playerVelocity.y * playerVelocity.y
    );

    // Mapeamos velocidad a zoom: quieto → ZOOM_MAX, corriendo → ZOOM_MIN
    float speedNorm = std::min(speed / 120.f, 1.f); // normalizado 0→1
    m_targetZoom = ZOOM_MAX - (ZOOM_MAX - ZOOM_MIN) * speedNorm;

    // Lerp del zoom — más lento que la posición para que no maree
    float zt = 1.f - std::exp(-ZOOM_LERP * dt);
    m_pauseZoom = m_pauseZoom + (m_pauseTarget - m_pauseZoom) * zt;
    m_currentZoom = m_currentZoom + (m_targetZoom - m_currentZoom) * zt;

    // Aplicamos el zoom como tamaño de vista
    sf::Vector2f baseSize = sf::Vector2f(m_window.getSize());
    m_view.setSize(baseSize * m_currentZoom * m_pauseZoom);

    // Aplicamos el shake encima
    updateShake(dt);
}

void Camera::triggerShake(float duration, float intensity) {
    m_shakeTimer = duration;
    m_shakeDuration = duration;
    m_shakeIntensity = intensity;
}

void Camera::updateShake(float dt) {
    if (m_shakeTimer <= 0.f) return;

    m_shakeTimer -= dt;

    float progress = m_shakeTimer / m_shakeDuration;
    float currentI = m_shakeIntensity * progress;

    float offsetX = ((rand() % 200) - 100) / 100.f * currentI;
    float offsetY = ((rand() % 200) - 100) / 100.f * currentI;

    m_view.setCenter({
        m_baseCenter.x + offsetX,
        m_baseCenter.y + offsetY
        });
}

void Camera::setPauseZoom(bool paused) {
    m_pauseTarget = paused ? PAUSE_ZOOM_OUT : 1.f;
}