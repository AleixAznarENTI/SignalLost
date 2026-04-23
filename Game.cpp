#include "Game.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <stdexcept>
#include <iostream>

// ----------------------------------------------------------------
// Constructor: initialize all systems, load resources, generate the map and position the player
// ----------------------------------------------------------------
Game::Game()
    : m_window(sf::VideoMode({ 1920u, 1080u }),
        "Signal Lost",
        sf::State::Fullscreen) // if I dont want fullscreen, just delete this
    , m_map(MAP_W, MAP_H)
    , m_player({ 0.f, 0.f }, TILE_SIZE)   // posición provisional, reset() la fija
    , m_camera(m_window, TILE_SIZE)
    , m_renderer(m_window, TILE_SIZE, m_font)
    , m_flashlight(m_window.getSize(), 180.f)
    , m_energy(100.f, 5.f)
    , m_hud(m_window, m_font, m_audio)
    , m_particles(200)
    , m_state(GameState::Intro)
    , m_prevState(GameState::Intro)
    , m_minimap(m_window, 3.f, 16.f)
    , m_postProcess(m_window)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    m_window.setFramerateLimit(60);

    if (!m_font.openFromFile("assets/font.ttf"))
        throw std::runtime_error("Unable to load assets/font.ttf");

    if (!m_audio.loadAll("assets/"))
        throw std::runtime_error("Unable to load audio files");

    m_hud.onStateChanged(GameState::Intro);
    initMap();
}

// ----------------------------------------------------------------
// run: the main game loop — input → update → render
// ----------------------------------------------------------------
void Game::run() {
    while (m_window.isOpen()) {
        float dt = m_clock.restart().asSeconds();

		// Big dt protects against physics glitches and particles flying away, but also against audio desync and other time-based issues.
        if (dt > 0.1f) dt = 0.1f;

        processInput();
        update(dt);
        render();
    }
}

// ----------------------------------------------------------------
// processInput: eventos y teclado
// ----------------------------------------------------------------
void Game::processInput() {
    while (const auto event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            m_window.close();

         if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Space &&
                m_state == GameState::Intro) {
                m_state = GameState::Waking;
                m_wakingTimer = 0.f;
                m_audio.playAmbient();
            }
            if (key->code == sf::Keyboard::Key::R &&
                m_state != GameState::Playing &&
                m_state != GameState::Intro) {
                reset();
            }
            if (key->code == sf::Keyboard::Key::Escape)
                m_window.close();
        }
    }
}

// ----------------------------------------------------------------
// update: game logic
// ----------------------------------------------------------------
// Game.cpp

void Game::update(float dt) {
    m_hud.update(dt, m_state);

    // --- Waking ---
    if (m_state == GameState::Waking) {
        m_wakingTimer += dt;
        m_hud.setWakingTimer(m_wakingTimer);

        // Fase 2: linterna arranca progresivamente
        float lightProgress = 0.f;
        if (m_wakingTimer > 0.8f) {
            lightProgress = (m_wakingTimer - 0.8f) / 0.7f; // 0→1 en 0.7s
            lightProgress = std::min(lightProgress, 1.f);

            // Parpadeo al arrancar
            float flicker = std::abs(std::sin(m_wakingTimer * 20.f));
            lightProgress *= (0.6f + 0.4f * flicker);
        }

        m_flashlight.setRadius(160.f * lightProgress);
        m_camera.update(m_player.getPosition(),
            sf::Vector2f(0.f, 0.f), dt);
        m_flashlight.update(m_player.getPosition(),
            m_camera.getView(),
            m_window, m_map, TILE_SIZE);

        if (m_wakingTimer >= WAKING_DURATION) {
            m_state = GameState::Playing;
            m_flashlight.setRadius(160.f); // radio normal
            m_hud.triggerUIFadeIn();
        }
        return;
    }


    // --- Dying ---
    if (m_state == GameState::Dying) {
        m_camera.update(
            m_player.getPosition(),
            sf::Vector2f(0.f,0.f),
            dt
        );

        if (!m_camera.isShaking()) {
            m_state = GameState::GameOver;
            m_prevState = GameState::GameOver;
            m_hud.onStateChanged(GameState::GameOver);
        }
        return;
    }

    if (m_state != GameState::Playing) return;

    updateMovement(dt);
    updateRoomEffects(dt);
    updateHazardEffects(dt);
    updateEnemies(dt);
    updateBatteries(dt);
    updatePowerUps(dt);
    updateEnemyProximity();
    checkEndConditions();

    m_stats.update(dt, m_player.getPosition(), TILE_SIZE);

    sf::Vector2i playerTile(
        static_cast<int>(m_player.getPosition().x / TILE_SIZE),
        static_cast<int>(m_player.getPosition().y / TILE_SIZE)
    );
    const auto& rooms = m_map.getRooms();
    for (size_t i = 0; i < rooms.size(); ++i) {
        if (rooms[i].contains(playerTile.x, playerTile.y))
            m_stats.registerRoomVisit(static_cast<int>(i));
    }

    m_hud.setStats(
        m_stats.getTime(),
        m_stats.getBatteriesCollected(),
        m_stats.getDistanceTraveled(),
        m_stats.getRoomsVisited()
    );

    // Sistemas que dependen de todo lo anterior
    m_particles.emit(m_player.getPosition(), dt);
    m_particles.update(dt);
    m_camera.update(m_player.getPosition(), m_player.getVelocity(), dt);
    m_flashlight.update(m_player.getPosition(),
        m_camera.getView(),
        m_window, m_map, TILE_SIZE);
}

void Game::updateMovement(float dt) {
    sf::Vector2f velocity(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) velocity.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) velocity.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) velocity.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) velocity.x += 1.f;

    float speedMult = m_powerUps.isActive(PowerUpType::Speed) ? 1.8f : 1.f;
    m_player.setSpeedMultiplier(speedMult);

    m_player.move(velocity, dt, m_map);
    m_energy.update(dt);
    m_audio.update(m_energy.getPercentage());
    for (const auto& enemy : m_enemies) {
        if (!enemy.catchesPlayer(m_player.getPosition(), TILE_SIZE)) continue;

        if (m_powerUps.hasShield()) {
            m_powerUps.consumeShield();
            m_camera.triggerShake(0.3f, 6.f);
            m_hud.triggerZoneNotification("SHIELD BROKEN",
                sf::Color(100, 180, 255));
        }
        else {
            m_state = GameState::Dying;
            m_camera.triggerShake(0.6f, 12.f);
            m_audio.playGameOver();
            m_hud.triggerGameOver();
        }
        return;
    }
}

void Game::updateEnemies(float dt) {
    for (auto& enemy : m_enemies) {
        enemy.update(
            dt,
            m_player.getPosition(),
            m_map,
            m_currentHazard == HazardType::Radiation
        );

        m_particles.emitEnemy(
            enemy.getPosition(),
            enemy.getState() == EnemyState::Chase,
            dt
        );
    }
}

void Game::updateEnemyProximity() {
    if (m_enemies.empty()) {
        m_hud.setEnemyProximity(0.f);
        return;
    }

    float minDist = std::numeric_limits<float>::max();
    for (const auto& enemy : m_enemies) {
        sf::Vector2f diff = m_player.getPosition() - enemy.getPosition();
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        minDist = std::min(minDist, dist);
    }

    const float startRange = 250.f;
    const float fullRange = 80.f;

    float alpha = 0.f;
    if (minDist < startRange) {
        alpha = 1.f - (minDist - fullRange) / (startRange - fullRange);
        alpha = std::max(0.f, std::min(1.f, alpha));
    }

    m_hud.setEnemyProximity(alpha);
    m_audio.updateStalkerMusic(alpha);

    bool anyChasing = false;
    for (const auto& enemy : m_enemies)
        if (enemy.getState() == EnemyState::Chase) { anyChasing = true; break; }

    if (anyChasing && !m_enemyAlertPlayed) {
        m_audio.playEnemyAlert();
        m_enemyAlertPlayed = true;
    }
    else if (!anyChasing) {
        m_enemyAlertPlayed = false;
    }

}

void Game::updateRoomEffects(float dt) {
    sf::Vector2i playerTile(
        static_cast<int>(m_player.getPosition().x / TILE_SIZE),
        static_cast<int>(m_player.getPosition().y / TILE_SIZE)
    );

    RoomType currentRoom = m_map.getRoomTypeAt(playerTile.x, playerTile.y);

    if (currentRoom == RoomType::Danger) {
        m_energy.applyPenalty(15.f * dt);
    }

    float baseRadius = m_powerUps.isActive(PowerUpType::Flashlight)
        ? 280.f
        : 160.f;

    float flickerRadius = baseRadius * (0.5f + 0.5f * m_energy.getPercentage());    if (currentRoom == RoomType::Danger) {
        flickerRadius *= 1.f + 0.15f * std::sin(
            m_clock.getElapsedTime().asSeconds() * 15.f);
    }
    m_flashlight.setRadius(flickerRadius);

    m_hud.setSignalInfo(
        m_player.getPosition(),
        m_signalPos,
        currentRoom == RoomType::Control
    );

    m_minimap.revealAt(
        playerTile,
        m_flashlight.getAngle(),
        m_flashlight.getHalfAperture(),
        m_flashlight.getRadius(),
        m_map, TILE_SIZE
    );

    m_hud.setCurrentRoom(currentRoom);
}

void Game::updateBatteries(float dt) {
    for (auto& battery : m_batteries) {
        if (battery.isCollected()) continue;
        if (!battery.isCollected())
            m_particles.emitBattery(battery.getPosition(), dt);

        sf::Vector2f diff = m_player.getPosition() - battery.getPosition();
        float        distSq = diff.x * diff.x + diff.y * diff.y;
        float        threshold = TILE_SIZE * 0.8f * TILE_SIZE * 0.8f;

        if (distSq < threshold) {
            battery.collect();
            m_energy.restore(battery.getRestoreAmount());
            m_audio.playBatteryPickup();
            m_hud.triggerBatteryPickup();
            m_stats.registerBatteryPickup();
        }
    }
}

void Game::updatePowerUps(float dt) {
    m_powerUps.update(dt);

    auto collected = m_powerUps.tryCollect(
        m_player.getPosition(), TILE_SIZE);

    if (!collected) return;

    PowerUpType type = *collected;
    m_audio.playBatteryPickup(); // reutilizamos sonido por ahora

    // Efecto inmediato para instantáneos
    if (type == PowerUpType::Battery) {
        m_energy.restore(50.f);
        m_hud.triggerBatteryPickup();
        return; // no se activa como durable
    }

    m_powerUps.activate(type);

    // Notificación de zona reutilizada para el power-up
    PowerUpDef def = getPowerUpDef(type);
    m_hud.triggerZoneNotification(def.name, def.color);
}

void Game::updateHazardEffects(float dt) {
    sf::Vector2i playerTile(
        static_cast<int>(m_player.getPosition().x / TILE_SIZE),
        static_cast<int>(m_player.getPosition().y / TILE_SIZE)
    );

    m_currentHazard = m_hazards.getHazardAt(playerTile.x, playerTile.y);

    // Detectar cambio de zona → notificación
    if (m_currentHazard != m_prevHazard) {
        if (m_currentHazard != HazardType::None) {
            // Buscamos la zona para obtener nombre y color
            for (const auto& zone : m_hazards.getZones()) {
                if (zone.contains(playerTile.x, playerTile.y)) {
                    m_hud.triggerZoneNotification(zone.getName(), zone.getColor());
                    break;
                }
            }
        }
        m_prevHazard = m_currentHazard;
    }

    // Aplicar efectos según la zona
    switch (m_currentHazard) {
    case HazardType::Radiation:
        m_energy.applyPenalty(10.f * dt);   // drain extra
        m_player.setSpeedMultiplier(1.f);
        break;

    case HazardType::Cold:
        m_player.setSpeedMultiplier(0.45f); // ralentiza mucho
        break;

    case HazardType::Electric: {
        // Parpadeo agresivo de linterna
        float buzz = 1.f + 0.25f * std::sin(
            m_clock.getElapsedTime().asSeconds() * 25.f);
        m_flashlight.setRadius(
            160.f * m_energy.getPercentage() * buzz);
        m_player.setSpeedMultiplier(1.f);
        break;
    }

    default:
        m_player.setSpeedMultiplier(1.f);   // velocidad normal
        break;
    }

    m_particles.emitHazard(m_player.getPosition(), m_currentHazard, dt);

    sf::Color targetColor;
    switch (m_currentHazard) {
    case HazardType::Radiation:
        targetColor = sf::Color(120, 255, 100); // verde radiactivo
        break;
    case HazardType::Cold:
        targetColor = sf::Color(100, 180, 255); // azul frío
        break;
    case HazardType::Electric:
        targetColor = sf::Color(255, 255, 100); // amarillo eléctrico
        break;
    default:
        // Color según sala
        switch (m_map.getRoomTypeAt(
            static_cast<int>(m_player.getPosition().x / TILE_SIZE),
            static_cast<int>(m_player.getPosition().y / TILE_SIZE)))
        {
        case RoomType::Danger:
            targetColor = sf::Color(255, 160, 100); break; // naranja
        case RoomType::Control:
            targetColor = sf::Color(140, 180, 255); break; // azul suave
        case RoomType::Storage:
            targetColor = sf::Color(180, 255, 180); break; // verde suave
        default:
            targetColor = sf::Color(255, 240, 200); break; // cálido normal
        }
    }

    // Lerp suave entre color actual y objetivo (t=0.05 por frame a 60fps)
    sf::Color current = m_currentLightColor;
    m_currentLightColor = sf::Color(
        static_cast<uint8_t>(current.r + (targetColor.r - current.r) * 0.05f),
        static_cast<uint8_t>(current.g + (targetColor.g - current.g) * 0.05f),
        static_cast<uint8_t>(current.b + (targetColor.b - current.b) * 0.05f)
    );

    m_flashlight.setLightColor(m_currentLightColor);
    m_audio.updateZoneAudio(m_currentHazard);
}

void Game::checkEndConditions() {
    sf::Vector2f diff = m_player.getPosition() - m_signalPos;
    float        distSq = diff.x * diff.x + diff.y * diff.y;
    float        threshold = TILE_SIZE * 1.2f * TILE_SIZE * 1.2f;

    if (distSq < threshold)        m_state = GameState::Victory;
    if (m_energy.isDepleted())     m_state = GameState::GameOver;

    if (m_state != m_prevState) {
        if (m_state == GameState::Victory) { 
            m_audio.playSignalFound(); 
            m_hud.triggerSignalFound();
        }
        if (m_state == GameState::GameOver) { 
            m_audio.playGameOver(); 
            m_hud.triggerGameOver();
        }
        m_hud.onStateChanged(m_state);
        m_prevState = m_state;
    }
}

// ----------------------------------------------------------------
// render: draw everything
// ----------------------------------------------------------------
void Game::render() {
    m_window.clear(sf::Color(10, 10, 20));

    // --- World rendering (everything except Intro) ---
    if (m_state != GameState::Intro) {
        m_window.setView(m_camera.getView());

        m_renderer.drawMap();
        m_renderer.drawBatteries(m_batteries);
        m_renderer.drawPowerUps(m_powerUps.getPickups());
        m_renderer.drawSignal(m_signalPos);
        m_renderer.drawEnemies(m_enemies);
        m_renderer.drawPlayer(m_player.getPosition());
        m_flashlight.draw(m_window);
        m_particles.draw(m_window);
    }
    else {
        m_window.setView(m_window.getDefaultView());
    }

    // --- Minimap (only while playing) ---
    if (m_state == GameState::Playing) {
        m_minimap.setAlpha(m_hud.getUIAlpha());
        m_minimap.draw(m_player.getPosition(), TILE_SIZE);
        m_minimap.drawEnemyDots(
            m_enemies,
            m_player.getPosition(),
            TILE_SIZE,
            m_powerUps.isActive(PowerUpType::Radar)
        );
    }

    // --- HUD ---
    m_hud.draw(m_energy.getPercentage(), m_state);

    // --- Waking overlay — fades from black as the player "wakes up" ---
    if (m_state == GameState::Waking) {
        float alpha = 0.f;

        if (m_wakingTimer < 0.8f) {
            alpha = 1.f; // fully black during boot phase
        }
        else {
            float fadeProgress = (m_wakingTimer - 0.8f) / (WAKING_DURATION - 0.8f);
            alpha = 1.f - std::min(fadeProgress, 1.f);
            alpha = alpha * alpha; // quadratic — more dramatic
        }

        sf::RectangleShape overlay(sf::Vector2f(m_window.getSize()));
        overlay.setFillColor(sf::Color(0, 0, 0,
            static_cast<uint8_t>(alpha * 255.f)));

        m_window.setView(m_window.getDefaultView());
        m_window.draw(overlay);
    }

    // --- Post process always last ---
    m_postProcess.draw(0.4f);
    m_window.display();
}
// ----------------------------------------------------------------
// initMap: Generates map without interfering with main start
// ----------------------------------------------------------------
void Game::initMap() {
    m_map.generate(3000);
    m_hazards.generate(m_map, 5); // 5 hazard zones
    m_renderer.bakeMap(m_map);
    m_minimap.bake(m_map);
    m_player = Player(sf::Vector2f(m_map.getStartPosition()) * TILE_SIZE, TILE_SIZE);
    m_energy = EnergySystem(100.f, 5.f);
    m_batteries = m_map.getBatteries();
    m_signalPos = sf::Vector2f(m_map.getSignalPosition()) * TILE_SIZE;
    spawnEnemies();
    m_stats.reset();
    m_powerUps.generate(m_map, TILE_SIZE);
    m_hud.setPowerUpSystem(&m_powerUps);
}

void Game::spawnEnemies() {
    m_enemies.clear();

    const auto& rooms = m_map.getRooms();
    sf::Vector2i startRoom = m_map.getStartPosition();
    sf::Vector2i signalRoom = m_map.getSignalPosition();

    for (const auto& room : rooms) {
        sf::Vector2i center = room.center();

        if (center == startRoom || center == signalRoom) continue;

        if (rand() % 10 < 3) {
            sf::Vector2f worldPos(
                (center.x + 0.5f) * TILE_SIZE,
                (center.y + 0.5f) * TILE_SIZE
            );
            m_enemies.emplace_back(worldPos, TILE_SIZE);
        }
    }
}

// ----------------------------------------------------------------
// reset: restarts the game by regenerating the map, repositioning the player, resetting energy and batteries, and playing ambient music
// ----------------------------------------------------------------
void Game::reset() {
    m_audio.stopAll();
    initMap();
    m_state = GameState::Playing;
    m_prevState = GameState::Playing;
    m_hud.onStateChanged(GameState::Playing);
    m_audio.playAmbient();
}

// ----------------------------------------------------------------
// drawEndScreen: restart text when the game is over or won, drawn on top of the default view so it doesn't move with the camera
// ----------------------------------------------------------------
void Game::drawEndScreen() {
    sf::View prev = m_window.getView();
    m_window.setView(m_window.getDefaultView());

    sf::Text hint(m_font, "Press R to restart  |  Escape to quit", 16);
    hint.setFillColor(sf::Color(180, 180, 180));

    sf::FloatRect b = hint.getLocalBounds();
    hint.setOrigin({ b.size.x / 2.f, 0.f });
    hint.setPosition({
        m_window.getSize().x / 2.f,
        m_window.getSize().y / 2.f + 40.f
        });

    m_window.draw(hint);
    m_window.setView(prev);
}